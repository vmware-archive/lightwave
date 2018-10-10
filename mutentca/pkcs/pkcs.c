/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
_LwCAOpenSSLGetNIDIndex(
    X509_NAME   *pszSubjName,
    DWORD       dwNIDType,
    int         iPosIn,
    int         *pIPosOut
    );

static
DWORD
_LwCAGetSubjectName(
    X509        *pCert,
    X509_NAME   **ppszSubjName
    );

static
DWORD
_LwCAGetX509NameEntryCount(
    X509_NAME   *pszSubjName,
    size_t      *pszNumDNs
    );

static
DWORD
_LwCAX509NameGetEntry(
    X509_NAME       *pszSubjName,
    int             iPos,
    X509_NAME_ENTRY **ppEntry
    );

static
DWORD
_LwCAX509NameEntryGetData(
    X509_NAME_ENTRY *pEntry,
    PSTR            *ppszValueString,
    size_t          *pszEntryLength
    );

static
DWORD
_LwCAOpenSSLGetValuesFromSubjectName(
    PLWCA_CERTIFICATE               pCert,
    DWORD                           dwNIDType,
    PLWCA_STRING_ARRAY              *ppszValues
    );

static
DWORD
_LwCAPEMToPrivateKey(
    PCSTR   pcszKey,
    PCSTR   pcszPassPhrase,
    RSA     **ppPrivateKey
    );

DWORD
LwCAGetCommonNameFromSubject(
    PLWCA_CERTIFICATE   pCert,
    PSTR                *ppszCommonName
    )
{
    DWORD                   dwError = 0;
    PLWCA_STRING_ARRAY      pszCommonNames = NULL;
    PSTR                    pszCommonName = NULL;

    dwError = _LwCAOpenSSLGetValuesFromSubjectName(
        pCert,
        LWCA_OPENSSL_NID_CN,
        &pszCommonNames
        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
        &pszCommonName,
        "%s=%s",
        LWCA_COMMON_NAME,
        pszCommonNames->ppData[0]
        );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCommonName = pszCommonName;

error:
    LwCAFreeStringArray(pszCommonNames);
    return dwError;
}

DWORD
LwCAPEMToX509(
    PCSTR       pCertificate,
    X509        **ppX509Cert
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    BIO *pBioMem = NULL;

    if (IsNullOrEmptyString(pCertificate) || !ppX509Cert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID) pCertificate, -1);
    if (pBioMem == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pCert  = PEM_read_bio_X509(pBioMem, NULL, NULL, NULL);
    if (pCert  == NULL)
    {
        dwError = LWCA_CERT_IO_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppX509Cert = pCert;

cleanup:
    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:
    if (pCert)
    {
        X509_free(pCert);
    }
    if (ppX509Cert)
    {
        *ppX509Cert = NULL;
    }
    goto cleanup;
}


DWORD
LwCAValidateCertificate(
    X509    *pCert,
    PCSTR   pcszPrivateKey,
    PCSTR   pcszPassPhrase
    )
{
    DWORD dwError = 0;
    EVP_PKEY *pKey = NULL;
    RSA *pRsa = NULL;

    if (!pCert || IsNullOrEmptyString(pcszPrivateKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pKey = EVP_PKEY_new();
    if (pKey == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAPEMToPrivateKey(pcszPrivateKey, pcszPassPhrase, &pRsa);
    BAIL_ON_LWCA_ERROR(dwError);

    EVP_PKEY_assign_RSA(pKey, pRsa);

    if (X509_check_private_key(pCert, pKey) == X509_CERT_PRIVATE_KEY_MISMATCH)
    {
        dwError = LWCA_CERT_PRIVATE_KEY_MISMATCH;
    }
    BAIL_ON_LWCA_ERROR(dwError);

error :
    // This will free RSA too.
    if (pKey)
    {
        EVP_PKEY_free(pKey);
    }

    return dwError;
}

DWORD
LwCAGetCertSubjectName(
    X509 *pCert,
    PSTR *ppszSubjectName
    )
{
    DWORD dwError = 0;
    X509_NAME *pSubjectName = NULL;
    PSTR pszTempSubjectName = NULL;
    PSTR pszSubjectName = NULL;

    if (!pCert || !ppszSubjectName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pSubjectName = X509_get_subject_name(pCert);
    if (pSubjectName == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pszSubjectName = X509_NAME_oneline(pSubjectName, NULL , 0);
    if( pszSubjectName == NULL )
    {
        dwError = LWCA_ERROR_INVALID_CERTIFICATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    dwError = LwCAAllocateStringA(pszSubjectName, &pszTempSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszSubjectName = pszTempSubjectName;

cleanup:
    LWCA_SAFE_FREE_MEMORY(pszSubjectName);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszTempSubjectName);
    if (ppszSubjectName)
    {
        *ppszSubjectName = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGetCertIssuerName(
    X509 *pCert,
    PSTR *ppszIssuerName
    )
{
    DWORD dwError = 0;
    X509_NAME *pIssuerName = NULL;
    PSTR pszIssuerName = NULL;
    PSTR pszTempIssuerName = NULL;

    if (!pCert || !ppszIssuerName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pIssuerName = X509_get_issuer_name(pCert);
    if (pIssuerName == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pszIssuerName = X509_NAME_oneline(pIssuerName, NULL , 0);
    if (pszIssuerName == NULL)
    {
        dwError = LWCA_ERROR_INVALID_CERTIFICATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }
    dwError = LwCAAllocateStringA(pszIssuerName, &pszTempIssuerName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszIssuerName = pszTempIssuerName;

cleanup:
    LWCA_SAFE_FREE_MEMORY(pszIssuerName);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszTempIssuerName);
    if (ppszIssuerName)
    {
        *ppszIssuerName = NULL;
    }
    goto cleanup;
}

DWORD
LwCACheckCACert(
    X509 *pCert
    )
{
    DWORD dwError = 0;

    if (!pCert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = X509_check_ca(pCert);
    if (dwError == 1) // it is  a CA
    {
        dwError = 0;
    } else { // 3 , 4 means it is a CA, but we don't care
        dwError = LWCA_NOT_CA_CERT;
    }
    BAIL_ON_LWCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_LwCAOpenSSLGetValuesFromSubjectName(
    PLWCA_CERTIFICATE               pCert,
    DWORD                           dwNIDType,
    PLWCA_STRING_ARRAY              *ppszValues
    )
{
    DWORD                           dwError = 0;
    PLWCA_STRING_ARRAY              pszValues = NULL;
    PSTR                            pszValueString = NULL;
    X509                            *pX509Cert = NULL;
    X509_NAME                       *pszSubjName = NULL;
    X509_NAME_ENTRY                 *pEntry = NULL;
    int                             iPos = -1;
    size_t                          szNumDNs = 0;
    size_t                          szEntryLength = 0;

    if (IsNullOrEmptyString(pCert) ||
        !ppszValues)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPEMToX509(pCert, &pX509Cert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetSubjectName(pX509Cert, &pszSubjName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetX509NameEntryCount(pszSubjName, &szNumDNs);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(
        sizeof (LWCA_STRING_ARRAY),
        (PVOID*) &pszValues
        );
    BAIL_ON_LWCA_ERROR(dwError);

    if (!szNumDNs)
    {
        dwError = LWCA_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(
        sizeof(PSTR) * szNumDNs,
        (PVOID *) &pszValues->ppData
        );
    BAIL_ON_LWCA_ERROR(dwError);

    for (;;)
    {
        dwError = _LwCAOpenSSLGetNIDIndex(pszSubjName, dwNIDType, iPos, &iPos);
        BAIL_ON_LWCA_ERROR(dwError);
        if (iPos == -1)
        {
            break;
        }

        dwError = _LwCAX509NameGetEntry(pszSubjName, iPos, &pEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAX509NameEntryGetData(
            pEntry,
            &pszValueString,
            &szEntryLength
            );
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateStringA(
            pszValueString,
            &pszValues->ppData[pszValues->dwCount++]
            );
        BAIL_ON_LWCA_ERROR(dwError);

        if (pszValueString)
        {
            OPENSSL_free(pszValueString);
            pszValueString = NULL;
        }
    }

    if (szNumDNs != pszValues->dwCount)
    {
        dwError = LWCA_CERT_DECODE_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszValues = pszValues;

cleanup:
    if (pX509Cert)
    {
        X509_free(pX509Cert);
    }
    if (pszValueString)
    {
        OPENSSL_free(pszValueString);
    }

    return dwError;

error:
    LwCAFreeStringArray(pszValues);
    if (ppszValues)
    {
        *ppszValues = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAOpenSSLGetNIDIndex(
    X509_NAME   *pszSubjName,
    DWORD       dwNIDType,
    int         iPosIn,
    int         *pIPosOut
    )
{
    DWORD       dwError = 0;
    int         iPosOut = 0;

    switch (dwNIDType)
    {
        case LWCA_OPENSSL_NID_O:
            iPosOut = X509_NAME_get_index_by_NID(
                pszSubjName,
                NID_organizationName,
                iPosIn);
            break;

        case LWCA_OPENSSL_NID_CN:
            iPosOut = X509_NAME_get_index_by_NID(
                pszSubjName,
                NID_commonName,
                iPosIn);
            break;

        default:
            iPosOut = -1;
    }

    *pIPosOut = iPosOut;

    return dwError;
}

static
DWORD
_LwCAGetSubjectName(
    X509        *pCert,
    X509_NAME   **ppszSubjName
    )
{
    DWORD       dwError = 0;
    X509_NAME   *pszSubjName = NULL;

    pszSubjName = X509_get_subject_name(pCert);
    if (pszSubjName == NULL)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszSubjName = pszSubjName;

error:
    return dwError;
}

static
DWORD
_LwCAGetX509NameEntryCount(
    X509_NAME   *pszSubjName,
    size_t      *pszNumDNs
    )
{
    DWORD   dwError = 0;
    size_t  szNumDNs = 0;

    szNumDNs = X509_NAME_entry_count(pszSubjName);
    if (szNumDNs == 0)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pszNumDNs = szNumDNs;

error:
    return dwError;
}

static
DWORD
_LwCAX509NameGetEntry(
    X509_NAME   *pszSubjName,
    int         iPos,
    X509_NAME_ENTRY **ppEntry
    )
{
    DWORD               dwError = 0;
    X509_NAME_ENTRY     *pEntry = NULL;

    pEntry = X509_NAME_get_entry(pszSubjName, iPos);
    if (pEntry == NULL)
    {
        dwError = LWCA_CERT_DECODE_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppEntry = pEntry;

error:
    return dwError;
}

static
DWORD
_LwCAX509NameEntryGetData(
    X509_NAME_ENTRY *pEntry,
    PSTR            *ppszValueString,
    size_t          *pszEntryLength
    )
{
    DWORD       dwError = 0;
    ASN1_STRING *pEntryAsn1 = NULL;
    PSTR        pszValueString = NULL;
    size_t      szEntryLength = 0;

    pEntryAsn1 = X509_NAME_ENTRY_get_data(pEntry);
    if (pEntryAsn1 == NULL)
    {
        dwError = LWCA_CERT_DECODE_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    szEntryLength = ASN1_STRING_to_UTF8(
        (PBYTE *)&pszValueString,
        pEntryAsn1
        );
    if (!pszValueString || szEntryLength != strlen(pszValueString))
    {
        dwError = LWCA_CERT_DECODE_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszValueString = pszValueString;
    *pszEntryLength = szEntryLength;

error:
    return dwError;
}


static
int
_LwCAPemPasswordCallback(
    PSTR    pszBuf,
    int     bufLen,
    int     flag,
    PVOID   pCtx
    )
{
    DWORD dwError = 0;
    size_t len = 0;

    len = LwCAStringLenA(pCtx);
    if(len > bufLen)
    {
        return 0;
    }

    dwError = LwCACopyMemory(pszBuf, bufLen, pCtx, len);
    if (dwError != 0)
    {
        return 0;
    }
    return len;
}

static
DWORD
_LwCAPEMToPrivateKey(
    PCSTR   pcszKey,
    PCSTR   pcszPassPhrase,
    RSA     **ppPrivateKey
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    RSA *pRsa = NULL;
    PSTR pszPassPhrase = NULL;

    pBioMem = BIO_new_mem_buf((PVOID) pcszKey, -1);
    if (pBioMem == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    OpenSSL_add_all_algorithms();

    if (!IsNullOrEmptyString(pcszPassPhrase))
    {
        dwError = LwCAAllocateStringA(pcszPassPhrase, &pszPassPhrase);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRsa  = PEM_read_bio_RSAPrivateKey(pBioMem, NULL, _LwCAPemPasswordCallback, pszPassPhrase);
    if (pRsa  == NULL)
    {
        dwError = LWCA_KEY_IO_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppPrivateKey = pRsa;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPassPhrase);
    if (pBioMem)
    {
        BIO_free(pBioMem);
    }
    return dwError;

error:
    if (pRsa)
    {
        RSA_free(pRsa);
    }
    if (ppPrivateKey)
    {
        *ppPrivateKey = NULL;
    }
    goto cleanup;
}
