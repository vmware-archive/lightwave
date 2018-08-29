/*
 * Copyright ©2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */



/*
 * Module Name: clusterstate
 *
 * Filename: statecache.c
 *
 * Abstract:
 *
 * Load and maintain gpClusterState global cache
 *
 */

#include "includes.h"

static
DWORD
_VmDirClusterAddNode_inWlock(
    PVDIR_ENTRY pEntry
    );

static
DWORD
_VmDirClusteAddNodeSite_inWlock(
    PVMDIR_NODE_STATE   pNode
    );

static
VOID
_VmDirClusterFreeNodeMap(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

static
VOID
_VmDirClusterFreeRegionNodeList(
    PVMDIR_CLUSTER_SITE_LIST   pSiteNodeList
    );

static
VOID
_VmDirClusterFreeNode(
    PVMDIR_NODE_STATE   pNode
    );

static
DWORD
_VmDirClusterAllocSiteList(
    PCSTR   pszSite,
    PVMDIR_CLUSTER_SITE_LIST*   ppSiteList
    );

static
VOID
_VmDirClusterFreeSiteList(
    PVMDIR_CLUSTER_SITE_LIST    pSiteList
    );

static
VOID
_VmDirClusterRemoveNode_inWlock(
    PSTR                pszKey,
    PVMDIR_NODE_STATE   pNode
    );

/*  TODO
 * 1. promotion flow
 * 2. node join flow
 * 3. node leave flow
 */
VOID
VmDirClusterSetCacheReload(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);

    gpClusterState->bReload = TRUE;
    gRaftState.initialized = FALSE;

    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

    return;
}

VOID
VmDirClusterFreeCache(
    VOID
    )
{
    LW_HASHMAP_ITER     nodeIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair = {NULL, NULL};
    PVMDIR_NODE_STATE   pNode = NULL;

    // free gClusterState content
    if (gpClusterState->phmNodes)
    {
        while (LwRtlHashMapIterate(gpClusterState->phmNodes, &nodeIter, &pair))
        {
            pNode = (PVMDIR_NODE_STATE)pair.pValue;
            VDIR_SAFE_UNBIND_EXT_S(pNode->nodeLDP.pLd);
        }

        LwRtlHashMapClear(gpClusterState->phmNodes, _VmDirClusterFreeNodeMap, NULL);
        LwRtlFreeHashMap(&gpClusterState->phmNodes);
    }

    if (gpClusterState->pSiteList)
    {
        PVMDIR_CLUSTER_SITE_LIST pCurr = gpClusterState->pSiteList;
        PVMDIR_CLUSTER_SITE_LIST pNext = NULL;

        for (pNext = pCurr->pNextSiteList; pCurr; pCurr = pNext)
        {
            _VmDirClusterFreeRegionNodeList(pCurr);
        }
    }

    VMDIR_SAFE_FREE_RWLOCK(gpClusterState->pRWLock);

    memset(gpClusterState, 0, sizeof(*gpClusterState));

    return;
}

/*
 * if cache reload is needed, clusterthr call this.
 */
DWORD
VmDirClusterLoadCache(
    VOID
    )
{
    DWORD   dwError = 0;
    PSTR    pszCfgBaseDN = NULL;
    size_t  iCnt = 0;
    BOOLEAN bInLock = FALSE;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    LW_HASHMAP_ITER     nodeIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair = {NULL, NULL};

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN ||
        gVmdirServerGlobals.systemDomainDN.lberbv.bv_len == 0)
    {
        //Called during shutdown or not promoted yet.
        goto cleanup;
    }

    dwError = VmDirAllocateStringPrintf(
        &pszCfgBaseDN,
        "cn=%s,%s",
        VMDIR_CONFIGURATION_CONTAINER_NAME,
        gVmdirServerGlobals.systemDomainDN.lberbv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
        pszCfgBaseDN,
        LDAP_SCOPE_SUBTREE,
        ATTR_OBJECT_CLASS,
        OC_DIR_SERVER,
        &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);

    if (gpClusterState->phmNodes == NULL)
    {
        //Saw this when running valgrind
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (LwRtlHashMapIterate(gpClusterState->phmNodes, &nodeIter, &pair))
    {
        ((PVMDIR_NODE_STATE)pair.pValue)->bIsActive = FALSE;   // initialize all nodes to be inactive first
    }

    for (iCnt = 0; iCnt < entryArray.iSize; iCnt++)
    {
        // load nodes based on current data.  this set node.bisActive to TRUE.
        dwError = _VmDirClusterAddNode_inWlock(entryArray.pEntry+iCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    LwRtlHashMapResetIter(&nodeIter);
    while (LwRtlHashMapIterate(gpClusterState->phmNodes, &nodeIter, &pair))
    {
        PSTR                pszKey = (PSTR)pair.pKey;
        PVMDIR_NODE_STATE   pNode = (PVMDIR_NODE_STATE)pair.pValue;

        if (!pNode->bIsSelf)
        {
            VmDirClusterNodeLDPConn(pNode, FALSE); // ignore error
        }

        if (!pNode->bIsActive)
        {
            _VmDirClusterRemoveNode_inWlock(pszKey, pNode);
        }
    }

cleanup:
    gpClusterState->bReload = FALSE;
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

    VMDIR_SAFE_FREE_MEMORY(pszCfgBaseDN);
    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:
    goto cleanup;
}

/*
 * Only clusterthr accesses VMDIR_LDP_CONN
 */
DWORD
VmDirClusterNodeLDPConn(
    PVMDIR_NODE_STATE   pNode,
    BOOLEAN             bForce
    )
{
    DWORD dwError = 0;
    PVMDIR_LDP_CONN pLdpConn = &pNode->nodeLDP;
    PSTR pszDomainName = NULL;

    dwError = VmDirDomainDNToName(gVmdirServerGlobals.systemDomainDN.bvnorm_val, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VDIR_SAFE_UNBIND_EXT_S(pLdpConn->pLd);
    dwError = VmDirConnectLDAPServerWithMachineAccount(pNode->srvObj.pszFQDN, pszDomainName, &pLdpConn->pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLdpConn->dwLdapError = 0;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirClusterCacheCloneSrvObj(
    PVDIR_LINKED_LIST*  ppSrvObjList
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bInLock = FALSE;
    LW_HASHMAP_ITER         nodeIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR         pair = {NULL, NULL};
    PVMDIR_SERVER_OBJECT    pSrvObj = 0;
    PVMDIR_NODE_STATE       pNodeState = NULL;
    PVDIR_LINKED_LIST       pLocalSrvObjList = NULL;

    if (!ppSrvObjList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListCreate(&pLocalSrvObjList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_READLOCK(bInLock, gpClusterState->pRWLock, 0);

    while (LwRtlHashMapIterate(gpClusterState->phmNodes, &nodeIter, &pair))
    {
        pNodeState = (PVMDIR_NODE_STATE)pair.pValue;

        dwError = VmDirCloneServerObject(&pNodeState->srvObj, &pSrvObj);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(pLocalSrvObjList, pSrvObj, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pSrvObj = NULL;
    }

    *ppSrvObjList = pLocalSrvObjList;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
    VmDirFreeServerObject(pSrvObj);
    return dwError;

error:
    VmDirFreeSrvObjLinkedList(pLocalSrvObjList);
    goto cleanup;
}

VOID
VmDirFreeSrvObjLinkedList(
    PVDIR_LINKED_LIST   pSrvObjList
    )
{
    PVDIR_LINKED_LIST_NODE  pNode = NULL;

    if (pSrvObjList)
    {
        VmDirLinkedListGetHead(pSrvObjList, &pNode);
        while (pNode)
        {
            if (pNode->pElement)
            {
                VmDirFreeServerObject((PVMDIR_SERVER_OBJECT) pNode->pElement);
            }
            pNode = pNode->pNext;
        }
        VmDirFreeLinkedList(pSrvObjList);
    }
}

VOID
VmDirFreeServerObjectContent(
    PVMDIR_SERVER_OBJECT    pSrvObj
    )
{
    if (pSrvObj)
    {
        VMDIR_SAFE_FREE_MEMORY(pSrvObj->pszDN);
        VMDIR_SAFE_FREE_MEMORY(pSrvObj->pszFQDN);
        VMDIR_SAFE_FREE_MEMORY(pSrvObj->pszInvocationId);
        VMDIR_SAFE_FREE_MEMORY(pSrvObj->pszSite);
    }
}

VOID
VmDirFreeServerObject(
    PVMDIR_SERVER_OBJECT    pSrvObj
    )
{
    if (pSrvObj)
    {
        VmDirFreeServerObjectContent(pSrvObj);
        VMDIR_SAFE_FREE_MEMORY(pSrvObj);
    }
}

DWORD
VmDirCloneServerObject(
    PVMDIR_SERVER_OBJECT    pSrvObj,
    PVMDIR_SERVER_OBJECT*   ppOutSrvObj
    )
{
    DWORD   dwError = 0;
    PVMDIR_SERVER_OBJECT pLocalSrvObj = NULL;

    if (!pSrvObj || !ppOutSrvObj)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_SERVER_OBJECT), (PVOID)&pLocalSrvObj);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pSrvObj->pszDN, &pLocalSrvObj->pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pSrvObj->pszFQDN, &pLocalSrvObj->pszFQDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pSrvObj->pszInvocationId, &pLocalSrvObj->pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pSrvObj->pszSite, &pLocalSrvObj->pszSite);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLocalSrvObj->dwServerId = pSrvObj->dwServerId;

    *ppOutSrvObj = pLocalSrvObj;

cleanup:
    return dwError;

error:
    VmDirFreeServerObject(pLocalSrvObj);
    goto cleanup;
}

static
VOID
_VmDirClusterRemoveNode_inWlock(
    PSTR                pszKey,
    PVMDIR_NODE_STATE   pNode
    )
{
    DWORD   dwSize = 0;
    BOOLEAN bDone = FALSE;
    PVMDIR_CLUSTER_SITE_LIST pSiteList = gpClusterState->pSiteList;

    // safe to remove self in LwRtlHashMapIterate loop
    LwRtlHashMapRemove(gpClusterState->phmNodes, pszKey, NULL); // ignore error
    // free pszKey
    // free pNode?

    for (; pSiteList && !bDone; pSiteList = pSiteList->pNextSiteList)
    {
        for (dwSize = 0; dwSize < pSiteList->dwArySize && !bDone; dwSize++)
        {
            if (pSiteList->ppNodeStateAry[dwSize] == pNode)
            {
                pSiteList->ppNodeStateAry[dwSize] = NULL;
                bDone = TRUE;
            }
        }
    }

    return;
}

static
DWORD
_VmDirClusterAddNode_inWlock(
    PVDIR_ENTRY pEntry
    )
{
    DWORD   dwError = 0;
    PSTR    pszHMKey = NULL;
    PVMDIR_NODE_STATE pNode = NULL;
    PVDIR_ATTRIBUTE pAttCN = NULL;
    PVDIR_ATTRIBUTE pAttInvocationId = NULL;
    PVDIR_ATTRIBUTE pAttServerId = NULL;

    if ((pAttCN = VmDirFindAttrByName(pEntry, ATTR_CN)) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    if (LwRtlHashMapFindKey(gpClusterState->phmNodes, (PVOID*)&pNode, pAttCN->vals[0].lberbv_val) == 0)
    {
        pNode->bIsActive = TRUE;
        goto cleanup;
    }

    // construct node
    dwError = VmDirAllocateMemory(sizeof(*pNode), (PVOID)&pNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pEntry->dn.lberbv_val, &pNode->srvObj.pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pAttCN->vals[0].lberbv_val, &pNode->srvObj.pszFQDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((pAttInvocationId = VmDirFindAttrByName(pEntry, ATTR_INVOCATION_ID)) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }
    dwError = VmDirAllocateStringA(pAttInvocationId->vals[0].lberbv_val, &pNode->srvObj.pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((pAttServerId = VmDirFindAttrByName(pEntry, ATTR_SERVER_ID)) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = VmDirStringToUINT32(pAttServerId->vals[0].lberbv_val, NULL, &pNode->srvObj.dwServerId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirServerDNToSite(pNode->srvObj.pszDN, (PSTR*)&pNode->srvObj.pszSite);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode->bIsActive = TRUE;
    pNode->nodeLDP.connState = DC_CONNECTION_STATE_CONNECTING;

    // is this self node?
    if (!gpClusterState->pNodeSelf &&
        VmDirStringCompareA(pNode->srvObj.pszFQDN, gVmdirServerGlobals.bvServerObjName.lberbv_val, FALSE) == 0
       )
    {
        gpClusterState->pNodeSelf = pNode;
        pNode->bIsSelf = TRUE;
    }

    // init site structures
    dwError = _VmDirClusteAddNodeSite_inWlock(pNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    // add to gpClusterState->phmNodes
    dwError = VmDirAllocateStringA(pNode->srvObj.pszFQDN, &pszHMKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(gpClusterState->phmNodes, pszHMKey, pNode, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pszHMKey = NULL;    // map takes over key and content
    pNode = NULL;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszHMKey);
    _VmDirClusterFreeNode(pNode);
    goto cleanup;
}

static
DWORD
_VmDirClusterAllocSiteList(
    PCSTR   pszSite,
    PVMDIR_CLUSTER_SITE_LIST*   ppSiteList
    )
{
    DWORD   dwError = 0;
    PVMDIR_CLUSTER_SITE_LIST    pLocalList = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pLocalList), (PVOID)&pLocalList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszSite, &pLocalList->pszSite);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLocalList->dwArySize = 8;
    dwError = VmDirAllocateMemory(
        sizeof(pLocalList->ppNodeStateAry) * pLocalList->dwArySize,
        (PVOID)&pLocalList->ppNodeStateAry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSiteList = pLocalList;
    pLocalList = NULL;

cleanup:
    return dwError;

error:
    _VmDirClusterFreeSiteList(pLocalList);
    goto cleanup;
}

static
VOID
_VmDirClusterFreeSiteList(
    PVMDIR_CLUSTER_SITE_LIST    pSiteList
    )
{
    if (pSiteList)
    {
        VMDIR_SAFE_FREE_MEMORY(pSiteList->ppNodeStateAry);
        VMDIR_SAFE_FREE_MEMORY(pSiteList->pszSite);
        VMDIR_SAFE_FREE_MEMORY(pSiteList);
    }

    return;
}

/*
 * add a node into site list
 */
static
DWORD
_VmDirClusteAddNodeSite_inWlock(
    PVMDIR_NODE_STATE   pNode
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_CLUSTER_SITE_LIST    pLocalList = NULL;
    PVMDIR_CLUSTER_SITE_LIST    pTmpList = gpClusterState->pSiteList;

    for (; pTmpList && pTmpList->dwArySize; pTmpList = pTmpList->pNextSiteList)
    {
        if (VmDirStringCompareA(pNode->srvObj.pszSite, pTmpList->pszSite, FALSE) == 0)
        {
            break;
        }
    }

    if (!pTmpList || !pTmpList->dwArySize)
    {
        dwError = _VmDirClusterAllocSiteList(pNode->srvObj.pszSite, &pLocalList);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (gpClusterState->pSiteList)
        {
            pLocalList->pNextSiteList = gpClusterState->pSiteList;
        }
        gpClusterState->pSiteList = pLocalList;
        pTmpList = pLocalList;
        pLocalList = NULL;
    }

    for (dwCnt = 0; dwCnt < pTmpList->dwArySize; dwCnt++)
    {
        if (!pTmpList->ppNodeStateAry[dwCnt])
        {
            pTmpList->ppNodeStateAry[dwCnt] = pNode;
            break;
        }
    }

    if (dwCnt == pTmpList->dwArySize)

    {
        dwError = VmDirReallocateMemoryWithInit(
            pTmpList->ppNodeStateAry,
            (PVOID*)&pTmpList->ppNodeStateAry,
            sizeof(pTmpList->ppNodeStateAry) * pTmpList->dwArySize * 2,
            sizeof(pTmpList->ppNodeStateAry) * pTmpList->dwArySize);
        BAIL_ON_VMDIR_ERROR(dwError);

        pTmpList->ppNodeStateAry[pTmpList->dwArySize] = pNode;
        pTmpList->dwArySize = pTmpList->dwArySize * 2;
    }

    if (pNode->bIsSelf)
    {
        gpClusterState->pSiteListSelf = pTmpList;
    }

cleanup:
    return dwError;

error:
    _VmDirClusterFreeSiteList(pLocalList);
    goto cleanup;
}

static
VOID
_VmDirClusterFreeNode(
    PVMDIR_NODE_STATE   pNode
    )
{
    if (pNode)
    {
        VmDirFreeServerObjectContent(&pNode->srvObj);
        VDIR_SAFE_UNBIND_EXT_S(pNode->nodeLDP.pLd);

        VMDIR_SAFE_FREE_MEMORY(pNode);
    }

    return;
}

static
VOID
_VmDirClusterFreeRegionNodeList(
    PVMDIR_CLUSTER_SITE_LIST   pSiteNodeList
    )
{
    if (pSiteNodeList)
    {
        VMDIR_SAFE_FREE_MEMORY(pSiteNodeList->pszSite);
        VMDIR_SAFE_FREE_MEMORY(pSiteNodeList->ppNodeStateAry);
        VMDIR_SAFE_FREE_MEMORY(pSiteNodeList);
    }

    return;
}

static
VOID
_VmDirClusterFreeNodeMap(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    //VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    //VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
}
