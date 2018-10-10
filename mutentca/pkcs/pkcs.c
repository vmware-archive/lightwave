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
_LwCAGetX509CertSubjectName(
    X509        *pCert,
    X509_NAME   **ppSubjectName
    );

static
DWORD
_LwCAGetX509CertIssuerName(
    X509        *pCert,
    X509_NAME   **ppIssuerName
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

static
DWORD
_LwCAAddExtension(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509V3_CTX                  *pCtx,
    int                         NID,
    PSTR                        pszValue
    );

static
DWORD
_LwCAAppendAlternateNameString(
    PSTR                pszDestinationString,
    size_t              cDestinationString,
    PCSTR               pcszSourceType,
    PLWCA_STRING_ARRAY  pSource
    );

static
DWORD
_LwCAX509NameAddEntryByTxt(
    X509_NAME       *pCertName,
    PCSTR           pcszField,
    PCSTR           pcszEntry
    );

static
DWORD
_LwCAX509NameAddEntriesByTxt(
    X509_NAME           *pCertName,
    PCSTR               pcszField,
    PLWCA_STRING_ARRAY  pEntries
    );

static
DWORD
_LwCACreateX509Request(
    X509_REQ **ppReq
    );

static
DWORD
_LwCAX509ReqGetCertificateName(
    X509_REQ    *pReq,
    X509_NAME   **ppCertName
    );

static
DWORD
_LwCAX509ReqSetCertificateName(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    X509_REQ                *pReq
    );

static
DWORD
_LwCACreateX509Extensions(
    PLWCA_PKCS_10_REQ_DATA      pCertRequest,
    STACK_OF(X509_EXTENSION)    **ppStack
    );

static
DWORD
_LwCAX509NameSetDC(
     X509_NAME  *pCertName,
     PSTR       pszDomainName
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
        SN_commonName,
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

    dwError = _LwCAGetX509CertSubjectName(pCert, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    pszSubjectName = X509_NAME_oneline(pSubjectName, NULL , 0);
    if (pszSubjectName == NULL )
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

    dwError = _LwCAGetX509CertIssuerName(pCert, &pIssuerName);
    BAIL_ON_LWCA_ERROR(dwError);

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

DWORD
LwCACSRToPEM(
    X509_REQ                *pX509Req,
    PLWCA_CERT_REQUEST      *ppCertReq
    )
{
    DWORD dwError = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    PLWCA_CERT_REQUEST pCertReq = NULL;

    if (!pX509Req || !ppCertReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = PEM_write_bio_X509_REQ(pBioMem, pX509Req);
    BAIL_ON_SSL_ERROR(dwError, LWCA_REQUEST_ERROR);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = LwCAAllocateStringWithLengthA(pBuffMem->data, pBuffMem->length - 1, &pCertReq);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCertReq = pCertReq;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }
    return dwError;

error :
    LWCA_SAFE_FREE_STRINGA(pCertReq);
    if (ppCertReq)
    {
        *ppCertReq = NULL;
    }

    goto cleanup;
}

DWORD
LwCACreateCertificateSignRequest(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    X509_REQ                **ppReq
    )
{
    DWORD dwError = 0;
    X509_REQ *pReq = NULL;
    STACK_OF(X509_EXTENSION) *pStack = NULL;

    if (!pCertRequest || !ppReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCACreateX509Request(&pReq);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509ReqSetCertificateName(pCertRequest, pReq);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCACreateX509Extensions(pCertRequest, &pStack);
    BAIL_ON_LWCA_ERROR(dwError);

    //TODO

    *ppReq = pReq;

cleanup:
    if (pStack)
    {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;

error:
    if (pReq)
    {
        X509_REQ_free(pReq);
    }
    if (ppReq)
    {
        *ppReq = NULL;
    }

    goto cleanup;
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
    X509_NAME                       *pSubjName = NULL;
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

    dwError = _LwCAGetX509CertSubjectName(pX509Cert, &pSubjName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetX509NameEntryCount(pSubjName, &szNumDNs);
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
        dwError = _LwCAOpenSSLGetNIDIndex(pSubjName, dwNIDType, iPos, &iPos);
        BAIL_ON_LWCA_ERROR(dwError);
        if (iPos == -1)
        {
            break;
        }

        dwError = _LwCAX509NameGetEntry(pSubjName, iPos, &pEntry);
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
_LwCAGetX509CertSubjectName(
    X509        *pCert,
    X509_NAME   **ppSubjectName
    )
{
    DWORD dwError = 0;
    X509_NAME *pSubjectName = NULL;

    pSubjectName = X509_get_subject_name(pCert);
    if (pSubjectName == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppSubjectName = pSubjectName;

cleanup:
    return dwError;

error:
    if (ppSubjectName)
    {
        *ppSubjectName = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetX509CertIssuerName(
    X509        *pCert,
    X509_NAME   **ppIssuerName
    )
{
    DWORD dwError = 0;
    X509_NAME *pIssuerName = NULL;

    pIssuerName = X509_get_issuer_name(pCert);
    if (pIssuerName == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppIssuerName = pIssuerName;

cleanup:
    return dwError;

error:
    if (ppIssuerName)
    {
        *ppIssuerName = pIssuerName;
    }
    goto cleanup;
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
    if (len > bufLen)
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

static
DWORD
_LwCAAddExtension(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509V3_CTX                  *pCtx,
    int                         NID,
    PSTR                        pszValue
    )
{
    DWORD dwError = 0;
    X509_EXTENSION *pExtension = NULL;

    pExtension = X509V3_EXT_conf_nid(NULL, pCtx, NID, pszValue);
    if (pExtension == NULL)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    sk_X509_EXTENSION_push(pStack, pExtension);

error:
    return dwError;
}

static
DWORD
_LwCAAppendAlternateNameString(
    PSTR                pszDestinationString,
    size_t              destinationStringLen,
    PCSTR               pcszSourceType,
    PLWCA_STRING_ARRAY  pSource
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    size_t nCurrentSize = 0;
    size_t sizeStr =  0;

    if (!pszDestinationString || IsNullOrEmptyString(pcszSourceType) ||
        !pSource || destinationStringLen == 0)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    nCurrentSize = LwCAStringLenA(pszDestinationString);

    for (; iEntry < pSource->dwCount; ++iEntry)
    {
        if (nCurrentSize != 0 )
        {
            dwError = LwCAStringCatA(pszDestinationString, destinationStringLen, ", ");
            BAIL_ON_LWCA_ERROR(dwError);
            nCurrentSize += 2;
        }

        sizeStr = LwCAStringLenA(pSource->ppData[iEntry]) + LwCAStringLenA(pcszSourceType);
        if ( (destinationStringLen - nCurrentSize) > sizeStr)
        {
            dwError = LwCAStringCatA(pszDestinationString, destinationStringLen, pcszSourceType);
            BAIL_ON_LWCA_ERROR(dwError);

            dwError = LwCAStringCatA(pszDestinationString, destinationStringLen, pSource->ppData[iEntry]);
            BAIL_ON_LWCA_ERROR(dwError);

            nCurrentSize += sizeStr;
        }
        else
        {
            dwError = LWCA_OUT_OF_MEMORY_ERROR;
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_LwCAX509NameAddEntryByTxt(
    X509_NAME       *pCertName,
    PCSTR           pcszField,
    PCSTR           pcszEntry
    )
{
    DWORD dwError = 0;

    dwError = X509_NAME_add_entry_by_txt(pCertName,
                pcszField, MBSTRING_UTF8,
                pcszEntry, -1, -1, 0);
    ERR_print_errors_fp(stdout);
    BAIL_ON_SSL_ERROR(dwError, LWCA_INVALID_CSR_FIELD);

error:
    return dwError;
}

static
DWORD
_LwCAX509NameAddEntriesByTxt(
    X509_NAME           *pCertName,
    PCSTR               pcszField,
    PLWCA_STRING_ARRAY  pEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;

    for (; iEntry < pEntries->dwCount; ++iEntry)
    {
        dwError = _LwCAX509NameAddEntryByTxt(
                                            pCertName,
                                            pcszField,
                                            pEntries->ppData[iEntry]
                                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_LwCACreateX509Request(
    X509_REQ **ppReq
    )
{
    DWORD dwError = 0;
    X509_REQ *pReq = NULL;

    pReq = X509_REQ_new();
    if (pReq == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppReq = pReq;

cleanup:
    return dwError;

error:
    if (pReq)
    {
        X509_REQ_free(pReq);
    }
    if (ppReq)
    {
        *ppReq = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAX509ReqGetCertificateName(
    X509_REQ    *pReq,
    X509_NAME   **ppCertName
    )
{
    DWORD dwError = 0;
    X509_NAME *pCertName = NULL;

    if (!pReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pCertName = X509_REQ_get_subject_name(pReq);
    if (pCertName == NULL)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppCertName = pCertName;

cleanup:
    return dwError;

error:
    if (ppCertName)
    {
        *ppCertName = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAX509ReqSetCertificateName(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    X509_REQ                *pReq
    )
{
    DWORD dwError = 0;
    X509_NAME *pCertName = NULL;

    dwError = _LwCAX509ReqGetCertificateName(pReq, &pCertName);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCertRequest->pszName)
    {
        dwError = _LwCAX509NameAddEntryByTxt(pCertName, SN_commonName, pCertRequest->pszName);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pszDomainName)
    {
        dwError = _LwCAX509NameSetDC(pCertName, pCertRequest->pszDomainName);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pCountryList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pCertName, SN_countryName, pCertRequest->pCountryList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pStateList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pCertName, SN_stateOrProvinceName, pCertRequest->pStateList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pLocalityList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pCertName, SN_localityName, pCertRequest->pLocalityList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pOrganizationList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pCertName, SN_organizationName, pCertRequest->pOrganizationList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pOUList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pCertName, SN_organizationalUnitName, pCertRequest->pOUList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

error :
    return dwError;
}

static
DWORD
_LwCACreateX509Extensions(
    PLWCA_PKCS_10_REQ_DATA      pCertRequest,
    STACK_OF(X509_EXTENSION)    **ppStack
    )
{
    DWORD dwError = 0;
    char extensionString[1024] = { 0 };
    char subAltName[2048] = { 0 };
    STACK_OF(X509_EXTENSION) *pStack = NULL;
    BOOLEAN bExtension = FALSE;

    pStack = sk_X509_EXTENSION_new_null();
    if (pStack == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_DIGITAL_SIGNATURE))
    {
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_DIGITAL_SIGNATURE);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_NON_REPUDIATION))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_NON_REPUDIATION);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_KEY_ENCIPHERMENT))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_KEY_ENCIPHERMENT);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_DATA_ENCIPHERMENT))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_DATA_ENCIPHERMENT);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_KEY_AGREEMENT))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_KEY_AGREEMENT);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_KEY_CERT_SIGN))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_KEY_CERT_SIGN);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_KEY_CRL_SIGN))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_CRL_SIGN);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_ENCIPHER_ONLY))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_ENCIPHER_ONLY);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints, LWCA_DECIPHER_ONLY))
    {
        if (bExtension)
        {
            dwError = LwCAStringCatA(extensionString, sizeof(extensionString), ", ");
            BAIL_ON_LWCA_ERROR(dwError);
        }
        dwError = LwCAStringCatA(extensionString, sizeof(extensionString), LWCA_CERT_EXTENSION_DECIPHER_ONLY);
        BAIL_ON_LWCA_ERROR(dwError);
        bExtension = TRUE;
    }

    if (LwCAisBitSet(pCertRequest->dwKeyUsageConstraints , LWCA_KEY_CERT_SIGN))
    {
        dwError = _LwCAAddExtension(
                            pStack,
                            NULL,
                            NID_basic_constraints,
                            LWCA_CERT_EXTENSION_KEY_CERT_SIGN_NID_VALUE
                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (bExtension)
    {
        dwError = _LwCAAddExtension(
                            pStack,
                            NULL,
                            NID_key_usage,
                            extensionString
                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pEmailList)
    {
        dwError = _LwCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            LWCA_CERT_ALT_STRING_KEY_NAME_EMAIL,
                            pCertRequest->pEmailList
                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pIPAddressList)
    {
        dwError = _LwCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            LWCA_CERT_ALT_STRING_KEY_NAME_IP,
                            pCertRequest->pIPAddressList
                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pURIList)
    {
        dwError = _LwCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            LWCA_CERT_ALT_STRING_KEY_NAME_URI,
                            pCertRequest->pURIList
                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pDNSList)
    {
        dwError = _LwCAAppendAlternateNameString(
                            subAltName,
                            sizeof(subAltName),
                            LWCA_CERT_ALT_STRING_KEY_NAME_DNS,
                            pCertRequest->pDNSList
                            );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (LwCAStringLenA(subAltName) > 0)
    {
        dwError = _LwCAAddExtension(
                        pStack,
                        NULL,
                        NID_subject_alt_name,
                        subAltName
                        );
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppStack = pStack;

cleanup:
    return dwError;

error:
    if (pStack)
    {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    if (ppStack)
    {
        *ppStack = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAX509NameSetDC(
     X509_NAME  *pCertName,
     PSTR       pszDomainName
     )
{
    DWORD dwError = 0;
    PSTR pszToken = NULL;
    PSTR pszNextTok = NULL;
    PLWCA_STRING_ARRAY pRDNStrArray = NULL;
    BOOLEAN bIsValidDN = FALSE;

    // Accepts domainname format lightwave.local or dc=lightwave,dc=local

    dwError = LwCAIsValidDN(pszDomainName, &bIsValidDN);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bIsValidDN)
    {
        dwError = LwCADNToRDNArray(pszDomainName, TRUE, &pRDNStrArray);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAX509NameAddEntriesByTxt(pCertName, SN_domainComponent, pRDNStrArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        pszToken = LwCAStringTokA(pszDomainName, ".", &pszNextTok);
        while (pszToken)
        {
            dwError = _LwCAX509NameAddEntryByTxt(pCertName, SN_domainComponent, pszToken);
            BAIL_ON_LWCA_ERROR(dwError);

            pszToken = LwCAStringTokA(NULL, ".", &pszNextTok);
        }
    }

cleanup:
    LwCAFreeStringArray(pRDNStrArray);
    return dwError;

error:
    goto cleanup;
}
