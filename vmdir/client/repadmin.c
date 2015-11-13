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
