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

#define ATTR_KRB_UPN  "userPrincipalName"
#define ATTR_MEMBEROF "memberOf"

static
DWORD
VmDnsDirGetMachineAccountInfoA(
    PSTR*               ppszAccount,
    PSTR*               ppszDomainName,
    PSTR*               ppszPassword
    );

static
DWORD
VmDnsDirGetPnid(
    PSTR* ppszPnid
    );

static
DWORD
VmDnsDirLDAPConnect(
    PCSTR               pszHostName,
    DWORD               dwPort,
    PCSTR               pszUpn,
    PCSTR               pszPassword,
    LDAP**              ppLotus
    );

static
DWORD
VmDnsDirBuildZoneDN(
    PCSTR       pszBaseDN,
    PCSTR       pszZoneName,
    PSTR*       ppszUpnDN
    );

static
DWORD
VmDnsDirBuildRecordDN(
    PCSTR           pszZoneDN,
    PVMDNS_RECORD   pRecord,
    PSTR*           ppszRecordDN
    );

static
DWORD
VmDnsDirGetDomainZonesDN(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PSTR*               ppDomainZonesDN
    );

static
DWORD
VmDnsDirCreateChildZone(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszBaseDN,
    PVMDNS_ZONE_INFO    pZoneInfo
    );

static
DWORD
VmDnsDirDeleteZoneByDN(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN
    );

static
DWORD
VmDnsDirCreateRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pZoneDN,
    PVMDNS_RECORD       pRecord
    );

static
DWORD
VmDnsDirAddRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pZoneDN,
    PVMDNS_RECORD       pRecord
    );

static
DWORD
VmDnsDirDeleteRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord
    );

static
DWORD
VmDnsDirBuildRecordEntry(
    PVMDNS_RECORD           pDnsRecornd,
    PBYTE*                  ppRecordBytes,
    size_t*                 pcRecordBytes
    );

static
DWORD
VmDnsDirGetZoneList(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_DNS_INFO     pZoneInfo
    );

static
DWORD
VmDnsDirPopulateZoneRecords(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_ZONE_ENTRY   pZoneEntry
    );

static
DWORD
VmDnsDirPopulateZone(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_DNS_INFO     pZoneInfo
    );

static
VOID
VmDnsCleanupZoneEntry(
    PVMDNS_DIR_ZONE_ENTRY   pZoneEntry
    );

static
VOID
VmDnsCleanupRecordEntry(
    PVMDNS_RECORD_ENTRY     pRecordEntry
    );

static
DWORD
VmDnsDirUpdateCachedZoneRecords(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_DIR_ZONE_ENTRY       pZoneInfo
    );

static
DWORD
VmDnsDirProcessRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord,
    int                 mod_op
    );

static
DWORD
VmDnsDirUpdateCachedZone(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_DIR_ZONE_ENTRY       pZoneEntry
    );

static
DWORD
VmDnsDirZoneInfoCopy(
    PVMDNS_DIR_ZONE_ENTRY   pZoneEntry,
    PVMDNS_ZONE_INFO        pZoneInfo
    );

static
DWORD
VmDnsDirUpdateState(
    PVMDNS_DIR_ZONE_ENTRY pZoneEntry
    );

VOID
VmDnsDirClose(
    PVMDNS_DIR_CONTEXT  pDirContext
    );

DWORD
VmDnsDirConnect(
    PCSTR               szHostName,
    PVMDNS_DIR_CONTEXT* ppDirContext
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszAccount = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUPN = NULL;
    LDAP* pLotus = NULL;

    dwError = VmDnsDirGetMachineAccountInfoA(
                        &pszAccount,
                        &pszDomainName,
                        &pszPassword);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_DIR_CONTEXT),
                        (void**)&pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                        &pszUPN,
                        "%s@%s",
                        pszAccount,
                        pszDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirLDAPConnect(
                        szHostName,
                        LDAP_PORT,
                        pszUPN,
                        pszPassword,
                        &pLotus);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDirContext->pLdap = pLotus;
    *ppDirContext = pDirContext;

    pDirContext = NULL;

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszUPN);
    VMDNS_SAFE_FREE_STRINGA(pszAccount);
    VMDNS_SAFE_FREE_STRINGA(pszDomainName);
    VMDNS_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    VmDnsDirClose(pDirContext);
    goto cleanup;

}

VOID
VmDnsDirClose(
    PVMDNS_DIR_CONTEXT pDirContext
    )
{
    if (pDirContext)
    {
        if (pDirContext->pLdap)
        {
            ldap_unbind_ext_s(pDirContext->pLdap, NULL, NULL);
        }
        VmDnsFreeMemory(pDirContext);
    }
}

DWORD
VmDnsDirGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_CONNECTION pConnection = NULL;
    PVMDNS_CFG_KEY pRootKey = NULL;
    CHAR szParamsKeyPath[] = VMDIR_CONFIG_PARAMETER_KEY_PATH;
    CHAR szRegValName_Acct[] = VMDIR_REG_KEY_DC_ACCOUNT;
    CHAR szRegValName_Pwd[] = VMDIR_REG_KEY_DC_PASSWORD;
    CHAR szAfdParamsKeyPath[] = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    CHAR szRegValDomain_Name[] = VMAFD_REG_KEY_DOMAIN_NAME;
    PVMDNS_CFG_KEY pParamsKey = NULL;
    PVMDNS_CFG_KEY pAfdParamsKey = NULL;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDomainName = NULL;

    dwError = VmDnsConfigOpenConnection(&pConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenRootKey(
                        pConnection,
                        "HKEY_LOCAL_MACHINE",
                        0,
                        KEY_READ,
                        &pRootKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenKey(
                        pConnection,
                        pRootKey,
                        &szParamsKeyPath[0],
                        0,
                        KEY_READ,
                        &pParamsKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigReadStringValue(
                        pParamsKey,
                        NULL,
                        &szRegValName_Acct[0],
                        &pszAccount);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigReadStringValue(
                        pParamsKey,
                        NULL,
                        &szRegValName_Pwd[0],
                        &pszPassword);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenKey(
                        pConnection,
                        pRootKey,
                        &szAfdParamsKeyPath[0],
                        0,
                        KEY_READ,
                        &pAfdParamsKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigReadStringValue(
                        pAfdParamsKey,
                        NULL,
                        &szRegValDomain_Name[0],
                        &pszDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszDomainName = pszDomainName;
    *ppszPassword = pszPassword;
cleanup:

    if (pParamsKey)
    {
        VmDnsConfigCloseKey(pParamsKey);
    }
    if (pAfdParamsKey)
    {
        VmDnsConfigCloseKey(pAfdParamsKey);
    }
    if (pRootKey)
    {
        VmDnsConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmDnsConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    VMDNS_SAFE_FREE_STRINGA(pszAccount);
    VMDNS_SAFE_FREE_STRINGA(pszDomainName);
    VMDNS_SAFE_FREE_STRINGA(pszPassword);

    goto cleanup;
}

DWORD
VmDnsDirLDAPConnect(
    PCSTR   pszHostName,
    DWORD   dwPort,
    PCSTR   pszUpn,
    PCSTR   pszPassword,
    LDAP**  ppLotus
    )
{
    DWORD dwError = 0;
    LDAP* pDirectory = NULL;
    PSTR pszLdapURI = NULL;

    if (dwPort == 0)
    {
        dwPort = LDAP_PORT;
    }

    dwError = VmDnsAllocateStringPrintfA(
            &pszLdapURI,
            "ldap://%s:%d",
            pszHostName,
            dwPort);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                    &pDirectory,
                    pszHostName,
                    pszUpn,
                    pszPassword);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppLotus = pDirectory;

cleanup:
    VMDNS_SAFE_FREE_MEMORY(pszLdapURI);

    return dwError;

error:

    *ppLotus = NULL;

    if (pDirectory != NULL)
    {
        ldap_unbind_ext(pDirectory, NULL, NULL);
    }

    goto cleanup;
}

DWORD
VmDnsLdapGetMemberships(
    PVMDNS_DIR_CONTEXT pConnection,
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszAttrMemberOf = ATTR_MEMBEROF; // memberOf
    PSTR  ppszAttrs[] = { pszAttrMemberOf, NULL};
    DWORD dwCount = 0;
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;
    DWORD i = 0;
    LDAP *pLd = NULL;

    if (pConnection == NULL ||
        pConnection->pLdap == NULL ||
        IsNullOrEmptyString(pszUPNName) ||
        pppszMemberships == NULL ||
        pdwMemberships == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pLd = pConnection->pLdap;

    dwError = VmDnsAllocateStringPrintfA(&pszFilter, "(%s=%s)", ATTR_KRB_UPN, pszUPNName); // userPrincipalName
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    "",
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    (PSTR*)ppszAttrs,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    -1,
                    &pResult);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwCount = ldap_count_entries(pLd, pResult);
    if (dwCount == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (dwCount > 1)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, pszAttrMemberOf);
    if (!ppValues)
    {
        dwMemberships = 0;
    }
    else
    {
        dwMemberships = ldap_count_values_len(ppValues);
    }

    if (dwMemberships)
    {
        dwError = VmDnsAllocateMemory(dwMemberships * sizeof(PSTR), (PVOID*)&ppszMemberships);
        BAIL_ON_VMDNS_ERROR(dwError);

        for (i = 0; ppValues[i] != NULL; i++)
        {
            PCSTR pszMemberOf = ppValues[i]->bv_val;

            dwError = VmDnsAllocateStringA(pszMemberOf, &ppszMemberships[i]);
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *pppszMemberships = ppszMemberships;
    *pdwMemberships = dwMemberships;

cleanup:

    if(ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMDNS_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:
    if (ppszMemberships != NULL && dwMemberships > 0)
    {
        for (i = 0; i < dwMemberships; i++)
        {
            VMDNS_SAFE_FREE_STRINGA(ppszMemberships[i]);
        }
        VMDNS_SAFE_FREE_MEMORY(ppszMemberships);
    }
    goto cleanup;
}


DWORD
VmDnsGetDSERootAttribute(
    PVMDNS_DIR_CONTEXT pContext,
    PSTR  pszAttribute,
    PSTR* ppszAttrValue
    )
{
    DWORD dwError = 0; // LDAP_SUCCESS
    PSTR pAttribute = NULL;
    BerValue** ppValue = NULL;
    BerElement* pBer = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pResults = NULL;
    PCHAR pszFilter = "(objectClass=*)";
    PCHAR ppszAttrs[] = { pszAttribute, NULL };

    dwError = ldap_search_ext_s(
                            pContext->pLdap,     // Session handle
                            "",              // DN to start search
                            LDAP_SCOPE_BASE, // Scope
                            pszFilter,       // Filter
                            ppszAttrs,       // Retrieve list of attributes
                            0,               // Get both attributes and values
                            NULL,
                            NULL,
                            NULL,
                            0,
                            &pSearchResult); // [out] Search results
    BAIL_ON_VMDNS_ERROR(dwError);

    if (ldap_count_entries(pContext->pLdap, pSearchResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // There should be only one result for this type
    pResults = ldap_first_entry(pContext->pLdap, pSearchResult);
    if (pResults == NULL)
    {
        ldap_get_option(pContext->pLdap, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pAttribute = ldap_first_attribute(pContext->pLdap, pResults, &pBer);
    if (pAttribute == NULL)
    {
        ldap_get_option(pContext->pLdap, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ppValue = ldap_get_values_len(pContext->pLdap, pResults, pszAttribute);
    if (ppValue == NULL)
    {
        ldap_get_option(pContext->pLdap, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(ppValue[0]->bv_val, ppszAttrValue);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (ppValue != NULL)
    {
        ldap_value_free_len(ppValue);
    }

    if (pAttribute != NULL)
    {
        ldap_memfree(pAttribute);
    }

    if (pBer != NULL)
    {
        ber_free(pBer, 0);
    }

    if (pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsGetDefaultDomainName(
    PVMDNS_DIR_CONTEXT pConnection,
    PSTR* ppDomainName
    )
{
    DWORD dwError = 0;
    PCHAR pszDomainNameAttr = "rootdomainnamingcontext";
    PSTR pszDomainName = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppDomainName, dwError);

    dwError = VmDnsGetDSERootAttribute(
                    pConnection,
                    pszDomainNameAttr,
                    &pszDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDomainName = pszDomainName;

cleanup:

    return dwError;

error :

    if (ppDomainName)
    {
        *ppDomainName = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsDirGetDomainZonesDN(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PSTR*               ppszDomainZonesDN
    )
{
    DWORD dwError = 0;
    PSTR pszRootDN = NULL, pszDomainZonesDN = NULL;

    dwError = VmDnsGetDSERootAttribute(pDirContext, "rootDomainNamingContext", &pszRootDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszRootDN, VMDNS_DOMAINDNSZONES_NAME, &pszDomainZonesDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszDomainZonesDN = pszDomainZonesDN;
    pszDomainZonesDN = NULL;

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszRootDN);

    return dwError;

error :

    VMDNS_SAFE_FREE_STRINGA(pszDomainZonesDN);

    goto cleanup;

}


DWORD
VmDnsDirCreateBaseContainer(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszBaseDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDCName = VMDNS_DOMAINDNSZONES_NAME;
    PSTR modv_0[] =
    {
        VMDNS_LDAP_OC_VMWDNSCONFIG,
        VMDNS_LDAP_OC_DOMAINDNS,
        VMDNS_LDAP_OC_DOMAIN,
        VMDNS_LDAP_OC_TOP,
        NULL
    };
    PSTR modv_1[] = { pszDCName, NULL };
    LDAPMod  dnsZoneObjectClass = { 0 };
    LDAPMod  dnsZoneDC = { 0 };
    LDAPMod* pDnsZoneCtrAttrs[] =
    {
        &dnsZoneObjectClass,
        &dnsZoneDC,
        NULL
    };

    dnsZoneObjectClass.mod_op = LDAP_MOD_ADD;
    dnsZoneObjectClass.mod_type = VMDNS_LDAP_ATTR_OBJECTCLASS;
    dnsZoneObjectClass.mod_values = modv_0;

    dnsZoneDC.mod_op = LDAP_MOD_ADD;
    dnsZoneDC.mod_type = VMDNS_LDAP_ATTR_DC;
    dnsZoneDC.mod_values = modv_1;

    dwError = ldap_add_ext_s(
                    pDirContext->pLdap,
                    pszBaseDN,
                    &pDnsZoneCtrAttrs[0],
                    NULL,
                    NULL);
    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = 0;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsCreateInitZoneContainer()
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszRootDN = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszRootDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirCreateBaseContainer(pDirContext, pszRootDN);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszRootDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirCreateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PVMDNS_RECORD pSoaRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirCreateBaseContainer(pDirContext, pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCreateSoaRecord(pZoneInfo, &pSoaRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirCreateChildZone(pDirContext, pszBaseDN, pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirCreateZoneRecord(pZoneInfo->pszName, pSoaRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    VMDNS_LOG_DEBUG("Successfully created dir zone %s.", pZoneInfo->pszName);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VMDNS_FREE_RECORD(pSoaRecord);
    VmDnsDirClose(pDirContext);

    return dwError;

error:
    VMDNS_LOG_ERROR("Failed creating dir zone %s, error %u.",
                    pZoneInfo->pszName, dwError);

    goto cleanup;
}

DWORD
VmDnsDirUpdateZone(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PVMDNS_RECORD pSoaRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    // TODO: Make this atomic
    dwError = VmDnsCreateSoaRecord(pZoneInfo, &pSoaRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirUpdateZoneRecord(pZoneInfo->pszName, pSoaRecord, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirCreateChildZone(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszBaseDN,
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    PSTR modv_0[] = {VMDNS_LDAP_OC_DNSZONE, VMDNS_LDAP_OC_TOP, NULL};
    PSTR modv_1[] = { (PSTR)pZoneInfo->pszName, NULL };
    LDAPMod  dnsZoneObjectClass = {0};
    LDAPMod  dnsZoneDC = {0};
    LDAPMod* pDnsZoneCtrAttrs[] =
    {
            &dnsZoneObjectClass,
            &dnsZoneDC,
            NULL
    };
    PSTR pszZoneDN = NULL;

    dwError = VmDnsDirBuildZoneDN(pszBaseDN, pZoneInfo->pszName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dnsZoneObjectClass.mod_op     = LDAP_MOD_ADD;
    dnsZoneObjectClass.mod_type   = VMDNS_LDAP_ATTR_OBJECTCLASS;
    dnsZoneObjectClass.mod_values = modv_0;

    dnsZoneDC.mod_op     = LDAP_MOD_ADD;
    dnsZoneDC.mod_type   = VMDNS_LDAP_ATTR_DC;
    dnsZoneDC.mod_values = modv_1;

    dwError = ldap_add_ext_s(
                    pDirContext->pLdap,
                    pszZoneDN,
                    &pDnsZoneCtrAttrs[0],
                    NULL,
                    NULL);
    BAIL_ON_VMDNS_ERROR(dwError);
    VMDNS_LOG_DEBUG("Successfully created dir zone %s.", pZoneInfo->pszName);

cleanup:

    if (pszZoneDN)
    {
        VmDnsFreeStringA(pszZoneDN);
    }

    return dwError;

error:
    VMDNS_LOG_ERROR("Failed creating dir zone %s, ldap error %u.",
                    pZoneInfo->pszName, dwError);

    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = ERROR_ALREADY_EXISTS;
    }

    goto cleanup;
}

DWORD
VmDnsDirCreateZoneRecord(
    PCSTR               pZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PSTR pszZoneDN = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszBaseDN, pZoneName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirCreateRecord(pDirContext, pszZoneDN, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VMDNS_SAFE_FREE_STRINGA(pszZoneDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirCreateRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    PSTR modv_0[]  = { VMDNS_LDAP_OC_DNSNODE, VMDNS_LDAP_OC_TOP, NULL};
    PSTR modv_1[] = { (PSTR)pRecord->pszName, NULL };
    BerValue dnsRecordEntry = {0};
    BerValue* modv_2[] = { &dnsRecordEntry, NULL };
    LDAPMod  dnsNodeObjectClass = {0};
    LDAPMod  dnsNodeDC = {0};
    LDAPMod  dnsNodeName = {0};
    LDAPMod  dnsNodeRecord = {0};
    LDAPMod* pDnsRecordCtrAttrs[] =
    {
            &dnsNodeObjectClass,
            &dnsNodeDC,
            &dnsNodeName,
            &dnsNodeRecord,
            NULL
    };
    PSTR   pszRecordDN = NULL;
    PBYTE  pRecordEntry = NULL;
    size_t recordLength = 0;

    dwError = VmDnsDirBuildRecordEntry(pRecord, &pRecordEntry, &recordLength);
    BAIL_ON_VMDNS_ERROR(dwError);

    dnsRecordEntry.bv_val = pRecordEntry;
    dnsRecordEntry.bv_len = recordLength;

    dwError = VmDnsDirBuildRecordDN(
                    pszZoneDN,
                    pRecord,
                    &pszRecordDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dnsNodeObjectClass.mod_op     = LDAP_MOD_ADD;
    dnsNodeObjectClass.mod_type   = VMDNS_LDAP_ATTR_OBJECTCLASS;
    dnsNodeObjectClass.mod_vals.modv_strvals = modv_0;

    dnsNodeDC.mod_op     = LDAP_MOD_ADD;
    dnsNodeDC.mod_type   = VMDNS_LDAP_ATTR_DC;
    dnsNodeDC.mod_vals.modv_strvals = modv_1;

    dnsNodeName.mod_op     = LDAP_MOD_ADD;
    dnsNodeName.mod_type   = VMDNS_LDAP_ATTR_NAME;
    dnsNodeName.mod_vals.modv_strvals = modv_1;

    dnsNodeRecord.mod_op     = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    dnsNodeRecord.mod_type   = VMDNS_LDAP_ATTR_DNS_RECORD;
    dnsNodeRecord.mod_vals.modv_bvals = modv_2;

    dwError = ldap_add_ext_s(
                    pDirContext->pLdap,
                    pszRecordDN,
                    &pDnsRecordCtrAttrs[0],
                    NULL,
                    NULL);
    BAIL_ON_VMDNS_ERROR(dwError);
    VMDNS_LOG_DEBUG("Successfully created dir record %s %u.",
                    pRecord->pszName, pRecord->dwType);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszRecordDN);
    VMDNS_SAFE_FREE_MEMORY(pRecordEntry);

    return dwError;

error:
    VMDNS_LOG_ERROR("Failed creating dir record %s %u, error %u.",
                    pRecord->pszName, pRecord->dwType, dwError);
    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = ERROR_ALREADY_EXISTS;
    }

    goto cleanup;
}

DWORD
VmDnsDirAddZoneRecord(
    PCSTR               pZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PSTR pszZoneDN = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszBaseDN, pZoneName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirAddRecord(pDirContext, pszZoneDN, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VMDNS_SAFE_FREE_STRINGA(pszZoneDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:
    VMDNS_LOG_ERROR("%s failed with error %u.", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmDnsDirAddRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsDirProcessRecord(
                            pDirContext,
                            pszZoneDN,
                            pRecord,
                            LDAP_MOD_ADD  | LDAP_MOD_BVALUES);
    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsDirCreateRecord(
                            pDirContext,
                            pszZoneDN,
                            pRecord);
    }
    return dwError;
}

DWORD
VmDnsDirDeleteRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord
    )
{
    return VmDnsDirProcessRecord(
                            pDirContext,
                            pszZoneDN,
                            pRecord,
                            LDAP_MOD_DELETE | LDAP_MOD_BVALUES);
}

DWORD
VmDnsDirDeleteZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PSTR pszZoneDN = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszBaseDN, pszZoneName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirDeleteRecord(pDirContext, pszZoneDN, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VMDNS_SAFE_FREE_STRINGA(pszZoneDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirUpdateRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord,
    BOOL                bCreateIfNotExists
    )
{
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsDirProcessRecord(
                            pDirContext,
                            pszZoneDN,
                            pRecord,
                            LDAP_MOD_REPLACE | LDAP_MOD_BVALUES);
    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsDirCreateRecord(
                            pDirContext,
                            pszZoneDN,
                            pRecord);
    }
    return dwError;
}

DWORD
VmDnsDirUpdateZoneRecord(
    PCSTR               pszZoneName,
    PVMDNS_RECORD       pRecord,
    BOOL                bCreateIfNotExists
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PSTR pszZoneDN = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszBaseDN, pszZoneName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirUpdateRecord(pDirContext, pszZoneDN, pRecord, bCreateIfNotExists);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VMDNS_SAFE_FREE_STRINGA(pszZoneDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirDeleteZone(
    PCSTR               pszZoneName
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PSTR pszBaseDN = NULL;
    PSTR pszZoneDN = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszBaseDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszBaseDN, pszZoneName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirDeleteZoneByDN(pDirContext, pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBaseDN);
    VMDNS_SAFE_FREE_STRINGA(pszZoneDN);
    VmDnsDirClose(pDirContext);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirDeleteZoneByDN(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN
    )
{
    DWORD dwError = 0;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    PSTR pszFilter = NULL;
    PSTR pszNodeDN = NULL;
    PSTR ppszAttrs[] = { VMDNS_LDAP_ATTR_DC, VMDNS_LDAP_ATTR_NAME, NULL };

    dwError = VmDnsAllocateStringPrintfA(
                        &pszFilter,
                        "(%s=%s)",
                        VMDNS_LDAP_ATTR_OBJECTCLASS,
                        VMDNS_LDAP_OC_DNSNODE
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = ldap_search_ext_s(
                        pDirContext->pLdap,
                        pszZoneDN,
                        LDAP_SCOPE_SUBTREE,
                        pszFilter,
                        ppszAttrs,
                        FALSE,             /* attrs only  */
                        NULL,              /* serverctrls */
                        NULL,              /* clientctrls */
                        NULL,              /* timeout */
                        0,
                        &pSearchRes);
    BAIL_ON_VMDNS_ERROR(dwError);

    pEntry = ldap_first_entry(
                    pDirContext->pLdap,
                    pSearchRes);

    while (pEntry)
    {
        pszNodeDN = ldap_get_dn(pDirContext->pLdap, pEntry);
        if (IsNullOrEmptyString(pszNodeDN))
        {
            dwError = ERROR_INVALID_DATA;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = ldap_delete_ext_s(
                        pDirContext->pLdap,
                        pszNodeDN,
                        NULL,
                        NULL);
        BAIL_ON_VMDNS_ERROR(dwError);
        ldap_memfree(pszNodeDN);
        pszNodeDN = NULL;

        pEntry = ldap_next_entry(pDirContext->pLdap, pEntry);
    }

    dwError = ldap_delete_ext_s(
                    pDirContext->pLdap,
                    pszZoneDN,
                    NULL,
                    NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    if (pSearchRes != NULL)
    {
        ldap_msgfree(pSearchRes);
    }
    VMDNS_SAFE_FREE_MEMORY(pszFilter);
    if (pszNodeDN)
    {
        ldap_memfree(pszNodeDN);
    }
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirBuildZoneDN(
    PCSTR               pszBaseDN,
    PCSTR               pszZoneName,
    PSTR*               ppszUpnDN
    )
{
    return VmDnsAllocateStringPrintfA(ppszUpnDN, "DC=%s, %s", pszZoneName, pszBaseDN);
}

DWORD
VmDnsDirBuildRecordDN(
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord,
    PSTR*               ppszRecordDN
)
{
    DWORD dwError = 0;
    PSTR  pszRecordCN = NULL;
    PSTR  pszRecordDN = NULL;

    dwError = VmDnsRecordGetCN(pRecord, &pszRecordCN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(&pszRecordDN, "DC=%s, %s", pszRecordCN, pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszRecordDN = pszRecordDN;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsDirBuildRecordEntry(
    PVMDNS_RECORD       pDnsRecornd,
    PBYTE*              ppRecordBytes,
    size_t*             pcRecordBytes
    )
{
    DWORD dwError = 0;

    PBYTE pRecordBytes = NULL;
    DWORD dwSize = 0;

    dwError = VmDnsSerializeDnsRecord(pDnsRecornd, &pRecordBytes, &dwSize, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordBytes = pRecordBytes;
    *pcRecordBytes = dwSize;

cleanup:

    return dwError;

error:

    goto cleanup;

}

DWORD
VmDnsDirGetLotusZoneInfo(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_DNS_INFO*    ppDirDnsInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_DNS_INFO pDirZoneInfo = NULL;

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_DIR_DNS_INFO), (PVOID *)&pDirZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetZoneList(pDirContext, pDirZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirPopulateZone(pDirContext, pDirZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDirDnsInfo = pDirZoneInfo;
    pDirZoneInfo = NULL;

cleanup:

    return dwError;

error:
    VmDnsCleanupZoneInfo(pDirZoneInfo);
    goto cleanup;
}

DWORD
VmDnsDirGetZoneList(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_DNS_INFO     pZoneInfo
    )
{
    DWORD dwError = 0;
    PSTR pszSearchBase = NULL;
    PSTR pszFilter = NULL;
    PSTR ppszAttrs[] = { VMDNS_LDAP_ATTR_DC, NULL };
    PSTR pszZoneName = NULL;

    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    struct berval** ppValues = NULL;
    PVMDNS_DIR_ZONE_ENTRY pZoneEntry = NULL;

    dwError = VmDnsAllocateStringPrintfA(
                            &pszFilter,
                            "(%s=%s)",
                            VMDNS_LDAP_ATTR_OBJECTCLASS,
                            VMDNS_LDAP_OC_DNSZONE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszSearchBase);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = ldap_search_ext_s(
                            pDirContext->pLdap,
                            pszSearchBase,
                            LDAP_SCOPE_SUBTREE,
                            pszFilter,
                            ppszAttrs,
                            FALSE,             /* attrs only  */
                            NULL,              /* serverctrls */
                            NULL,              /* clientctrls */
                            NULL,              /* timeout */
                            0,
                            &pSearchRes);
    BAIL_ON_VMDNS_ERROR(dwError);

    pEntry = ldap_first_entry(
                            pDirContext->pLdap,
                            pSearchRes);

    while (pEntry)
    {
        ppValues = ldap_get_values_len(
                            pDirContext->pLdap,
                            pEntry,
                            VMDNS_LDAP_ATTR_DC);

        if (!ppValues || (ldap_count_values_len(ppValues) != 1))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_DIR_ZONE_ENTRY),
                            (PVOID *)&pZoneEntry
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsAllocateStringPrintfA(
                            &pszZoneName,
                            "%s",
                            ppValues[0]->bv_val);
        BAIL_ON_VMDNS_ERROR(dwError);

        ldap_value_free_len(ppValues);
        ppValues = NULL;

        VMDNS_LOG_DEBUG("DirSync retrieved zone: %s", pszZoneName);

        pZoneEntry->ZoneName = pszZoneName;
        pszZoneName = NULL;

        PushEntryList(&pZoneInfo->Zones, &(pZoneEntry->ListEntry));
        pZoneEntry = NULL;

        pEntry = ldap_next_entry(pDirContext->pLdap, pEntry);
    }

cleanup:

    if (ppValues != NULL)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    if (pSearchRes != NULL)
    {
        ldap_msgfree(pSearchRes);
    }

    VMDNS_SAFE_FREE_MEMORY(pszSearchBase);
    VMDNS_SAFE_FREE_MEMORY(pszFilter);

    return dwError;
error:
    VmDnsCleanupZoneEntry(pZoneEntry);
    VMDNS_SAFE_FREE_MEMORY(pszZoneName);

    goto cleanup;
}

DWORD
VmDnsDirPopulateZone(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_DNS_INFO     pZoneInfo
    )
{
    DWORD dwError = 0;
    PSINGLE_LIST_ENTRY  pNextEntry = pZoneInfo->Zones.Next;
    PVMDNS_DIR_ZONE_ENTRY pZoneEntry = NULL;

    while (pNextEntry)
    {
        pZoneEntry = CONTAINING_RECORD(pNextEntry, VMDNS_DIR_ZONE_ENTRY, ListEntry);

        dwError = VmDnsDirPopulateZoneRecords(
                            pDirContext,
                            pZoneEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        pNextEntry = pNextEntry->Next;
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsDirPopulateZoneRecords(
    PVMDNS_DIR_CONTEXT      pDirContext,
    PVMDNS_DIR_ZONE_ENTRY   pZoneEntry
    )
{
    DWORD dwError = 0;
    PSTR pszSearchBase = NULL;
    PSTR pszZoneDN = NULL;
    PSTR pszFilter = NULL;
    PSTR ppszAttrs[] = { VMDNS_LDAP_ATTR_DC, VMDNS_LDAP_ATTR_NAME, VMDNS_LDAP_ATTR_DNS_RECORD, NULL };
    PSTR pszNewName = NULL;
    CHAR szDc[256];
    BOOL bFqdn = FALSE;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    PVMDNS_RECORD pRecord = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    struct berval** ppValues = NULL;

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszSearchBase);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirBuildZoneDN(pszSearchBase, pZoneEntry->ZoneName, &pszZoneDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                        &pszFilter,
                        "(%s=%s)",
                        VMDNS_LDAP_ATTR_OBJECTCLASS,
                        VMDNS_LDAP_OC_DNSNODE
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = ldap_search_ext_s(
                        pDirContext->pLdap,
                        pszZoneDN,
                        LDAP_SCOPE_SUBTREE,
                        pszFilter,
                        ppszAttrs,
                        FALSE,             /* attrs only  */
                        NULL,              /* serverctrls */
                        NULL,              /* clientctrls */
                        NULL,              /* timeout */
                        0,
                        &pSearchRes);
    BAIL_ON_VMDNS_ERROR(dwError);

    pEntry = ldap_first_entry(
                    pDirContext->pLdap,
                    pSearchRes);

    while (pEntry)
    {
        bFqdn = FALSE;
        szDc[0] = '\0';
        ppValues = ldap_get_values_len(
                        pDirContext->pLdap,
                        pEntry,
                        VMDNS_LDAP_ATTR_DC);
        if (ppValues)
        {
            if (ppValues[0]->bv_len > 0)
            {
                memcpy(szDc, ppValues[0]->bv_val, ppValues[0]->bv_len);
                szDc[ppValues[0]->bv_len] = '\0';
                bFqdn = szDc[ppValues[0]->bv_len - 1] == '.';
            }
            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        ppValues = ldap_get_values_len(
                        pDirContext->pLdap,
                        pEntry,
                        VMDNS_LDAP_ATTR_DNS_RECORD);

        if (ppValues)
        {
            int i = 0;
            while (ppValues[i])
            {
                dwError = VmDnsDeserializeDnsRecord(
                                    ppValues[i]->bv_val,
                                    ppValues[i]->bv_len,
                                    &pRecord,
                                    FALSE
                                    );
                BAIL_ON_VMDNS_ERROR(dwError);

                if (pRecord->dwType == VMDNS_RR_TYPE_A ||
                    pRecord->dwType == VMDNS_RR_TYPE_AAAA)
                {
                    if (bFqdn)
                    {
                        if (!VmDnsAllocateStringA(szDc, &pszNewName))
                        {
                            VmDnsFreeStringA(pRecord->pszName);
                            pRecord->pszName = pszNewName;
                        }
                    }
                }
                dwError = VmDnsAllocateMemory(
                                    sizeof(VMDNS_RECORD_ENTRY),
                                    (PVOID *)&pRecordEntry
                                    );
                BAIL_ON_VMDNS_ERROR(dwError);

                pRecordEntry->Record = pRecord;
                pRecord = NULL;

                PushEntryList(&pZoneEntry->Records, &(pRecordEntry->ListEntry));
                VMDNS_LOG_DEBUG("DirSync retrieved record: %s %u",
                                pRecordEntry->Record->pszName,
                                pRecordEntry->Record->dwType);
                pRecordEntry = NULL;
                ++i;
            }

            ldap_value_free_len(ppValues);
            ppValues = NULL;
        }

        pEntry = ldap_next_entry(pDirContext->pLdap, pEntry);
    }

cleanup:

    if (ppValues != NULL)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    if (pSearchRes != NULL)
    {
        ldap_msgfree(pSearchRes);
    }

    VMDNS_SAFE_FREE_MEMORY(pszSearchBase);
    VMDNS_SAFE_FREE_MEMORY(pszZoneDN);
    VMDNS_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:

    VmDnsCleanupRecordEntry(pRecordEntry);

goto cleanup;
}

VOID
VmDnsCleanupZoneInfo(
    PVMDNS_DIR_DNS_INFO     pDirZoneInfo
    )
{
    if (pDirZoneInfo)
    {
        PSINGLE_LIST_ENTRY  pNextEntry = pDirZoneInfo->Zones.Next;
        PVMDNS_DIR_ZONE_ENTRY pZoneEntry = NULL;

        while (pNextEntry)
        {

            pZoneEntry = CONTAINING_RECORD(pNextEntry, VMDNS_DIR_ZONE_ENTRY, ListEntry);
            pNextEntry = pNextEntry->Next;
            VmDnsCleanupZoneEntry(pZoneEntry);
        }

        VMDNS_SAFE_FREE_MEMORY(pDirZoneInfo);
    }
}

VOID
VmDnsCleanupZoneEntry(
    PVMDNS_DIR_ZONE_ENTRY   pZoneEntry
    )
{
    if (pZoneEntry)
    {
        PSINGLE_LIST_ENTRY  pNextEntry = pZoneEntry->Records.Next;
        PVMDNS_RECORD_ENTRY pRecordEntry = NULL;

        while (pNextEntry)
        {
            pRecordEntry = CONTAINING_RECORD(pNextEntry, VMDNS_RECORD_ENTRY, ListEntry);
            pNextEntry = pNextEntry->Next;
            VmDnsCleanupRecordEntry(pRecordEntry);
        }

        VMDNS_SAFE_FREE_STRINGA(pZoneEntry->ZoneName);
        VMDNS_SAFE_FREE_MEMORY(pZoneEntry);
    }
}

VOID
VmDnsCleanupRecordEntry(
    PVMDNS_RECORD_ENTRY pRecordEntry
    )
{
    if (pRecordEntry)
    {
        VMDNS_FREE_RECORD(pRecordEntry->Record);
        VMDNS_SAFE_FREE_MEMORY(pRecordEntry);
    }
}

DWORD
VmDnsDirUpdateCachedZoneInfo(
    PVMDNS_DIR_DNS_INFO     pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_INFO_ARRAY pZoneArray = NULL;
    PSINGLE_LIST_ENTRY  pNextEntry = pZoneInfo->Zones.Next;
    PVMDNS_DIR_ZONE_ENTRY pZoneEntry = NULL;
    BOOL* pKeepIndexes = NULL;
    DWORD i = 0;
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    VMDNS_LOCKWRITE(gpDNSDriverGlobals->pZoneList->pLock);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneList(&pZoneArray);
    if (dwError == 0 && pZoneArray->dwCount != 0)
    {
        dwError = VmDnsAllocateMemory(
                                sizeof(BOOL) * pZoneArray->dwCount,
                                (PVOID)&pKeepIndexes
                                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    while (pNextEntry)
    {
        pZoneEntry = CONTAINING_RECORD(pNextEntry, VMDNS_DIR_ZONE_ENTRY, ListEntry);

        dwError = VmDnsDirUpdateCachedZone(pCtx, pZoneEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsDirUpdateCachedZoneRecords(
                            pCtx,
                            pZoneEntry
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (pZoneArray)
        {
            for (i = 0; i < pZoneArray->dwCount; ++i)
            {
                if (0 == VmDnsStringCompareA(
                                    pZoneArray->ZoneInfos[i].pszName,
                                    pZoneEntry->ZoneName,
                                    TRUE))
                {
                    pKeepIndexes[i] = TRUE;
                    break;
                }
            }
        }

        pNextEntry = pNextEntry->Next;
    }

    if (pZoneArray)
    {
        for (i = 0; i < pZoneArray->dwCount; ++i)
        {
            if (!pKeepIndexes[i])
            {
                VmDnsZoneDelete(pCtx, pZoneArray->ZoneInfos[i].pszName);
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }

    dwError = VmdnsZoneEndUpdate(pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_UNLOCKWRITE(gpDNSDriverGlobals->pZoneList->pLock);
    VMDNS_SAFE_FREE_MEMORY(pKeepIndexes);
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsDirUpdateCachedZoneRecords(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_DIR_ZONE_ENTRY       pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_ARRAY pRecords = NULL;
    PSINGLE_LIST_ENTRY  pNextEntry = pZoneInfo->Records.Next;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    BOOL* pKeepIndexes = NULL;
    DWORD i = 0;
    BOOL bAddedSrvRecord = FALSE;

    dwError = VmDnsZoneListRecord(pZoneInfo->ZoneName, &pRecords);
    if (dwError == 0 && pRecords->dwCount != 0)
    {
        dwError = VmDnsAllocateMemory(
                                sizeof(BOOL) * pRecords->dwCount,
                                (PVOID)&pKeepIndexes
                                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = 0;

    while (pNextEntry)
    {
        pRecordEntry = CONTAINING_RECORD(pNextEntry, VMDNS_RECORD_ENTRY, ListEntry);

        dwError = VmDnsZoneAddRecord(pCtx, pZoneInfo->ZoneName, pRecordEntry->Record, FALSE);
        if (!dwError && pRecordEntry->Record->dwType == VMDNS_RR_TYPE_SRV)
        {
            bAddedSrvRecord = TRUE;
        }
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            dwError = ERROR_SUCCESS;
        }
        BAIL_ON_VMDNS_ERROR(dwError);

        if (pRecords && pRecords->dwCount != 0)
        {
            for (i = 0; i < pRecords->dwCount; ++i)
            {
                if (VmDnsCompareRecord(&pRecords->Records[i], pRecordEntry->Record))
                {
                    pKeepIndexes[i] = TRUE;
                    break;
                }
            }
        }

        pNextEntry = pNextEntry->Next;
    }

    if (pRecords && pRecords->dwCount != 0)
    {
        for (i = 0; i < pRecords->dwCount; ++i)
        {
            if (!pKeepIndexes[i])
            {
                VMDNS_LOG_DEBUG("Deleting record from zone repo per dirsync: %s %u",
                                pRecords->Records[0].pszName,
                                pRecords->Records[0].dwType);
                VmDnsZoneDeleteRecord(pCtx, pZoneInfo->ZoneName, &pRecords->Records[i], FALSE);
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }

    if (bAddedSrvRecord)
    {
        dwError = VmDnsDirUpdateState(pZoneInfo);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    VMDNS_SAFE_FREE_MEMORY(pKeepIndexes);
    VMDNS_FREE_RECORD_ARRAY(pRecords);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsDirProcessRecord(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PCSTR               pszZoneDN,
    PVMDNS_RECORD       pRecord,
    int                 mod_op
    )
{
    DWORD dwError = 0;
    BerValue dnsRecordEntry = {0};
    BerValue* modv[] = { &dnsRecordEntry, NULL };
    LDAPMod  dnsNodeRecord = {0};
    LDAPMod* pDnsRecordCtrAttrs[] =
    {
        &dnsNodeRecord,
        NULL
    };
    PSTR   pszRecordDN = NULL;
    PBYTE  pRecordEntry = NULL;
    size_t recordLength = 0;

    dwError = VmDnsDirBuildRecordEntry(pRecord, &pRecordEntry, &recordLength);
    BAIL_ON_VMDNS_ERROR(dwError);

    dnsRecordEntry.bv_val = pRecordEntry;
    dnsRecordEntry.bv_len = recordLength;

    dwError = VmDnsDirBuildRecordDN(
                    pszZoneDN,
                    pRecord,
                    &pszRecordDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dnsNodeRecord.mod_op     = mod_op;
    dnsNodeRecord.mod_type   = VMDNS_LDAP_ATTR_DNS_RECORD;
    dnsNodeRecord.mod_vals.modv_bvals = modv;

    dwError = ldap_modify_ext_s(
                    pDirContext->pLdap,
                    pszRecordDN,
                    &pDnsRecordCtrAttrs[0],
                    NULL,
                    NULL);
    BAIL_ON_VMDNS_ERROR(dwError);
    VMDNS_LOG_DEBUG("Successful op %u on %s dir record %s %u.",
                    mod_op, pRecord->pszName, pRecord->pszName, pRecord->dwType);

cleanup:
    return dwError;
error:
    VMDNS_LOG_DEBUG("Failed op %u on %s dir record %s %u.",
                    mod_op, pRecord->pszName, pRecord->pszName, pRecord->dwType);
    if (dwError == LDAP_NO_SUCH_OBJECT)
    {
        dwError = ERROR_NOT_FOUND;
    }
    goto cleanup;
}

static
DWORD
VmDnsDirUpdateCachedZone(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_DIR_ZONE_ENTRY       pZoneEntry
    )
{
    DWORD                   dwError = 0;
    VMDNS_ZONE_INFO         zoneInfo = { 0 };

    zoneInfo.pszName = pZoneEntry->ZoneName;

    dwError = VmDnsDirZoneInfoCopy(pZoneEntry, &zoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneUpdate(pCtx, &zoneInfo, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmDnsDirZoneInfoCopy(
    PVMDNS_DIR_ZONE_ENTRY   pZoneEntry,
    PVMDNS_ZONE_INFO        pZoneInfo
    )
{
    DWORD                   dwError = ERROR_INVALID_DATA;
    PSINGLE_LIST_ENTRY      pNextEntry = pZoneEntry->Records.Next;
    PVMDNS_RECORD_ENTRY     pRecordEntry = NULL;
    PVMDNS_RECORD           pRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    while (pNextEntry)
    {
        pRecordEntry = CONTAINING_RECORD(pNextEntry, VMDNS_RECORD_ENTRY, ListEntry);
        if (pRecordEntry->Record->dwType == VMDNS_RR_TYPE_SOA)
        {
            pRecord = pRecordEntry->Record;
            pZoneInfo->pszPrimaryDnsSrvName = pRecord->Data.SOA.pNamePrimaryServer;
            pZoneInfo->pszRName = pRecord->Data.SOA.pNameAdministrator;
            pZoneInfo->expire = pRecord->Data.SOA.dwExpire;
            pZoneInfo->minimum = pRecord->Data.SOA.dwDefaultTtl;
            pZoneInfo->refreshInterval = pRecord->Data.SOA.dwRefresh;
            pZoneInfo->retryInterval = pRecord->Data.SOA.dwRetry;
            pZoneInfo->serial = pRecord->Data.SOA.dwSerialNo;
            pZoneInfo->dwZoneType = VmDnsIsReverseZoneName(pZoneEntry->ZoneName) ?
                            VMDNS_ZONE_TYPE_REVERSE : VMDNS_ZONE_TYPE_FORWARD;

            dwError = ERROR_SUCCESS;

            break;
        }

        pNextEntry = pNextEntry->Next;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}


DWORD
VmDnsDirSetForwarders(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PSTR*               ppszForwarders,
    DWORD               dwCount
    )
{
    DWORD dwError = 0;
    PSTR* ppszForwardersLocal = NULL;
    PSTR  pszDomainZonesDN = NULL;
    LDAPMod  dnsForwarders = { 0 };
    LDAPMod* ppDnsForwardersAttrs[] =
    {
        &dnsForwarders,
        NULL
    };
    DWORD iForwarder = 0;

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszDomainZonesDN);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(PSTR) * (dwCount + 1),
                        (PVOID*)&ppszForwardersLocal);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; iForwarder < dwCount; iForwarder++)
    {
        ppszForwardersLocal[iForwarder] = ppszForwarders[iForwarder];
    }

    dnsForwarders.mod_op = LDAP_MOD_REPLACE;
    dnsForwarders.mod_type = VMDNS_LDAP_ATTR_FORWARDERS;
    dnsForwarders.mod_vals.modv_strvals = ppszForwardersLocal;

    dwError = ldap_modify_ext_s(
                        pDirContext->pLdap,
                        pszDomainZonesDN,
                        &ppDnsForwardersAttrs[0],
                        NULL,
                        NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (ppszForwardersLocal)
    {
        VmDnsFreeMemory(ppszForwardersLocal);
    }

    VMDNS_SAFE_FREE_STRINGA(pszDomainZonesDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirSaveForwarders(
    DWORD               dwCount,
    PSTR*               ppszForwarders
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirSetForwarders(
                        pDirContext,
                        ppszForwarders,
                        dwCount);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsDirClose(pDirContext);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsDirGetForwarders(
    PVMDNS_DIR_CONTEXT  pDirContext,
    PSTR**           pppszForwarders,
    PDWORD           pdwCount
    )
{
    DWORD dwError = 0;
    PCSTR pszFilter = "(objectclass=*)";
    PSTR pszSearchBase = NULL;
    LDAPMessage* pSearchRes = NULL;
    LDAPMessage* pEntry = NULL;
    struct berval** ppValues = NULL;
    DWORD dwCount = 0;
    PSTR* ppszValueArray = NULL;
    int i = 0;
    PSTR  ppszAttrs[] =
    {
        VMDNS_LDAP_ATTR_FORWARDERS,
        NULL
    };

    dwError = VmDnsDirGetDomainZonesDN(pDirContext, &pszSearchBase);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = ldap_search_ext_s(
                            pDirContext->pLdap,
                            pszSearchBase,
                            LDAP_SCOPE_SUBTREE,
                            pszFilter,
                            ppszAttrs,
                            FALSE,             /* attrs only  */
                            NULL,              /* serverctrls */
                            NULL,              /* clientctrls */
                            NULL,              /* timeout */
                            0,
                            &pSearchRes);
    BAIL_ON_VMDNS_ERROR(dwError);

    pEntry = ldap_first_entry(
                            pDirContext->pLdap,
                            pSearchRes);

    if (pEntry)
    {
        ppValues = ldap_get_values_len(
                            pDirContext->pLdap,
                            pEntry,
                            VMDNS_LDAP_ATTR_FORWARDERS);
        if (!ppValues || !*ppValues)
        {
            dwError = ERROR_NO_DATA;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwCount = ldap_count_values_len(ppValues);

        dwError = VmDnsAllocateMemory(
                            sizeof(PSTR) * dwCount,
                            (PVOID*)&ppszValueArray);
        BAIL_ON_VMDNS_ERROR(dwError);

        for (; i < dwCount; ++i)
        {
            dwError = VmDnsAllocateStringA(
                            ppValues[i]->bv_val,
                            &ppszValueArray[i]);
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        *pppszForwarders = ppszValueArray;
        *pdwCount = dwCount;
    }

cleanup:

    if (ppValues != NULL)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    if (pSearchRes != NULL)
    {
        ldap_msgfree(pSearchRes);
    }

    VMDNS_SAFE_FREE_MEMORY(pszSearchBase);

    return dwError;

error:

    if (pppszForwarders)
    {
        *pppszForwarders = NULL;
    }

    if (pdwCount)
    {
        *pdwCount = 0;
    }

    VmDnsFreeStringArrayA(ppszValueArray);

    goto cleanup;
}

static
DWORD
VmDnsDirUpdateState(
    PVMDNS_DIR_ZONE_ENTRY pZoneEntry
    )
{
    DWORD   dwError = 0;
    DWORD   dwCompare = 0;
    PSTR    pszAccount = NULL;
    PSTR    pszDomainName = NULL;
    PSTR    pszPassword = NULL;
    PSTR    pszPnid = NULL;
    PSTR    pszTarget = NULL;
    PSINGLE_LIST_ENTRY  pNextEntry = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneEntry, dwError);

    pNextEntry = pZoneEntry->Records.Next;

    VMDNS_STATE state = VmDnsGetState();
    if (state == VMDNS_UNINITIALIZED)
    {
        dwError = VmDnsDirGetMachineAccountInfoA(
                    &pszAccount,
                    &pszDomainName,
                    &pszPassword);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsDirGetPnid(&pszPnid);
        BAIL_ON_VMDNS_ERROR(dwError);

        VMDNS_LOG_INFO("%s: pnid: %s", __FUNCTION__, pszPnid);
        VmDnsStringTrimA(pszPnid, ".", TRUE);

        while (pNextEntry)
        {
            pRecordEntry = CONTAINING_RECORD(pNextEntry, VMDNS_RECORD_ENTRY, ListEntry);

            if (pRecordEntry->Record->dwType == VMDNS_RR_TYPE_SRV &&
                !VmDnsStringCompareA(
                        pRecordEntry->Record->pszName,
                        VMDNS_LDAP_SRV_NAME,
                        FALSE
                        ) &&
                !VmDnsStringCompareA(
                            pZoneEntry->ZoneName,
                            pszDomainName,
                            FALSE))
            {
                VMDNS_LOG_INFO(
                    "%s: _ldap._tcp target: %s",
                    __FUNCTION__,
                    pRecordEntry->Record->Data.SRV.pNameTarget);
                if (!VmDnsAllocateStringA(
                        pRecordEntry->Record->Data.SRV.pNameTarget,
                        &pszTarget
                        ))
                {
                    VmDnsStringTrimA(pszTarget, ".", TRUE);
                    VmDnsTrimDomainNameSuffix(pszPnid, pszDomainName);
                    VmDnsTrimDomainNameSuffix(pszTarget, pszDomainName);
                    VMDNS_LOG_INFO(
                        "%s: Comparing pnid [%s] against target [%s]",
                        __FUNCTION__,
                        pszPnid,
                        pszTarget);
                    dwCompare = VmDnsStringCompareA(
                                    pszTarget,
                                    pszPnid,
                                    FALSE
                                );
                    VmDnsFreeStringA(pszTarget);
                    if (!dwCompare)
                    {
                        VMDNS_LOG_INFO(
                            "%s found matching SRV record. "
                            "Attempting to update state from UNINIT to INIT.",
                            __FUNCTION__);
                        VmDnsConditionalSetState(VMDNS_INITIALIZED, VMDNS_UNINITIALIZED);
                        break;
                    }
                }
            }

            pNextEntry = pNextEntry->Next;
        }
    }

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszPnid);
    VMDNS_SAFE_FREE_STRINGA(pszAccount);
    VMDNS_SAFE_FREE_STRINGA(pszDomainName);
    VMDNS_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsDirGetPnid(
    PSTR* ppszPnid
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_CONNECTION pConnection = NULL;
    PVMDNS_CFG_KEY pRootKey = NULL;
    CHAR szAfdParamsKeyPath[] = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    CHAR szRegValPnid[] = VMAFD_REG_KEY_PNID;
    PVMDNS_CFG_KEY pAfdParamsKey = NULL;
    PSTR pszPnid = NULL;

    dwError = VmDnsConfigOpenConnection(&pConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenRootKey(
                        pConnection,
                        "HKEY_LOCAL_MACHINE",
                        0,
                        KEY_READ,
                        &pRootKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigOpenKey(
                        pConnection,
                        pRootKey,
                        &szAfdParamsKeyPath[0],
                        0,
                        KEY_READ,
                        &pAfdParamsKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigReadStringValue(
                        pAfdParamsKey,
                        NULL,
                        &szRegValPnid[0],
                        &pszPnid);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszPnid = pszPnid;

cleanup:

    if (pAfdParamsKey)
    {
        VmDnsConfigCloseKey(pAfdParamsKey);
    }
    if (pRootKey)
    {
        VmDnsConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmDnsConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    VMDNS_SAFE_FREE_STRINGA(pszPnid);

    goto cleanup;
}

