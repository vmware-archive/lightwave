/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
_LwCAX509Init(
    X509    **ppCert
    );

static
DWORD
_LwCAX509ReqInit(
    X509_REQ    **ppReq
    );

static
DWORD
_LwCAX509GenerateSerial(
    ASN1_INTEGER **ppSerial
    );

static
DWORD
_LwCAX509GetSubjectNameRef(
    X509        *pCert,
    X509_NAME   **ppSubjectName
    );

static
DWORD
_LwCAX509ReqGetSubjectNameRef(
    X509_REQ    *pReq,
    X509_NAME   **ppSubjectName
    );

static
DWORD
_LwCAX509ReqGetVerifiedSubjectNameRef(
    X509_REQ    *pReq,
    X509_NAME   **ppSubjectName
    );

static
DWORD
_LwCAX509GetIssuerNameRef(
    X509        *pCert,
    X509_NAME   **ppIssuerName
    );

static
DWORD
_LwCAX509NameGetValues(
    X509_NAME               *pSubjName,
    DWORD                   dwNIDType,
    PLWCA_STRING_ARRAY      *ppszValues
    );

static
DWORD
_LwCAX509NameGetNIDIndex(
    X509_NAME   *pszSubjName,
    DWORD       dwNIDType,
    int         iPosIn,
    int         *pIPosOut
    );

static
DWORD
_LwCAX509NameGetEntryCount(
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
int
_LwCAPemPasswordCallback(
    PSTR    pszBuf,
    int     bufLen,
    int     flag,
    PVOID   pCtx
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
_LwCAPEMToPublicKey(
    PCSTR       pcszPublicKey,
    EVP_PKEY    **ppKey
    );

static
DWORD
_LwCAX509ReqSetCertificateName(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    X509_REQ                *pReq
    );

static
DWORD
_LwCAX509NameAddEntryByTxt(
    X509_NAME           *pSubjectName,
    PCSTR               pcszField,
    PCSTR               pcszEntry
    );

static
DWORD
_LwCAX509NameAddEntriesByTxt(
    X509_NAME           *pSubjectName,
    PCSTR               pcszField,
    PLWCA_STRING_ARRAY  pEntries
    );

static
DWORD
_LwCAX509NameSetDC(
     X509_NAME          *pSubjectName,
     PSTR               pszDomainName
     );

static
DWORD
_LwCAX509ExtensionsCreate(
    PLWCA_PKCS_10_REQ_DATA      pCertRequest,
    STACK_OF(X509_EXTENSION)    **ppStack
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
_LwCAAddExtensionsToX509Cert(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCertificate
    );

static
DWORD
_LwCAAddExtensionsToX509Crl(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509_CRL                    *pCrl
    );

static
DWORD
_LwCAAppendAlternateNameString(
    PSTR                pszDestinationString,
    size_t              destinationStringLen,
    PCSTR               pcszSourceType,
    PLWCA_STRING_ARRAY  pSource
    );

static
DWORD
_LwCAX509ReqSetSubjectKeyIdentifier(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509_REQ                    *pReq
    );

static
DWORD
_LwCAVerifyCertValidity(
    PLWCA_CERT_VALIDITY pValidity
    );

static
DWORD
_LwCAX509SetValidity(
    X509                *pCert,
    X509                *pIssuer,
    PLWCA_CERT_VALIDITY pValidity
    );

static
DWORD
_LwCASetAuthorityInfoAccess(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCert,
    X509                        *pIssuer
    );

static
DWORD
_LwCASetAuthorityKeyIdentifier(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCert,
    X509                        *pIssuer
    );

static
DWORD
_LwCAX509CrlSetAuthorityKeyIdentifier(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCACert,
    X509_CRL                    *pCrl
    );

static
DWORD
_LwCAX509ReqGetPublicKey(
    X509_REQ *pReq,
    EVP_PKEY **ppKey
    );

static
DWORD
_LwCAConvertASNIntegertoString(
    const ASN1_INTEGER  *pAsnInteger,
    PSTR                *ppszStr
    );

static
DWORD
_LwCAConvertStringToASNInteger(
    PCSTR           pcszStr,
    ASN1_INTEGER    **ppAsnInteger
    );

static
DWORD
_LwCAConvertASNTimeToGeneralizedTime(
    ASN1_TIME   *pAsnTime,
    PSTR        *ppszGeneralizedTime
    );

static
DWORD
_LWCACreateX509Revoked(
    PLWCA_DB_CERT_DATA  pCertData,
    X509_REVOKED        **ppRevoked
    );

static
DWORD
_LwCACreateX509Crl(
    X509        *pCACert,
    X509_CRL    **ppCrl
    );

static
DWORD
_LwCAX509CrlAddCerts(
    X509_CRL                    *pCrl,
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray
    );

static
DWORD
_LwCAX509CrlAddCert(
    X509_CRL            *pCrl,
    PLWCA_DB_CERT_DATA  pCertData
    );

static
DWORD
_LwCAX509CrlSort(
    X509_CRL *pCrl
    );

static
DWORD
_LwCAConvertTimeToGeneralizedTime(
    time_t  tm,
    PSTR    *ppszGeneralizedTime
    );

static
DWORD
_LwCAAsn1BitStringToDword(
    ASN1_BIT_STRING *pBitString,
    DWORD           *pdwOut
    );

static
DWORD
_LwCAPrivateKeyToPEM(
    EVP_PKEY    *pPrivateKey,
    PSTR        *ppszPrivateKey
    );

static
DWORD
_LwCAGetPublicKey(
    EVP_PKEY *pPrivateKey,
    PSTR *ppszPublicKey
    );

DWORD
LwCAX509GetSerialNumber(
    X509    *pCert,
    PSTR    *ppszSerialNumber
    )
{
    DWORD dwError = 0;
    PSTR pszSerialNumber = NULL;
    const ASN1_INTEGER *pSerial = NULL;

    if (!pCert || !ppszSerialNumber)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pSerial = X509_get_serialNumber(pCert);
    if (pSerial == NULL)
    {
        dwError =  LWCA_ERROR_INVALID_CERTIFICATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertASNIntegertoString(pSerial, &pszSerialNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszSerialNumber = pszSerialNumber;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszSerialNumber);
    if (ppszSerialNumber)
    {
        *ppszSerialNumber = NULL;
    }
    goto cleanup;
}

DWORD
LwCAX509GetTimeValidFrom(
    X509 *pCert,
    PSTR *ppszTimeValidFrom
    )
{
    DWORD dwError = 0;
    PSTR pszTimeValidFrom = NULL;
    ASN1_TIME *pAsnTime = NULL;

    if (!pCert || !ppszTimeValidFrom)
    {
        dwError =  LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // No need to free pAsnTime.
    // Releasing pCert will release pAsnTime.
    pAsnTime = X509_get_notBefore(pCert);
    if (pAsnTime == NULL)
    {
        dwError = LWCA_ERROR_INVALID_CERTIFICATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertASNTimeToGeneralizedTime(pAsnTime, &pszTimeValidFrom);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszTimeValidFrom = pszTimeValidFrom;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszTimeValidFrom);
    if (ppszTimeValidFrom)
    {
        *ppszTimeValidFrom = NULL;
    }
    goto cleanup;
}

DWORD
LwCAX509GetTimeValidTo(
    X509 *pCert,
    PSTR *ppszTimeValidTo
    )
{
    DWORD dwError = 0;
    PSTR pszTimeValidTo = NULL;
    ASN1_TIME *pAsnTime = NULL;

    if (!pCert || !ppszTimeValidTo)
    {
        dwError =  LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // No need to free pAsnTime.
    // Releasing pCert will release pAsnTime.
    pAsnTime = X509_get_notAfter(pCert);
    if (pAsnTime == NULL)
    {
        dwError = LWCA_ERROR_INVALID_CERTIFICATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertASNTimeToGeneralizedTime(pAsnTime, &pszTimeValidTo);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszTimeValidTo = pszTimeValidTo;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszTimeValidTo);
    if (ppszTimeValidTo)
    {
        *ppszTimeValidTo = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGenerateCertRevokedDate(
    PSTR    *ppszRevokedDate
    )
{
    DWORD dwError = 0;
    PSTR pszRevokedDate = NULL;
    ASN1_TIME *pAsnTime = NULL;
    time_t revokedDate;

    if (!ppszRevokedDate)
    {
        dwError =  LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    revokedDate = time(NULL);
    pAsnTime = ASN1_TIME_set(NULL, revokedDate);
    if (pAsnTime == NULL)
    {
        dwError = LWCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertASNTimeToGeneralizedTime(pAsnTime, &pszRevokedDate);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszRevokedDate = pszRevokedDate;

cleanup:
    if (pAsnTime)
    {
        ASN1_STRING_free(pAsnTime);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszRevokedDate);
    if (ppszRevokedDate)
    {
        *ppszRevokedDate = NULL;
    }
    goto cleanup;
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
    LwCAX509Free(pCert);
    if (ppX509Cert)
    {
        *ppX509Cert = NULL;
    }

    goto cleanup;
}

DWORD
LwCAPEMToX509Req(
    PCSTR       pcszCSR,
    X509_REQ    **ppX509CertReq
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    X509_REQ *pReq = NULL;

    if (IsNullOrEmptyString(pcszCSR) || !ppX509CertReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID)pcszCSR,-1);
    if (pBioMem == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pReq  = PEM_read_bio_X509_REQ(pBioMem, NULL, NULL, NULL);
    if (pReq == NULL)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppX509CertReq = pReq;

cleanup:
    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:
    LwCAX509ReqFree(pReq);
    if (ppX509CertReq)
    {
        *ppX509CertReq = NULL;
    }

    goto cleanup;
}

DWORD
LwCAX509ToPEM(
    X509*       pCert,
    PSTR*       ppszCertificate
    )
{
    DWORD dwError = 0;
    PSTR pszCert = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;

    if (!pCert || !ppszCertificate)
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

    dwError = PEM_write_bio_X509(pBioMem, pCert);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = LwCAAllocateStringWithLengthA(pBuffMem->data, pBuffMem->length - 1, &pszCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCertificate = pszCert;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCert);
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }

    goto cleanup;
}

DWORD
LwCAX509ReqToPEM(
    X509_REQ    *pReq,
    PSTR        *ppszRequest
    )
{
    DWORD dwError = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    PSTR pRequest = NULL;

    if (!pReq || !ppszRequest)
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

    dwError = PEM_write_bio_X509_REQ(pBioMem, pReq);
    BAIL_ON_SSL_ERROR(dwError, LWCA_REQUEST_ERROR);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = LwCAAllocateStringWithLengthA(pBuffMem->data, pBuffMem->length - 1, &pRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszRequest = pRequest;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pRequest);
    if (ppszRequest)
    {
        *ppszRequest = NULL;
    }

    goto cleanup;
}

DWORD
LwCAX509CrlToPEM(
    X509_CRL    *pCrl,
    PSTR        *ppszCrl
    )
{
    DWORD dwError = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    PSTR pszCrl = NULL;

    if (!pCrl || !ppszCrl)
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

    dwError = PEM_write_bio_X509_CRL(pBioMem, pCrl);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_DECODE_ERROR);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = LwCAAllocateStringWithLengthA(pBuffMem->data, pBuffMem->length - 1, &pszCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCrl = pszCrl;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCrl);
    if (ppszCrl)
    {
        *ppszCrl = NULL;
    }
    goto cleanup;
}

VOID
LwCAX509Free(
    X509        *pX509
    )
{
    if (pX509)
    {
        X509_free(pX509);
    }
}

VOID
LwCAX509ReqFree(
    X509_REQ    *pX509Req
    )
{
    if (pX509Req)
    {
        X509_REQ_free(pX509Req);
    }
}

VOID
LwCAX509CrlFree(
    X509_CRL    *pX509Crl
    )
{
    if (pX509Crl)
    {
        X509_CRL_free(pX509Crl);
    }
}

DWORD
LwCAX509GetSubjectName(
    X509        *pCert,
    PSTR        *ppszSubjectName
    )
{
    DWORD dwError = 0;
    X509_NAME *pSubjectName = NULL;
    PSTR pszSubjectName = NULL;
    PSTR pszTempSubjectName = NULL;

    if (!pCert || !ppszSubjectName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509GetSubjectNameRef(pCert, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    pszSubjectName = X509_NAME_oneline(pSubjectName, NULL , 0);
    if (pszSubjectName == NULL)
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
LwCAX509ReqGetSubjectName(
    X509_REQ    *pReq,
    PSTR        *ppszSubjectName
    )
{
    DWORD dwError = 0;
    X509_NAME *pSubjectName = NULL;
    PSTR pszSubjectName = NULL;
    PSTR pszTempSubjectName = NULL;

    if (!pReq || !ppszSubjectName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509ReqGetSubjectNameRef(pReq, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    pszSubjectName = X509_NAME_oneline(pSubjectName, NULL , 0);
    if (pszSubjectName == NULL)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pszSubjectName, &pszTempSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszSubjectName = pszTempSubjectName;

cleanup:
    LWCA_SAFE_FREE_MEMORY(pszSubjectName);
    return dwError;

error:
    LWCA_SAFE_FREE_MEMORY(pszTempSubjectName);
    if (ppszSubjectName)
    {
        *ppszSubjectName = NULL;
    }

    goto cleanup;
}

DWORD
LwCAX509GetCommonName(
    X509        *pCert,
    PSTR        *ppszCommonName
    )
{
    DWORD                   dwError = 0;
    X509_NAME               *pSubjectName = NULL;
    PLWCA_STRING_ARRAY      pszCommonNamesArray = NULL;
    PSTR                    pszCommonName = NULL;

    if (!pCert || !ppszCommonName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509GetSubjectNameRef(pCert, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509NameGetValues(pSubjectName, NID_commonName, &pszCommonNamesArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pszCommonNamesArray->dwCount != 1)
    {
        dwError = LWCA_ERROR_INVALID_CERTIFICATE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pszCommonNamesArray->ppData[0], &pszCommonName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCommonName = pszCommonName;

cleanup:
    LwCAFreeStringArray(pszCommonNamesArray);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCommonName);
    goto cleanup;
}

DWORD
LwCAX509ReqGetCommonName(
    X509_REQ    *pReq,
    PSTR        *ppszCommonName
    )
{
    DWORD                   dwError = 0;
    X509_NAME               *pSubjectName = NULL;
    PLWCA_STRING_ARRAY      pszCommonNamesArray = NULL;
    PSTR                    pszCommonName = NULL;

    if (!pReq || !ppszCommonName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509ReqGetSubjectNameRef(pReq, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509NameGetValues(pSubjectName, NID_commonName, &pszCommonNamesArray);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pszCommonNamesArray->dwCount != 1)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pszCommonNamesArray->ppData[0], &pszCommonName);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCommonName = pszCommonName;

cleanup:
    LwCAFreeStringArray(pszCommonNamesArray);
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCommonName);
    goto cleanup;
}

DWORD
LwCAX509GetIssuerName(
    X509        *pCert,
    PSTR        *ppszIssuerName
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

    dwError = _LwCAX509GetIssuerNameRef(pCert, &pIssuerName);
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
LwCAX509GetIssuerCommonName(
    X509                    *pCert,
    PSTR                    *ppszIssuerCN
    )
{
    DWORD                   dwError = 0;
    X509_NAME               *pIssuer = NULL;
    PLWCA_STRING_ARRAY      pIssuerEntries = NULL;
    PSTR                    pszIssuerCN = NULL;

    if (!pCert || !ppszIssuerCN)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = _LwCAX509GetIssuerNameRef(pCert, &pIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509NameGetValues(pIssuer, NID_commonName, &pIssuerEntries);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pIssuerEntries->dwCount != 1)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_CERTIFICATE);
    }

    dwError = LwCAAllocateStringA(pIssuerEntries->ppData[0], &pszIssuerCN);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszIssuerCN = pszIssuerCN;


cleanup:

    LwCAFreeStringArray(pIssuerEntries);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszIssuerCN);
    if (ppszIssuerCN)
    {
        *ppszIssuerCN = NULL;
    }

    goto cleanup;
}

DWORD
LwCAX509GetOrganizations(
    X509                *pCert,
    PLWCA_STRING_ARRAY  *ppOrgList
    )
{
    DWORD                   dwError = 0;
    X509_NAME               *pSubjectName = NULL;
    PLWCA_STRING_ARRAY      pOrgList = NULL;

    if (!pCert || !ppOrgList)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509GetSubjectNameRef(pCert, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509NameGetValues(pSubjectName, NID_organizationName, &pOrgList);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pOrgList->dwCount == 0)
    {
        LwCAFreeStringArray(pOrgList);
        pOrgList = NULL;
    }

    *ppOrgList = pOrgList;

cleanup:
    return dwError;

error:
    LwCAFreeStringArray(pOrgList);
    if (ppOrgList)
    {
        *ppOrgList = NULL;
    }
    goto cleanup;
}

DWORD
LwCAX509ReqGetSubjectAltNames(
    X509_REQ            *pReq,
    PLWCA_STRING_ARRAY  *ppSANArray
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwIdx = 0;
    DWORD                       dwIdx2 = 0;
    STACK_OF(X509_EXTENSION)    *pExts = NULL;
    GENERAL_NAMES               *pSANNames = NULL;
    const GENERAL_NAME          *pCurrentName = NULL;
    size_t                      currentNameLen = 0;
    PSTR                        pszSANName = NULL;
    DWORD                       dwNumSANEntries = 0;
    PSTR                        *ppszSANEntries = NULL;
    PLWCA_STRING_ARRAY          pSANArray = NULL;

    if (!pReq || !ppSANArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pExts = X509_REQ_get_extensions(pReq);
    if (pExts == NULL)
    {
        dwError = LWCA_SSL_NO_EXTENSIONS;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pSANNames = X509V3_get_d2i(pExts, NID_subject_alt_name, NULL, NULL);
    if (pSANNames == NULL)
    {
        dwError = LWCA_SSL_NO_EXTENSIONS;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwNumSANEntries = sk_GENERAL_NAME_num(pSANNames);
    if (dwNumSANEntries == 0)
    {
        dwError = LWCA_SSL_NO_EXTENSIONS;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(PSTR) * dwNumSANEntries, (PVOID*)&ppszSANEntries);
    BAIL_ON_LWCA_ERROR(dwError);

    for ( ; dwIdx < dwNumSANEntries ; ++dwIdx )
    {
        pCurrentName = sk_GENERAL_NAME_value(pSANNames, dwIdx);

        if (pCurrentName->type == GEN_DNS)
        {
            currentNameLen = ASN1_STRING_to_UTF8(
                                (unsigned char **)&pszSANName,
                                (ASN1_IA5STRING *)pCurrentName->d.ptr);

            if (!pszSANName || currentNameLen != LwCAStringLenA(pszSANName))
            {
                dwError = LWCA_CERT_DECODE_FAILURE;
                BAIL_ON_LWCA_ERROR(dwError);
            }

            dwError = LwCAAllocateStringA(pszSANName, &ppszSANEntries[dwIdx2++]);
            BAIL_ON_LWCA_ERROR(dwError);
        }
        else if (pCurrentName->type == GEN_IPADD)
        {
            if (((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->length != 4)
            {
                dwError = LWCA_ERROR_SAN_IPADDR_INVALID;
                BAIL_ON_LWCA_ERROR(dwError);
            }

            dwError = LwCAAllocateStringPrintfA(
                        &pszSANName,
                        "%u.%u.%u.%u",
                        ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[0],
                        ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[1],
                        ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[2],
                        ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[3]);
            BAIL_ON_LWCA_ERROR(dwError);

            dwError = LwCAAllocateStringA(pszSANName, &ppszSANEntries[dwIdx2++]);
            BAIL_ON_LWCA_ERROR(dwError);
        }

        // Skipping parsing of email address and uri

        if (pszSANName)
        {
            OPENSSL_free(pszSANName);
            pszSANName = NULL;
        }
    }

    if (dwIdx2 != 0)
    {
        dwError = LwCACreateStringArray(ppszSANEntries, dwIdx2, &pSANArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppSANArray = pSANArray;

cleanup:
    if (pszSANName)
    {
        OPENSSL_free(pszSANName);
    }
    if (pSANNames)
    {
        sk_GENERAL_NAME_pop_free(pSANNames, GENERAL_NAME_free);
    }
    if (pExts)
    {
        sk_X509_EXTENSION_pop_free(pExts, X509_EXTENSION_free);
    }
    LwCAFreeStringArrayA(ppszSANEntries, dwNumSANEntries);

    return dwError;

error:
    if (dwError == LWCA_SSL_NO_EXTENSIONS)
    {
        dwError = 0;
    }
    LwCAFreeStringArray(pSANArray);
    if (ppSANArray)
    {
        *ppSANArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCAX509ReqGetKeyUsage(
    X509_REQ    *pReq,
    DWORD       *pdwKeyUsage
    )
{
    DWORD dwError = 0;
    STACK_OF(X509_EXTENSION) *pExts = NULL;
    ASN1_BIT_STRING *pKeyUsageBitString = NULL;
    DWORD dwKeyUsage = 0;

    if (!pReq || !pdwKeyUsage)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pExts = X509_REQ_get_extensions(pReq);
    if (pExts == NULL)
    {
        dwError = LWCA_SSL_NO_EXTENSIONS;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pKeyUsageBitString = X509V3_get_d2i(pExts, NID_key_usage, NULL, NULL);
    if (pKeyUsageBitString == NULL)
    {
        dwError = LWCA_SSL_NO_EXTENSIONS;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAAsn1BitStringToDword(pKeyUsageBitString, &dwKeyUsage);
    BAIL_ON_LWCA_ERROR(dwError);

    *pdwKeyUsage = dwKeyUsage;

cleanup:
    if (pKeyUsageBitString)
    {
        ASN1_BIT_STRING_free(pKeyUsageBitString);
    }
    if (pExts)
    {
        sk_X509_EXTENSION_pop_free(pExts, X509_EXTENSION_free);
    }

    return dwError;

error:
    if (dwError == LWCA_SSL_NO_EXTENSIONS)
    {
        dwError = 0;
    }
    if (pdwKeyUsage)
    {
        *pdwKeyUsage = 0;
    }

    goto cleanup;
}

DWORD
LwCAGetLatestCertificateFromArray(
    PLWCA_CERTIFICATE_ARRAY     pCerts,
    PLWCA_CERTIFICATE           *ppCert
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwIdx = 0;
    DWORD                       dwCurrentIdx = 0;
    int                         nDays = 0;
    int                         nSecs = 0;
    X509                        *pX509CurrentCert = NULL;
    X509                        *pX509TmpCert = NULL;
    ASN1_TIME                   *pASN1CurrentTime = NULL;
    ASN1_TIME                   *pASN1TmpTime = NULL;
    PLWCA_CERTIFICATE           pCert = NULL;

    if (!pCerts || !ppCert || pCerts->dwCount < 1)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (pCerts->dwCount == 1)
    {
        dwError = LwCACreateCertificate(pCerts->ppCertificates[0], &pCert);
        BAIL_ON_LWCA_ERROR(dwError);

        goto cleanup;
    }

    dwError = LwCAPEMToX509(pCerts->ppCertificates[0], &pX509CurrentCert);
    BAIL_ON_LWCA_ERROR(dwError);

    pASN1CurrentTime = X509_get_notBefore(pX509CurrentCert);
    if (pASN1CurrentTime == NULL)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_CERTIFICATE);
    }

    for (dwIdx = 1; dwIdx < pCerts->dwCount; ++dwIdx)
    {
        dwError = LwCAPEMToX509(pCerts->ppCertificates[dwIdx], &pX509TmpCert);
        BAIL_ON_LWCA_ERROR(dwError);

        pASN1TmpTime = X509_get_notBefore(pX509TmpCert);
        if (pASN1TmpTime == NULL)
        {
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_CERTIFICATE);
        }

        if (!ASN1_TIME_diff(&nDays, &nSecs, pASN1TmpTime, pASN1CurrentTime))
        {
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_SSL_TIME_ERROR);
        }

        if (nDays > 0 || nSecs > 0)
        {
            LwCAX509Free(pX509CurrentCert);
            pX509CurrentCert = NULL;
            pASN1CurrentTime = NULL;

            dwCurrentIdx = dwIdx;
            pX509CurrentCert = pX509TmpCert;
            pASN1CurrentTime = pASN1TmpTime;

            pX509TmpCert = NULL;
            pASN1TmpTime = NULL;
        }
        else
        {
            // Either pASN1TmpTime is earlier than pASN1CurrentTime or
            // the two times are equal.  Then use the current currentTime
            // value.

            LwCAX509Free(pX509TmpCert);
            pX509TmpCert = NULL;
            pASN1TmpTime = NULL;
        }

        nDays = 0;
        nSecs = 0;
    }

    dwError = LwCACreateCertificate(pCerts->ppCertificates[dwCurrentIdx], &pCert);
    BAIL_ON_LWCA_ERROR(dwError);


cleanup:

    LwCAX509Free(pX509CurrentCert);
    LwCAX509Free(pX509TmpCert);

    if (ppCert)
    {
        *ppCert = pCert;
    }

    return dwError;

error:

    LwCAFreeCertificate(pCert);
    pCert = NULL;

    goto cleanup;
}

DWORD
LwCACreateCertificateSignRequest(
    PLWCA_PKCS_10_REQ_DATA  pCertRequest,
    PCSTR                   pcszPublicKey,
    X509_REQ                **ppReq
    )
{
    DWORD dwError = 0;
    X509_REQ *pReq = NULL;
    STACK_OF(X509_EXTENSION) *pStack = NULL;
    EVP_PKEY *pPublicKey = NULL;

    if (!pCertRequest || IsNullOrEmptyString(pcszPublicKey) || !ppReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509ReqInit(&pReq);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509ReqSetCertificateName(pCertRequest, pReq);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509ExtensionsCreate(pCertRequest, &pStack);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAPEMToPublicKey(pcszPublicKey, &pPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_REQ_set_pubkey(pReq, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_PUBKEY_ERR);

    dwError = _LwCAX509ReqSetSubjectKeyIdentifier(pStack, pReq);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_REQ_add_extensions(pReq, pStack);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_ADD_EXTENSION);

    *ppReq = pReq;

cleanup:
    if (pStack)
    {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    if (pPublicKey)
    {
        EVP_PKEY_free(pPublicKey);
    }
    return dwError;

error:
    LwCAX509ReqFree(pReq);
    if (ppReq)
    {
        *ppReq = NULL;
    }

    goto cleanup;
}

DWORD
LwCAGenerateX509Certificate(
    X509_REQ                *pRequest,
    PLWCA_CERT_VALIDITY     pValidity,
    PLWCA_CERTIFICATE       pCACert,
    X509                    **ppCert
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    X509 *pIssuer = NULL;
    EVP_PKEY *pPublicKey = NULL;
    X509_NAME *pSubjectName = NULL;
    X509_NAME *pIssuerSubjectName = NULL;
    STACK_OF(X509_EXTENSION) *pStack  = NULL;

    if (!pRequest || !pValidity || !pCACert || !ppCert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPEMToX509(pCACert, &pIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509ReqGetPublicKey(pRequest, &pPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509ReqGetVerifiedSubjectNameRef(pRequest, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509Init(&pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAVerifyCertValidity(pValidity);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509SetValidity(pCert, pIssuer, pValidity);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_set_subject_name(pCert, pSubjectName);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    dwError = _LwCAX509GetSubjectNameRef(pIssuer, &pIssuerSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_set_issuer_name(pCert, pIssuerSubjectName);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    dwError = X509_set_pubkey(pCert, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    pStack = X509_REQ_get_extensions(pRequest);
    if (!pStack)
    {
        pStack = sk_X509_EXTENSION_new_null();
        if (pStack == NULL)
        {
            dwError = LWCA_OUT_OF_MEMORY_ERROR;
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

    dwError = _LwCASetAuthorityKeyIdentifier(pStack, pCert, pIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCASetAuthorityInfoAccess(pStack, pCert, pIssuer);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAAddExtensionsToX509Cert(pStack, pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCert = pCert;

cleanup:
    if (pIssuer)
    {
        X509_free(pIssuer);
    }
    if (pPublicKey)
    {
        EVP_PKEY_free(pPublicKey);
    }
    if (pStack)
    {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;

error:
    LwCAX509Free(pCert);
    if (ppCert)
    {
        *ppCert = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGenerateSelfSignX509Certificate(
    X509_REQ                *pRequest,
    PLWCA_CERT_VALIDITY     pValidity,
    X509                    **ppCert
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    EVP_PKEY *pPublicKey = NULL;
    X509_NAME *pSubjectName = NULL;
    STACK_OF(X509_EXTENSION) *pStack  = NULL;

    if (!pRequest || !pValidity || !ppCert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509ReqGetPublicKey(pRequest, &pPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509ReqGetVerifiedSubjectNameRef(pRequest, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509Init(&pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAVerifyCertValidity(pValidity);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!ASN1_TIME_set(X509_get_notBefore(pCert), pValidity->tmNotBefore))
    {
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_START_TIME);
    }

    if (!ASN1_TIME_set(X509_get_notAfter(pCert), pValidity->tmNotAfter))
    {
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_END_TIME);
    }

    dwError = X509_set_subject_name(pCert, pSubjectName);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    dwError = X509_set_issuer_name(pCert, pSubjectName);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    dwError = X509_set_pubkey(pCert, pPublicKey);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    pStack = X509_REQ_get_extensions(pRequest);
    if (!pStack)
    {
        pStack = sk_X509_EXTENSION_new_null();
        if (pStack == NULL)
        {
            dwError = LWCA_OUT_OF_MEMORY_ERROR;
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

    dwError = _LwCAAddExtensionsToX509Cert(pStack, pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCert = pCert;

cleanup:
    if (pPublicKey)
    {
        EVP_PKEY_free(pPublicKey);
    }
    if (pStack)
    {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;

error:
    if (pCert)
    {
        X509_free(pCert);
    }
    if (ppCert)
    {
        *ppCert = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGenerateX509Crl(
    PCSTR                       pcszCRLNumber,
    PCSTR                       pcszLastCRLUpdate,
    PCSTR                       pcszNextCRLUpdate,
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray,
    X509                        *pCACert,
    X509_CRL                    **ppCrl
    )
{
    DWORD dwError = 0;
    X509_CRL *pCrl = NULL;
    ASN1_INTEGER *pCrlNumber = NULL;
    ASN1_TIME *pAsnLastCRLUpdate = NULL;
    ASN1_TIME *pAsnNextCRLUpdate = NULL;
    STACK_OF(X509_EXTENSION) *pStack  = NULL;

    if (IsNullOrEmptyString(pcszCRLNumber) || IsNullOrEmptyString(pcszLastCRLUpdate)
        || IsNullOrEmptyString(pcszNextCRLUpdate) || !pCACert || !ppCrl)
    {
        dwError =  LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertStringToASNInteger(pcszCRLNumber, &pCrlNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError =  _LwCACreateX509Crl(pCACert, &pCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_CRL_add1_ext_i2d(pCrl, NID_crl_number, pCrlNumber, 0, 0);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_ERROR);

    if (pCertDataArray)
    {
        dwError = _LwCAX509CrlAddCerts(pCrl, pCertDataArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pAsnLastCRLUpdate = ASN1_TIME_new();
    if (pAsnLastCRLUpdate == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pAsnNextCRLUpdate = ASN1_TIME_new();
    if (pAsnNextCRLUpdate == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    ASN1_TIME_set_string(pAsnLastCRLUpdate, pcszLastCRLUpdate);
    ASN1_TIME_set_string(pAsnNextCRLUpdate, pcszNextCRLUpdate);

    dwError = X509_CRL_set_lastUpdate(pCrl, pAsnLastCRLUpdate);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_START_TIME);

    dwError = X509_CRL_set_nextUpdate(pCrl, pAsnNextCRLUpdate);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_END_TIME);

    pStack = sk_X509_EXTENSION_new_null();
    if (pStack == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509CrlSetAuthorityKeyIdentifier(pStack, pCACert, pCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAAddExtensionsToX509Crl(pStack, pCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAX509CrlSort(pCrl);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCrl = pCrl;

cleanup:
    if (pCrlNumber != NULL)
    {
        ASN1_INTEGER_free(pCrlNumber);
    }
    if (pAsnLastCRLUpdate != NULL)
    {
        ASN1_STRING_free(pAsnLastCRLUpdate);
    }
    if (pAsnNextCRLUpdate != NULL)
    {
        ASN1_STRING_free(pAsnNextCRLUpdate);
    }
    if (pStack != NULL)
    {
        sk_X509_EXTENSION_pop_free(pStack, X509_EXTENSION_free);
    }
    return dwError;

error:
    LwCAX509CrlFree(pCrl);
    if (ppCrl)
    {
        *ppCrl = NULL;
    }
    goto cleanup;
}

DWORD
LwCAX509SignCertificate(
    X509        *pCert,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    )
{
    DWORD dwError = 0;
    RSA *pRsa = NULL;
    EVP_PKEY *pKey = NULL;

    if (!pCert || IsNullOrEmptyString(pcszPrivateKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAPEMToPrivateKey(pcszPrivateKey, pcszPassPhrase, &pRsa);
    BAIL_ON_LWCA_ERROR(dwError);

    pKey = EVP_PKEY_new();
    if (pKey == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    EVP_PKEY_assign_RSA(pKey, pRsa);

    dwError = X509_sign(pCert, pKey, EVP_sha256());
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_REQ_SIGN_ERR);

cleanup:
    if(pKey)
    {
        EVP_PKEY_free(pKey); // will free RSA too
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAX509ReqSignRequest(
    X509_REQ    *pReq,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    )
{
    DWORD dwError = 0;
    RSA *pRsa = NULL;
    EVP_PKEY *pKey = NULL;

    if (!pReq || IsNullOrEmptyString(pcszPrivateKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAPEMToPrivateKey(pcszPrivateKey, pcszPassPhrase, &pRsa);
    BAIL_ON_LWCA_ERROR(dwError);

    pKey = EVP_PKEY_new();
    if (pKey == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    EVP_PKEY_assign_RSA(pKey, pRsa);

    dwError = X509_REQ_sign(pReq, pKey, EVP_sha256());
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_REQ_SIGN_ERR);

cleanup:
    if(pKey)
    {
        EVP_PKEY_free(pKey); // will free RSA too
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAX509CrlSign(
    X509_CRL    *pCrl,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
    )
{
    DWORD dwError = 0;
    RSA *pRsa = NULL;
    EVP_PKEY *pKey = NULL;

    if (!pCrl || IsNullOrEmptyString(pcszPrivateKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAPEMToPrivateKey(pcszPrivateKey, pcszPassPhrase, &pRsa);
    BAIL_ON_LWCA_ERROR(dwError);

    pKey = EVP_PKEY_new();
    if (pKey == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    EVP_PKEY_assign_RSA(pKey, pRsa);

    dwError = X509_CRL_sign(pCrl, pKey, EVP_sha256());
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_SIGN_FAIL);

cleanup:
    if(pKey)
    {
        EVP_PKEY_free(pKey); // will free RSA too
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCAX509ValidateCertificate(
    X509        *pCert,
    PCSTR       pcszPrivateKey,
    PCSTR       pcszPassPhrase
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
LwCAX509VerifyCertIssuer(
    X509        *pIssuerCert,
    X509        *pCert,
    PBOOLEAN    pbVerified
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bVerified = FALSE;

    if (!pIssuerCert || !pCert || !pbVerified)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (!X509_check_issued(pIssuerCert, pCert))
    {
        LWCA_LOG_ERROR("cannot verify issuer of certificate");
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_CERT_VERIFY_ERR);
    }

    bVerified = TRUE;
    *pbVerified = bVerified;


cleanup:

    return dwError;

error:

    if (pbVerified)
    {
        *pbVerified = FALSE;
    }

    goto cleanup;
}

DWORD
LwCAVerifyCertificateSign(
    X509    *pCert,
    X509    *pCACert
    )
{
    DWORD dwError = 0;
    EVP_PKEY *pKey = NULL;

    if ((pKey = X509_get_pubkey(pCACert)) == NULL )
    {
        LWCA_LOG_ERROR("Certificate does not have a public key");
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = X509_verify(pCert, pKey);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_CERT_VERIFY_ERR);

error:
    if (pKey != NULL)
    {
        EVP_PKEY_free(pKey);
    }
    return dwError;
}

/*
*  Verifies certficate against CA chain cert.
*  Input Parameters:
*    pCACerts - CA certificate chain
*    pCertificate - Certificate which needs to be verified
*/
DWORD
LwCAVerifyCertificate(
    PLWCA_CERTIFICATE_ARRAY pCACertChain,
    PLWCA_CERTIFICATE       pCertificate
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;
    X509 *pCACert = NULL;
    X509 *pCert = NULL;
    X509_STORE *pStore = NULL;
    X509_STORE_CTX  *pStoreCtx = NULL;

    if (!pCACertChain || !pCertificate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pStore = X509_STORE_new();
    if (pStore == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pStoreCtx = X509_STORE_CTX_new();
    if (pStoreCtx == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    for (iEntry=0; iEntry < pCACertChain->dwCount; ++iEntry)
    {
        dwError = LwCAPEMToX509(pCACertChain->ppCertificates[iEntry], &pCACert);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = X509_STORE_add_cert(pStore, pCACert);
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_STORE_ADD_CERT_FAIL);

        if (pCACert)
        {
            X509_free(pCACert);
            pCACert = NULL;
        }
    }

    dwError = LwCAPEMToX509(pCertificate, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_STORE_CTX_init(pStoreCtx, pStore, pCert, NULL);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_STORE_CTX_INIT_FAIL);

    if (!X509_verify_cert(pStoreCtx))
    {
        LWCA_LOG_INFO("Cert verification error: %s",
                        X509_verify_cert_error_string(pStoreCtx->error));

        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_CERT_VERIFY_ERR);
    }

error:
    if (pCACert)
    {
        X509_free(pCACert);
    }
    if (pCert)
    {
        X509_free(pCert);
    }
    if (pStoreCtx)
    {
        X509_STORE_CTX_free(pStoreCtx);
    }
    if (pStore)
    {
        X509_STORE_free(pStore);
    }
    return dwError;
}

DWORD
LwCAX509CheckIfCACert(
    X509     *pCert,
    PBOOLEAN pbIsCA
    )
{
    DWORD dwError = 0;

    if (!pCert || !pbIsCA)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (X509_check_ca(pCert))
    {
        *pbIsCA = TRUE;
    }
    else
    {
        *pbIsCA = FALSE;
    }

cleanup:
    return dwError;

error:
    if (pbIsCA)
    {
        *pbIsCA = FALSE;
    }
    goto cleanup;
}

DWORD
LwCAGetNextCrlNumber(
    PCSTR   pcszCRLNumber,
    PSTR    *ppszNextCRLNumber
    )
{
    DWORD dwError = 0;
    ASN1_INTEGER *pCrlNumber = NULL;
    PSTR pszNextCRLNumber = NULL;
    long nCrlNum = 0;

    if (IsNullOrEmptyString(pcszCRLNumber) || !ppszNextCRLNumber)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertStringToASNInteger(pcszCRLNumber, &pCrlNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    nCrlNum = ASN1_INTEGER_get(pCrlNumber);
    nCrlNum++;

    dwError = ASN1_INTEGER_set(pCrlNumber, nCrlNum);
    BAIL_ON_SSL_ERROR(dwError, LWCA_OUT_OF_MEMORY_ERROR);

    dwError = _LwCAConvertASNIntegertoString(pCrlNumber, &pszNextCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszNextCRLNumber = pszNextCRLNumber;

cleanup:
    if (pCrlNumber)
    {
        ASN1_INTEGER_free(pCrlNumber);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszNextCRLNumber);
    if (ppszNextCRLNumber)
    {
        *ppszNextCRLNumber = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGenerateCRLNumber(
    PSTR *ppszCRlNumber
    )
{
    DWORD dwError = 0;
    PSTR pszCRLNumber = NULL;
    ASN1_INTEGER *pSerial = NULL;

    if (!ppszCRlNumber)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509GenerateSerial(&pSerial);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAConvertASNIntegertoString(pSerial, &pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszCRlNumber = pszCRLNumber;

cleanup:
    if (pSerial)
    {
        ASN1_INTEGER_free(pSerial);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
    if (ppszCRlNumber)
    {
        *ppszCRlNumber = NULL;
    }
    goto cleanup;
}

DWORD
LwCAGenerateCrlTimestamps(
    time_t  tmCrlValidity,
    PSTR    *ppszLastCRLUpdate,
    PSTR    *ppszNextCRLUpdate
    )
{
    DWORD dwError = 0;
    PSTR pszLastCRLUpdate = NULL;
    PSTR pszNextCRLUpdate = NULL;
    time_t tmLastCRLUpdate;
    time_t tmNextCRLUpdate;

    if (!ppszLastCRLUpdate || !ppszNextCRLUpdate)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    tmLastCRLUpdate = time(NULL);
    tmNextCRLUpdate = tmLastCRLUpdate + tmCrlValidity;

    dwError = _LwCAConvertTimeToGeneralizedTime(tmLastCRLUpdate, &pszLastCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAConvertTimeToGeneralizedTime(tmNextCRLUpdate, &pszNextCRLUpdate);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszLastCRLUpdate = pszLastCRLUpdate;
    *ppszNextCRLUpdate = pszNextCRLUpdate;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszLastCRLUpdate);
    LWCA_SAFE_FREE_STRINGA(pszNextCRLUpdate);
    if (ppszLastCRLUpdate)
    {
        *ppszLastCRLUpdate = NULL;
    }
    if (ppszNextCRLUpdate)
    {
        *ppszNextCRLUpdate = NULL;
    }
    goto cleanup;
}

DWORD
LwCACreateKeyPair(
    size_t nKeyLength,
    PSTR *ppszPrivateKey,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;
    EVP_PKEY *pPrivateKey = NULL;
    RSA *pRSA = NULL;
    BIGNUM *pBigNum = NULL;
    PSTR pszPrivateKey = NULL;
    PSTR pszPublicKey = NULL;

    if ((nKeyLength < 1024) || (nKeyLength > (16 * 1024)))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPrivateKey = EVP_PKEY_new();
    if (!pPrivateKey)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRSA = RSA_new();
    if ( pRSA == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pBigNum = BN_new();
    if ( pBigNum == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!BN_set_word(pBigNum, RSA_F4))
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (RSA_generate_key_ex(pRSA, nKeyLength, pBigNum, NULL) <= 0)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!EVP_PKEY_assign_RSA(pPrivateKey, pRSA))
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRSA = NULL; /* pPrivateKey free will free pRSA */

    dwError = _LwCAPrivateKeyToPEM(pPrivateKey, &pszPrivateKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAGetPublicKey(pPrivateKey, &pszPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszPrivateKey = pszPrivateKey;
    *ppszPublicKey = pszPublicKey;

cleanup:
    if (pBigNum != NULL)
    {
        BN_free(pBigNum);
    }

    if (pPrivateKey != NULL)
    {
        EVP_PKEY_free(pPrivateKey);
    }

    if (pRSA)
    {
        RSA_free(pRSA);
    }
    return dwError;

error:
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPrivateKey, LwCAStringLenA(pszPrivateKey));
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPublicKey, LwCAStringLenA(pszPublicKey));
    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }
    if (ppszPublicKey)
    {
        *ppszPublicKey = NULL;
    }
    goto cleanup;
}

DWORD
LwCACreatePKCSRequest(
    PCSTR                   pcszName,
    PCSTR                   pcszDomainName,
    PLWCA_STRING_ARRAY      pCountryList,
    PLWCA_STRING_ARRAY      pLocalityList,
    PLWCA_STRING_ARRAY      pStateList,
    PLWCA_STRING_ARRAY      pOrganizationList,
    PLWCA_STRING_ARRAY      pOUList,
    PLWCA_STRING_ARRAY      pDNSList,
    PLWCA_STRING_ARRAY      pURIList,
    PLWCA_STRING_ARRAY      pEmailList,
    PLWCA_STRING_ARRAY      pIPAddressList,
    DWORD                   dwKeyUsageConstraints,
    PLWCA_PKCS_10_REQ_DATA  *ppPKCSRequest
    )
{
    DWORD dwError = 0;
    PLWCA_PKCS_10_REQ_DATA pPKCSRequest = NULL;

    if (!ppPKCSRequest)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError =  LwCAAllocateMemory(sizeof(LWCA_PKCS_10_REQ_DATA), (PVOID*)&pPKCSRequest);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsNullOrEmptyString(pcszName))
    {
        dwError = LwCAAllocateStringA(pcszName, &pPKCSRequest->pszName);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszDomainName))
    {
        dwError = LwCAAllocateStringA(pcszDomainName, &pPKCSRequest->pszDomainName);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCountryList)
    {
        dwError = LwCACopyStringArray(pCountryList, &pPKCSRequest->pCountryList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pLocalityList)
    {
        dwError = LwCACopyStringArray(pLocalityList, &pPKCSRequest->pLocalityList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pStateList)
    {
        dwError = LwCACopyStringArray(pStateList, &pPKCSRequest->pStateList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pOrganizationList)
    {
        dwError = LwCACopyStringArray(pOrganizationList, &pPKCSRequest->pOrganizationList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pOUList)
    {
        dwError = LwCACopyStringArray(pOUList, &pPKCSRequest->pOUList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pDNSList)
    {
        dwError = LwCACopyStringArray(pDNSList, &pPKCSRequest->pDNSList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pURIList)
    {
        dwError = LwCACopyStringArray(pURIList, &pPKCSRequest->pURIList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pEmailList)
    {
        dwError = LwCACopyStringArray(pEmailList, &pPKCSRequest->pEmailList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pIPAddressList)
    {
        dwError = LwCACopyStringArray(pIPAddressList, &pPKCSRequest->pIPAddressList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPKCSRequest->dwKeyUsageConstraints = dwKeyUsageConstraints;

    *ppPKCSRequest = pPKCSRequest;

cleanup:
    return dwError;

error:
    LwCAFreePKCSRequest(pPKCSRequest);
    if (ppPKCSRequest)
    {
        *ppPKCSRequest = NULL;
    }
    goto cleanup;
}

VOID
LwCAFreePKCSRequest(
    PLWCA_PKCS_10_REQ_DATA pPKCSRequest
    )
{
    if (pPKCSRequest)
    {
        LWCA_SAFE_FREE_STRINGA(pPKCSRequest->pszName);
        LWCA_SAFE_FREE_STRINGA(pPKCSRequest->pszDomainName);
        LwCAFreeStringArray(pPKCSRequest->pCountryList);
        LwCAFreeStringArray(pPKCSRequest->pLocalityList);
        LwCAFreeStringArray(pPKCSRequest->pStateList);
        LwCAFreeStringArray(pPKCSRequest->pOrganizationList);
        LwCAFreeStringArray(pPKCSRequest->pOUList);
        LwCAFreeStringArray(pPKCSRequest->pDNSList);
        LwCAFreeStringArray(pPKCSRequest->pURIList);
        LwCAFreeStringArray(pPKCSRequest->pEmailList);
        LwCAFreeStringArray(pPKCSRequest->pIPAddressList);
        LWCA_SAFE_FREE_MEMORY(pPKCSRequest);
    }
}

static
DWORD
_LwCAX509Init(
    X509    **ppCert
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    ASN1_INTEGER *pSerial = NULL;

    pCert = X509_new();
    if(pCert == NULL) {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // CA Supports only X509V3
    dwError = X509_set_version(pCert, 2);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    dwError = _LwCAX509GenerateSerial(&pSerial);
    BAIL_ON_LWCA_ERROR(dwError);

    X509_set_serialNumber(pCert, pSerial);

    *ppCert = pCert;

cleanup:
    if (pSerial)
    {
        ASN1_INTEGER_free(pSerial);
    }
    return dwError;

error:
    LwCAX509Free(pCert);
    if (ppCert)
    {
        *ppCert = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAX509ReqInit(
    X509_REQ    **ppReq
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
    LwCAX509ReqFree(pReq);
    if (ppReq)
    {
        *ppReq = NULL;
    }
    goto cleanup;
}

// This function creates the random serial numbers for certificates
static
DWORD
_LwCAX509GenerateSerial(
    ASN1_INTEGER **ppSerial
    )
{
    DWORD dwError = 0;
    #define RAND_BITS_SIZE 64
    BIGNUM *bn = NULL;
    ASN1_INTEGER *pSerial = NULL;

    pSerial = ASN1_INTEGER_new();
    if (pSerial == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    bn = BN_new();
    if (bn == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = BN_pseudo_rand(bn, RAND_BITS_SIZE, 1, 0);
    BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_RAND_ERR);

    BN_to_ASN1_INTEGER(bn, pSerial);

    *ppSerial = pSerial;

cleanup:
    if (bn != NULL)
    {
        BN_free(bn);
    }

    return dwError;

error:
    if (pSerial)
    {
        ASN1_INTEGER_free(pSerial);
    }
    if (ppSerial)
    {
        *ppSerial = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAX509GetSubjectNameRef(
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
_LwCAX509ReqGetSubjectNameRef(
    X509_REQ    *pReq,
    X509_NAME   **ppSubjectName
    )
{
    DWORD dwError = 0;
    X509_NAME *pSubjectName = NULL;

    pSubjectName = X509_REQ_get_subject_name(pReq);
    if (pSubjectName == NULL)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
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
_LwCAX509ReqGetVerifiedSubjectNameRef(
    X509_REQ    *pReq,
    X509_NAME   **ppSubjectName
    )
{
    DWORD dwError = 0;
    X509_NAME *pSubjectName = NULL;

    dwError = _LwCAX509ReqGetSubjectNameRef(pReq, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    if (X509_NAME_entry_count(pSubjectName) == 0)
    {
        dwError = LWCA_INVALID_CSR_FIELD;
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
_LwCAX509GetIssuerNameRef(
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
_LwCAX509NameGetValues(
    X509_NAME                       *pSubjName,
    DWORD                           dwNIDType,
    PLWCA_STRING_ARRAY              *ppszValues
    )
{
    DWORD                           dwError = 0;
    DWORD                           iEntry = 0;
    PLWCA_STRING_ARRAY              pszValues = NULL;
    PSTR                            pszValueString = NULL;
    X509_NAME_ENTRY                 *pEntry = NULL;
    int                             iPosIn = -1;
    int                             iPosOut = -1;
    size_t                          szNumDNs = 0;
    size_t                          szEntryLength = 0;

    dwError = _LwCAX509NameGetEntryCount(pSubjName, &szNumDNs);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*) &pszValues);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!szNumDNs)
    {
        dwError = LWCA_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(PSTR) * szNumDNs, (PVOID *) &pszValues->ppData);
    BAIL_ON_LWCA_ERROR(dwError);

    for (; iEntry < szNumDNs; ++iEntry)
    {
        dwError = _LwCAX509NameGetNIDIndex(pSubjName, dwNIDType, iPosIn, &iPosOut);
        BAIL_ON_LWCA_ERROR(dwError);
        if (iPosOut == -1)
        {
            break;
        }

        dwError = _LwCAX509NameGetEntry(pSubjName, iPosOut, &pEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAX509NameEntryGetData(pEntry, &pszValueString, &szEntryLength);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAAllocateStringA(pszValueString, &pszValues->ppData[pszValues->dwCount++]);
        BAIL_ON_LWCA_ERROR(dwError);

        if (pszValueString)
        {
            OPENSSL_free(pszValueString);
            pszValueString = NULL;
        }

        iPosIn = iPosOut;
    }

    *ppszValues = pszValues;

cleanup:
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
_LwCAX509NameGetNIDIndex(
    X509_NAME   *pszSubjName,
    DWORD       dwNIDType,
    int         iPosIn,
    int         *pIPosOut
    )
{
    DWORD       dwError = 0;
    int         iPosOut = 0;

    iPosOut = X509_NAME_get_index_by_NID(pszSubjName, dwNIDType, iPosIn);

    if (iPosOut == -2)
    {
        dwError = LWCA_SSL_INVALID_NID;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pIPosOut = iPosOut;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAX509NameGetEntryCount(
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
    X509_NAME       *pszSubjName,
    int             iPos,
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

    szEntryLength = ASN1_STRING_to_UTF8((PBYTE *)&pszValueString, pEntryAsn1);
    if (!pszValueString || szEntryLength != LwCAStringLenA(pszValueString))
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
_LwCAPEMToPublicKey(
    PCSTR       pcszPublicKey,
    EVP_PKEY    **ppKey
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    EVP_PKEY *pKey = NULL;

    pBioMem = BIO_new_mem_buf((PVOID) pcszPublicKey, -1);
    if (pBioMem == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pKey  = PEM_read_bio_PUBKEY(pBioMem, NULL, NULL, NULL);
    if (pKey  == NULL)
    {
        dwError = LWCA_KEY_IO_FAILURE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppKey = pKey;

cleanup:
    if (pBioMem)
    {
        BIO_free(pBioMem);
    }
    return dwError;

error:
    if (pKey)
    {
        EVP_PKEY_free(pKey);
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
    X509_NAME *pSubjectName = NULL;

    dwError = _LwCAX509ReqGetSubjectNameRef(pReq, &pSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pCertRequest->pszName)
    {
        dwError = _LwCAX509NameAddEntryByTxt(pSubjectName, SN_commonName, pCertRequest->pszName);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pszDomainName)
    {
        dwError = _LwCAX509NameSetDC(pSubjectName, pCertRequest->pszDomainName);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pCountryList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pSubjectName, SN_countryName, pCertRequest->pCountryList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pStateList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pSubjectName, SN_stateOrProvinceName, pCertRequest->pStateList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pLocalityList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pSubjectName, SN_localityName, pCertRequest->pLocalityList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pOrganizationList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pSubjectName, SN_organizationName, pCertRequest->pOrganizationList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pCertRequest->pOUList)
    {
        dwError = _LwCAX509NameAddEntriesByTxt(pSubjectName, SN_organizationalUnitName, pCertRequest->pOUList);
        BAIL_ON_LWCA_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_LwCAX509NameAddEntryByTxt(
    X509_NAME           *pSubjectName,
    PCSTR               pcszField,
    PCSTR               pcszEntry
    )
{
    DWORD dwError = 0;

    dwError = X509_NAME_add_entry_by_txt(pSubjectName, pcszField, MBSTRING_UTF8, pcszEntry, -1, -1, 0);
    ERR_print_errors_fp(stdout);
    BAIL_ON_SSL_ERROR(dwError, LWCA_INVALID_CSR_FIELD);

error:
    return dwError;
}

static
DWORD
_LwCAX509NameAddEntriesByTxt(
    X509_NAME           *pSubjectName,
    PCSTR               pcszField,
    PLWCA_STRING_ARRAY  pEntries
    )
{
    DWORD dwError = 0;
    DWORD iEntry = 0;

    for (; iEntry < pEntries->dwCount; ++iEntry)
    {
        dwError = _LwCAX509NameAddEntryByTxt(pSubjectName, pcszField, pEntries->ppData[iEntry]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_LwCAX509NameSetDC(
     X509_NAME          *pSubjectName,
     PSTR               pszDomainName
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

        dwError = _LwCAX509NameAddEntriesByTxt(pSubjectName, SN_domainComponent, pRDNStrArray);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        pszToken = LwCAStringTokA(pszDomainName, ".", &pszNextTok);
        while (pszToken)
        {
            dwError = _LwCAX509NameAddEntryByTxt(pSubjectName, SN_domainComponent, pszToken);
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

static
DWORD
_LwCAX509ExtensionsCreate(
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
_LwCAAddExtensionsToX509Cert(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCertificate
    )
{
    DWORD dwError = 0;
    X509_EXTENSION *pExtension = NULL;
    int extCount = 0;
    int nCounter = 0;

    extCount = sk_X509_EXTENSION_num(pStack);
    for (nCounter = 0; nCounter < extCount; nCounter ++)
    {
        pExtension = sk_X509_EXTENSION_value(pStack, nCounter);

        dwError = X509_add_ext(pCertificate, pExtension, -1);
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_EXT_ERR);
    }

error:
    return dwError;
}

static
DWORD
_LwCAAddExtensionsToX509Crl(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509_CRL                    *pCrl
    )
{
    DWORD dwError = 0;
    X509_EXTENSION *pExtension = NULL;
    int extCount = 0;
    int nCounter = 0;

    extCount = sk_X509_EXTENSION_num(pStack);
    for (nCounter = 0; nCounter < extCount; nCounter ++)
    {
        pExtension = sk_X509_EXTENSION_value(pStack, nCounter);

        dwError = X509_CRL_add_ext(pCrl, pExtension, -1);
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_EXT_ERR);
    }

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
    size_t nSourceSize = 0;
    size_t sizeStr =  0;

    if (!pszDestinationString || IsNullOrEmptyString(pcszSourceType) ||
        !pSource || destinationStringLen == 0)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    nCurrentSize = LwCAStringLenA(pszDestinationString);
    nSourceSize = LwCAStringLenA(pcszSourceType);

    for (; iEntry < pSource->dwCount; ++iEntry)
    {
        if (nCurrentSize != 0 )
        {
            dwError = LwCAStringCatA(pszDestinationString, destinationStringLen, ", ");
            BAIL_ON_LWCA_ERROR(dwError);
            nCurrentSize += 2;
        }

        sizeStr = nSourceSize + LwCAStringLenA(pSource->ppData[iEntry]);
        if ( (destinationStringLen - nCurrentSize) > sizeStr )
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
_LwCAX509ReqSetSubjectKeyIdentifier(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509_REQ                    *pReq
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, NULL, NULL, pReq, NULL, 0);

    dwError = _LwCAAddExtension(pStack,
                                &ctx,
                                NID_subject_key_identifier,
                                LWCA_CERT_EXTENSION_NID_KEY_IDENTIFIER);
    BAIL_ON_LWCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_LwCAVerifyCertValidity(
    PLWCA_CERT_VALIDITY pValidity
    )
{
    DWORD dwError = 0;
    time_t tmNow = 0;

    time(&tmNow);
    if (pValidity->tmNotBefore < (tmNow - LWCA_VALIDITY_SYNC_BACK_DATE))
    {
        LWCA_LOG_DEBUG("Invalid start date. Start date: %ld",
                        pValidity->tmNotBefore
                        );
        dwError = LWCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if ((pValidity->tmNotAfter - pValidity->tmNotBefore) < 0)
    {
        LWCA_LOG_DEBUG("Invalid validity period requested. \
                        Start date: %ld, End date:%ld",
                        pValidity->tmNotBefore,  pValidity->tmNotAfter
                        );
        dwError = LWCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if ((pValidity->tmNotAfter - pValidity->tmNotBefore) > LWCA_MAX_CERT_DURATION)
    {
        LWCA_LOG_DEBUG("Validity period exceeded max cert duration. \
                        Start date: %ld, End date:%ld",
                        pValidity->tmNotBefore,  pValidity->tmNotAfter
                        );
        dwError = LWCA_INVALID_TIME_SPECIFIED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_LwCAX509SetValidity(
    X509                *pCert,
    X509                *pIssuer,
    PLWCA_CERT_VALIDITY pValidity
    )
{
    DWORD dwError = 0;

    if (X509_cmp_time(X509_get_notBefore(pIssuer), &pValidity->tmNotBefore) >= 0)
    {
        LWCA_LOG_DEBUG("Invalid start date. CA certificate is not valid \
                        during the requested start date. \
                        Start date: %ld, CA start date:%s",
                        pValidity->tmNotBefore,  X509_get_notBefore(pIssuer)->data
                        );
        dwError = LWCA_SSL_SET_START_TIME;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!ASN1_TIME_set(X509_get_notBefore(pCert), pValidity->tmNotBefore))
    {
        BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_START_TIME);
    }

    // If the request is beyond CA cert validity use CA cert validity
    if (X509_cmp_time(X509_get_notAfter(pIssuer), &pValidity->tmNotAfter) <= 0)
    {
        if (!ASN1_TIME_set_string(X509_get_notAfter(pCert), X509_get_notAfter(pIssuer)->data))
        {
            BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_END_TIME);
        }
    }
    else
    {
        if (!ASN1_TIME_set(X509_get_notAfter(pCert), pValidity->tmNotAfter))
        {
            BAIL_ON_SSL_ERROR(dwError, LWCA_SSL_SET_END_TIME);
        }
    }

error:
    return dwError;
}

static
DWORD
_LwCASetAuthorityInfoAccess(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCert,
    X509                        *pIssuer
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;
    PSTR pszIPAddress = NULL;
    PSTR pszAIAString = NULL;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pIssuer, pCert, NULL, NULL, 0);

    //TODO: pszIPAddress must be set to CA address
    dwError = LwCAAllocateStringA("localhost", &pszIPAddress);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringPrintfA(
                                &pszAIAString,
                                "caIssuers;URI:https://%s/afd/vecs/ssl",
                                pszIPAddress);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCAAddExtension(pStack, &ctx, NID_info_access, pszAIAString);
    BAIL_ON_LWCA_ERROR(dwError);

error:
    LWCA_SAFE_FREE_MEMORY(pszIPAddress);
    LWCA_SAFE_FREE_MEMORY(pszAIAString);
    return dwError;
}

static
DWORD
_LwCASetAuthorityKeyIdentifier(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCert,
    X509                        *pIssuer
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pIssuer, pCert, NULL, NULL, 0);

    dwError = _LwCAAddExtension(pStack,
                                &ctx,
                                NID_authority_key_identifier,
                                LWCA_CERT_EXTENSION_NID_AUTHORITY_KEY_IDENTIFIER
                                );
    BAIL_ON_LWCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_LwCAX509CrlSetAuthorityKeyIdentifier(
    STACK_OF(X509_EXTENSION)    *pStack,
    X509                        *pCACert,
    X509_CRL                    *pCrl
    )
{
    DWORD dwError = 0;
    X509V3_CTX ctx;

    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, pCACert, NULL, NULL, pCrl, 0);

    dwError = _LwCAAddExtension(pStack,
                                &ctx,
                                NID_authority_key_identifier,
                                LWCA_CERT_EXTENSION_NID_AUTHORITY_KEY_IDENTIFIER
                                );
    BAIL_ON_LWCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_LwCAX509ReqGetPublicKey(
    X509_REQ *pReq,
    EVP_PKEY **ppKey
    )
{
    DWORD dwError = 0;
    EVP_PKEY *pKey = NULL;

    if ((pKey = X509_REQ_get_pubkey(pReq)) == NULL )
    {
        LWCA_LOG_INFO("CSR does not have a public key");
        dwError = LWCA_INVALID_CSR_FIELD;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pKey->type != EVP_PKEY_RSA ||
        BN_num_bits(pKey->pkey.rsa->n) < LWCA_MIN_CA_CERT_PRIV_KEY_LENGTH)
    {
        LWCA_LOG_INFO("Key length not supported");
        dwError = LWCA_ERROR_INVALID_KEY_LENGTH;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = X509_REQ_verify(pReq, pKey);
    BAIL_ON_SSL_ERROR(dwError, LWCA_INVALID_CSR_FIELD);

    *ppKey = pKey;

cleanup:
    return dwError;

error:
    if (pKey != NULL)
    {
        EVP_PKEY_free(pKey);
    }
    if (ppKey)
    {
        *ppKey = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAConvertASNIntegertoString(
    const ASN1_INTEGER  *pAsnInteger,
    PSTR                *ppszStr
    )
{
    DWORD dwError = 0;
    BIGNUM *bn = NULL;
    PSTR pszTempStr = NULL;
    PSTR pszStr = NULL;

    bn = BN_new();
    if (bn == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    ASN1_INTEGER_to_BN(pAsnInteger, bn);

    pszTempStr = BN_bn2dec(bn);
    if (IsNullOrEmptyString(pszTempStr))
    {
        dwError =  LWCA_ERROR_INVALID_DATA;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pszTempStr, &pszStr);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    if (bn)
    {
        BN_free(bn);
    }
    if (pszTempStr)
    {
        OPENSSL_free(pszTempStr);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszStr);
    if (ppszStr)
    {
        *ppszStr = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAConvertStringToASNInteger(
    PCSTR           pcszStr,
    ASN1_INTEGER    **ppAsnInteger
    )
{
    DWORD dwError = 0;
    BIGNUM *bn = NULL;
    ASN1_INTEGER *pAsnInteger = NULL;

    bn = BN_new();
    if (bn == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = BN_dec2bn(&bn, pcszStr);
    BAIL_ON_SSL_ERROR(dwError, LWCA_ERROR_INVALID_DATA);

    pAsnInteger = ASN1_INTEGER_new();
    if (pAsnInteger == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    BN_to_ASN1_INTEGER(bn, pAsnInteger);

    *ppAsnInteger = pAsnInteger;

cleanup:
    if (bn)
    {
        BN_free(bn);
    }
    return dwError;

error:
    if (pAsnInteger)
    {
        ASN1_INTEGER_free(pAsnInteger);
    }
    if (ppAsnInteger)
    {
        *ppAsnInteger = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAConvertASNTimeToGeneralizedTime(
    ASN1_TIME   *pAsnTime,
    PSTR        *ppszGeneralizedTime
    )
{
    DWORD dwError = 0;
    ASN1_GENERALIZEDTIME *pAsnGTm = NULL;
    PSTR pszGeneralizedTime = NULL;

    pAsnGTm = ASN1_TIME_to_generalizedtime(pAsnTime, NULL);
    if (pAsnGTm == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(pAsnGTm->data, &pszGeneralizedTime);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszGeneralizedTime = pszGeneralizedTime;

cleanup:
    if (pAsnGTm)
    {
        ASN1_STRING_free(pAsnGTm);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszGeneralizedTime);
    if (ppszGeneralizedTime)
    {
        *ppszGeneralizedTime = NULL;
    }

    goto cleanup;
}

static
DWORD
_LWCACreateX509Revoked(
    PLWCA_DB_CERT_DATA  pCertData,
    X509_REVOKED        **ppRevoked
    )
{
    DWORD dwError = 0;
    X509_REVOKED *pRevoked = NULL;
    ASN1_TIME *pRevTime = NULL;
    ASN1_ENUMERATED *pCode = NULL;
    ASN1_INTEGER *pSerial = NULL;

    pCode = ASN1_ENUMERATED_new();
    if (pCode == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRevoked = X509_REVOKED_new();
    if (pRevoked == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRevTime = ASN1_TIME_new();
    if (pRevTime == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAConvertStringToASNInteger(pCertData->pszSerialNumber, &pSerial);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_REVOKED_set_serialNumber(pRevoked, pSerial);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_SET_SERIAL_FAIL);

    ASN1_TIME_set_string(pRevTime, pCertData->pszRevokedDate);

    dwError = X509_REVOKED_set_revocationDate(pRevoked, pRevTime);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_SET_TIME_FAIL);

    ASN1_ENUMERATED_set(pCode, pCertData->revokedReason);

    dwError = X509_REVOKED_add1_ext_i2d(pRevoked, NID_crl_reason, pCode, 0, 0);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_REASON_FAIL);

    *ppRevoked = pRevoked;

cleanup:
    if (pRevTime != NULL)
    {
        ASN1_STRING_free(pRevTime);
    }
    if (pCode != NULL)
    {
        ASN1_ENUMERATED_free(pCode);
    }
    if (pSerial != NULL)
    {
        ASN1_INTEGER_free(pSerial);
    }
    return dwError;

error:
    if (pRevoked != NULL)
    {
        X509_REVOKED_free(pRevoked);
    }
    if (ppRevoked)
    {
        *ppRevoked = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCACreateX509Crl(
    X509        *pCACert,
    X509_CRL    **ppCrl
    )
{
    DWORD dwError = 0;
    X509_CRL *pCrl = NULL;
    X509_NAME *pCertName = NULL;

    pCrl = X509_CRL_new();
    if (pCrl == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAX509GetSubjectNameRef(pCACert, &pCertName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_CRL_set_issuer_name(pCrl, pCertName);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CERT_IO_FAILURE);

    dwError = X509_CRL_set_version(pCrl, 1);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_ERROR);

    *ppCrl = pCrl;

cleanup:
    return dwError;

error:
    LwCAX509CrlFree(pCrl);
    if (ppCrl)
    {
        *ppCrl = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAX509CrlAddCerts(
    X509_CRL                    *pCrl,
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;

    for (; dwIdx < pCertDataArray->dwCount; ++dwIdx)
    {
        dwError = _LwCAX509CrlAddCert(pCrl, pCertDataArray->ppCertData[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_LwCAX509CrlAddCert(
    X509_CRL            *pCrl,
    PLWCA_DB_CERT_DATA  pCertData
    )
{
    DWORD dwError = 0;
    X509_REVOKED *pRevoked = NULL;

    if (pCertData->status == LWCA_CERT_STATUS_ACTIVE)
    {
        // Nothing to do
        // Certificate is not revoked
        goto cleanup;
    }

    dwError = _LWCACreateX509Revoked(pCertData, &pRevoked);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = X509_CRL_add0_revoked(pCrl, pRevoked);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_ERROR);

cleanup:
    return dwError;

error:
    if (pRevoked)
    {
        X509_REVOKED_free(pRevoked);
    }
    goto cleanup;
}

static
DWORD
_LwCAX509CrlSort(
    X509_CRL *pCrl
    )
{
    DWORD dwError = 0;

    dwError = X509_CRL_sort(pCrl);
    BAIL_ON_SSL_ERROR(dwError, LWCA_CRL_SORT_FAILED);

error:
    return dwError;
}

static
DWORD
_LwCAConvertTimeToGeneralizedTime(
    time_t  tm,
    PSTR    *ppszGeneralizedTime
    )
{
    DWORD dwError = 0;
    ASN1_TIME *pAsnTime = NULL;
    PSTR pszGeneralizedTime = NULL;

    if (!ppszGeneralizedTime)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pAsnTime = ASN1_TIME_new();
    if (pAsnTime == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    ASN1_TIME_set(pAsnTime, tm);

    dwError = _LwCAConvertASNTimeToGeneralizedTime(pAsnTime, &pszGeneralizedTime);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszGeneralizedTime = pszGeneralizedTime;

cleanup:
    if (pAsnTime)
    {
        ASN1_TIME_free(pAsnTime);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszGeneralizedTime);
    if (ppszGeneralizedTime)
    {
        *ppszGeneralizedTime = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAAsn1BitStringToDword(
    ASN1_BIT_STRING *pBitString,
    DWORD           *pdwOut
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    BYTE temp = 0;
    DWORD dwTemp = 0;
    DWORD dwMove = 0;
    DWORD dwOut = 0;

    if (!pBitString || pBitString->length > 4)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    for ( ; dwIdx < pBitString->length ; ++dwIdx)
    {
        VM_BYTE_REVERSE_BITS(pBitString->data[dwIdx], temp);
        dwTemp = (DWORD) temp;

        dwOut = dwOut | (dwTemp << dwMove);
        dwMove += 8;
    }

    *pdwOut = dwOut;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAPrivateKeyToPEM(
    EVP_PKEY    *pPrivateKey,
    PSTR        *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    PKCS8_PRIV_KEY_INFO *pPkcs8FormatKey = NULL;
    PSTR pszPrivateKey = NULL;

    if(pPrivateKey == NULL)
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

    pPkcs8FormatKey = EVP_PKEY2PKCS8_broken(pPrivateKey, PKCS8_OK);
    if (pPkcs8FormatKey == NULL)
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!PEM_write_bio_PKCS8_PRIV_KEY_INFO(pBioMem, pPkcs8FormatKey))
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = LwCAAllocateStringWithLengthA(pBuffMem->data, pBuffMem->length - 1, &pszPrivateKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszPrivateKey = pszPrivateKey;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free_all(pBioMem);
    }
    if (pPkcs8FormatKey)
    {
        PKCS8_PRIV_KEY_INFO_free(pPkcs8FormatKey);
    }

    return dwError;

error:
    LWCA_SECURE_SAFE_FREE_MEMORY(pszPrivateKey, LwCAStringLenA(pszPrivateKey));
    if (ppszPrivateKey)
    {
        *ppszPrivateKey = NULL;
    }
    goto cleanup;
}

static
DWORD
_LwCAGetPublicKey(
    EVP_PKEY *pPrivateKey,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;
    PSTR pszPublicKey = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO *pBioMem = NULL;

    if(pPrivateKey == NULL)
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

    if (!PEM_write_bio_PUBKEY(pBioMem, pPrivateKey))
    {
        dwError = LWCA_OUT_OF_MEMORY_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = LwCAAllocateStringWithLengthA(pBuffMem->data, pBuffMem->length - 1, &pszPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszPublicKey = pszPublicKey;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }
    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszPublicKey);
    if (ppszPublicKey)
    {
        *ppszPublicKey = NULL;
    }
    goto cleanup;
}
