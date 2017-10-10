/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
PVDIR_DOMAIN_SID_GEN_STATE
VmDirFindSidGenStateWithDN_inlock(
    PCSTR pszDomainDN
    );

static
DWORD
_VmDirGenerateDomainGuid(
    PCSTR   pszGuid, /* optional */
    uuid_t* pDomainGuid
    );

static
DWORD
VmDirGenerateDomainGuidSid_inlock(
    PCSTR   pszGuid, /* optional, if given used directly after check to make sure it is unique*/
    PSTR*   ppszDomainSid
    );

static
DWORD
VmDirGenerateObjectRid(
    PDWORD  pdwRidSequence,
    PDWORD  pdwObjectRid
    );

static
DWORD
_VmDirSynchronizeRidSequence(
    PVDIR_DOMAIN_SID_GEN_STATE  pDomainSidState
    );

static
DWORD
_VmDirAllocateSidGenStackNode(
    PVMDIR_SID_GEN_STACK_NODE *ppSidGenStackNode,
    DWORD dwDomainRidSequence,
    PCSTR pszDomainDn)
{
    DWORD dwError = 0;
    PVMDIR_SID_GEN_STACK_NODE pSidGenStackNode = NULL;
    SIZE_T sStringSize = 0;

    sStringSize = VmDirStringLenA(pszDomainDn) + 1;
    dwError = VmDirAllocateMemory(
                sStringSize + sizeof(*pSidGenStackNode),
                (PVOID*)&pSidGenStackNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSidGenStackNode->dwDomainRidSequence = dwDomainRidSequence;
    pSidGenStackNode->pszDomainDn = (PSTR)((PBYTE)pSidGenStackNode + sizeof(*pSidGenStackNode));
    dwError = VmDirStringCpyA(
                pSidGenStackNode->pszDomainDn,
                sStringSize,
                pszDomainDn);
    assert(dwError == 0);

    *ppSidGenStackNode = pSidGenStackNode;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pSidGenStackNode);
    goto cleanup;
}


/*
 * During backup restore, per domain RID also needs to advance to a safe value to avoid objectsid collision.
 */
DWORD
VmDirAdvanceDomainRID(
    DWORD   dwCnt
    )
{
    DWORD               dwError = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ATTRIBUTE     pAttr = NULL;
    size_t              iIdx = 0;
    DWORD               dwRidSeq = 0;
    DWORD               dwOrgRidSeq = 0;
    VDIR_BERVALUE       bvRID = VDIR_BERVALUE_INIT;

    if ( dwCnt > 0 )
    {
        dwError = VmDirSimpleEqualFilterInternalSearch(
                        "",
                        LDAP_SCOPE_SUBTREE,
                        ATTR_OBJECT_CLASS,
                        OC_DC_OBJECT,
                        &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        for ( iIdx = 0; iIdx < entryArray.iSize; iIdx++ )
        {
            char buf9[9] = {0};

            pAttr =  VmDirEntryFindAttribute(VDIR_ATTRIBUTE_SEQUENCE_RID, entryArray.pEntry+iIdx );
            if ( pAttr )
            {
                dwOrgRidSeq = VmDirStringToIA( (PCSTR)pAttr->vals[0].lberbv.bv_val );
                // dwOrgRidSeq is current DB value, it is most likely NOT in sync with the cache value
                // when the backup was taken.  Thus, advance a full batch of RID +1 for safety.
                dwRidSeq = dwOrgRidSeq + dwCnt + MAX_COUNT_PRIOR_WRITE + 1;

                if ( dwRidSeq < dwOrgRidSeq )
                {
                    VMDIR_LOG_ERROR(
                            VMDIR_LOG_MASK_ALL,
                            "%s, Domain [%s] RID rollover to value (%u)",
                            __FUNCTION__,
                            entryArray.pEntry[iIdx].dn.lberbv_val ,
                            dwRidSeq);

                    BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_STATE);
                }

                dwError = VmDirStringNPrintFA( buf9, sizeof(buf9), sizeof(buf9)-1, "%u", dwRidSeq );
                BAIL_ON_VMDIR_ERROR(dwError);

                bvRID.lberbv_val = buf9;
                bvRID.lberbv_len = VmDirStringLenA(buf9);
                dwError = VmDirInternalEntryAttributeReplace( NULL,
                                                              entryArray.pEntry[iIdx].dn.lberbv_val,
                                                              VDIR_ATTRIBUTE_SEQUENCE_RID,
                                                              &bvRID);
                BAIL_ON_VMDIR_ERROR(dwError);

                VMDIR_LOG_INFO(
                        VMDIR_LOG_MASK_ALL,
                        "%s: Domain [%s] RID advanced from (%u) to (%u)",
                        __FUNCTION__,
                        entryArray.pEntry[iIdx].dn.lberbv_val,
                        dwOrgRidSeq,
                        dwRidSeq);
            }
        }
    }

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: advance (%u) failed error (%u)",
            __FUNCTION__,
            dwCnt,
            dwError);

    goto cleanup;
}

DWORD
VmDirGenerateObjectSid(
    PVDIR_ENTRY pEntry,
    PSTR *      ppszObjectSid
    )
{
    DWORD                       dwError = 0;
    BOOLEAN                     bInLock = FALSE;
    // Do not free ref
    PVDIR_DOMAIN_SID_GEN_STATE  pSidGenState = NULL;
    DWORD                       dwObjectRid = 0;
    PSTR                        pszObjectSid = NULL;
    PSTR                        pszObjectDN = NULL;
    PSTR                        pszDomainDn = NULL;
    BOOLEAN                     IsDomainObject = FALSE;
    PVMDIR_SID_GEN_STACK_NODE   pSidGenStackNode = NULL;

    pszObjectDN = BERVAL_NORM_VAL(pEntry->dn);

    // Check to see whether the entry is an OC_DC_OBJECT
    dwError = VmDirIsDomainObjectWithEntry(pEntry, &IsDomainObject);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!IsDomainObject)
    {
        pszDomainDn = (PSTR)VmDirFindDomainDN(pszObjectDN);
        if (IsNullOrEmptyString(pszDomainDn))
        {
            // special object for instance
            // SUB_SCHEMA_SUB_ENTRY_DN, CFG_ROOT_DN, CFG_MANAGER_ENTRY_DN
            // Do nothing
            // and object cannot find an existing domain object
            BAIL_WITH_VMDIR_ERROR(dwError, ERROR_NO_OBJECT_SID_GEN);
        }
    }
    else
    {
        pszDomainDn = pszObjectDN;
    }

    // S-1-7-21-sub1-sub2-sub3-sub4 --> domainSid  (vmwobjectSid)
    // S-1-7-21-sub1-sub2-sub3-sub4-rid --> objectSid    (vmwobjectSid)
    //PVDIR_O_SID_GEN_STATE pOrgSidGenState = NULL;
    VMDIR_LOCK_MUTEX(bInLock, gSidGenState.mutex);

    // pSidGenState refers to the state found in gSidGenState
    dwError = VmDirGetSidGenStateIfDomain_inlock(pszDomainDn, IsDomainObject ? pEntry->pszGuid : NULL, &pSidGenState);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert(pSidGenState!=NULL);

    // for non-domain object, generate RID
    if (!IsDomainObject)
    {
        dwError = VmDirGenerateObjectRid(&pSidGenState->dwDomainRidSeqence,
                                         &dwObjectRid);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                           "allocate objectsid: (%s)(%u)(%u)(%u)",
                           VDIR_SAFE_STRING(pSidGenState->pszDomainSid),
                           gVmdirServerGlobals.serverId,
                           dwObjectRid,
                           pSidGenState->dwDomainRidSeqence);

        pSidGenState->dwCount++;
        if (pSidGenState->dwCount >= MAX_COUNT_PRIOR_WRITE)
        {
            dwError = _VmDirAllocateSidGenStackNode(
                        &pSidGenStackNode,
                        pSidGenState->dwDomainRidSeqence,
                        pSidGenState->pszDomainDn);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirPushTSStack(gSidGenState.pStack, pSidGenStackNode) != 0)
            {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                                   "gSidGenState.pStack push failed, (%s)",
                                   pSidGenStackNode->pszDomainDn);
            }
            else
            {
                pSidGenStackNode = NULL;
                pSidGenState->dwCount = 0;
                VmDirSrvThrSignal( gSidGenState.pRIDSyncThr );
            }
        }

        dwError = VmDirAllocateStringPrintf(
                        &pszObjectSid,
                        "%s-%u-%u",
                        pSidGenState->pszDomainSid,
                        gVmdirServerGlobals.serverId,
                        dwObjectRid
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(
                        pSidGenState->pszDomainSid,
                        &pszObjectSid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);

    VMDIR_SAFE_FREE_MEMORY(pSidGenStackNode);

    if (dwError != 0)
    {
        VMDIR_SAFE_FREE_MEMORY(pszObjectSid);
        pszObjectSid = NULL;
    }

    *ppszObjectSid = pszObjectSid;

    return dwError;
}

void
VmDirFindDomainRidSequenceWithDN(
    PCSTR   pszDomainDN,
    PDWORD  pRidSeq
    )
{
    PVDIR_DOMAIN_SID_GEN_STATE  pFoundState = NULL;
    BOOLEAN                     bInLock = FALSE;
    DWORD                       dwError = 0;

    VMDIR_LOCK_MUTEX(bInLock, gSidGenState.mutex);

    dwError = VmDirGetSidGenStateIfDomain_inlock( pszDomainDN, NULL, &pFoundState );

    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);

    *pRidSeq = ((dwError == 0) && pFoundState) ? pFoundState->dwDomainRidSeqence : 0;

    return;
}

DWORD
VmDirIsDomainObjectWithEntry(
    PVDIR_ENTRY pEntry,
    PBOOLEAN    pbIsDomainObject
    )
{
    DWORD           dwError = 0;
    BOOLEAN         bIsDomainObject = FALSE;
    PVDIR_ATTRIBUTE pAttr = NULL;
    unsigned int    iCnt = 0;

    if (!pEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_ENTRY);
    }

    pAttr = VmDirEntryFindAttribute(ATTR_OBJECT_CLASS, pEntry);
    if (!pAttr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_ENTRY);
    }

    for (; iCnt < pAttr->numVals; iCnt++)
    {
        if (!VmDirStringCompareA((PSTR)pAttr->vals[iCnt].lberbv.bv_val, OC_DC_OBJECT, FALSE))
        {
            bIsDomainObject = TRUE;
            break;
        }
    }

error:
    if (dwError != ERROR_SUCCESS)
    {
        bIsDomainObject = FALSE;
    }

    *pbIsDomainObject = bIsDomainObject;

    return dwError;
}

PCSTR
VmDirFindDomainDN(
    PCSTR   pszObjectDN
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszCurrDn = (PSTR)pszObjectDN;
    PVDIR_DOMAIN_SID_GEN_STATE  pSidGenState = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gSidGenState.mutex);

    while (!IsNullOrEmptyString(pszCurrDn))
    {
        dwError = VmDirGetSidGenStateIfDomain_inlock( pszCurrDn, NULL, &pSidGenState );
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pSidGenState)
        {
            break;
        }

        pszCurrDn = VmDirStringChrA(pszCurrDn, RDN_SEPARATOR_CHAR);
        pszCurrDn = pszCurrDn ? pszCurrDn + 1 : NULL;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);
    return pszCurrDn;

error:
    pszCurrDn = NULL;
    goto cleanup;
}

PCSTR
VmDirFindDomainSid(
    PCSTR   pszObjectDN
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszCurrDn = (PSTR)pszObjectDN;
    PVDIR_DOMAIN_SID_GEN_STATE  pSidGenState = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gSidGenState.mutex);

    while (!IsNullOrEmptyString(pszCurrDn))
    {
        dwError = VmDirGetSidGenStateIfDomain_inlock( pszCurrDn, NULL, &pSidGenState );
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pSidGenState)
        {
            break;
        }

        pszCurrDn = VmDirStringChrA(pszCurrDn, RDN_SEPARATOR_CHAR);
        pszCurrDn = pszCurrDn ? pszCurrDn + 1 : NULL;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);
    return pSidGenState ? pSidGenState->pszDomainSid : NULL;

error:
    goto cleanup;
}

VOID
VmDirFreeOrgState(
    PVOID pOrgStat
    )
{
    if (pOrgStat)
    {
        PVDIR_DOMAIN_SID_GEN_STATE pOrgCurrState =  pOrgStat;

        VMDIR_SAFE_FREE_MEMORY(pOrgCurrState->pszDomainDn);
        VMDIR_SAFE_FREE_MEMORY(pOrgCurrState->pszDomainSid);

        VMDIR_SAFE_FREE_MEMORY(pOrgCurrState);
    }
}

DWORD
VmDirInternalRemoveOrgConfig(
    PVDIR_OPERATION pOperation,
    PSTR            pszDomainDN
    )
{
    DWORD                       dwError = 0;
    BOOLEAN                     bInLock = FALSE;
    PVDIR_DOMAIN_SID_GEN_STATE pFoundState = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gSidGenState.mutex);

    pFoundState = VmDirFindSidGenStateWithDN_inlock(pszDomainDN);
    if (!pFoundState)
    {
        goto cleanup;
    }
    dwError = LwRtlHashTableRemove(gSidGenState.pHashtable, &pFoundState->Node);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirFreeOrgState(pFoundState);

    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);

    return dwError;

error:
    goto cleanup;
}

/* Given a domain DN, and a well-known RID
 * generate a valid well-known objectSid
 *
 * This API uses global cache, so not backend operations*/
DWORD
VmDirGenerateWellknownSid(
    PCSTR   pszDomainDN,
    DWORD   dwWellKnowRid,
    PSTR*   ppszWellKnownSid
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainSid = NULL;
    PSTR    pszWellKnownSid = NULL;
    BOOLEAN bInLock = FALSE;
    // Do not free ref
    PVDIR_DOMAIN_SID_GEN_STATE  pSidGenState = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gSidGenState.mutex);

    if (IsNullOrEmptyString(pszDomainDN))
    {
        // use null SID template if domain is not specified
        pszDomainSid = VMDIR_NULL_SID_TEMPLATE;
    }
    else
    {
        // pSidGenState refers to the state found in gSidGenState
        dwError = VmDirGetSidGenStateIfDomain_inlock(
                pszDomainDN, NULL, &pSidGenState);
        BAIL_ON_VMDIR_ERROR(dwError);
        assert(pSidGenState!=NULL);

        pszDomainSid = pSidGenState->pszDomainSid;
    }

    dwError = VmDirAllocateStringPrintf(
            &pszWellKnownSid, "%s-%u", pszDomainSid, dwWellKnowRid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszWellKnownSid = pszWellKnownSid;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gSidGenState.mutex);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszWellKnownSid);
    goto cleanup;
}



/* Holding 'gSidGenState.mutex' to find/create the state entry for a given DN in global 'gSidGenState' cache
 * if it corresponds to a domain object.
 *
 * PCSTR    pszObjectDN: may or may not be a domain DN
 *
 * PSTR     pszGuid: may or may not be NULL. Non-null => domain DN => create cache entry if not already there
 *
 * Function performs the following tasks (bit overloaded):
 *
 *  1) Checks if a given pszObjectDN is a domain (OC_DC_OBECT) DN or NOT by:
 *      - First checking cache
 *      - If not in cache, by reading the entry from DB, and checking if it is a OC_DC_OBJECT
 *
 *  2) Creates an entry in cache if it is a domain DN:
 *      - SID is either picked from the entry present in DB, OR
 *      - SID is generated from the given GUID
 *
 *  Returns:
 *      - non-NULL PVDIR_DOMAIN_SID_GEN_STATE* ppDomainState if pszObjectDN is a domain DN
 *      - NULL PVDIR_DOMAIN_SID_GEN_STATE* ppDomainState if pszObjectDN is NOT a domain DN
 *
 */

DWORD
VmDirGetSidGenStateIfDomain_inlock(
    PCSTR                       pszObjectDN,
    PCSTR                       pszGuid,
    PVDIR_DOMAIN_SID_GEN_STATE* ppDomainState
    )
{
    DWORD                       dwError = 0;
    PVDIR_DOMAIN_SID_GEN_STATE  pOrgState = NULL;
    PVDIR_ENTRY                 pEntry = NULL;
    BOOLEAN                     IsDomainObject = FALSE;
    PVDIR_ATTRIBUTE             pObjSidAttr = NULL;
    PVDIR_ATTRIBUTE             pRidSeqAttr = NULL;
    DWORD                       dwDomainRidSequence = VMDIR_ACL_RID_BASE;
    PSTR                        pszLocalErrorMsg = NULL;

    pOrgState = VmDirFindSidGenStateWithDN_inlock(pszObjectDN);
    if (pOrgState) // Found in cache
    {
        goto cleanup;
    }

    if (pszGuid == NULL)
    {
        if ((dwError = VmDirSimpleDNToEntry(pszObjectDN, &pEntry)) != 0)
        {
            if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND || dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
            {
                dwError = 0;
                goto cleanup;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirIsDomainObjectWithEntry(pEntry, &IsDomainObject);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!IsDomainObject)
        {
            goto cleanup;
        }


        if ((pObjSidAttr = VmDirEntryFindAttribute(ATTR_OBJECT_SID, pEntry)) == NULL)
        {
            dwError = VMDIR_ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    dwError,
                    pszLocalErrorMsg,
                    "VmDirGetSidGenStateIfDomain_inlock(): Domain object (%s), ATTR_OBJECT_SID not found",
                    pszObjectDN);
        }

        if ((pRidSeqAttr = VmDirEntryFindAttribute(VDIR_ATTRIBUTE_SEQUENCE_RID, pEntry)) != NULL)
        {
            dwDomainRidSequence = VmDirStringToIA( (PSTR)pRidSeqAttr->vals[0].lberbv.bv_val );
        }
    }

    // For domain object create new state
    dwError = VmDirAllocateMemory( sizeof(VDIR_DOMAIN_SID_GEN_STATE), (PVOID*)&pOrgState );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pszObjectDN, &pOrgState->pszDomainDn );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pObjSidAttr)
    {
        dwError = VmDirAllocateStringA( (PSTR)pObjSidAttr->vals[0].lberbv.bv_val, &pOrgState->pszDomainSid );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        // Generate domain SID
        dwError = VmDirGenerateDomainGuidSid_inlock( pszGuid, &pOrgState->pszDomainSid );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pOrgState->dwDomainRidSeqence = dwDomainRidSequence;
    dwError = _VmDirSynchronizeRidSequence( pOrgState );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Init Sid cache (%s) RID (%u)",
            pOrgState->pszDomainDn,
            pOrgState->dwDomainRidSeqence);

    LwRtlHashTableResizeAndInsert(gSidGenState.pHashtable, &pOrgState->Node, NULL);

cleanup:
    *ppDomainState = pOrgState;

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }
    return dwError;

error:
    VmDirFreeOrgState(pOrgState);
    pOrgState = NULL;

    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "VmDirGetSidGenStateIfDomain_inlock() failed. Error code = %d, objectDN = %s, "
            "local error message = %s",
            dwError,
            VDIR_SAFE_STRING(pszObjectDN),
            VDIR_SAFE_STRING(pszLocalErrorMsg));

    goto cleanup;
}

/* Holding 'gSidGenState.mutex' to find a reference to the state entry for a given domain
 * DN in global 'gSidGenState' cache */
static
PVDIR_DOMAIN_SID_GEN_STATE
VmDirFindSidGenStateWithDN_inlock(
    PCSTR pszDomainDN
    )
{
    DWORD                       dwError = 0;
    PVDIR_DOMAIN_SID_GEN_STATE  pOrgState = NULL;
    PLW_HASHTABLE_NODE          pNode = NULL;

    dwError = LwRtlHashTableFindKey(gSidGenState.pHashtable, &pNode, (PVOID)pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    pOrgState = (PVDIR_DOMAIN_SID_GEN_STATE)LW_STRUCT_FROM_FIELD(pNode, VDIR_DOMAIN_SID_GEN_STATE, Node);

error:
    return pOrgState;
}

static
DWORD
_VmDirGenerateDomainGuid(
    PCSTR   pszGuid, /* optional */
    uuid_t* pDomainGuid
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pDomainGuid, dwError);

    if (!IsNullOrEmptyString(pszGuid))
    {
        dwError = VmDirUuidFromString(pszGuid, pDomainGuid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirUuidGenerate(pDomainGuid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

 cleanup:
     return dwError;

 error:
    goto cleanup;
}

/* gSidGenState.mutex held calling this function */
static
DWORD
VmDirGenerateDomainGuidSid_inlock(
    PCSTR   pszGuid, /* optional, if given used directly. caller should only use a unique Guid once.*/
    PSTR*   ppszDomainSid
    )
{
    // S-1-7-21-sub1-sub2-sub3-sub4 --> organizationSid  (vmwobjectSid)
    DWORD                       dwError = 0;
    uuid_t                      OrgGuid = {0};
    uuid_t*                     pOrgGuid = NULL;
    PSTR                        pszDomainSid = NULL;
    PSID                        pOrgSid = NULL;
    ULONG                       ulOrgSidLength = 0;
    SID_IDENTIFIER_AUTHORITY    IdAuthority = { SECURITY_VMWARE_AUTHORITY };


    dwError = _VmDirGenerateDomainGuid(pszGuid, &OrgGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(uuid_t), (PVOID*)&pOrgGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
        pOrgGuid, sizeof(uuid_t), &OrgGuid, sizeof(uuid_t) );
    BAIL_ON_VMDIR_ERROR(dwError);

    ulOrgSidLength = VmDirLengthRequiredSid(ORGANIZATION_SUB_AUTHORITY_NUMBER);

    dwError = VmDirAllocateMemory(ulOrgSidLength,
                                 (PVOID*)&pOrgSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitializeSid(pOrgSid,
                                &IdAuthority,
                                ORGANIZATION_SUB_AUTHORITY_NUMBER);
    BAIL_ON_VMDIR_ERROR(dwError);

    // SECURITY_SUBAUTHORITY_ORGANIZATION-sub1-sub2-sub3-sub4 (total 5 parts)
    dwError = VmDirSetSidSubAuthority( pOrgSid, 4, *((PULONG)pOrgGuid) );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSidSubAuthority( pOrgSid, 3, *((PULONG)pOrgGuid + 1) );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSidSubAuthority( pOrgSid, 2, *((PULONG)pOrgGuid + 2) );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSidSubAuthority( pOrgSid, 1, *((PULONG)pOrgGuid + 3) );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSidSubAuthority( pOrgSid, 0, (ULONG)SECURITY_SUBAUTHORITY_ORGANIZATION );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCStringFromSid(&pszDomainSid,pOrgSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDomainSid = pszDomainSid;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pOrgGuid);
    VMDIR_SAFE_FREE_MEMORY(pOrgSid);
    return dwError;

error:
    dwError = VMDIR_ERROR_ORG_ID_GEN_FAILED;
    VMDIR_SAFE_FREE_MEMORY(pszDomainSid);
    *ppszDomainSid = NULL;
    goto cleanup;
}

/*
 * Get the next value from our per-domain counter. This value constitutes
 * part of the object's SID.
 */
static
DWORD
VmDirGenerateObjectRid(
    PDWORD pdwRidSequence,
    PDWORD pdwObjectRid
    )
{
    DWORD dwError = 0;
    DWORD dwRid = *pdwRidSequence;

    // Check to see whether current Rid hits the MAX
    if (dwRid+1 > MAX_RID_SEQUENCE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_RID_LIMIT_EXCEEDED);
    }

    dwRid++;
    *pdwRidSequence = dwRid;
    *pdwObjectRid = dwRid;

error:
    if (dwError)
    {
        *pdwObjectRid = 0;
    }

    return dwError;
}

/*
 * Called during PVDIR_DOMAIN_SID_GEN_STATE instantiation.
 *
 * Make sure rid sequence number from db is up-to-date.
 * For example, if vmdird is not shutdown properly, rid sequence number in DB will be old.
 * If we do NOT re-synchronize, we will see ldap error (19) for ldap add operations.
 */
static
DWORD
_VmDirSynchronizeRidSequence(
    PVDIR_DOMAIN_SID_GEN_STATE  pDomainSidState
    )
{
    DWORD               dwError = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PSTR                pszObjectSid = NULL;
    DWORD               dwCnt = 0;
    VDIR_BERVALUE       bvRID = VDIR_BERVALUE_INIT;
    char                buf64[ 64+1 ] = {0};

    // Walk down from the next batch and look for the first used
    // With potential issue from 5.5. upgrade, extend the scan to 5 batches
    for ( dwCnt = pDomainSidState->dwDomainRidSeqence + (MAX_COUNT_PRIOR_WRITE * 5);
          dwCnt > pDomainSidState->dwDomainRidSeqence;
          dwCnt--
        )
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszObjectSid,
                        "%s-%u-%u",
                        pDomainSidState->pszDomainSid,
                        gVmdirServerGlobals.serverId,
                        dwCnt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSimpleEqualFilterInternalSearch(
                        "",
                        LDAP_SCOPE_SUBTREE,
                        ATTR_OBJECT_SID,    // objectsid is unique indexed.
                        pszObjectSid,
                        &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize != 0)
        {   // done, last used objectsid.
            VmDirFreeEntryArrayContent(&entryArray);
            break;
        }

        VMDIR_SAFE_FREE_MEMORY(pszObjectSid);
    }

    if (dwCnt != pDomainSidState->dwDomainRidSeqence || gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB)
    {
        // dwCnt may also not equal to pDomainSidState->dwDomainRidSeqence if no entries have been added
        //  with new SID format: ...-serverId-RidSequence,

        if  (gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB)
        {
               //The database is just copied from its partner, reset RID
               dwCnt = VMDIR_ACL_RID_BASE;
        }
        dwError = VmDirStringPrintFA( buf64, sizeof(buf64), "%u", dwCnt );
        BAIL_ON_VMDIR_ERROR(dwError);

        bvRID.lberbv_val = buf64;
        bvRID.lberbv_len = VmDirStringLenA(buf64);

        // TODO
        // This causes mdb deadlock during non-first node promotion
        // Need to move this to proper location
//        dwError = VmDirInternalEntryAttributeReplace( NULL,
//                                                      pDomainSidState->pszDomainDn,
//                                                      VDIR_ATTRIBUTE_SEQUENCE_RID,
//                                                      &bvRID);
//        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // dwCnt + 1 is the next available RID
    pDomainSidState->dwDomainRidSeqence = dwCnt;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszObjectSid);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "_VmDirSynchronizeRidSequence failed (%d)",
            dwError);

    goto cleanup;
}
