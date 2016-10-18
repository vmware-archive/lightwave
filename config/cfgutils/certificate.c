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

DWORD
VmwDeployMakeRootCACert(
    PCSTR pszHostname,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PSTR* ppszCert
    )
{
    DWORD    dwError = 0;
    PSTR     pszServername = "localhost";
    PVMCA_SERVER_CONTEXT pBinding = NULL;
    PSTR     pszPassPhrase = NULL;
    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPublicKey = NULL;
    PVMCA_PKCS_10_REQ_DATA pCertRequest = NULL;
    PVMCA_CSR pCSR = NULL;
    PVMCA_CERTIFICATE pCert = NULL;
    PSTR  pszCert = NULL;
    time_t tmNotBefore = time(NULL) - VMW_DEFAULT_ROOTCERT_PREDATE;
    time_t tmNotAfter = tmNotBefore + VMW_DEFAULT_CERT_VALIDITY;
    VMCA_KEY_USAGE_FLAG  keyUsageFlag = ( VMCA_KEY_USAGE_FLAG_KEY_CERT_SIGN |
                                          VMCA_KEY_USAGE_FLAG_KEY_CRL_SIGN);
    PCSTR  pszCountry = VMW_DEFAULT_COUNTRY_CODE;
    PSTR   pszDomainDN = NULL;
    PSTR   pszDN = NULL;

    if (IsNullOrEmptyString(pszHostname) ||
        IsNullOrEmptyString(pszUsername) ||
        !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VMCAOpenServerA(
                    pszServername,
                    pszUsername,
                    pszDomain,
                    pszPassword,
                    0,
                    NULL,
                    &pBinding);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCACreatePrivateKey(
                    pszPassPhrase,
                    VMW_KEY_SIZE,
                    &pPrivateKey,
                    &pPublicKey );
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAAllocatePKCS10Data(&pCertRequest);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployGetDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringPrintf(&pszDN, "CA,%s", pszDomainDN);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAInitPKCS10DataA(
                  pszDN,
                  pszHostname,
                  NULL,
                  NULL,
                  pszCountry,
                  NULL,
                  NULL,
                  pCertRequest);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCASetKeyUsageConstraints(pCertRequest, keyUsageFlag);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCACreateSelfSignedCertificate(
                    pCertRequest,
                    pPrivateKey,
                    pszPassPhrase,
                    tmNotBefore,
                    tmNotAfter,
                    &pCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAAddRootCertificateH(
                    pBinding,
                    pszServername,
                    pCert,
                    pszPassPhrase,
                    pPrivateKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pCert, &pszCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszCert = pszCert;

cleanup:

    if (pCSR)
    {
        VMCAFreeCSR(pCSR);
    }
    if (pCertRequest)
    {
        VMCAFreePKCS10Data(pCertRequest);
    }
    if (pBinding)
    {
        VMCACloseServer(pBinding);
    }
    if (pPrivateKey)
    {
        VMCAFreeKey(pPrivateKey);
    }
    if (pPublicKey)
    {
        VMCAFreeKey(pPublicKey);
    }
    if (pCert)
    {
        VMCAFreeCertificate(pCert);
    }
    if (pszDomainDN)
    {
        VmwDeployFreeMemory(pszDomainDN);
    }
    if (pszDN)
    {
        VmwDeployFreeMemory(pszDN);
    }

    return dwError;

error:

    *ppszCert = NULL;

    goto cleanup;
}

DWORD
VmwDeployGetRootCACert(
    PCSTR pszServername,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PSTR* ppszCert
    )
{
    DWORD    dwError = 0;
    PVMCA_SERVER_CONTEXT pBinding = NULL;
    PVMCA_CERTIFICATE pCert = NULL;
    PSTR      pszCert = NULL;

    if (IsNullOrEmptyString(pszServername) ||
        IsNullOrEmptyString(pszUsername) ||
        !pszPassword)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VMCAOpenServerA(
                    pszServername,
                    pszUsername,
                    pszDomain,
                    pszPassword,
                    0,
                    NULL,
                    &pBinding);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAGetRootCACertificateH(
                    pBinding,
                    pszServername,
                    &pCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pCert, &pszCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszCert = pszCert;

cleanup:

    if (pBinding)
    {
        VMCACloseServer(pBinding);
    }

    return dwError;

error:

    *ppszCert = NULL;

    goto cleanup;
}
DWORD

VmwDeployCreateMachineSSLCert(
    PCSTR pszServername,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszSubjectName,
    PCSTR pszSubjectAltName,
    PSTR* ppszPrivateKey,
    PSTR* ppszCert
    )
{
    DWORD dwError = 0;
    PCSTR pszOrganization = NULL;
    PCSTR pszState = NULL;
    PCSTR pszCountry = VMW_DEFAULT_COUNTRY;
    PCSTR pszEmail = NULL;
    PCSTR pszOU = NULL;
    PVMCA_SERVER_CONTEXT pBinding = NULL;
    PVMCA_PKCS_10_REQ_DATA pCertRequest = NULL;
    PSTR pszPassPhrase = NULL;
    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPublicKey = NULL;
    PVMCA_CSR pCSR = NULL;
    PVMCA_CERTIFICATE pCert = NULL;
    PSTR      pszCert = NULL;
    PSTR      pszPrivateKey = NULL;
    time_t tmNotBefore = time(NULL) - VMW_CERT_EXPIRY_START_LAG;
    time_t tmNotAfter = tmNotBefore + VMW_DEFAULT_CERT_VALIDITY;

    if (IsNullOrEmptyString(pszServername) ||
        IsNullOrEmptyString(pszUsername) ||
        !pszPassword ||
        IsNullOrEmptyString(pszSubjectName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VMCAOpenServerA(
                    pszServername,
                    pszUsername,
                    pszDomain,
                    pszPassword,
                    0,
                    NULL,
                    &pBinding);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCACreatePrivateKey(
                    pszPassPhrase,
                    VMW_KEY_SIZE,
                    &pPrivateKey,
                    &pPublicKey );
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAAllocatePKCS10Data(&pCertRequest);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAInitPKCS10DataA(
                    pszSubjectName,
                    pszOrganization,
                    pszOU,
                    pszState,
                    pszCountry,
                    pszEmail,
                    NULL,
                    pCertRequest);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!IsNullOrEmptyString(pszSubjectAltName))
    {
        if (VmwDeployIsIPAddress(pszSubjectAltName))
        {
            dwError = VMCASetCertValueA(
                              VMCA_OID_IPADDRESS,
                              pCertRequest,
                              (PSTR)pszSubjectAltName);
        }
        else
        {
            dwError = VMCASetCertValueA(
                              VMCA_OID_DNS,
                              pCertRequest,
                              (PSTR)pszSubjectAltName);
        }
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VMCACreateSigningRequest(
                    pCertRequest,
                    pPrivateKey,
                    pszPassPhrase,
                    &pCSR);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VMCAGetSignedCertificateFromCSRH(
                    pBinding,
                    pszServername,
                    pCSR,
                    tmNotBefore,
                    tmNotAfter,
                    &pCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pCert, &pszCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pPrivateKey, &pszPrivateKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszPrivateKey = pszPrivateKey;
    *ppszCert = pszCert;

cleanup:

    if (pCSR)
    {
        VMCAFreeCSR(pCSR);
    }
    if (pCertRequest)
    {
        VMCAFreePKCS10Data(pCertRequest);
    }
    if (pBinding)
    {
        VMCACloseServer(pBinding);
    }
    if (pPrivateKey)
    {
        VMCAFreeKey(pPrivateKey);
    }
    if (pPublicKey)
    {
        VMCAFreeKey(pPublicKey);
    }
    if (pCert)
    {
        VMCAFreeCertificate(pCert);
    }

    return dwError;

error:

    *ppszPrivateKey = NULL;
    *ppszCert = NULL;

    if (pszPrivateKey)
    {
        VmwDeployFreeMemory(pszPrivateKey);
    }
    if (pszCert)
    {
        VmwDeployFreeMemory(pszCert);
    }

    goto cleanup;
}

DWORD
VmwDeployAddTrustedRoot(
    PCSTR pszServername,
    PCSTR pszCACert
    )
{
    DWORD dwError = 0;
    PCSTR pszHostname = "localhost";
    PVECS_STORE pStore = NULL;

    if (IsNullOrEmptyString(pszServername) || IsNullOrEmptyString(pszCACert))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VecsCreateCertStoreA(
                    pszHostname,
                    TRUSTED_ROOTS_STORE_NAME,
                    NULL,
                    &pStore);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = VecsOpenCertStoreA(
                      pszHostname,
                      TRUSTED_ROOTS_STORE_NAME,
                      NULL,
                      &pStore);
    }
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError =  VecsAddEntryA(
                pStore,
                CERT_ENTRY_TYPE_TRUSTED_CERT,
                pszServername,
                pszCACert,
                NULL,
                NULL,
                0);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeployGetDomainDN(
    PCSTR pszDomain,
    PSTR* ppszDomainDN
    )
{
    DWORD dwError = 0;
    const char* pszDelim = ".";
    const char* pszPrefix = "DC=";
    size_t len_prefix = sizeof("DC=")-1;
    const char* pszCommaDelim = ",";
    size_t len_comma_delim = sizeof(",")-1;
    size_t len = 0;
    PSTR  pszDomainDN = NULL;
    PCSTR pszReadCursor = pszDomain;
    PSTR  pszWriteCursor = NULL;

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (len > 0)
            {
                len += len_comma_delim;
            }
            len += len_prefix;
            len += len_name;
        }

        pszReadCursor += len_name;

        if (!IsNullOrEmptyString(pszReadCursor))
        {
            size_t len_delim = strspn(pszReadCursor, pszDelim);

            pszReadCursor += len_delim;
        }
    }

    if (!len)
    {
        dwError = ERROR_INVALID_DOMAINNAME;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateMemory(len+1, (PVOID*)&pszDomainDN);
    BAIL_ON_DEPLOY_ERROR(dwError);

    pszReadCursor  = pszDomain;
    pszWriteCursor = pszDomainDN;

    while (!IsNullOrEmptyString(pszReadCursor))
    {
        size_t len_name = strcspn(pszReadCursor, pszDelim);

        if (len_name > 0)
        {
            if (pszWriteCursor > pszDomainDN)
            {
                memcpy(pszWriteCursor, pszCommaDelim, len_comma_delim);
                pszWriteCursor += len_comma_delim;
            }

            memcpy(pszWriteCursor, pszPrefix, len_prefix);
            pszWriteCursor += len_prefix;

            memcpy(pszWriteCursor, pszReadCursor, len_name);

            pszReadCursor += len_name;
            pszWriteCursor += len_name;
        }

        if (!IsNullOrEmptyString(pszReadCursor))
        {
            size_t len_delim = strspn(pszReadCursor, pszDelim);

            pszReadCursor += len_delim;
        }
    }

    *ppszDomainDN = pszDomainDN;

cleanup:

    return dwError;

error:

    *ppszDomainDN = NULL;

    if (pszDomainDN)
    {
        VmwDeployFreeMemory(pszDomainDN);
    }

    goto cleanup;
}

