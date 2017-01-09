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



/*
 * Module Name: vdcrepadmin
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcrepadmin main module entry point
 *
 */

#include "includes.h"

static
DWORD
_VmDirGetAttributeMetadata(
    PCSTR pszHostName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszEntryDn,
    PCSTR pszAttribute
    );

static
VOID
_VmDirPrintAttributeMetadata(
    PVMDIR_METADATA pAttrMetadata
    );


static
DWORD
_VmDirGetConnection(
    PCSTR   pszHostName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PVMDIR_CONNECTION* ppConnection
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = NULL;
    PSTR    pszURI = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    if ( VmDirIsIPV6AddrFormat( pszHostName ) )
    {
        dwError = VmDirAllocateStringPrintf( &pszURI, "ldap://[%s]", pszHostName );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf( &pszURI, "ldap://%s", pszHostName );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName( pszHostName, &pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectionOpen( pszURI, pszDomainName, pszUserName, pszPassword, &pConnection );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszURI);
    return dwError;

error:
    VmDirConnectionClose(pConnection);
    goto cleanup;
}

static
DWORD
_VmDirGetDCList(
    PCSTR   pszHostName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PVMDIR_STRING_LIST*  ppDCList
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszName = NULL;
    PVMDIR_SERVER_INFO  pServerInfo = NULL;
    DWORD               dwServerInfoCount = 0;
    PVMDIR_STRING_LIST  pDCList = NULL;

    dwError = VmDirStringListInitialize(&pDCList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServers(
                pszHostName,
                pszUserName,
                pszPassword,
                &pServerInfo,
                &dwServerInfoCount
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<dwServerInfoCount; dwCnt++)
    {

        VMDIR_SAFE_FREE_MEMORY(pszName);

        dwError = VmDirDnLastRDNToCn(pServerInfo[dwCnt].pszServerDN,
                                    &pszName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pDCList, pszName);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszName = NULL;
    }

    *ppDCList = pDCList;

cleanup:
    for (dwCnt=0; dwCnt<dwServerInfoCount; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pServerInfo[dwCnt].pszServerDN);
    }

    VMDIR_SAFE_FREE_MEMORY(pServerInfo);
    VMDIR_SAFE_FREE_MEMORY(pszName);

    return dwError;

error:
    VmDirStringListFree(pDCList);
    goto cleanup;
}

static
VOID
_VmDirPrintReplStateList(
    PVMDIR_REPL_STATE *pReplStateList,
    DWORD dwListCount)
{
    PVMDIR_REPL_UTDVECTOR       pVector = NULL;
    DWORD dwCount = 0;
    DWORD dwICount = 0;
    USN partnerReplUsn = 0;
    USN partnerOrigUsn = 0;
    USN partnerLocalUsn = 0;
    USN partnerLocalOrigUsn = 0;
    PSTR pszPartnerName = NULL;
    PSTR pszHighestReplUsn = NULL;
    PSTR pszHighestOrigUsn = NULL;
    PSTR pszLag = NULL;
    BOOLEAN bPartnerFound = FALSE;
    BOOLEAN bPartnerRaFound = FALSE;
    PVMDIR_REPL_REPL_AGREEMENT  pRA = NULL;
    DWORD dwError = 0;

    for (dwCount = 0; dwCount < dwListCount; dwCount++)
    {

        printf("\nDomain Controller: %s\n",VDIR_SAFE_STRING(pReplStateList[dwCount]->pszHost));
        printf("  Invocation ID: ......... %s\n",VDIR_SAFE_STRING(pReplStateList[dwCount]->pszInvocationId));
        printf("  Replication Cycles: .... %d\n",pReplStateList[dwCount]->dwCycleCount);
        printf("  Highest Replicable  USN: %lu\n",pReplStateList[dwCount]->maxConsumableUSN);
        printf("  Highest Originating USN: %lu\n",pReplStateList[dwCount]->maxOriginatingUSN);


        pVector = pReplStateList[dwCount]->pReplUTDVec;

        if (pVector)
        {
            printf("\n"
                   "  Replication Status:                             Highest      Highest\n"
                   "                                               Replicated  Originating   Trailing\n"
                   "  Domain Controller Name/ID                           USN          USN     Behind\n"
                   "  ------------------------------------------   ----------   ----------   --------\n");
        }

        // Add a line for each entry in server's UTD vector
        while (pVector)
        {

            // reset locals

            VMDIR_SAFE_FREE_MEMORY(pszHighestReplUsn);
            VMDIR_SAFE_FREE_MEMORY(pszHighestOrigUsn);
            VMDIR_SAFE_FREE_MEMORY(pszLag);

            pszPartnerName = NULL;
            partnerLocalUsn = 0;
            partnerReplUsn = 0;
            partnerLocalOrigUsn = 0;
            bPartnerFound = FALSE;
            bPartnerRaFound = FALSE;
            pszHighestReplUsn = NULL;
            pszHighestOrigUsn = NULL;
            pszLag = NULL;

            // Look through ReplStateList for partner info to compare with
            for (dwICount = 0; dwICount < dwListCount; dwICount++)
            {
                if (VmDirStringCompareA(pReplStateList[dwICount]->pszInvocationId,
                                        pVector->pszPartnerInvocationId,
                                        FALSE) == 0)
                {
                    bPartnerFound = TRUE;
                    pszPartnerName = pReplStateList[dwICount]->pszHost;
                    partnerLocalUsn = pReplStateList[dwICount]->maxConsumableUSN;
                    partnerLocalOrigUsn = pReplStateList[dwICount]->maxOriginatingUSN;
                    break;
                }

            }

            if (!pszPartnerName)
            {
                pszPartnerName = pVector->pszPartnerInvocationId;
            }
            else
            {
                pRA = pReplStateList[dwCount]->pReplRA;
                while (pRA)
                {
                    if (pRA->pszPartnerName && VmDirStringCompareA(pRA->pszPartnerName,
                                                                   pszPartnerName,
                                                                   FALSE) == 0)
                    {
                        partnerReplUsn = pRA->maxProcessedUSN;
                        bPartnerRaFound = TRUE;
                        break;
                    }
                    pRA = pRA->next;
                }
            }

            partnerOrigUsn = pVector->maxOriginatingUSN;

            dwError = VmDirAllocateStringPrintf(&pszHighestOrigUsn, "%10lu", partnerOrigUsn);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (bPartnerFound)
            {
                if (!bPartnerRaFound)
                {

                    dwError = VmDirAllocateStringPrintf(
                                                &pszHighestReplUsn,
                                                "-"
                                                );
                    BAIL_ON_VMDIR_ERROR(dwError);

                    if (partnerLocalOrigUsn == 0)
                    {
                        dwError = VmDirAllocateStringPrintf(
                                                &pszLag,
                                                "N/A"
                                                );

                    }
                    else
                    {

                        dwError = VmDirAllocateStringPrintf(
                                                &pszLag,
                                                "%ld",
                                                partnerLocalOrigUsn - partnerOrigUsn
                                                );
                    }
                    BAIL_ON_VMDIR_ERROR(dwError);

                }
                else
                {
                    dwError = VmDirAllocateStringPrintf(
                                                &pszHighestReplUsn,
                                                "%lu",
                                                partnerReplUsn
                                                );
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringPrintf(
                                                &pszLag,
                                                "%ld",
                                                partnerLocalUsn - partnerReplUsn
                                                );
                    BAIL_ON_VMDIR_ERROR(dwError);

                }
            }
            else
            {
                    dwError = VmDirAllocateStringPrintf(
                                                &pszHighestReplUsn,
                                                "-"
                                                );
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirAllocateStringPrintf(
                                                &pszLag,
                                                "-"
                                                );
                    BAIL_ON_VMDIR_ERROR(dwError);
            }

            printf("  %-42s   %10s   %10s   %8s\n",
                   pszPartnerName,
                   pszHighestReplUsn,
                   pszHighestOrigUsn,
                   pszLag);

            pVector = pVector->next;
        }


        printf("\n\n");

    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszHighestReplUsn);
    VMDIR_SAFE_FREE_MEMORY(pszHighestOrigUsn);
    VMDIR_SAFE_FREE_MEMORY(pszLag);

    return;

error:
    goto cleanup;
}

static
VOID
_VmDirPrintAttributeMetadata(
    PVMDIR_METADATA pAttrMetadata
    )
{
    if (pAttrMetadata)
    {
        printf("\tAttribute: %s\n", VDIR_SAFE_STRING(pAttrMetadata->pszAttribute));
        printf("\tLocal USN: %lu\n", pAttrMetadata->localUsn);
        printf("\tVersion: %u\n", pAttrMetadata->dwVersion);
        printf("\tOriginating Id: %s\n", VDIR_SAFE_STRING(pAttrMetadata->pszOriginatingId));
        printf("\tOriginating time: %s\n", VDIR_SAFE_STRING(pAttrMetadata->pszOriginatingTime));
        printf("\tOriginating USN: %lu\n", pAttrMetadata->originatingUsn);
    }
    else
    {
        printf("\tAttribute metadata NOT found\n");
    }

    printf("\n");
}

static
DWORD
VmDirGetFederationStatus(
    PCSTR   pszHostName,
    PCSTR   pszUserName,
    PCSTR   pszPassword
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_STRING_LIST pDCList = NULL;
    PVMDIR_CONNECTION  pConnection = NULL;
    PVMDIR_REPL_STATE  pReplState = NULL;
    PVMDIR_REPL_STATE  *ppReplStateList = NULL;
    DWORD dwListCount = 0;
    PSTR pszDomainName = NULL;
    PSTR pszUPN = NULL;

    dwError = _VmDirGetDCList(
                pszHostName,
                pszUserName,
                pszPassword,
                &pDCList);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pDCList->dwCount == 0)
    {
        goto cleanup;
    }

    dwError = VmDirAllocateMemory(sizeof(PVMDIR_REPL_STATE)*pDCList->dwCount,
                                  (PVOID *)&ppReplStateList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<pDCList->dwCount; dwCnt++)
    {

        VmDirConnectionClose( pConnection );
        pConnection = NULL;

        dwError = _VmDirGetConnection(
                        pDCList->pStringList[dwCnt],
                        pszUserName,
                        pszPassword,
                        &pConnection);
        if (dwError == VMDIR_ERROR_SERVER_DOWN)
        {
            printf("Domain Controller: %s is NOT available\n\n", pDCList->pStringList[dwCnt]);
            dwError = 0;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        pReplState = NULL;
        dwError = VmDirGetReplicationState(pConnection, &pReplState);
        if (dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
        {
            printf("Domain Controller: %s is NOT supported\n\n", pDCList->pStringList[dwCnt]);
            VmDirFreeReplicationState(pReplState);
            dwError = 0;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        ppReplStateList[dwListCount++] = pReplState;
    }

    _VmDirPrintReplStateList(ppReplStateList, dwListCount);

cleanup:
    for (dwCnt = 0; dwCnt < dwListCount; dwCnt++)
    {
        VmDirFreeReplicationState(ppReplStateList[dwCnt]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppReplStateList);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VmDirStringListFree(pDCList);
    VmDirConnectionClose(pConnection);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirGetReplicateStatusCycle(
    PCSTR   pszHostName,
    PCSTR   pszUserName,
    PCSTR   pszPassword
    )
{
    DWORD   dwError = 0;
    DWORD   dwCycleCount = 0;
    PVMDIR_CONNECTION pConnection = NULL;

    dwError = _VmDirGetConnection( pszHostName,
                                   pszUserName,
                                   pszPassword,
                                   &pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetReplicationCycleCount( pConnection, &dwCycleCount );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwCycleCount == 0)
    {
        printf("First replication cycle done: FALSE\n");
    }
    else
    {
        printf("First replication cycle done: TRUE\n");
    }

cleanup:
    VmDirConnectionClose( pConnection );

    return dwError;

error:
    printf("First replication cycle done: UNKNOWN, error code (%u)\n", dwError);
    goto cleanup;
}

DWORD
VmDirShowReplicationPartnerInfo(
    DWORD               dwNumPartner,
    PVMDIR_REPL_PARTNER_INFO   pReplPartnerInfo
)
{
    DWORD dwError = 0;
    DWORD i = 0;
    for (i=0; i<dwNumPartner; i++)
    {
        printf("%s\n", pReplPartnerInfo[i].pszURI);
    }
    return dwError;
}

DWORD
VmDirShowReplicationPartnerStatus(
    DWORD               dwNumPartner,
    PVMDIR_REPL_PARTNER_STATUS   pReplPartnerStatus
)
{
    DWORD dwError = 0;
    DWORD i = 0;
    USN lag = 0;
    for (i=0; i<dwNumPartner; i++)
    {
        printf("Partner: %s\n", pReplPartnerStatus[i].pszHost);
        printf("Host available:   %s\n", pReplPartnerStatus[i].bHostAvailable ? "Yes" : "No");
        if (pReplPartnerStatus[i].bHostAvailable)
        {
            printf("Status available: %s\n", pReplPartnerStatus[i].bStatusAvailable ? "Yes" : "No");
            if (pReplPartnerStatus[i].bStatusAvailable)
            {
                lag = pReplPartnerStatus[i].targetUsn - pReplPartnerStatus[i].partnerUsn;
#ifdef _WIN32
                printf("My last change number:             %I64d\n", (_Longlong)pReplPartnerStatus[i].targetUsn);
                printf("Partner has seen my change number: %I64d\n", (_Longlong)pReplPartnerStatus[i].partnerUsn);
                printf("Partner is %I64d changes behind.\n", (_Longlong)lag);
#else
                printf("My last change number:             %" PRId64 "\n", (int64_t)pReplPartnerStatus[i].targetUsn);
                printf("Partner has seen my change number: %" PRId64 "\n", (int64_t)pReplPartnerStatus[i].partnerUsn);
                printf("Partner is %" PRId64 " changes behind.\n", (int64_t)lag);
#endif
            }
        }
        if (i < dwNumPartner - 1)
        {
            printf("\n");
        }
    }
    return dwError;
}

DWORD
VmDirShowServerInfo(
    DWORD               dwNumServer,
    PVMDIR_SERVER_INFO   pServerInfo
)
{
    DWORD dwError = 0;
    DWORD i = 0;
    for (i=0; i<dwNumServer; i++)
    {
        printf("%s\n", pServerInfo[i].pszServerDN);
    }
    return dwError;
}

static
DWORD
_VdcLdapReplaceAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    )
{
    DWORD    dwError = 0;
    LDAPMod  addReplace;
    LDAPMod *mods[2];

    addReplace.mod_op     = LDAP_MOD_REPLACE;
    addReplace.mod_type   = (PSTR) pszAttribute;
    addReplace.mod_values = (PSTR*) ppszAttributeValues;

    mods[0] = &addReplace;
    mods[1] = NULL;

    dwError = ldap_modify_ext_s(pLd, pszDN, mods, NULL, NULL);

    return dwError;
}

static
DWORD
_VdcLdapGetAttributeValue(
    LDAP *pLd,
    PCSTR pszDCDN,
    PCSTR pszAttribute,
    BOOL  bOptional,
    PSTR *ppszAttrVal
    )
{
    DWORD dwError = 0;
    PCSTR ppszAttrs[2] = {pszAttribute, NULL};
    LDAPMessage *pResult = NULL;
    PSTR  pszAttrVal = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszDCDN,
                LDAP_SCOPE_BASE,
                NULL,
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

        for (; pEntry != NULL;
             pEntry = ldap_next_entry(pLd,pEntry))
        {
            dwError = VmDirGetSingleAttributeFromEntry(pLd,
                                                       pEntry,
                                                       (PSTR)pszAttribute,
                                                       bOptional,
                                                       &pszAttrVal);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppszAttrVal = pszAttrVal;

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;
error:
    VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
    goto cleanup;
}

/**
 * This is an innocuous domain modification to trigger a USN change at all
 * nodes in a domain. The write will be to the 'comment' attribute for each
 * DCAccountDN listed in the given FQDN's 'Domain Controllers' entry for the
 * domain as gleened from the user UPN. The value, or lack thereof, for the
 * attribute will be restored after the write.
 *
 * @param pszHostName The FQDN of node to be written to. This will be used
 *                    to create the DCAccountDN in form of:
 *                    cn=<FQDN>,ou=Domain Controllers,dc=vsphere,dc=local
 * @param pszUserName The user UPN in which to validate.
 * @param pszPassword The password for the given user.
 * @return 0 if successful, else a non-zero error code.
 */
DWORD
_VmDirDummyDomainWrite(
    PCSTR   pszHostName,
    PCSTR   pszUserName,
    PCSTR   pszPassword
)
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = NULL;
    PSTR    pszDomainDN = NULL;
    PSTR    pszServerName = NULL;
    PSTR    pszName = NULL;
    PSTR    pszAttrVal = NULL;
    LDAP*   pLd = NULL;
    PSTR    ppszVals [] = { "foobar", NULL };
    PVMDIR_STRING_LIST pDCList = NULL;
    DWORD   dwCnt = 0;

    if( !pszPassword|| !pszHostName || !pszUserName ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError =  VmDirUPNToNameAndDomain(pszUserName, &pszName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(&pLd, pszHostName, pszUserName, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDCDNList(
                pLd,
                pszDomainDN,
                &pDCList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<pDCList->dwCount; dwCnt++)
    {
        if (pLd)
        {
            ldap_unbind_ext_s(pLd, NULL, NULL);
        }

        VMDIR_SAFE_FREE_MEMORY(pszServerName);

        dwError = VmDirDnLastRDNToCn(pDCList->pStringList[dwCnt], &pszServerName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSafeLDAPBind(&pLd,
                                    pszServerName,
                                    pszUserName,
                                    pszPassword);

        if (dwError)
        {
            printf("Domain Controller: %s is NOT available. Error [%d]\n\n",
                   pszServerName,
                   dwError);
            pLd = NULL;
            dwError = 0;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        /* Get current value of attribute to write back */
        dwError = _VdcLdapGetAttributeValue(
                        pLd,
                        pDCList->pStringList[dwCnt],
                        ATTR_COMMENT,
                        TRUE,
                        &pszAttrVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        /* write dummy value to attribute of the DC */
        dwError = _VdcLdapReplaceAttributeValues(
                        pLd,
                        pDCList->pStringList[dwCnt],
                        ATTR_COMMENT,
                        (PCSTR*)ppszVals);
        BAIL_ON_VMDIR_ERROR(dwError);

        // restore previous value of attribute
        ppszVals[0] = pszAttrVal;

        dwError = _VdcLdapReplaceAttributeValues( pLd,
                                                  pDCList->pStringList[dwCnt],
                                                  ATTR_COMMENT,
                                                  (PCSTR*) ppszVals);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirStringListFree(pDCList);
    VMDIR_SAFE_FREE_STRINGA(pszAttrVal);
    VMDIR_SAFE_FREE_STRINGA(pszName);
    VMDIR_SAFE_FREE_STRINGA(pszServerName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainDN);

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
_VmDirGetAttributeMetadata(
    PCSTR pszHostName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszEntryDn,
    PCSTR pszAttribute
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    DWORD dwAttrs = 0;
    PVMDIR_STRING_LIST pDCList = NULL;
    PVMDIR_CONNECTION  pConnection = NULL;
    PVMDIR_METADATA_LIST pMetadataList = NULL;

    dwError = _VmDirGetDCList(
                pszHostName,
                pszUserName,
                pszPassword,
                &pDCList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<pDCList->dwCount; dwCnt++)
    {
        // VOID function, no return value to check
        VmDirConnectionClose(pConnection);

        pConnection = NULL;

        dwError = _VmDirGetConnection(
                        pDCList->pStringList[dwCnt],
                        pszUserName,
                        pszPassword,
                        &pConnection);

        if (dwError == VMDIR_ERROR_SERVER_DOWN)
        {
            printf("Domain Controller: %s is NOT available\n\n", VDIR_SAFE_STRING(pDCList->pStringList[dwCnt]));

            dwError = 0;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("Domain Controller: %s\n", VDIR_SAFE_STRING(pDCList->pStringList[dwCnt]));

        // VOID function, no return value to check
        VmDirFreeMetadataList(pMetadataList);

        pMetadataList = NULL;

        dwError = VmDirGetAttributeMetadata(
                        pConnection,
                        pszEntryDn,
                        pszAttribute,
                        &pMetadataList);

        if (dwError == LDAP_NO_SUCH_OBJECT)
        {
            printf("\tEntry NOT found\n\n");
            dwError = 0;
            continue;

        }

        if (dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE)
        {
            printf("\tAttribute NOT found\n\n");
            dwError = 0;
            continue;

        }
        BAIL_ON_VMDIR_ERROR(dwError);

        for (dwAttrs = 0; dwAttrs < pMetadataList->dwCount; dwAttrs++)
        {
            _VmDirPrintAttributeMetadata(pMetadataList->ppMetadataArray[dwAttrs]);
        }
    }

cleanup:
    VmDirStringListFree(pDCList);
    VmDirConnectionClose(pConnection);
    VmDirFreeMetadataList(pMetadataList);
    return dwError;

error:
    goto cleanup;
}

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD       dwError = 0;
    DWORD       i       = 0;
    PSTR        pszFeatureSet  = NULL;
    PSTR        pszSrcHostName = NULL;
    PSTR        pszSrcPort     = NULL;
    PSTR        pszSrcUserName = NULL;
    PSTR        pszSrcPassword = NULL;
    PSTR        pszTgtHostName = NULL;
    PSTR        pszTgtPort     = NULL;
    PSTR        pszEntryDn     = NULL;
    PSTR        pszAttribute   = NULL;
    BOOLEAN     bVerbose       = FALSE;
    BOOLEAN     bTwoWayRepl    = FALSE;
    PSTR        pszErrMsg      = NULL;
    CHAR        pszPasswordBuf[VMDIR_MAX_PWD_LEN + 1] = {0};
    PVMDIR_REPL_PARTNER_INFO    pReplPartnerInfo       = NULL;
    PVMDIR_REPL_PARTNER_STATUS  pReplPartnerStatus     = NULL;
    PVMDIR_SERVER_INFO          pServerInfo            = NULL;
    DWORD                       dwReplPartnerInfoCount = 0;
    DWORD                       dwReplPartnerStatusCount = 0;
    DWORD                       dwServerInfoCount      = 0;

    CHAR        pszPath[MAX_PATH];

#ifndef _WIN32
    setlocale(LC_ALL,"");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath,
                                   "vdcrepadmin.log");
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirLogInitialize(
                pszPath,
                FALSE,
                NULL,
                VMDIR_LOG_INFO,
                VMDIR_LOG_MASK_ALL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //get commandline parameters
    dwError = VmDirParseArgs(
                    argc,
                    argv,
                    &pszFeatureSet,
                    &bTwoWayRepl,
                    &pszSrcHostName,
                    &pszSrcPort,
                    &pszSrcUserName,
                    &pszSrcPassword,
                    &pszTgtHostName,
                    &pszTgtPort,
                    &pszEntryDn,
                    &pszAttribute,
                    &bVerbose
                    );

    if (bVerbose)
    {
        VmDirSetLogLevel( "VERBOSE" );
    }

    if (dwError)
    {
        ShowUsage();
        goto cleanup;
    }

    if (pszSrcPassword == NULL)
    {
        // read password from stdin
        VmDirReadString(
            "password: ",
            pszPasswordBuf,
            sizeof(pszPasswordBuf),
            TRUE);
        pszSrcPassword = pszPasswordBuf;
    }

    if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_PARTNERS,
                             pszFeatureSet,
                             TRUE) == 0 )
    {
        dwError = VmDirGetReplicationPartners(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pReplPartnerInfo,
                        &dwReplPartnerInfoCount
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        //Show replication partner info
        dwError = VmDirShowReplicationPartnerInfo(
                                dwReplPartnerInfoCount,
                                pReplPartnerInfo
                                );

        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_PARTNER_STATUS,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        dwError = VmDirGetReplicationPartnerStatus(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pReplPartnerStatus,
                        &dwReplPartnerStatusCount
                        );

        BAIL_ON_VMDIR_ERROR(dwError);

        //Show replication partner info
        dwError = VmDirShowReplicationPartnerStatus(
                                dwReplPartnerStatusCount,
                                pReplPartnerStatus
                                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_FEDERATION_STATUS,
                                  pszFeatureSet,
                                  TRUE) == 0 )
       {
           dwError = VmDirGetFederationStatus(
                           pszSrcHostName,
                           pszSrcUserName,
                           pszSrcPassword
                           );
           BAIL_ON_VMDIR_ERROR(dwError);

       }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_SERVER_ATTRIBUTE,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        dwError = VmDirGetServers(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pServerInfo,
                        &dwServerInfoCount
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        //Show replication partner info
        dwError = VmDirShowServerInfo(
                                dwServerInfoCount,
                                pServerInfo
                                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_CREATE_AGREEMENT,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        dwError = VmDirAddReplicationAgreement(
                    bTwoWayRepl,
                    pszSrcHostName,
                    pszSrcPort,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszTgtHostName,
                    pszTgtPort
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_REMOVE_AGREEMENT,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        dwError = VmDirRemoveReplicationAgreement(
                    bTwoWayRepl,
                    pszSrcHostName,
                    pszSrcPort,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszTgtHostName,
                    pszTgtPort
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_QUERY_IS_FIRST_CYCLE_DONE,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        dwError = _VmDirGetReplicateStatusCycle(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_DUMMY_DOMAIN_WRITE,
                                  pszFeatureSet,
                                  TRUE) == 0 )
    {
        dwError = _VmDirDummyDomainWrite(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_ATTRIBUTE_METADATA,
                                  pszFeatureSet,
                                  TRUE) == 0 )
       {
           dwError = _VmDirGetAttributeMetadata(
                                pszSrcHostName,
                                pszSrcUserName,
                                pszSrcPassword,
                                pszEntryDn,
                                pszAttribute
                                );
           BAIL_ON_VMDIR_ERROR(dwError);

       }

cleanup:
    // Free internal memory used
    for (i=0; i<dwReplPartnerInfoCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo[i].pszURI);
    }
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo);

    for (i=0; i<dwReplPartnerStatusCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerStatus[i].pszHost);
    }
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerStatus);
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);

    // Free internal memory used
    for (i=0; i<dwServerInfoCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pServerInfo[i].pszServerDN);
    }

    VMDIR_SAFE_FREE_MEMORY(pServerInfo);
    memset(pszPasswordBuf, 0, sizeof(pszPasswordBuf));

    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrMsg); // ignore error
    printf("Vdcrepadmin failed. Error [%s] [%d]\n",
           VDIR_SAFE_STRING(pszErrMsg),
           dwError);
    goto cleanup;
}

#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwError = 0;
    PSTR* ppszArgs = NULL;
    int   iArg = 0;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmDirAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMain(argc, ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }
        VmDirFreeMemory(ppszArgs);
    }

    return dwError;
}
#else

int main(int argc, char* argv[])
{
    return VmDirMain(argc, argv);
}

#endif
