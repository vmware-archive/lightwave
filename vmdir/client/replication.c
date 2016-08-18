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

#define LDAP_USN_QUERY_LIMIT 500

static
DWORD
_VmDirGetUtdAndIdForHostFromRADN(
    LDAP* pLD,
    PCSTR pszHost,
    PSTR  *ppszUtdVector,
    PSTR  *ppszInvocationId
   );

static
DWORD
_VmDirGetReplicationCookieForHost(
    LDAP* pLD,
    PCSTR pszHost,

    PCSTR pszPartner,
    PVMDIR_REPLICATION_COOKIE *ppCookie
   );

static
VOID
_VmDirFreeReplicationCookie(
    PVMDIR_REPLICATION_COOKIE pCookie
   );

static
DWORD
_VmDirReplicationEntriesExist(
    PCSTR pszDomain,
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PVMDIR_REPLICATION_COOKIE pCookie,
    PBOOLEAN pbEntriesExist
    );

static
DWORD
_VmDirRAsToStruct(
    PCSTR   pszStr,
    PVMDIR_REPL_REPL_AGREEMENT*  ppRA
    );

static
DWORD
_VmDirQueryReplStateUSN(
    LDAP*   pLd,
    PCSTR   pszInvocationId,
    USN     currentUSN,
    USN*    maxConsumableUSN
    );

static
DWORD
_VmDirEntryToReplState(
    LDAP*           pLd,
    LDAPMessage*    pEntry,
    PVMDIR_REPL_STATE  pReplState
    );

static
VOID
_VmDirFreeReplRA(
    PVMDIR_REPL_REPL_AGREEMENT  pRA
    );

static
DWORD
_VmDirGetServerObjectBC(
    LDAP*               pLd,
    PCSTR               pszServerDN,
    PVMDIR_REPL_STATE   pReplState
    );

static
DWORD
_VmDirGetRAObjectBC(
    LDAP*               pLd,
    PCSTR               pszServerDN,
    PVMDIR_REPL_STATE   pReplState
    );

static
DWORD
_VmDirGetReplicationStateBC(
    LDAP*               pLd,
    PVMDIR_REPL_STATE   pReplState
    );

/*
 * The UpToDate vector and Invocation Id attributes are on the same entry.
 */
static
DWORD
_VmDirGetUtdAndIdForHostFromRADN(
    LDAP *pLD,
    PCSTR pszRADN,
    PSTR *ppszUtdVector,
    PSTR *ppszInvocationId
    )
{
    DWORD dwError = 0;
    PSTR        pszAttrUpToDateVector = ATTR_UP_TO_DATE_VECTOR;
    PSTR        pszAttrInvocationId = ATTR_INVOCATION_ID;
    PSTR        ppszAttrs[] = { pszAttrUpToDateVector, pszAttrInvocationId, NULL };
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PCSTR       pszEndOfRADN = NULL;
    PCSTR       pszServerDn = NULL;
    PSTR        pszUtdVector = NULL;
    PSTR        pszInvocationId = NULL;

    /*
     * Given labeledUri=ldaps://server:636,cn=Replication Agreements,cn=foo,...
     * we want just the cn=foo,...
     */
    pszEndOfRADN = pszRADN + strlen(pszRADN);
    pszServerDn = VmDirStringStrA(pszRADN, VMDIR_REPL_AGRS_CONTAINER_NAME);
    pszServerDn += strlen(VMDIR_REPL_AGRS_CONTAINER_NAME);
    if (pszServerDn >= pszEndOfRADN || *pszServerDn != ',')
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pszServerDn++;

    dwError = ldap_search_ext_s(
                pLD,
                pszServerDn,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLD, pResult) != 1) /* expect only one object */
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLD, pResult);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLD, pEntry, pszAttrUpToDateVector);
    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(ppValues[0]->bv_val, &pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);
    ldap_value_free_len(ppValues);
    ppValues = NULL;

    ppValues = ldap_get_values_len(pLD, pEntry, pszAttrInvocationId);
    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(ppValues[0]->bv_val, &pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);
    ldap_value_free_len(ppValues);
    ppValues = NULL;


    *ppszUtdVector = pszUtdVector;
    *ppszInvocationId = pszInvocationId;

cleanup:
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszUtdVector);
    VMDIR_SAFE_FREE_STRINGA(pszInvocationId);
    goto cleanup;
}

DWORD
VmDirGetLastLocalUsnProcessedForHostFromRADN(
    LDAP *pLD,
    PCSTR pszRADN,
    USN* pUsn
    )
{
    DWORD dwError = 0;
    PSTR        pszAttrLastLocalUsnProcessed = ATTR_LAST_LOCAL_USN_PROCESSED;
    PSTR        ppszAttrs[] = { pszAttrLastLocalUsnProcessed, NULL };
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;

    dwError = ldap_search_ext_s(
                pLD,
                pszRADN,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLD, pResult) != 1) /* expect only one object */
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLD, pResult);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLD, pEntry, pszAttrLastLocalUsnProcessed);
    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pUsn = VmDirStringToLA(ppValues[0]->bv_val, NULL, 10);

cleanup:
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;

error:
    goto cleanup;
}

/*
 * Query all information needed for 'pLD' to consume from pszHost.
 */
static
DWORD
_VmDirGetReplicationCookieForHost(
    LDAP* pLD,
    PCSTR pszHost,
    PCSTR pszPartner,
    PVMDIR_REPLICATION_COOKIE *ppCookie
    )
{
    DWORD       dwError=0;
    DWORD       dwSize=0;
    DWORD       dwCnt=0;
    PSTR*       ppszRADNs=NULL;
    PVMDIR_REPLICATION_COOKIE pCookie = NULL;
    USN         usn = 0;
    PSTR        pszUtdVector = NULL;
    PSTR        pszInvocationId = NULL;
    PSTR        pszPartnerRA = NULL;
    PSTR        pszCN = ",cn=";
    PSTR        pszEndOfRA = NULL;

    dwError = VmDirGetAllRAToHost( pLD, pszHost, &ppszRADNs, &dwSize );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        VMDIR_SAFE_FREE_STRINGA(pszUtdVector);
        VMDIR_SAFE_FREE_STRINGA(pszInvocationId);

        if (pszPartner != NULL)
        {
            pszEndOfRA = ppszRADNs[dwCnt] + strlen(ppszRADNs[dwCnt]);
            pszPartnerRA = VmDirStringStrA(ppszRADNs[dwCnt], VMDIR_REPL_AGRS_CONTAINER_NAME);

            if(pszPartnerRA == NULL)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            pszPartnerRA += strlen(VMDIR_REPL_AGRS_CONTAINER_NAME);
            if (pszPartnerRA >= pszEndOfRA || *pszPartnerRA != ',')
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszPartnerRA += strlen(pszCN);
            if (pszPartnerRA >= pszEndOfRA)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (VmDirStringNCompareA(pszPartner, pszPartnerRA, strlen(pszPartner), FALSE) != 0)
            {
                continue;
            }
        }

        dwError = VmDirGetLastLocalUsnProcessedForHostFromRADN(pLD, ppszRADNs[dwCnt], &usn);
        if (dwError == 0)
        {
            break;
        }
    }
    if (dwCnt >= dwSize)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirGetUtdAndIdForHostFromRADN(pLD, ppszRADNs[dwCnt], &pszUtdVector, &pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(*pCookie), (PVOID) &pCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCookie->lastLocalUsnProcessed = usn;
    pCookie->pszInvocationId = pszInvocationId;
    pCookie->pszUtdVector = pszUtdVector;
    *ppCookie = pCookie;

cleanup:

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY( ppszRADNs[dwCnt] );
    }
    VMDIR_SAFE_FREE_MEMORY(ppszRADNs);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirGetReplicationCookieForHost (%u)", dwError);

    VMDIR_SAFE_FREE_STRINGA(pszUtdVector);
    VMDIR_SAFE_FREE_STRINGA(pszInvocationId);
    goto cleanup;
}

static
VOID
_VmDirFreeReplicationCookie(
    PVMDIR_REPLICATION_COOKIE pCookie
   )
{
    if (pCookie)
    {
        VMDIR_SAFE_FREE_STRINGA(pCookie->pszUtdVector);
        VMDIR_SAFE_FREE_STRINGA(pCookie->pszInvocationId);
        VMDIR_SAFE_FREE_MEMORY(pCookie);
    }
}

/*
 * Using the replication cookie constructed from a partner host,
 * determine if all entries on the localhost have been transferred
 * to the partner host by performing the same search query it would
 * use.
 */
static
DWORD
_VmDirReplicationEntriesExist(
    PCSTR pszDomain,
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PVMDIR_REPLICATION_COOKIE pCookie,
    PBOOLEAN pbEntriesExist
    )
{
    DWORD       dwError=0;
    LDAP*       pLd                     = NULL;
    PSTR        pszFilter = NULL;
    LDAPControl *srvCtrls[2] = {NULL, NULL};
    LDAPControl syncReqCtrl = {0};
    LDAPMessage *pResult = NULL;
    BOOLEAN     bEntriesExist = FALSE;

    // bind to server
    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszServerName,
                            pszDomain,
                            pszUserName,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszFilter, "%s>=%ld",
                ATTR_USN_CHANGED,
                pCookie->lastLocalUsnProcessed + 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateSyncRequestControl(pCookie->pszInvocationId, pCookie->lastLocalUsnProcessed, pCookie->pszUtdVector, &syncReqCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    srvCtrls[0] = &syncReqCtrl;
    srvCtrls[1] = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                "",
                LDAP_SCOPE_SUBTREE,
                pszFilter,
                NULL,
                FALSE, /* get values      */
                srvCtrls,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                1,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult))
    {
        bEntriesExist = TRUE;
    }

    *pbEntriesExist = bEntriesExist;

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VmDirDeleteSyncRequestControl(&syncReqCtrl);
    if (pLd)
    {
        VmDirLdapUnbind(&pLd);
    }
    VMDIR_SAFE_FREE_MEMORY( pszFilter );

    return dwError;

error:

    goto cleanup;
}

/*
 * Determine if the partner at pLD is up to date with respect to
 * account pszAccount.
 */
DWORD
VmDirIsPartnerReplicationUpToDate(
    LDAP *pLD,
    PCSTR pszPartnerName,
    PCSTR pszDomain,
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PBOOLEAN pbUpToDate
    )
{
    DWORD dwError = 0;
    BOOLEAN bEntriesExist = FALSE;
    PVMDIR_REPLICATION_COOKIE pCookie = NULL;

    if (pLD == NULL || pszDomain == NULL || pszServerName == NULL ||
        pszUserName == NULL || pszPassword == NULL || pbUpToDate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirGetReplicationCookieForHost(pLD, pszServerName, pszPartnerName, &pCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirReplicationEntriesExist(pszDomain, pszServerName, pszUserName, pszPassword, pCookie, &bEntriesExist);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbUpToDate = !bEntriesExist;

cleanup:
    _VmDirFreeReplicationCookie(pCookie);
    return dwError;

error:
     goto cleanup;
}

static
DWORD
_VmDirQueryUsn(
    LDAP*                       pLd,
    PVMDIR_REPLICATION_COOKIE   pCookie,
    USN                         startUsn,
    USN*                        pMaxUsn,
    DWORD*                      pdwCount
    )
{
    DWORD       dwError = 0;
    int         errCode = 0;
    DWORD       dwCount = 0;
    PSTR        pszFilter = NULL;
    LDAPControl *srvCtrls[2] = {NULL, NULL};
    LDAPControl syncReqCtrl = {0};
    LDAPMessage *pResult = NULL;
    LDAPControl **searchResCtrls = NULL;
    BerValue    bvLastLocalUsnProcessed = {0};

    dwError = VmDirAllocateStringPrintf(
                &pszFilter, "%s>=%ld",
                ATTR_USN_CHANGED,
                startUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateSyncRequestControl(
                pCookie->pszInvocationId,
                pCookie->lastLocalUsnProcessed,
                pCookie->pszUtdVector, &syncReqCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    srvCtrls[0] = &syncReqCtrl;
    srvCtrls[1] = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                "",
                LDAP_SCOPE_SUBTREE,
                pszFilter,
                NULL,
                FALSE, /* get values      */
                srvCtrls,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                LDAP_USN_QUERY_LIMIT,     /* size limit */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwCount = ldap_count_entries(pLd, pResult);
    if (dwCount == 0)
    {
        *pdwCount = dwCount;
        goto cleanup;
    }

    dwError = (DWORD)ldap_parse_result(pLd, pResult, &errCode,
                NULL, NULL, NULL, &searchResCtrls, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (searchResCtrls[0] == NULL)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "_VmDirQueryUsn: ldap_parse_result returned empty ctrl.");
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    bvLastLocalUsnProcessed.bv_val = searchResCtrls[0]->ldctl_value.bv_val;
    bvLastLocalUsnProcessed.bv_len = VmDirStringChrA(bvLastLocalUsnProcessed.bv_val, ',')
                                            - bvLastLocalUsnProcessed.bv_val;
    bvLastLocalUsnProcessed.bv_val[bvLastLocalUsnProcessed.bv_len] = '\0';

    *pMaxUsn = VmDirStringToLA(bvLastLocalUsnProcessed.bv_val, NULL, 10);
    *pdwCount = dwCount;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFilter);

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    if (searchResCtrls)
    {
        ldap_controls_free(searchResCtrls);
        searchResCtrls = NULL;
    }

    VmDirDeleteSyncRequestControl(&syncReqCtrl);

    return dwError;
error:
    goto cleanup;
}

/*
 * Using the replication cookie constructed from a partner host,
 * determine the max USN of the host.
 */
static
DWORD
_VmDirGetReplicationPartnerUsnLag(
    PVMDIR_REPLICATION_COOKIE pCookie,
    USN*                      pMaxUsn
    )
{
    DWORD       dwError = 0;
    PSTR        pszPassword = NULL;
    PSTR        pszDCAccount = NULL;
    PSTR        pszDomain = NULL;
    PSTR        pszServerName = NULL;
    LDAP*       pLd = NULL;
    USN         usn = pCookie->lastLocalUsnProcessed;
    DWORD       dwCnt = 0;

    dwError = VmDirReadDCAccountPassword( &pszPassword );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegReadDCAccount( &pszDCAccount );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerName( pszDCAccount, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get domain name
    dwError = VmDirGetDomainName(
                    pszServerName,
                    &pszDomain
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    // bind to server
    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszServerName,
                            pszDomain,
                            pszDCAccount,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    do
    {
        dwError = _VmDirQueryUsn(pLd, pCookie, usn + 1, &usn, &dwCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    while (dwCnt == LDAP_USN_QUERY_LIMIT);

    *pMaxUsn = usn;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszServerName);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszPassword);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:

    goto cleanup;
}


/*
 * Determine the partner's replication status with the node with respect to
 * account pszAccount.
 */
DWORD
VmDirGetPartnerReplicationStatus(
    LDAP *pLD,
    PCSTR pszAccount,
    PVMDIR_REPL_PARTNER_STATUS pPartnerStatus
    )
{
    DWORD dwError = 0;
    PVMDIR_REPLICATION_COOKIE pCookie = NULL;

    if (pLD == NULL ||
        pszAccount == NULL ||
        pPartnerStatus == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirGetReplicationCookieForHost(pLD, pszAccount, pPartnerStatus->pszHost, &pCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetReplicationPartnerUsnLag(pCookie, &pPartnerStatus->targetUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pPartnerStatus->partnerUsn = pCookie->lastLocalUsnProcessed;

cleanup:
    _VmDirFreeReplicationCookie(pCookie);
    return dwError;

error:
     goto cleanup;
}

DWORD
VmDirGetReplicationStateInternal(
    LDAP*               pLd,
    PVMDIR_REPL_STATE*  ppReplState
    )
{
    DWORD   dwError = 0;
    PCSTR   pszBaseDN = "cn=replicationstatus";
    PCSTR   pszFilter = "objectclass=*";
    PCSTR   pszReplStatus = ATTR_SERVER_RUNTIME_STATUS;
    PCSTR   ppszAttrs[] = { pszReplStatus, NULL };
    LDAPMessage*        pResult = NULL;
    LDAPMessage*        pEntry = NULL;
    PVMDIR_REPL_STATE   pReplState = NULL;

    if ( pLd == NULL || ppReplState == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(*pReplState), (PVOID*)&pReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                pLd,
                pszBaseDN,
                LDAP_SCOPE_BASE,
                pszFilter,
                (PSTR*)ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit */
                &pResult);
    dwError = VmDirMapLdapError(dwError);
    if (dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
    {   // 6.0GA and prior.
        dwError = _VmDirGetReplicationStateBC(
                    pLd,
                    pReplState);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        BAIL_ON_VMDIR_ERROR(dwError);

        if (ldap_count_entries(pLd, pResult) == 0)
        {
            dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pEntry = ldap_first_entry(pLd, pResult);
        dwError = _VmDirEntryToReplState(pLd, pEntry, pReplState);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppReplState = pReplState;

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    VmDirFreeReplicationStateInternal(pReplState);
    goto cleanup;
}

VOID
VmDirFreeReplicationStateInternal(
    PVMDIR_REPL_STATE   pReplState
    )
{
    if (pReplState)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplState->pszHost);
        VMDIR_SAFE_FREE_MEMORY(pReplState->pszInvocationId);
        VmDirFreeReplVector(pReplState->pReplUTDVec);
        _VmDirFreeReplRA(pReplState->pReplRA);
        VMDIR_SAFE_FREE_MEMORY(pReplState);
    }

    return;
}

static
DWORD
_VmDirGetRAObjectBC(
    LDAP*               pLd,
    PCSTR               pszServerDN,
    PVMDIR_REPL_STATE   pReplState
    )
{
    DWORD   dwError = 0;
    PCSTR   pszUSNProcessed = ATTR_LAST_LOCAL_USN_PROCESSED;
    PCSTR   pszLabeledURI = ATTR_LABELED_URI;
    PCSTR   ppszAttrs[] = { pszUSNProcessed, pszLabeledURI, NULL };
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppUSNProcessed = NULL;
    struct berval** ppLabeledURI = NULL;
    PVMDIR_REPL_REPL_AGREEMENT  pRA = NULL;
    PVMDIR_REPL_REPL_AGREEMENT  pTmpRA = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszServerDN,
                LDAP_SCOPE_SUBTREE,
                "objectclass=vmwreplicationagreement",
                (PSTR*)ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit */
                &pResult);
    dwError = VmDirMapLdapError(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pLd, pResult);
         pEntry;
         pEntry = ldap_next_entry(pLd, pEntry))
    {

        dwError = VmDirAllocateMemory(sizeof(*pTmpRA), (PVOID*)&pTmpRA);
        BAIL_ON_VMDIR_ERROR(dwError);

        pTmpRA->next = pRA;
        pRA          = pTmpRA;
        pTmpRA       = NULL;

        if (ppUSNProcessed)
        {
            ldap_value_free_len(ppUSNProcessed);
            ppUSNProcessed = NULL;
        }
        if (ppLabeledURI)
        {
            ldap_value_free_len(ppLabeledURI);
            ppLabeledURI = NULL;
        }

        ppUSNProcessed  = ldap_get_values_len(pLd, pEntry, pszUSNProcessed);
        ppLabeledURI    = ldap_get_values_len(pLd, pEntry, pszLabeledURI);

        if (ppUSNProcessed[0])
        {
            pRA->maxProcessedUSN = atol(ppUSNProcessed[0]->bv_val);
        }
        if (ppLabeledURI[0])
        {
            dwError = VmDirReplURIToHostname(ppLabeledURI[0]->bv_val, &(pRA->pszPartnerName));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    pReplState->pReplRA = pRA;

cleanup:
    if (ppUSNProcessed)
    {
        ldap_value_free_len(ppUSNProcessed);
    }
    if (ppLabeledURI)
    {
        ldap_value_free_len(ppLabeledURI);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    _VmDirFreeReplRA(pRA);
    goto cleanup;
}

static
DWORD
_VmDirGetServerObjectBC(
    LDAP*               pLd,
    PCSTR               pszServerDN,
    PVMDIR_REPL_STATE   pReplState
    )
{
    DWORD   dwError = 0;
    PCSTR   pszCN = ATTR_CN;
    PCSTR   pszInvocationid = ATTR_INVOCATION_ID;
    PCSTR   pszUtdVector = ATTR_UP_TO_DATE_VECTOR;
    PCSTR   ppszAttrs[] = { pszCN, pszInvocationid, pszUtdVector, NULL };
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppCN = NULL;
    struct berval** ppInvocationid = NULL;
    struct berval** ppUtdVector = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszServerDN,
                LDAP_SCOPE_BASE,
                NULL,
                (PSTR*)ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit */
                &pResult);
    dwError = VmDirMapLdapError(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry          = ldap_first_entry(pLd, pResult);
    ppCN            = ldap_get_values_len(pLd, pEntry, pszCN);
    ppInvocationid  = ldap_get_values_len(pLd, pEntry, pszInvocationid);
    ppUtdVector     = ldap_get_values_len(pLd, pEntry, pszUtdVector);

    if (ppCN[0])
    {
        dwError = VmDirAllocateStringA(ppCN[0]->bv_val, &(pReplState->pszHost));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (ppInvocationid[0])
    {
        dwError = VmDirAllocateStringA(ppInvocationid[0]->bv_val, &(pReplState->pszInvocationId));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (ppUtdVector[0])
    {
        dwError = VmDirUTDVectorToStruct(ppUtdVector[0]->bv_val, &(pReplState->pReplUTDVec));
        BAIL_ON_VMDIR_ERROR(dwError);
    }


cleanup:
    if (ppCN)
    {
        ldap_value_free_len(ppCN);
    }
    if (ppInvocationid)
    {
        ldap_value_free_len(ppInvocationid);
    }
    if (ppUtdVector)
    {
        ldap_value_free_len(ppUtdVector);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:

    goto cleanup;
}
/*
 * Older version (6.0 GA and prior) does not have cn=replicationstatus sudo entry.
 * Collect data from server and replication entries directly.
 */
static
DWORD
_VmDirGetReplicationStateBC(
    LDAP*               pLd,
    PVMDIR_REPL_STATE   pReplState
    )
{
    DWORD   dwError = 0;
    PSTR    pszServerDN = NULL;
    DWORD   dwLen = 0;

    dwError = VmDirLdapGetSingleAttribute(
                pLd,
                "",
                ATTR_SERVER_NAME,
                (PBYTE*)&pszServerDN,
                &dwLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetServerObjectBC( pLd, pszServerDN, pReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetRAObjectBC( pLd, pszServerDN, pReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszServerDN);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirRAsToStruct(
    PCSTR   pszStr,
    PVMDIR_REPL_REPL_AGREEMENT*  ppRA
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_STRING_LIST          pStrList = NULL;
    PVMDIR_REPL_REPL_AGREEMENT  pRA = NULL;
    PVMDIR_REPL_REPL_AGREEMENT  pTmpRA = NULL;
    PCSTR                       pDelimiter = ",";

    // No RA processed USN
    if ( VmDirStringCompareA(pszStr, "Unknown", FALSE) == 0 )
    {
        *ppRA = pRA;
        goto cleanup;
    }

    dwError = VmDirStringToTokenList(pszStr, pDelimiter, &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<pStrList->dwCount; dwCnt++)
    {
        if (pStrList->pStringList[dwCnt][0] == '\0')
        {
            continue;
        }

        dwError = VmDirAllocateMemory(sizeof(*pTmpRA), (PVOID*)&pTmpRA);
        BAIL_ON_VMDIR_ERROR(dwError);

        pTmpRA->next = pRA;
        pRA          = pTmpRA;
        pTmpRA       = NULL;

        dwError = VmDirStrToNameAndNumber( pStrList->pStringList[dwCnt], '|',
                                            &pRA->pszPartnerName,
                                            &pRA->maxProcessedUSN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppRA = pRA;

cleanup:
    VmDirStringListFree(pStrList);
    return dwError;

error:
    _VmDirFreeReplRA(pRA);
    goto cleanup;
}

/*
 * Search the last MAX_REPL_STATE_USN_SEARCH of entries based on USNChanged.
 * Best effort to derive:
 *  maxConsumable  USN
 */
static
DWORD
_VmDirQueryReplStateUSN(
    LDAP*   pLd,
    PCSTR   pszInvocationId,
    USN     currentUSN,
    USN*    maxConsumableUSN
    )
{
    DWORD   dwError = 0;
    int     i = 0;
    PSTR    pszFilter = NULL;
    PCSTR   pszUSNChanged = ATTR_USN_CHANGED;
    PCSTR   pszMetadata = ATTR_ATTR_META_DATA;
    PCSTR   pszObjectclass = ATTR_OBJECT_CLASS;
    PCSTR   ppszAttrs[] = { pszUSNChanged, pszMetadata, pszObjectclass, NULL };
    struct berval** ppUSNValues = NULL;
    struct berval** ppMetadataValues = NULL;
    struct berval** ppObjectClassValues = NULL;
    USN     consumableUSN = 0;

    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;

    dwError = VmDirAllocateStringAVsnprintf(&pszFilter, "usnchanged>=%u",
                                            VMDIR_MAX( currentUSN-MAX_REPL_STATE_USN_SEARCH, 0));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                pLd,
                "",
                LDAP_SCOPE_SUBTREE,
                pszFilter,
                (PSTR*)ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                MAX_REPL_STATE_USN_SEARCH,     /* size limit */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pLd, pResult);
         pEntry;
         pEntry = ldap_next_entry(pLd, pEntry))
    {
        BOOLEAN bConsumable = TRUE;

        if (ppUSNValues)
        {
            ldap_value_free_len(ppUSNValues);
            ppUSNValues = NULL;
        }
        if (ppMetadataValues)
        {
            ldap_value_free_len(ppMetadataValues);
            ppMetadataValues = NULL;
        }
        if (ppObjectClassValues)
        {
            ldap_value_free_len(ppObjectClassValues);
            ppObjectClassValues = NULL;
        }

        ppUSNValues         = ldap_get_values_len(pLd, pEntry, ATTR_USN_CHANGED);
        ppMetadataValues    = ldap_get_values_len(pLd, pEntry, ATTR_ATTR_META_DATA);
        ppObjectClassValues  = ldap_get_values_len(pLd, pEntry, ATTR_OBJECT_CLASS);

        for (i=0; ppObjectClassValues[i] != NULL; i++)
        {
            // per ldap-head/result.c, we do not send VMWDIRSERVER and VMWREPLICATIONAGREEMENT changes
            // to partner during replication pull request.
            if ( VmDirStringCompareA(ppObjectClassValues[i]->bv_val, OC_DIR_SERVER, FALSE) == 0  ||
                 VmDirStringCompareA(ppObjectClassValues[i]->bv_val, OC_REPLICATION_AGREEMENT, FALSE) == 0
               )
            {
                bConsumable = FALSE;
                break;
            }
        }

        if (bConsumable)
        {
            USN thisUSN = atol( ppUSNValues[0] ? ppUSNValues[0]->bv_val:"0" );
            consumableUSN = VMDIR_MAX(thisUSN, consumableUSN);
        }
    }

    *maxConsumableUSN = consumableUSN;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    if (ppUSNValues)
    {
        ldap_value_free_len(ppUSNValues);
    }
    if (ppMetadataValues)
    {
        ldap_value_free_len(ppMetadataValues);
    }
    if (ppObjectClassValues)
    {
        ldap_value_free_len(ppObjectClassValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;
error:
    goto cleanup;
}

static
DWORD
_VmDirEntryToReplState(
    LDAP*           pLd,
    LDAPMessage*    pEntry,
    PVMDIR_REPL_STATE  pReplState
    )
{
    DWORD   dwError = 0;
    int     iCnt = 0;
    struct  berval** ppValues = NULL;

    ppValues = ldap_get_values_len(
                    pLd,
                    pEntry,
                    ATTR_SERVER_RUNTIME_STATUS);

    for (iCnt=0; iCnt < ldap_count_values_len(ppValues); iCnt++)
    {
        if (VmDirStringNCompareA(ppValues[iCnt]->bv_val, REPL_STATUS_SERVER_NAME,
                                 REPL_STATUS_SERVER_NAME_LEN, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(ppValues[iCnt]->bv_val+REPL_STATUS_SERVER_NAME_LEN, &pReplState->pszHost);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringNCompareA(ppValues[iCnt]->bv_val, REPL_STATUS_VISIBLE_USN,
                                      REPL_STATUS_VISIBLE_USN_LEN, FALSE) == 0)
        {
            pReplState->maxVisibleUSN = atoi(ppValues[iCnt]->bv_val+REPL_STATUS_VISIBLE_USN_LEN);
        }
        else if (VmDirStringNCompareA(ppValues[iCnt]->bv_val, REPL_STATUS_CYCLE_COUNT,
                                      REPL_STATUS_CYCLE_COUNT_LEN, FALSE) == 0)
        {
            pReplState->dwCycleCount = atoi(ppValues[iCnt]->bv_val+REPL_STATUS_CYCLE_COUNT_LEN);
        }
        else if (VmDirStringNCompareA(ppValues[iCnt]->bv_val, REPL_STATUS_INVOCATION_ID,
                                      REPL_STATUS_INVOCATION_ID_LEN, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(ppValues[iCnt]->bv_val+REPL_STATUS_INVOCATION_ID_LEN, &pReplState->pszInvocationId);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringNCompareA(ppValues[iCnt]->bv_val, REPL_STATUS_UTDVECTOR,
                                      REPL_STATUS_UTDVECTOR_LEN, FALSE) == 0)
        {
            dwError = VmDirUTDVectorToStruct(ppValues[iCnt]->bv_val+REPL_STATUS_UTDVECTOR_LEN,
                                              &pReplState->pReplUTDVec);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringNCompareA(ppValues[iCnt]->bv_val, REPL_STATUS_PROCESSED_USN_VECTOR,
                                      REPL_STATUS_PROCESSED_USN_VECTOR_LEN, FALSE) == 0)
        {
            dwError = _VmDirRAsToStruct( ppValues[iCnt]->bv_val+REPL_STATUS_PROCESSED_USN_VECTOR_LEN,
                                        &pReplState->pReplRA);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringNCompareA(ppValues[iCnt]->bv_val,
                                      REPL_STATUS_ORIGINATING_USN,
                                      REPL_STATUS_ORIGINATING_USN_LEN,
                                      FALSE) == 0)
        {
            pReplState->maxOriginatingUSN =
                atoi(ppValues[iCnt]->bv_val+REPL_STATUS_ORIGINATING_USN_LEN);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (pReplState->maxVisibleUSN > 0)
    {
        dwError = _VmDirQueryReplStateUSN( pLd,
                                           pReplState->pszInvocationId,
                                           pReplState->maxVisibleUSN,
                                           &pReplState->maxConsumableUSN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    return dwError;

error:
    goto cleanup;
}

static
VOID
_VmDirFreeReplRA(
    PVMDIR_REPL_REPL_AGREEMENT  pRA
    )
{
    while (pRA)
    {
        PVMDIR_REPL_REPL_AGREEMENT pNext = pRA->next;

        VMDIR_SAFE_FREE_MEMORY(pRA->pszPartnerName);
        VMDIR_SAFE_FREE_MEMORY(pRA);
        pRA = pNext;
    }

    return;
}

