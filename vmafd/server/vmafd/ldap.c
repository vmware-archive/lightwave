/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : ldap.c
 *
 * Abstract :
 *
 */
#include "includes.h"

//TODO: Should remove this function after it is included in vmdirclient.h
#ifdef __cplusplus
extern "C"
#endif
DWORD
VmDirSafeLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszHost,
    PCSTR       pszUPN,         // opt, if exists, will try SRP mech
    PCSTR       pszPassword     // opt, if exists, will try SRP mech
    );

static
int
VmAfdCountResultAttribute(
    LDAP        *pLotus,
    LDAPMessage *pSearchResult,
    PCSTR       pszAttribute
    );

static
DWORD
VmAfdCopyQueryResultAttributeString(
    LDAP*        pLotus,
    LDAPMessage* pCAResult,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
);

DWORD
VmAfdGetDSERootAttribute(
    LDAP* pLotus,
    PSTR  pszAttribute,
    PSTR* ppAttrValue
    )
{
    DWORD dwError = 0; // LDAP_SUCCESS
    PCHAR pDcFilter = "(objectClass=*)";
    PCHAR pDcAttr[] = { pszAttribute, NULL };
    PSTR pAttribute = NULL;
    BerElement* pBer = NULL;
    BerValue** ppValue = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pResults = NULL;

    dwError = ldap_search_ext_s(
                  pLotus,
                  "",
                  LDAP_SCOPE_BASE,
                  pDcFilter,
                  pDcAttr,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  0,
                  &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ldap_count_entries(pLotus, pSearchResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pResults = ldap_first_entry(pLotus, pSearchResult);
    if (pResults == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pAttribute = ldap_first_attribute(pLotus,pResults,&pBer);
    if (pAttribute == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ppValue = ldap_get_values_len(pLotus, pResults, pAttribute);
    if (ppValue == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(ppValue[0]->bv_val, ppAttrValue);
    BAIL_ON_VMAFD_ERROR(dwError);

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
        ber_free(pBer,0);
    }
    if (pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:

    *ppAttrValue = NULL;

    goto cleanup;
}

DWORD
VmAfdLDAPConnect(
    PSTR   pszHostName,
    DWORD  dwPort,
    PCSTR   pszUpn,
    PCSTR   pszPassword,
    LDAP** ppLotus
    )
{
    DWORD dwError = 0;
    LDAP* pDirectory = NULL;
    PSTR pszLdapURI = NULL;

    if (dwPort == 0)
    {
        dwPort = LDAP_PORT;
    }

    if (VmAfdIsIPV6AddrFormat(pszHostName))
    {
        dwError = VmAfdAllocateStringPrintf(
                &pszLdapURI,
                "ldap://[%s]:%d",
                pszHostName,
                dwPort);
    }
    else
    {
        dwError = VmAfdAllocateStringPrintf(
                &pszLdapURI,
                "ldap://%s:%d",
                pszHostName,
                dwPort);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                    &pDirectory,
                    pszHostName,
                    pszUpn,
                    pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppLotus = pDirectory;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszLdapURI);

    return dwError;

error:

    *ppLotus = NULL;

    if (pDirectory != NULL)
    {
        ldap_unbind_ext(pDirectory, NULL, NULL);
    }

    goto cleanup;
}

VOID
VmAfdLdapClose(
    LDAP* pHandle
    )
{
    if (pHandle != NULL)
    {
        ldap_unbind_ext(pHandle, NULL, NULL);
    }
}

DWORD
VmAfdQueryCACertAndCrlAttributes(
    LDAP* pLotus,
    PVMAFD_CERT_ARRAY*          ppCertificates,
    PVMAFD_CRL_FILE_CONTAINER*  ppCrls
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = "(objectClass=vmwCertificationAuthority)";
    PSTR pszAttrCert  = "cACertificate";
    PSTR pszAttrCrl  = "certificateRevocationList";
    PSTR pszSearchBaseDN = NULL;
    PSTR pszDomainName = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pCAResult = NULL;

    PCHAR attrs[] = { pszAttrCert, pszAttrCrl, NULL};
    int nCertCount = 0;
    int nCrlCount = 0;
    PVMAFD_CERT_ARRAY pCertArray = NULL;
    PVMAFD_CRL_FILE_CONTAINER pCrlContainer = NULL;
    struct berval** ppValues = NULL;

    if (!ppCertificates || !ppCrls)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDefaultDomainName(
                 pLotus,
                 &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                 &pszSearchBaseDN,
                 "cn=Configuration,%s",
                 pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                 pLotus,
                 pszSearchBaseDN,
                 LDAP_SCOPE_SUBTREE,
                 pszFilter,
                 attrs,
                 0, /* get values and attrs */
                 NULL,
                 NULL,
                 NULL,
                 0,
                 &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    for ( pCAResult = ldap_first_entry(pLotus, pSearchResult);
          pCAResult != NULL;
          pCAResult = ldap_next_entry(pLotus, pCAResult))
    {
       nCertCount += VmAfdCountResultAttribute(
           pLotus, pCAResult, pszAttrCert);

       nCrlCount += VmAfdCountResultAttribute(
           pLotus, pCAResult, pszAttrCrl);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CERT_ARRAY),
                    (PVOID*)&pCertArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    pCertArray->dwCount = nCertCount;

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CRL_FILE_CONTAINER),
                    (PVOID*)&pCrlContainer);
    BAIL_ON_VMAFD_ERROR(dwError);

    pCrlContainer->dwCount = nCrlCount;

    if (nCertCount || nCrlCount)
    {
        int certIndex = 0;
        int crlIndex = 0;
        if (nCertCount)
        {
            dwError = VmAfdAllocateMemory(
                        sizeof(VMAFD_CERT_CONTAINER) * nCertCount,
                        (PVOID*)&pCertArray->certificates);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (nCrlCount)
        {
            dwError = VmAfdAllocateMemory(
                        sizeof(VMAFD_CRL_DATA) * nCrlCount,
                        (PVOID*)&pCrlContainer->crls);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        for ( pCAResult = ldap_first_entry(pLotus, pSearchResult);
              pCAResult != NULL;
              pCAResult = ldap_next_entry(pLotus, pCAResult))
        {
            // Copy certs
            ppValues = ldap_get_values_len(
                                        pLotus,
                                        pCAResult,
                                        pszAttrCert);
            if (ppValues)
            {
                int i = 0;
                while(ppValues[i])
                {
                    dwError = VmAfdAllocateMemory(
                                    sizeof(CHAR) * ppValues[i]->bv_len + 1,
                                    (PVOID)&pCertArray->certificates[certIndex].pCert);
                    BAIL_ON_VMAFD_ERROR(dwError);
                    memcpy(
                      (PVOID) pCertArray->certificates[certIndex].pCert,
                      (PVOID) ppValues[i]->bv_val,
                      (size_t) ppValues[i]->bv_len);
                    i++;
                    certIndex++;
                }
                ldap_value_free_len(ppValues);
                ppValues = NULL;
            }

            // Copy CRLs
            ppValues = ldap_get_values_len(
                                        pLotus,
                                        pCAResult,
                                        pszAttrCrl);
            if (ppValues)
            {
                int i = 0;
                while(ppValues[i])
                {
                    dwError = VmAfdAllocateMemory(
                                    sizeof(CHAR) * ppValues[i]->bv_len + 1,
                                    (PVOID)&pCrlContainer->crls[crlIndex].buffer);
                    BAIL_ON_VMAFD_ERROR(dwError);
                    memcpy(
                      (PVOID) pCrlContainer->crls[crlIndex].buffer,
                      (PVOID) ppValues[i]->bv_val,
                      (size_t) ppValues[i]->bv_len);
                    i++;
                    crlIndex++;
                }
                ldap_value_free_len(ppValues);
                ppValues = NULL;
            }
        }
    }

    *ppCertificates = pCertArray;
    *ppCrls = pCrlContainer;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);

    if (ppValues != NULL)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    if(pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:

    *ppCertificates = NULL;

    VMAFD_SAFE_FREE_MEMORY(pCrlContainer);
    if (ppCertificates)
    {
        *ppCertificates = NULL;
    }
    if(ppCrls)
    {
        *ppCrls = NULL;
    }
    if (pCertArray )
    {
        VecsFreeCertArray(pCertArray);
    }

    goto cleanup;
}

DWORD
VmAfdQueryCACerts(
    LDAP* pLotus,
    PCSTR pszCACN,
    BOOL  bDetail,
    PVMAFD_CA_CERT_ARRAY* ppCACertificates
    )
{
    DWORD dwError = 0;
    PSTR pszClassFilter = "(objectClass=vmwCertificationAuthority)";
    PSTR pszAttrEntryDN = "entryDN";
    PSTR pszAttrCADN    = "cACertificateDN";
    PSTR pszAttrCert    = "cACertificate";
    PSTR pszAttrCrl     = "certificateRevocationList";
    PSTR pszFilter      = NULL;
    PSTR pszComboFilter = NULL;
    PSTR pszSearchBaseDN= NULL;
    PSTR pszDomainName  = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pCAResult = NULL;
    PVMAFD_CA_CERT_ARRAY pCertArray = NULL;

    PCHAR attrs[] = { pszAttrEntryDN, pszAttrCADN,
                      pszAttrCert, pszAttrCrl, NULL};
    struct berval** ppValues = NULL;
    int nCertCount = 0;
    int certIndex = 0;

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_CA_CERT_ARRAY),
                    (PVOID*)&pCertArray);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetDefaultDomainName(
                 pLotus,
                 &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                 &pszSearchBaseDN,
                 "cn=Configuration,%s",
                 pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszCACN)
    {
        dwError = VmAfdAllocateStringPrintf(
                &pszComboFilter, "(&(CN=%s)%s)",
                pszCACN, pszClassFilter);
        BAIL_ON_VMAFD_ERROR(dwError);
        pszFilter = pszComboFilter;
    }
    else
    {
        pszFilter = pszClassFilter;
    }

    dwError = ldap_search_ext_s(
                 pLotus,
                 pszSearchBaseDN,
                 LDAP_SCOPE_SUBTREE,
                 pszFilter,
                 attrs,
                 0, /* get values and attrs */
                 NULL,
                 NULL,
                 NULL,
                 0,
                 &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    nCertCount = ldap_count_entries(pLotus, pSearchResult);
    if (nCertCount > 0)
    {
        pCertArray->dwCount = nCertCount;

        dwError = VmAfdAllocateMemory(
                        sizeof(VMAFD_CA_CERT) * nCertCount,
                        (PVOID*)&pCertArray->pCACerts);
        BAIL_ON_VMAFD_ERROR(dwError);

        for ( pCAResult = ldap_first_entry(pLotus, pSearchResult);
              pCAResult != NULL;
              pCAResult = ldap_next_entry(pLotus, pCAResult), ++certIndex)
        {
            // The following assumes there's only one certificate for each CA
            // object. In the future if the whole chain is store, we will
            // update accordingly.

            // Copy CN
            dwError = VmAfdCopyQueryResultAttributeString(
                    pLotus, pCAResult, pszAttrEntryDN, FALSE,
                    (PSTR*)&pCertArray->pCACerts[certIndex].pCN);
            BAIL_ON_VMAFD_ERROR(dwError);

            // Copy subject DN
            dwError = VmAfdCopyQueryResultAttributeString(
                    pLotus, pCAResult, pszAttrCADN, FALSE,
                    (PSTR*)&pCertArray->pCACerts[certIndex].pSubjectDN);
            BAIL_ON_VMAFD_ERROR(dwError);

            if (bDetail)
            {
                // Copy certificate
                dwError = VmAfdCopyQueryResultAttributeString(
                        pLotus, pCAResult, pszAttrCert, FALSE,
                        (PSTR*)&pCertArray->pCACerts[certIndex].pCert);
                BAIL_ON_VMAFD_ERROR(dwError);

                dwError = VmAfdCopyQueryResultAttributeString(
                        pLotus, pCAResult, pszAttrCrl, TRUE,
                        (PSTR*)&pCertArray->pCACerts[certIndex].pCrl);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

    *ppCACertificates = pCertArray;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pszComboFilter);

    if (ppValues != NULL)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    if(pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }

    return dwError;

error:

    if (pCertArray )
    {
        VecsFreeCACertArray(pCertArray);
    }
    if (ppCACertificates)
    {
        *ppCACertificates = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetDomainFunctionLevel(
    LDAP* pLotus,
    PSTR* ppDomainFunctionLevel
    )
{
    DWORD dwError = 0;
    PSTR pszDomainFunctionLevel = NULL;
    PCHAR pszFunctionalLevelAttr = "vmwDomainFunctionalLevel";
    PSTR pszDomainName = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pResults = NULL;
    BerElement* pBer = NULL;
    BerValue** ppValue = NULL;
    PCHAR attrs[] = {pszFunctionalLevelAttr, NULL};
    PCHAR pszFilter = "(objectClass=*)";
    PSTR pAttribute = NULL;

    if (!pLotus || !ppDomainFunctionLevel)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDefaultDomainName(
                 pLotus,
                 &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ldap_search_ext_s(
                 pLotus,
                 pszDomainName,
                 LDAP_SCOPE_BASE,
                 pszFilter,
                 attrs,
                 0, /* get values and attrs */
                 NULL,
                 NULL,
                 NULL,
                 0,
                 &pSearchResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ldap_count_entries(pLotus, pSearchResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pResults = ldap_first_entry(pLotus, pSearchResult);
    if (pResults == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pAttribute = ldap_first_attribute(pLotus,pResults,&pBer);
    if (pAttribute == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    ppValue = ldap_get_values_len(pLotus, pResults, pAttribute);
    if (ppValue == NULL)
    {
        ldap_get_option(pLotus, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(ppValue[0]->bv_val, &pszDomainFunctionLevel);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppDomainFunctionLevel = pszDomainFunctionLevel;

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
        ber_free(pBer,0);
    }
    if (pSearchResult != NULL)
    {
        ldap_msgfree(pSearchResult);
    }
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);

    return dwError;

error :

    VMAFD_SAFE_FREE_STRINGA(pszDomainFunctionLevel);
    if (ppDomainFunctionLevel)
    {
        *ppDomainFunctionLevel = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdGetDefaultDomainName(
    LDAP* pLotus,
    PSTR* ppDomainName
    )
{
    DWORD dwError = 0;
    PCHAR pszDomainNameAttr = "rootdomainnamingcontext";
    PSTR pszDomainName = NULL;

    if (!pLotus || !ppDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetDSERootAttribute(
                    pLotus,
                    pszDomainNameAttr,
                    &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

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

static
int
VmAfdCountResultAttribute(
    LDAP        *pLotus,
    LDAPMessage *pSearchResult,
    PCSTR       pszAttribute
)
{
    int i = 0;
    int count = 0;
    struct berval** ppValues = NULL;

    ppValues = ldap_get_values_len(pLotus,
                                pSearchResult,
                                pszAttribute);
    if (ppValues)
    {
        while(ppValues[i++])
        {
            count ++;
        }

        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }

    return count;

}

static
DWORD
VmAfdCopyQueryResultAttributeString(
    LDAP*        pLotus,
    LDAPMessage* pCAResult,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
)
{
    DWORD   dwError = 0;
    struct berval** ppValues = NULL;
    PSTR   pszOut = NULL;

    ppValues = ldap_get_values_len(
                                pLotus,
                                pCAResult,
                                pszAttribute);
    if (ppValues && ppValues[0])
    {
        dwError = VmAfdAllocateMemory(
                        sizeof(CHAR) * ppValues[0]->bv_len + 1,
                        (PVOID)&pszOut);
        BAIL_ON_VMAFD_ERROR(dwError);
        memcpy(
            (PVOID) pszOut,
            (PVOID) ppValues[0]->bv_val,
            (size_t) ppValues[0]->bv_len);
    }
    else if (!bOptional)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszOut = pszOut;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }
    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pszOut);
    if (ppszOut)
    {
        *ppszOut = NULL;
    }
    goto cleanup;
}

