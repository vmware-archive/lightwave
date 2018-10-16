/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

static
DWORD
_VmDirConnectToRaft(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    LDAP**  ppLd
    );

static
DWORD
_VmDirConnectToRaftLeader(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    LDAP**  ppLd
    );

static
DWORD
_VmDirRaftStateBerValueToCluster(
    BerValue**              ppBerValues,
    PVMDIR_RAFT_CLUSTER*    ppCluster
    );

static
DWORD
_VmDirRaftAllocAndAddNode(
    PVMDIR_RAFT_CLUSTER pCluster,
    PCSTR               pszName,
    VMDIR_RAFT_ROLE     role
    );

static
DWORD
_VmDirRaftClusterAddNode(
    PVMDIR_RAFT_CLUSTER pCluster,
    PVMDIR_RAFT_NODE    pNode
    );

BOOLEAN
VmDirRaftServerExists(
    PCSTR       pszHostName,
    PCSTR       pszDomainName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PCSTR       pszRaftHostName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainDN = NULL;
    PSTR    pszRaftDN = NULL;
    LDAP*   pLd = NULL;
    BOOLEAN bRaftServerExists = TRUE;

    dwError = _VmDirConnectToRaft(pszHostName, pszDomainName, pszUserName, pszPassword, &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDomainNameToDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszRaftDN,
            "%s=%s,%s=%s,%s",
            ATTR_CN,
            pszRaftHostName,
            ATTR_OU,
            VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    bRaftServerExists = VmDirIfDNExist(pLd, pszRaftDN);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszRaftDN);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return bRaftServerExists;

error:
    goto cleanup;
}

DWORD
VmDirRaftLeader(
    PCSTR   pszServerName,
    PSTR*   ppszLeader
    )
{
    DWORD   dwError = 0;

    dwError = VmDirGetDSERootAttribute(
                    pszServerName,
                    ATTR_RAFT_LEADER,
                    ppszLeader);

    if (dwError == LDAP_NO_SUCH_ATTRIBUTE)
    {
        dwError = VMDIR_LDAP_ERROR_NO_LEADER;
    }

    return dwError;
}

DWORD
VmDirRaftListCluster(
    PCSTR                   pszServerName,
    PVMDIR_RAFT_CLUSTER*    ppRaftCluster
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    int     iNum = 0;
    PSTR    pszLeader = NULL;
    LDAP*   pLd = NULL;
    PVMDIR_RAFT_CLUSTER pCluster = NULL;
    PCSTR   ppszAttrs[] = {ATTR_RAFT_LEADER, ATTR_RAFT_FOLLOWERS, ATTR_RAFT_MEMBERS, NULL};
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    BerValue**      ppBerValues = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        !ppRaftCluster)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if ((dwError = _VmDirConnectToRaftLeader(pszServerName, NULL, NULL, NULL, &pLd)) != 0)
    {
        dwError = _VmDirConnectToRaft(pszServerName, NULL, NULL, NULL, &pLd);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                    pLd,
                    "",
                    LDAP_SCOPE_BASE,
                    NULL,
                    (PSTR*)ppszAttrs,
                    FALSE, /* attr only       */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = VmDirAllocateMemory(sizeof(*pCluster), (PVOID*)&pCluster);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; ppszAttrs[iCnt]; iCnt++)
    {
        if(ppBerValues)
        {
            ldap_value_free_len(ppBerValues);
        }
        if ((ppBerValues = ldap_get_values_len(pLd, pEntry, ppszAttrs[iCnt])) == NULL)
        {
            continue;
        }

        if (VmDirStringCompareA(ppszAttrs[iCnt], ATTR_RAFT_LEADER, FALSE) == 0)
        {
            dwError = _VmDirRaftAllocAndAddNode(pCluster, ppBerValues[0]->bv_val, VMDIRD_RAFT_ROLE_LEADER);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ppszAttrs[iCnt], ATTR_RAFT_FOLLOWERS, FALSE) == 0)
        {
            for (iNum = 0; ppBerValues[iNum]; iNum++)
            {
                dwError = _VmDirRaftAllocAndAddNode(pCluster, ppBerValues[iNum]->bv_val, VMDIRD_RAFT_ROLE_FOLLOWER);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (VmDirStringCompareA(ppszAttrs[iCnt], ATTR_RAFT_MEMBERS, FALSE) == 0)
        {
            for (iNum = 0; ppBerValues[iNum]; iNum++)
            {
                dwError = _VmDirRaftAllocAndAddNode(pCluster, ppBerValues[iNum]->bv_val, VMDIRD_RAFT_ROLE_CANDIDATE);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    *ppRaftCluster = pCluster;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLeader);
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:
    VmDirFreeRaftCluster(pCluster);

    goto cleanup;
}

DWORD
VmDirRaftShowClusterState(
    PCSTR                   pszServerName,
    PCSTR                   pszDomainName,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PVMDIR_RAFT_CLUSTER*    ppRaftCluster
    )
{
    DWORD   dwError = 0;
    LDAP*   pLd = NULL;
    PVMDIR_RAFT_CLUSTER pCluster = NULL;
    PCSTR   ppszAttrs[] = {ATTR_RAFT_STATE, NULL};
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    BerValue**      ppBerValues = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        !ppRaftCluster)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirConnectToRaftLeader(
                pszServerName,
                pszDomainName,
                pszUserName,
                pszPassword,
                &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    RAFT_STATE_DN,
                    LDAP_SCOPE_BASE,
                    "objectclass=*",
                    (PSTR*)ppszAttrs,
                    FALSE, /* attr only       */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    ppBerValues = ldap_get_values_len(pLd, pEntry, ppszAttrs[0]);
    if (!ppBerValues || !ppBerValues[0]->bv_val)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = _VmDirRaftStateBerValueToCluster(ppBerValues, &pCluster);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppRaftCluster = pCluster;
    pCluster = NULL;

cleanup:
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    return dwError;

error:
	VmDirFreeRaftCluster(pCluster);

    goto cleanup;
}

DWORD
VmDirRaftLeaveCluster(
    PCSTR                   pszServerName,
    PCSTR                   pszDomainName,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PCSTR                   pszLeaveNode
    )
{
    DWORD   dwError = 0;
    LDAP*   pLd = NULL;
    PSTR    pszLeaveNodeUPN = NULL;
    PSTR    pszLeaveNodeDN = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszLeaveNode))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirConnectToRaftLeader(
                pszServerName,
                pszDomainName,
                pszUserName,
                pszPassword,
                &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszLeaveNodeUPN,
            "%s@%s",
            pszLeaveNode,
            pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConvertUPNToDN(pLd, pszLeaveNodeUPN, &pszLeaveNodeDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_delete_ext_s(pLd, pszLeaveNodeDN, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLeaveNodeUPN);
    VMDIR_SAFE_FREE_MEMORY(pszLeaveNodeDN);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:
    goto cleanup;
}

VOID
VmDirFreeRaftNode(
    PVMDIR_RAFT_NODE    pNode
    )
{
    if (pNode)
    {
        VMDIR_SAFE_FREE_MEMORY(pNode->pszName);
        VMDIR_SAFE_FREE_MEMORY(pNode);
    }
}

VOID
VmDirFreeRaftCluster(
    PVMDIR_RAFT_CLUSTER     pRaftCluster
    )
{
    if (pRaftCluster)
    {
        PVMDIR_RAFT_NODE    pCurrent = NULL;
        PVMDIR_RAFT_NODE    pNextNode = NULL;

        VMDIR_SAFE_FREE_MEMORY(pRaftCluster->pszLeader);

        for (pCurrent = pRaftCluster->pNode; pCurrent; pCurrent = pNextNode)
        {
            pNextNode = pCurrent->pNext;
            VmDirFreeRaftNode(pCurrent);
        }
    }
}

DWORD
VmDirRaftStartVoteClient(
    PCSTR   pszLogin,
    PCSTR   pszPassword,
    PCSTR   pszServerName
    )
{
    DWORD       dwError = 0;
    DWORD       dwBindError = 0;
    handle_t    hBinding = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszLogin) ||
        IsNullOrEmptyString(pszPassword))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // Create binding for this server name
    dwError = VmDirCreateBindingHandleAuthA(
            pszServerName,
            NULL,
            pszLogin,
            NULL,
            pszPassword,
            &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Call RPC for starting vote
    dwError = VmDirRaftStartVote(hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    rpc_binding_free(&hBinding, &dwBindError);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirConnectToRaft(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    LDAP**  ppLd
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalHostURI = NULL;
    LDAP*   pLd = NULL;
    DWORD   dwLdapPort = DEFAULT_LDAP_PORT_NUM;

    if (pszUserName && pszPassword)
    {
        dwError = VmDirConnectLDAPServer(
                    &pLd,
                    pszServerName,
                    pszDomainName,
                    pszUserName,
                    pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        // ignore error
        VmDirGetRegKeyValueDword(
                    VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
                    VMDIR_REG_KEY_LDAP_PORT,
                    &dwLdapPort,
                    DEFAULT_LDAP_PORT_NUM);

        if ( VmDirIsIPV6AddrFormat( pszServerName ) )
        {
            dwError = VmDirAllocateStringPrintf(
                    &pszLocalHostURI,
                    "%s://[%s]:%d",
                    VMDIR_LDAP_PROTOCOL,
                    pszServerName,
                    dwLdapPort);
        }
        else
        {
            dwError = VmDirAllocateStringPrintf(
                    &pszLocalHostURI,
                    "%s://%s:%d",
                    VMDIR_LDAP_PROTOCOL,
                    pszServerName,
                    dwLdapPort);
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAnonymousLDAPBind( &pLd, pszLocalHostURI );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppLd = pLd;
    pLd = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalHostURI);

    return dwError;

error:
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    goto cleanup;
}

static
DWORD
_VmDirConnectToRaftLeader(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    LDAP**  ppLd
    )
{
    DWORD   dwError = 0;
    PCSTR   ppszAttrs[] = {ATTR_RAFT_LEADER, NULL};
    LDAP*   pLd = NULL;
    LDAP*   pLeaderLd = NULL;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    BerValue**      ppBerValues = NULL;

    dwError = _VmDirConnectToRaft(
                pszServerName,
                pszDomainName,
                pszUserName,
                pszPassword,
                &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    "",
                    LDAP_SCOPE_BASE,
                    NULL,
                    (PSTR*)ppszAttrs,
                    FALSE, /* attr only       */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    ppBerValues = ldap_get_values_len(pLd, pEntry, ppszAttrs[0]);
    if (!ppBerValues || !ppBerValues[0]->bv_val)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_LDAP_ERROR_NO_LEADER);
    }

    if (VmDirStringCompareA(pszServerName, ppBerValues[0]->bv_val, FALSE) != 0)
    {
        dwError = _VmDirConnectToRaft(
                    ppBerValues[0]->bv_val,
                    pszDomainName,
                    pszUserName,
                    pszPassword,
                    &pLeaderLd);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pLeaderLd)
    {
        *ppLd = pLeaderLd;
        pLeaderLd = NULL;
    }
    else
    {
        *ppLd = pLd;
        pLd = NULL;
    }

cleanup:
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    if (pLeaderLd)
    {
        ldap_unbind_ext_s(pLeaderLd, NULL, NULL);
    }
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirRaftAllocAndAddNode(
    PVMDIR_RAFT_CLUSTER pCluster,
    PCSTR               pszName,
    VMDIR_RAFT_ROLE     role
    )
{
    DWORD   dwError = 0;
    PVMDIR_RAFT_NODE    pNode = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pNode), (PVOID*)&pNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszName, &pNode->pszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode->bActive = TRUE;
    pNode->role = role;

    dwError = _VmDirRaftClusterAddNode(pCluster, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

cleanup:
    return dwError;

error:
    VmDirFreeRaftNode(pNode);

    goto cleanup;
}

static
DWORD
_VmDirRaftClusterAddNode(
    PVMDIR_RAFT_CLUSTER pCluster,
    PVMDIR_RAFT_NODE    pNode
    )
{
    DWORD   dwError = 0;
    PVMDIR_RAFT_NODE    pTmp = pCluster->pNode;

    if (pNode->role == VMDIRD_RAFT_ROLE_CANDIDATE)
    {
        pCluster->dwNumMmember++;
    }

    while (pTmp)
    {
        if (VmDirStringCompareA(pNode->pszName, pTmp->pszName, FALSE) == 0)
        {
            VmDirFreeRaftNode(pNode);
            goto cleanup;
        }
        pTmp = pTmp->pNext;
    }

    if (pNode->role == VMDIRD_RAFT_ROLE_LEADER)
    {
        dwError = VmDirAllocateStringA(pNode->pszName, &pCluster->pszLeader);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pNode->role == VMDIRD_RAFT_ROLE_FOLLOWER)
    {
        pCluster->dwNumActiveFollower++;
    }

    pNode->pNext = pCluster->pNode;
    pCluster->pNode = pNode;

cleanup:
    return dwError;

error:
    goto cleanup;
}

#define VMDIR_RAFT_NODE_PREFIX              "node: "
#define VMDIR_RAFT_ROLE_PREFIX              "role: "
#define VMDIR_RAFT_LASTOINDEX_PREFIX        "lastIndex: "
#define VMDIR_RAFT_LASTAPPLIESINDEX_PREFIX  "lastAppliedIndex: "
#define VMDIR_RAFT_TERM_PREFIX              "term: "
#define VMDIR_RAFT_LEADER_PREFIX            "leader: "
#define VMDIR_RAFT_NODE_SEPERATOR_PREFIX    "-"

/*
 * We expect following -
 * 1. node is separated by "-"
 * 2. each node start with "node: xxx"
 */
static
DWORD
_VmDirRaftStateBerValueToCluster(
    BerValue**              ppBerValues,
    PVMDIR_RAFT_CLUSTER*    ppCluster
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    PVMDIR_RAFT_CLUSTER pCluster = NULL;
    PVMDIR_RAFT_NODE    pLocalNode = NULL;
    PVMDIR_RAFT_NODE    pNode = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pCluster), (PVOID*)&pCluster);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt=0; ppBerValues[iCnt] && ppBerValues[iCnt]->bv_len > 0; iCnt++)
    {
        if (VmDirStringStartsWith(ppBerValues[iCnt]->bv_val, VMDIR_RAFT_NODE_PREFIX, FALSE))
        {
            if (pLocalNode)
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
            }

            dwError = VmDirAllocateMemory(sizeof(*pLocalNode), (PVOID*)&pLocalNode);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(ppBerValues[iCnt]->bv_val+strlen(VMDIR_RAFT_NODE_PREFIX), &pLocalNode->pszName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringStartsWith(ppBerValues[iCnt]->bv_val, VMDIR_RAFT_ROLE_PREFIX, FALSE))
        {
            if (!pLocalNode)
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
            }

            if (VmDirStringCompareA(ppBerValues[iCnt]->bv_val+strlen(VMDIR_RAFT_ROLE_PREFIX), "leader", FALSE) == 0)
            {
                pLocalNode->role = VMDIRD_RAFT_ROLE_LEADER;
            }
            else if (VmDirStringCompareA(ppBerValues[iCnt]->bv_val+strlen(VMDIR_RAFT_ROLE_PREFIX), "follower", FALSE) == 0)
            {
                pLocalNode->role = VMDIRD_RAFT_ROLE_FOLLOWER;
            }
            else
            {
                pLocalNode->role = VMDIRD_RAFT_ROLE_CANDIDATE;
            }
        }
        else if (VmDirStringStartsWith(ppBerValues[iCnt]->bv_val, VMDIR_RAFT_LASTOINDEX_PREFIX, FALSE))
        {
            if (!pLocalNode)
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
            }

            pLocalNode->iLastLogIndex = atoll(ppBerValues[iCnt]->bv_val+strlen(VMDIR_RAFT_LASTOINDEX_PREFIX));
        }
        else if (VmDirStringStartsWith(ppBerValues[iCnt]->bv_val, VMDIR_RAFT_LASTAPPLIESINDEX_PREFIX, FALSE))
        {
            if (!pLocalNode)
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
            }

            pLocalNode->iLastAppliedIndex = atoll(ppBerValues[iCnt]->bv_val+strlen(VMDIR_RAFT_LASTAPPLIESINDEX_PREFIX));
        }
        else if (VmDirStringStartsWith(ppBerValues[iCnt]->bv_val, VMDIR_RAFT_TERM_PREFIX, FALSE))
        {
            if (!pLocalNode)
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
            }

            pLocalNode->iRaftTerm = atoll(ppBerValues[iCnt]->bv_val+strlen(VMDIR_RAFT_TERM_PREFIX));
        }
        else if (VmDirStringStartsWith(ppBerValues[iCnt]->bv_val, VMDIR_RAFT_NODE_SEPERATOR_PREFIX, FALSE))
        {
            if (!pLocalNode)
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
            }

            pLocalNode->pNext = pCluster->pNode;
            pCluster->pNode = pLocalNode;
            pLocalNode = NULL;
        }
    }

    for (pNode = pCluster->pNode; pNode; pNode = pNode->pNext)
    {
        if (pNode->role == VMDIRD_RAFT_ROLE_LEADER)
        {
            pNode->bActive = TRUE;
            dwError = VmDirAllocateStringA(pNode->pszName, &pCluster->pszLeader);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (pNode->role == VMDIRD_RAFT_ROLE_FOLLOWER)
        {
            pNode->bActive = TRUE;
            pCluster->dwNumActiveFollower++;
        }
        else
        {
            pCluster->dwNumMmember++;
        }
    }

    *ppCluster = pCluster;

cleanup:
    return dwError;

error:
    VmDirFreeRaftNode(pLocalNode);
    VmDirFreeRaftCluster(pCluster);
    goto cleanup;
}
