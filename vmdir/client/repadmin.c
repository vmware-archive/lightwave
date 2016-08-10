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
BOOLEAN
_VmDirIsClassReplicable(
    struct berval** ppObjectClassValues
    );

static
DWORD
_VmDirIsHostAPartner(
    LDAP *pLd,
    PCSTR pszHostDn,
    PCSTR pszDCAccount,
    PBOOLEAN pIsParter,
    PSTR *ppszPartnerRaDn
    );

static
DWORD
VmDirGetServersInfoOnSite(
    LDAP* pLd,
    PCSTR pszSiteName,
    PCSTR pszHost,
    PCSTR pszDomain,
    PINTERNAL_SERVER_INFO* ppInternalServerInfo,
    DWORD* pdwInfoCount
    );

/*
 * Get all vmdir server info from pLd
 */

DWORD
VmDirGetServersInfo(
    LDAP* pLd,
    PCSTR pszHost,
    PCSTR pszDomain,
    PINTERNAL_SERVER_INFO* ppInternalServerInfo,
    DWORD* pdwInfoCount
    )
{
    return VmDirGetServersInfoOnSite(pLd, NULL, pszHost, pszDomain, ppInternalServerInfo, pdwInfoCount);
}

/*
 *  Bind to a host with the handle to be used later
 */
DWORD VmDirCreateLdAtHostViaMachineAccount(
    PCSTR  pszHostName,
    LDAP** ppLd
)
{
    DWORD dwError = 0;
    PSTR pszDCAccount = NULL;
    PSTR pszDCAccountPassword = NULL;
    PSTR pszServerName = NULL;
    PSTR pszDomain = NULL;
    char bufUPN[VMDIR_MAX_UPN_LEN] = {0};
    LDAP* pLd = NULL;

    dwError = VmDirRegReadDCAccount( &pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword( &pszDCAccountPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName( pszServerName, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s", pszDCAccount, pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLd, pszServerName, bufUPN, pszDCAccountPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    VMDIR_SECURE_FREE_STRINGA(pszDCAccountPassword);
    VMDIR_SAFE_FREE_STRINGA(pszServerName);
    VMDIR_SAFE_FREE_STRINGA(pszDomain);
    return dwError;

error:
    goto cleanup;
}

/*
 * Query the host (pszHostName) for servers topology, and
 * follow those servers (partners) to get the highest USN
 */
DWORD
VmDirGetUsnFromPartners(
    PCSTR pszHostName,
    USN   *pUsn
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszDomain = NULL;
    PINTERNAL_SERVER_INFO pInternalServerInfo = NULL;
    DWORD i = 0;
    DWORD dwInfoCount = 0;
    LDAP* pLd = NULL;
    LDAP* pPartnerLd = NULL;
    BOOLEAN isPartner = FALSE;
    PSTR pszDCAccount = NULL;
    PSTR pPartnerHost = NULL;
    USN usn = {0};
    USN highestUsn = {0};
    PSTR pPartnerRaDn = NULL;

    //Get all vmdir servers in the forest.
    dwError = VmDirCreateLdAtHostViaMachineAccount(pszHostName, &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName( pszServerName, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServersInfoOnSite( pLd, NULL,  pszServerName, pszDomain, &pInternalServerInfo, &dwInfoCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<dwInfoCount; i++)
    {
        VMDIR_SAFE_FREE_STRINGA(pPartnerRaDn);
        dwError = _VmDirIsHostAPartner(pLd, pInternalServerInfo[i].pszServerDN,
                                      pszDCAccount, &isPartner, &pPartnerRaDn);
        if (dwError !=0)
        {
            //Ignore this host as if it is not a partner, and try the next server
            continue;
        }

        if (isPartner)
        {
            VMDIR_SAFE_FREE_STRINGA(pPartnerHost);
            dwError = VmDirDnLastRDNToCn(pInternalServerInfo[i].pszServerDN, &pPartnerHost);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pPartnerLd)
            {
                ldap_unbind_ext_s(pPartnerLd, NULL, NULL);
                pPartnerLd = NULL;
            }
            //Bind to the partner
            dwError = VmDirCreateLdAtHostViaMachineAccount(pPartnerHost, &pPartnerLd);
            if (dwError != 0)
            {
                //Cannot connect/bind to the partner - treat is as non-partner (i.e. best-effort approach)
                dwError = 0;
                continue;
            }
            //Get LastLocalUsnProcessed
            dwError = VmDirGetLastLocalUsnProcessedForHostFromRADN(pPartnerLd,
                              pPartnerRaDn, &usn);
            if (dwError != 0)
            {
                dwError = 0;
                continue;
            }
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirGetUsnFromPartners: USN from partner %s: %lu", pPartnerHost, usn);
            if (usn > highestUsn)
            {
                highestUsn = usn;
            }
        }
    }
    if (highestUsn == 0)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *pUsn = highestUsn;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszServerName);
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    VMDIR_SAFE_FREE_STRINGA(pszDomain);
    VMDIR_SAFE_FREE_STRINGA(pPartnerHost);
    VMDIR_SAFE_FREE_STRINGA(pPartnerRaDn);
    VMDIR_SAFE_FREE_MEMORY(pInternalServerInfo);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
        pLd = NULL;
    }
    if (pPartnerLd)
    {
        ldap_unbind_ext_s(pPartnerLd, NULL, NULL);
        pPartnerLd = NULL;
    }
    return dwError;
error:
    goto cleanup;
}

VOID
VmDirFreeMetadataInternal(
    PVMDIR_METADATA pMetadata
    )
{
    if (pMetadata)
    {
        VMDIR_SAFE_FREE_STRINGA(pMetadata->pszAttribute);
        VMDIR_SAFE_FREE_STRINGA(pMetadata->pszOriginatingId);
        VMDIR_SAFE_FREE_STRINGA(pMetadata->pszOriginatingTime);
        VMDIR_SAFE_FREE_MEMORY(pMetadata);
    }
}

VOID
VmDirFreeMetadataListInternal(
    PVMDIR_METADATA_LIST pMetadataList
    )
{
    DWORD dwCnt = 0;

    if(!pMetadataList)
    {
        return;
    }

    for(dwCnt = 0; dwCnt < pMetadataList->dwCount; dwCnt++)
    {
        VmDirFreeMetadataInternal(pMetadataList->ppMetadataArray[dwCnt]);
    }

    VMDIR_SAFE_FREE_MEMORY(pMetadataList->ppMetadataArray);
    VMDIR_SAFE_FREE_MEMORY(pMetadataList);
}

/*
 * Get the attribute metadata for the given EntryDn.
 * If an attribute is specified, only return metadata for that attribute.
 * Returns a metadata list, or NULL if attribute(s) or entry is not found.
 */
DWORD
VmDirGetAttributeMetadataInternal(
    PVMDIR_CONNECTION   pConnection,
    PCSTR               pszEntryDn,
    PCSTR               pszAttribute, // OPTIONAL
    PVMDIR_METADATA_LIST*    ppMetadataList
    )
{
    DWORD               dwError = 0;
    LDAPMessage*        pResult = NULL;
    PCSTR               pszMetadata = ATTR_ATTR_META_DATA;
    PCSTR               ppszAttrs[] = { pszMetadata, NULL };
    PVMDIR_METADATA     pMetadata = NULL;
    LDAPMessage*        pEntry = NULL;
    struct berval**     ppMetadataValues = NULL;
    int                 iCnt = 0;
    PVMDIR_METADATA_LIST pMetadataList = NULL;
    DWORD               dwMetadataListSize = 0;
    LDAPControl*        pCtrl = NULL;
    LDAPControl*        pServerCtrl[2] = { NULL, NULL };

    if ( IsNullOrEmptyString(pszEntryDn) ||
         !ppMetadataList )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_METADATA_LIST),
                (PVOID*)&pMetadataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Initialize array to reasonable size.
    if (pszAttribute)
    {
        dwMetadataListSize = 2;
    }
    else
    {
        dwMetadataListSize = 10;
    }

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_METADATA) * dwMetadataListSize,
                (PVOID*)&pMetadataList->ppMetadataArray);
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = ldap_control_create(
                        VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS,
                        0,
                        NULL,
                        0,
                        &pCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerCtrl[0] = pCtrl;

    dwError = ldap_search_ext_s(
                pConnection->pLd,
                pszEntryDn,
                LDAP_SCOPE_BASE,
                NULL,
                (PSTR*)ppszAttrs,
                FALSE, /* get values      */
                pServerCtrl,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for ( pEntry = ldap_first_entry(pConnection->pLd, pResult);
          pEntry != NULL;
          pEntry = ldap_next_entry(pConnection->pLd, pEntry) )
    {
        if (ppMetadataValues)
        {
            ldap_value_free_len(ppMetadataValues);
            ppMetadataValues = NULL;
        }

        ppMetadataValues = ldap_get_values_len(
                                pConnection->pLd,
                                pEntry,
                                ATTR_ATTR_META_DATA);

        for (iCnt=0; ppMetadataValues[iCnt] != NULL; iCnt++)
        {
            if (pszAttribute)
            {
                // VOID function, no return value to check
                VmDirFreeMetadata(pMetadata);
            }

            pMetadata = NULL;

            dwError =  VmDirParseMetadata(
                                ppMetadataValues[iCnt]->bv_val,
                                &pMetadata);
            BAIL_ON_VMDIR_ERROR(dwError);

            // Only return metadata if the attribute matches
            if( pszAttribute && VmDirStringCompareA(pszAttribute,
                                                    pMetadata->pszAttribute,
                                                    FALSE) == 0)
            {
                pMetadataList->ppMetadataArray[pMetadataList->dwCount++] = pMetadata;
                *ppMetadataList = pMetadataList;
                goto cleanup;
            }
            else if (!pszAttribute)
            {
                // Is array big enough?
                if (dwMetadataListSize <= pMetadataList->dwCount + 1)
                {
                    dwError = VmDirReallocateMemoryWithInit(
                                (PVOID)pMetadataList->ppMetadataArray,
                                (PVOID*)&pMetadataList->ppMetadataArray,
                                sizeof(VMDIR_METADATA) * (dwMetadataListSize + 10),
                                sizeof(VMDIR_METADATA) * dwMetadataListSize);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwMetadataListSize += 10;
                }

                pMetadataList->ppMetadataArray[pMetadataList->dwCount++] = pMetadata;
            }
        }
    }

    if (pMetadataList->dwCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_SUCH_ATTRIBUTE);
    }

    *ppMetadataList = pMetadataList;

cleanup:
    if (ppMetadataValues)
    {
        ldap_value_free_len(ppMetadataValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    if (pCtrl)
    {
        ldap_control_free(pCtrl);
    }

    return dwError;

error:
    VmDirFreeMetadataList(pMetadataList);
    VmDirFreeMetadata(pMetadata);

    goto cleanup;
}

/*
Format is: <attribute>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
*/
DWORD
VmDirParseMetadata(
    PCSTR  pszMetadata,
    PVMDIR_METADATA *ppMetadata
    )
{
    DWORD               dwError = 0;
    PVMDIR_METADATA     pMetadata = NULL;
    PSTR                pszAttribute = NULL;
    PSTR                pszOriginatingId = NULL;
    PSTR                pszOriginatingTime = NULL;
    PVMDIR_STRING_LIST  pList = NULL;

    if ( ppMetadata == NULL || IsNullOrEmptyString(pszMetadata))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof(VMDIR_METADATA),
                                   (PVOID*)&pMetadata );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToTokenList(pszMetadata, ":", &pList);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Is the string formatted as expected?
    if (pList == NULL || pList->dwCount != METADATA_TOKEN_COUNT)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BAD_ATTRIBUTE_DATA);
    }

    // Attribute name
    dwError = VmDirAllocateStringA(pList->pStringList[0], &pszAttribute);
    BAIL_ON_VMDIR_ERROR(dwError);

    // OriginatingId
    dwError = VmDirAllocateStringA(pList->pStringList[3], &pszOriginatingId);
    BAIL_ON_VMDIR_ERROR(dwError);

    // OriginatingTime
    dwError = VmDirAllocateStringA(pList->pStringList[4], &pszOriginatingTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMetadata->pszAttribute = pszAttribute;
    pMetadata->localUsn = atoi(pList->pStringList[1]);
    pMetadata->dwVersion = atoi(pList->pStringList[2]);
    pMetadata->pszOriginatingId = pszOriginatingId;
    pMetadata->pszOriginatingTime = pszOriginatingTime;
    pMetadata->originatingUsn = atoi(pList->pStringList[5]);

    *ppMetadata = pMetadata;

cleanup:
    VmDirStringListFree(pList);

    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszAttribute);
    VMDIR_SAFE_FREE_STRINGA(pszOriginatingId);
    VMDIR_SAFE_FREE_STRINGA(pszOriginatingTime);
    VmDirFreeMetadata(pMetadata);

    goto cleanup;
}

/*
 * When creating a replication agreement, we want to take into consideration
 * that the node may already be part of the federation and may have seen replicated
 * changes through its other replication partners. Return a reasonable highwater mark
 * to be set in the replication agreement. Use a buffer of 10,000 which is 10 replication
 * pages.
 *
 * Algorithm to find the highwater mark:
 * 1) Find the current USN for partner.
 * 2) Search backwards to find its recent replicable changes.
 * 3) Get the Invocation ID and Originating USN for each change.
 * 4) Check this UTDVector for Originating USN for Invocation ID from above.
 * 5) Compare USNs. If change's USN is less than orginating USN according to the UTDvector,
 *    it was replicated to this node. Use as highwater mark.
 *
 */
DWORD
VmDirLdapGetHighWatermark(
    LDAP*      pLocalLd,
    PCSTR      pszLocalHost,
    PCSTR      pszPartnerHost,
    PCSTR      pszDomainName,
    PCSTR      pszUsername,
    PCSTR      pszPassword,
    USN*       pLastLocalUsn
    )
{
    DWORD   dwError = 0;
    USN     partnerVisibleUSN = 0;
    LDAP    *pPartnerLd = NULL;
    PVMDIR_REPL_STATE pPartnerReplState = NULL;
    PVMDIR_REPL_STATE pLocalReplState = NULL;
    PCSTR   pszUSNChanged = ATTR_USN_CHANGED;
    PCSTR   pszMetadata = ATTR_ATTR_META_DATA;
    PCSTR   pszObjectclass = ATTR_OBJECT_CLASS;
    PCSTR   ppszAttrs[] = { pszUSNChanged, pszMetadata, pszObjectclass, NULL };
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppUSNValues = NULL;
    struct berval** ppMetadataValues = NULL;
    struct berval** ppObjectClassValues = NULL;
    PSTR    pszFilter = NULL;
    int     iCnt = 0;
    USN     consumableUsn = 0;
    PSTR    pszOrigId = NULL;
    USN     origUsn = 0;
    PVMDIR_REPL_UTDVECTOR pUtdVec = NULL;
    PSTR    pszUsnChanged = NULL;
    PVMDIR_METADATA pMetadata = NULL;

    if (pLocalLd == NULL || pLastLocalUsn == NULL || IsNullOrEmptyString(pszPartnerHost) ||
        IsNullOrEmptyString(pszDomainName) || IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword) || IsNullOrEmptyString(pszLocalHost))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Get partner's replication state to discover its latest USN.
    dwError = VmDirConnectLDAPServer(
                        &pPartnerLd,
                        pszPartnerHost,
                        pszDomainName,
                        pszUsername,
                        pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetReplicationStateInternal(pPartnerLd, &pPartnerReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    partnerVisibleUSN = pPartnerReplState->maxVisibleUSN;

    // Get local host's replication state for its UTDVector.
    dwError = VmDirGetReplicationStateInternal(pLocalLd, &pLocalReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    //    Search for replicable changes starting with  partner's lastest USN
    //    and working backwards in steps of 100
    for (; partnerVisibleUSN >= 0; partnerVisibleUSN -= HIGHWATER_USN_STEP)
    {
        VMDIR_SAFE_FREE_MEMORY(pszFilter);

        dwError = VmDirAllocateStringPrintf(
                        &pszFilter, "(&(%s>=%ld)(%s<=%ld))",
                        ATTR_USN_CHANGED,
                        VMDIR_MAX( partnerVisibleUSN - HIGHWATER_USN_STEP + 1, 0 ),
                        ATTR_USN_CHANGED,
                        partnerVisibleUSN
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pResult)
        {
            ldap_msgfree(pResult);
            pResult = NULL;
        }

        dwError = ldap_search_ext_s(
                        pPartnerLd,
                        "",
                        LDAP_SCOPE_SUBTREE,
                        pszFilter,
                        (PSTR*)ppszAttrs,
                        FALSE, /* get values      */
                        NULL,  /* server controls */
                        NULL,  /* client controls */
                        NULL,  /* timeout         */
                        0,     /* size limit */
                        &pResult);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (pEntry = ldap_first_entry(pPartnerLd, pResult);
             pEntry;
             pEntry = ldap_next_entry(pPartnerLd, pEntry))
        {
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

            ppUSNValues         = ldap_get_values_len(pPartnerLd, pEntry, ATTR_USN_CHANGED);
            ppMetadataValues    = ldap_get_values_len(pPartnerLd, pEntry, ATTR_ATTR_META_DATA);
            ppObjectClassValues = ldap_get_values_len(pPartnerLd, pEntry, ATTR_OBJECT_CLASS);

            // Ignore changes that are not replicable
            if (_VmDirIsClassReplicable(ppObjectClassValues))
            {

                consumableUsn = atol( ppUSNValues[0] ? ppUSNValues[0]->bv_val:"0");

                // Get the Originating ID and USN from meta-data.
                for (iCnt=0; ppMetadataValues[iCnt] != NULL; iCnt++)
                {
                    pszUsnChanged = VmDirStringStrA(ppMetadataValues[iCnt]->bv_val,
                                                    ATTR_USN_CHANGED);
                    if ( !IsNullOrEmptyString(pszUsnChanged) )
                    {
                        origUsn = 0;
                        VmDirFreeMetadata(pMetadata);
                        pMetadata = NULL;

                        dwError =  VmDirParseMetadata( pszUsnChanged,
                                                        &pMetadata);
                        BAIL_ON_VMDIR_ERROR(dwError);

                        pszOrigId = pMetadata->pszOriginatingId;
                        origUsn = pMetadata->originatingUsn;

                        // Find originating ID in the utdvector and compare its USN with the change's USN
                        for ( pUtdVec = pLocalReplState->pReplUTDVec;
                              pUtdVec != NULL;
                              pUtdVec = pUtdVec->next)
                        {
                            if (VmDirStringCompareA( pUtdVec->pszPartnerInvocationId,
                                                    pszOrigId, FALSE) == 0)

                            {
                                if (pUtdVec->maxOriginatingUSN >= origUsn)
                                {

                                    *pLastLocalUsn = (DWORD)VMDIR_MAX(consumableUsn - HIGHWATER_USN_REPL_BUFFER, 0);

                                    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                                                    "VmDirLdapGetHighWatermark Host (%s), Partner (%s), Setting high watermark to (%u)",
                                                    pszLocalHost,
                                                    pszPartnerHost,
                                                    *pLastLocalUsn);

                                    goto cleanup;
                                }

                                // Found the originating id we were looking for.
                                break;

                            } // If matches partner ID
                        } // For each entry in UTDvector
                    } // If metadata is for uSNChanged
                } // For each metadata
            } // If entry is Consumable
        } // For each entry
    } // For loop - partner visible USN step backwards

    // Haven't seen any changes of partner, set highwater mark accordingly.
    *pLastLocalUsn = 0;

    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                       "VmDirLdapGetHighWatermark Host (%s), Partner (%s), High watermark not found, setting to (%u)",
                       pszLocalHost,
                       pszPartnerHost,
                       *pLastLocalUsn);

cleanup:
    VMDIR_SAFE_FREE_STRINGA( pszFilter );
    VmDirFreeMetadata( pMetadata );
    VmDirLdapUnbind( &pPartnerLd );
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
    VmDirFreeReplicationStateInternal(pLocalReplState);
    VmDirFreeReplicationStateInternal(pPartnerReplState);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)",
                    __FUNCTION__, dwError);
    goto cleanup;

}

static
BOOLEAN
_VmDirIsClassReplicable(
    struct berval** ppObjectClassValues
    )
{
    int iCnt = 0;

    assert(ppObjectClassValues);

    for (iCnt=0; ppObjectClassValues[iCnt] != NULL; iCnt++)
    {
        if ( VmDirStringCompareA(ppObjectClassValues[iCnt]->bv_val,
                                 OC_DIR_SERVER, FALSE) == 0  ||
             VmDirStringCompareA(ppObjectClassValues[iCnt]->bv_val,
                                 OC_REPLICATION_AGREEMENT, FALSE) == 0
            )
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Provide a site name to get
 * all vmdir server info from pLd.
 */
static
DWORD
VmDirGetServersInfoOnSite(
    LDAP* pLd,
    PCSTR pszSiteName,
    PCSTR pszHost,
    PCSTR pszDomain,
    PINTERNAL_SERVER_INFO* ppInternalServerInfo,
    DWORD* pdwInfoCount
    )
{
    DWORD               dwError = 0;
    PSTR                pszSearchBaseDN = NULL;
    LDAPMessage*        pMessages = NULL;
    LDAPMessage*        pMessage = NULL;
    PINTERNAL_SERVER_INFO   pInternalServerInfo = NULL;
    int                 i = 0;
    DWORD               dwInfoCount = 0;
    PSTR                pszDomainDN = NULL;
    PSTR                pszServerDN = NULL;
    int                 searchLevel = LDAP_SCOPE_ONELEVEL;
    PSTR                pFilter = NULL;

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszSiteName == NULL)
    {
        dwError = VmDirAllocateStringAVsnprintf(
                      &pszSearchBaseDN,
                      "cn=Sites,cn=Configuration,%s",
                      pszDomainDN
                      );
        searchLevel = LDAP_SCOPE_SUBTREE;
    } else
    {
        dwError = VmDirAllocateStringAVsnprintf(
                      &pszSearchBaseDN,
                      "cn=Servers,cn=%s,cn=Sites,cn=Configuration,%s",
                      pszSiteName,
                      pszDomainDN
                      );
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                      &pFilter,
                      "%s=%s",
                      ATTR_OBJECT_CLASS,
                      OC_DIR_SERVER);

    dwError = ldap_search_ext_s(
                             pLd,
                             pszSearchBaseDN,
                             searchLevel,
                             pFilter,  /* filter */
                             NULL,  /* attrs[]*/
                             FALSE,
                             NULL,  /* serverctrls */
                             NULL,  /* clientctrls */
                             NULL,  /* timeout */
                             -1,
                             &pMessages);
    if (dwError != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetServersInfoOnSite: Search from host %s: no dir servers found under %s, error %d",
                        pszHost, pszSearchBaseDN, dwError);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwInfoCount = ldap_count_entries(pLd, pMessages);
    if (dwInfoCount == 0)
    {
        dwError = ERROR_NOT_FOUND;
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Search from host %s: no dir servers found under %s, error %d",
                        pszHost, pszSearchBaseDN, dwError);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                    dwInfoCount * sizeof(INTERNAL_SERVER_INFO),
                    (PVOID*)&pInternalServerInfo
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0, pMessage = ldap_first_entry(pLd, pMessages);
         pMessage != NULL;
         i++, pMessage = ldap_next_entry(pLd, pMessage))
    {
        pszServerDN = ldap_get_dn(pLd, pMessage);
        if (IsNullOrEmptyString(pszServerDN))
        {
            dwError = ERROR_INVALID_DN;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        dwError = VmDirStringCpyA(
                        pInternalServerInfo[i].pszServerDN,
                        VMDIR_MAX_DN_LEN,
                        pszServerDN
                        );
        ldap_memfree(pszServerDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppInternalServerInfo = pInternalServerInfo;
    *pdwInfoCount = dwInfoCount;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pFilter);

    if (pMessages)
    {
        ldap_msgfree(pMessages);
    }
    return dwError;

error:
    VmDirFreeMemory(pInternalServerInfo);
    *ppInternalServerInfo = NULL;
    *pdwInfoCount = 0;

    VmDirLog(LDAP_DEBUG_TRACE, "VmDirGetServersInfo failed. Error(%u)", dwError);
    goto cleanup;
}


/*
 * Test if the remote host is an partner of this host, return the RA DN if it is.
 * The algorithm is to search replication agreement entries for attribute LABELED_URI.
 * If the atribute value (host portion) on any such entries matches the DCAccount of
 * the local host, then that host is a partner.
 * An sample of pszHostDn: cn=sea2-office-dhcp-97-124.eng.vmware.com,cn=Servers,
 *              cn=default-first-site,cn=Sites,cn=Configuration,dc=vsphere,dc=loca
 */
static
DWORD
_VmDirIsHostAPartner(
    LDAP *pLd,
    PCSTR pszHostDn,
    PCSTR pszDCAccount,
    PBOOLEAN pIsParter,
    PSTR *ppszPartnerRaDn
    )
{
    DWORD dwError = 0;
    LDAPMessage* pMessages = NULL;
    int i = 0;
    PSTR pszLabeledURI = ATTR_LABELED_URI;
    PSTR ppszAttrs[] = { pszLabeledURI, NULL };
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR pFilter = NULL;
    DWORD dwInfoCount = 0;
    PSTR pszPartnerHostName = NULL;
    PSTR pszPartnerRaDn = NULL;

    *pIsParter = FALSE;
    dwError = VmDirAllocateStringPrintf(
                      &pFilter,
                      "%s=%s",
                      ATTR_OBJECT_CLASS,
                      OC_REPLICATION_AGREEMENT);

    dwError = ldap_search_ext_s(
                             pLd,
                             pszHostDn,
                             LDAP_SCOPE_SUB,
                             pFilter,  /* filter */
                             ppszAttrs,  /* attrs[]*/
                             FALSE, /* get values  */
                             NULL,  /* serverctrls */
                             NULL,  /* clientctrls */
                             NULL,  /* timeout */
                             -1,
                             &pMessages);
    if (dwError !=0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirIsHostAPartner: Cannot get replication agreement entries under %s, error %d",
                        pszHostDn, dwError);
        //When this occurs, bail out on the current server, and then try the next server.
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwInfoCount = ldap_count_entries(pLd, pMessages);
    if (dwInfoCount == 0)
    {
        dwError = ERROR_NOT_FOUND;
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirIsHostAPartner: No replication agreement entries found under %s, error %d",
                        pszHostDn, dwError);
        //When this occurs, bail out on the current server, and then try the next server.
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0, pEntry = ldap_first_entry(pLd, pMessages);
         pEntry != NULL;
         i++, pEntry = ldap_next_entry(pLd, pEntry))
    {
       if (ppValues)
       {
           ldap_value_free_len(ppValues);
           ppValues = NULL;
       }
       ppValues = ldap_get_values_len(pLd, pEntry, pszLabeledURI);
       if (!ppValues || (ldap_count_values_len(ppValues) != 1))
       {
           dwError = ERROR_NO_SUCH_ATTRIBUTE;
           BAIL_ON_VMDIR_ERROR(dwError);
       }
       VMDIR_SAFE_FREE_STRINGA(pszPartnerHostName);
       dwError = VmDirReplURIToHostname(ppValues[0]->bv_val, &pszPartnerHostName);
       BAIL_ON_VMDIR_ERROR(dwError);

       if (VmDirStringCompareA(pszPartnerHostName, pszDCAccount, FALSE) == 0)
       {
           *pIsParter = TRUE;
           pszPartnerRaDn = ldap_get_dn(pLd, pEntry);
           dwError = VmDirAllocateStringAVsnprintf(ppszPartnerRaDn, "%s", pszPartnerRaDn);
           BAIL_ON_VMDIR_ERROR(dwError);
           goto cleanup;
       }
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pFilter);
    VMDIR_SAFE_FREE_STRINGA(pszPartnerHostName);
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pszPartnerRaDn)
    {
        ldap_memfree(pszPartnerRaDn);
    }
    if (pMessages)
    {
        ldap_msgfree(pMessages);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirIsHostAPartner failed. Error(%u)", dwError);
    goto cleanup;
}
