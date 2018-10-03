/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_LwCAPEMToX509(
    PLWCA_CERTIFICATE   pCertificate,
    X509                **ppX509Cert
    );

static
DWORD
_LwCAOpenSSLGetValuesFromSubjectName(
    PLWCA_CERTIFICATE               pCert,
    DWORD                           dwNIDType,
    PLWCA_STRING_ARRAY              *ppszValues
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

    dwError = _LwCAPEMToX509(pCert, &pX509Cert);
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
DWORD
_LwCAPEMToX509(
    PLWCA_CERTIFICATE   pCertificate,
    X509                **ppX509Cert
    )
{
    X509 *pCert = NULL;
    BIO *pBioMem = NULL;
    DWORD dwError = 0;

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
    if(pBioMem) {
        BIO_free(pBioMem);
    }

    return dwError;

error:
    if (pCert)
    {
        X509_free(pCert);
    }
    goto cleanup;
}
