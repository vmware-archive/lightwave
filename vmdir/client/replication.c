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
     * Given labeledUri=ldaps://server:636,cn=Relplication Agreements,cn=foo,...
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

    dwError = VmDirGetAllRAToHost( pLD, pszHost, &ppszRADNs, &dwSize );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        VMDIR_SAFE_FREE_STRINGA(pszUtdVector);
        VMDIR_SAFE_FREE_STRINGA(pszInvocationId);

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
    PCSTR pszDomain,
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassowrd,
    PBOOLEAN pbUpToDate
    )
{
    DWORD dwError = 0;
    BOOLEAN bEntriesExist = FALSE;
    PVMDIR_REPLICATION_COOKIE pCookie = NULL;

    if (pLD == NULL || pszDomain == NULL || pszServerName == NULL ||
        pszUserName == NULL || pszPassowrd == NULL || pbUpToDate == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirGetReplicationCookieForHost(pLD, pszServerName, &pCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirReplicationEntriesExist(pszDomain, pszServerName, pszUserName, pszPassowrd, pCookie, &bEntriesExist);
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

    dwError = _VmDirGetReplicationCookieForHost(pLD, pszAccount, &pCookie);
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

