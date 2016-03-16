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
const EVP_MD*
VecsGetSSLAlgorithm(
    VECS_ENCRYPTION_ALGORITHM alg
    );

static
DWORD
VmAfdExtractCNFromDN(
    PCSTR pszDN,
    PSTR* ppszCN
);

VOID
VecsFreeCertArray(
    PVMAFD_CERT_ARRAY pArray
    )
{
    ENTER_LOG();

    if (pArray)
    {
        if (pArray->certificates)
        {
            unsigned int uCounter = 0;

            for(; uCounter < pArray->dwCount; uCounter ++)
            {
                PVMAFD_CERT_CONTAINER pCursor = &pArray->certificates[uCounter];

                VMAFD_SAFE_FREE_MEMORY(pCursor->pCert);
                VMAFD_SAFE_FREE_MEMORY(pCursor->pAlias);
                VMAFD_SAFE_FREE_MEMORY(pCursor->pPassword);
                VMAFD_SAFE_FREE_MEMORY(pCursor->pPrivateKey);
            }
            VmAfdFreeMemory(pArray->certificates);
        }
        VmAfdFreeMemory(pArray);
    }

    EXIT_LOG();
}

VOID
VecsFreeCrlArray(
    PVMAFD_CRL_FILE_CONTAINER pArray
    )
{
    if (pArray != NULL)
    {
        if (pArray->crls)
        {
            DWORD i = 0;

            for (i = 0; i < pArray->dwCount; i++)
            {
                PVMAFD_CRL_DATA pData = &pArray->crls[i];

                VMAFD_SAFE_FREE_MEMORY(pData->buffer);
            }

            VmAfdFreeMemory(pArray->crls);
        }

        VmAfdFreeMemory(pArray);
    }
}

VOID
VecsFreeCACertArray(PVMAFD_CA_CERT_ARRAY pArray)
{
    ENTER_LOG();

    if (pArray)
    {
        if (pArray->pCACerts)
        {
            unsigned int uCounter = 0;

            for(; uCounter < pArray->dwCount; uCounter ++)
            {
                PVMAFD_CA_CERT pCursor = &pArray->pCACerts[uCounter];

                VMAFD_SAFE_FREE_MEMORY(pCursor->pCN);
                VMAFD_SAFE_FREE_MEMORY(pCursor->pSubjectDN);
                VMAFD_SAFE_FREE_MEMORY(pCursor->pCert);
                VMAFD_SAFE_FREE_MEMORY(pCursor->pCrl);
            }
            VmAfdFreeMemory(pArray->pCACerts);
        }
        VmAfdFreeMemory(pArray);
    }

    EXIT_LOG();
}

PCSTR
VecsMapEntryType(
    CERT_ENTRY_TYPE entryType
    )
{
    PCSTR pszEntryType = NULL;

    switch (entryType)
    {
        case CERT_ENTRY_TYPE_PRIVATE_KEY:

            pszEntryType = "Private Key";

            break;

        case CERT_ENTRY_TYPE_SECRET_KEY:

            pszEntryType = "Secret Key";

            break;

        case CERT_ENTRY_TYPE_TRUSTED_CERT:

            pszEntryType = "Trusted Cert";

            break;

        case CERT_ENTRY_TYPE_REVOKED_CERT_LIST:

            pszEntryType = "Certification Revocation List";

            break;

        case CERT_ENTRY_TYPE_ENCRYPTED_PRIVATE_KEY:

            pszEntryType = "Encrypted Private Key";

            break;

        default:

            pszEntryType = "Unknown";

            break;
    }

    return pszEntryType;
}

DWORD
VecsPrintCertificate(
    PCSTR pszCertificate
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    BIO *pBio = NULL;
    pBio = BIO_new_mem_buf((void*)pszCertificate, -1);
    if (pBio == NULL)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    pCert = PEM_read_bio_X509(pBio, NULL, 0, NULL);

    if (pCert)
    {
        X509_print_fp(stdout, pCert);
    }

cleanup:
    if (pCert)
    {
        X509_free(pCert);
    }
    if (pBio)
    {
        BIO_free_all(pBio);
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
VecsPrintCrl(
    PCSTR pszCrl
    )
{
    DWORD dwError = 0;
    X509_CRL *pCrl = NULL;
    BIO *pBio = NULL;
    pBio = BIO_new_mem_buf((void*)pszCrl, -1);
    if ( pBio == NULL)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    pCrl = PEM_read_bio_X509_CRL(pBio, NULL, NULL, NULL);

    if (pCrl)
    {
        X509_CRL_print_fp(stdout, pCrl);
    }

cleanup:
    if (pCrl)
    {
        X509_CRL_free(pCrl);
    }
    if (pBio)
    {
        BIO_free_all(pBio);
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfdOpenCRL(
    PSTR pszCRLFileName,
    X509_CRL **ppCRL
)
{
    DWORD dwError = 0;
    BIO *pCRLFile = NULL;
    X509_CRL *pCRL = NULL;

    if (IsNullOrEmptyString(pszCRLFileName)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(ppCRL == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCRLFile = BIO_new(BIO_s_file());
    if ( pCRLFile == NULL ) {
        dwError = VECS_GENERIC_FILE_IO;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = BIO_read_filename(pCRLFile,pszCRLFileName);
    dwError = !dwError; // BIO_read_filename, 1 - success, 0 - failure :)
    if(dwError != 0)
    {
        dwError = VECS_CRL_OPEN_ERROR;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    // Assume that CRL file is in PEM format first.
    // if that fails we try to Open the file in DER
    // encoded format.

    pCRL=PEM_read_bio_X509_CRL(pCRLFile,NULL,NULL,NULL);
    if ( pCRL == NULL ) {
      // May be the file was in DER format.
      pCRL = d2i_X509_CRL_bio(pCRLFile,NULL);
    }

    if( pCRL == NULL ) {
        // We have tried both options and if it is still failing
        // We have a real CRL error.
        dwError = VECS_CRL_OPEN_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppCRL = pCRL;

cleanup:
    if (pCRLFile) {
        BIO_free(pCRLFile);
    }
    return dwError;

error :
    if(pCRL) {
        X509_CRL_free(pCRL);
    }
    goto cleanup;
}

DWORD
VmAfdGetCRLVersion(
    X509_CRL *pCRL,
    DWORD  *pdwVersion
)
{
    DWORD dwError = 0;
    long lVersion = 0;

    if (pCRL == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(pdwVersion == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    lVersion = X509_CRL_get_version(pCRL);
    *pdwVersion = lVersion;
error :
    return dwError;
}

DWORD
VmAfdGetCRLName(
    X509_CRL *pCRL,
    PWSTR *ppwszName
)
{
    DWORD dwError = 0;
    PSTR pszName = NULL;
    if(pCRL == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(ppwszName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pszName = X509_NAME_oneline(X509_CRL_get_issuer(pCRL),NULL,0);
    dwError = VmAfdAllocateStringWFromA(pszName, ppwszName);
    BAIL_ON_VMAFD_ERROR(dwError);

error :
    if(pszName) {
        OPENSSL_free(pszName);
    }
    return dwError;
}


DWORD
VmAfdConvertASN1Time2String(
    ASN1_TIME *tm,
    PWSTR *ppwszDate
)
{
    DWORD dwError = 0;
    BIO *bio = NULL;
    char buffer[20] = { 0 };


    if( (tm == NULL) ||
        (ppwszDate == NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bio = BIO_new(BIO_s_mem());
    if (bio) {
        if (ASN1_TIME_print(bio, tm)) {
                BIO_read(bio, buffer, sizeof(buffer)-1);
            }
        BIO_free(bio);
    }

    dwError = VmAfdAllocateStringWFromA(buffer, ppwszDate);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmAfdGetCRLLastUpdate(
    X509_CRL *pCRL,
    PWSTR *ppwszDate
)
{
    DWORD dwError = 0;
    ASN1_TIME * tm = NULL;

    if ( ( pCRL == NULL) ||
         (ppwszDate == NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    tm = X509_CRL_get_lastUpdate(pCRL);

    dwError = VmAfdConvertASN1Time2String(tm,ppwszDate);
    BAIL_ON_VMAFD_ERROR(dwError);

error :
    return dwError;
}

DWORD
VmAfdGetCRLNextUpdate(
    X509_CRL *pCRL,
    PWSTR *ppwszDate
)
{
    DWORD dwError = 0;
    ASN1_TIME * tm = NULL;

    if ( ( pCRL == NULL) ||
         (ppwszDate == NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    tm = X509_CRL_get_nextUpdate(pCRL);

    dwError = VmAfdConvertASN1Time2String(tm,ppwszDate);
    BAIL_ON_VMAFD_ERROR(dwError);

error :
    return dwError;

}



DWORD
VmAfdCloseCRL(
    X509_CRL *pCRL
)
{
    DWORD dwError = 0;
    if(pCRL) {
        X509_CRL_free(pCRL);
    }
    return dwError;
}

DWORD
VmAfdGetCertSubjectHash(
    X509*   pCert,
    BOOLEAN bOldHash,
    PSTR*   ppszHash
    )
{
    DWORD dwError = 0;
    X509_NAME* pCertName = NULL;
    PSTR pszHash = NULL;
    unsigned long hash = 0;

    if (!pCert || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCertName = X509_get_subject_name(pCert);
    if (!pCertName)
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    hash = bOldHash ? X509_NAME_hash_old(pCertName) : X509_NAME_hash(pCertName);

    dwError = VmAfdAllocateStringPrintf(
                    &pszHash,
                    "%08x",
                    hash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    return dwError;

error:

    if (ppszHash)
    {
        *ppszHash = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}

DWORD
VmAfdGetCrlAuthorityHash(
    X509_CRL*   pCrl,
    BOOLEAN     bOldHash,
    PSTR*       ppszHash
    )
{
    DWORD dwError = 0;
    X509_NAME* pIssuer = NULL;
    PSTR pszHash = NULL;
    unsigned long hash = 0;

    if (!pCrl || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pIssuer = X509_CRL_get_issuer(pCrl); // Don't free
    if (!pIssuer)
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    hash = bOldHash ? X509_NAME_hash_old(pIssuer) : X509_NAME_hash(pIssuer);

    dwError = VmAfdAllocateStringPrintf(
                    &pszHash,
                    "%08x",
                    hash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    return dwError;

error:

    if (ppszHash)
    {
        *ppszHash = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}

static
DWORD
VecsAllocateFormatFingerPrintA(
    UCHAR md[],
    DWORD count,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    UCHAR mdString[EVP_MAX_MD_SIZE] = { 0 };
    DWORD ndx = 0;
    PSTR pszHash = NULL;

    if (!ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (ndx=0; ndx < count; ndx++ )
    {
        sprintf(mdString + ndx * 2, "%02x", md[ndx]);
    }

    dwError = VmAfdAllocateStringA(mdString, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    return dwError;

 error :
    VMAFD_SAFE_FREE_MEMORY(pszHash);
    if (ppszHash)
    {
        *ppszHash = NULL;
    }
    goto cleanup;
}

DWORD
VecsComputeCertFingerPrint(
    X509 *pCert,
    VECS_ENCRYPTION_ALGORITHM encAlgo,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    UCHAR md[EVP_MAX_MD_SIZE] = { 0 };
    DWORD count = 0;
    const EVP_MD *pEncAlgo = VecsGetSSLAlgorithm(encAlgo);

    if (!pCert || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pEncAlgo)
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (X509_digest(pCert, pEncAlgo, md, &count) != 1)
    {
        dwError = VECS_CERT_IO_FAILURE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsAllocateFormatFingerPrintA(md, count, ppszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VecsComputeCrlFingerPrint(
    X509_CRL *pCrl,
    VECS_ENCRYPTION_ALGORITHM encAlgo,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    UCHAR md[EVP_MAX_MD_SIZE] = { 0 };
    DWORD count = 0;
    const EVP_MD *pEncAlgo = VecsGetSSLAlgorithm(encAlgo);

    if (!pCrl || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pEncAlgo)
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (X509_CRL_digest(pCrl, pEncAlgo, md, &count) != 1)
    {
        dwError = VECS_CRL_IO_ERROR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsAllocateFormatFingerPrintA(md, count, ppszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    return dwError;
}

static
const EVP_MD*
VecsGetSSLAlgorithm(
    VECS_ENCRYPTION_ALGORITHM alg
    )
{
    switch(alg)
    {
        case VECS_ENCRYPTION_ALGORITHM_MD5:

            return EVP_md5();

            break;

        case VECS_ENCRYPTION_ALGORITHM_SHA_1:

            return EVP_sha1();

            break;

        default:

            return NULL;
    }
}

static
DWORD
VmAfdExtractCNFromDN(
    PCSTR pszDN,
    PSTR* ppszCN
)
{
    DWORD dwError = 0;
    PSTR  pStart = NULL;
    PSTR  pEnd = NULL;
    PSTR  pszCN = NULL;
    int   length = 0;

    if (IsNullOrEmptyString(pszDN) || !ppszCN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (VmAfdStringNCompareA(pszDN, "CN=", 3, FALSE))
    {
        dwError = ERROR_INVALID_FORM_NAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pStart = VmAfdStringChrA(pszDN, '=');
    if (!pStart || !(*(pStart + 1)))
    {
        dwError = ERROR_INVALID_FORM_NAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pStart++;
    pEnd = VmAfdStringChrA(pszDN, ',');
    if (!pEnd || pEnd <= pStart)
    {
        dwError = ERROR_INVALID_FORM_NAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    length = pEnd - pStart;
    dwError = VmAfdAllocateMemory(sizeof(CHAR) * (length + 1), (PVOID*)&pszCN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringNCpyA(pszCN, length + 1, pStart, length);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszCN = pszCN;

cleanup:
    return dwError;

error :
    VMAFD_SAFE_FREE_MEMORY(pszCN);
    goto cleanup;
}

DWORD
VmAfdPrintCACertificates(
    PVMAFD_CA_CERT_ARRAY pCertArray
    )
{
    DWORD   dwError = 0;
    PSTR    pszCN = NULL;
    int     i = 0;
    PVMAFD_CA_CERT  pCert = NULL;

    if (!pCertArray || pCertArray->dwCount == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    fprintf(stdout,
            "Number of certificates:\t%d\n",
            pCertArray->dwCount
            );

    for (; i < pCertArray->dwCount; ++i)
    {
        fprintf(stdout, "#%d:\n", i + 1);

        pCert = &pCertArray->pCACerts[i];

        dwError = VmAfdExtractCNFromDN((PSTR)pCert->pCN, &pszCN);
        BAIL_ON_VMAFD_ERROR(dwError);

        fprintf(stdout, "CN(id):\t\t%s\n", pszCN);
        fprintf(stdout, "Subject DN:\t%s\n", (PSTR)pCert->pSubjectDN);
        fprintf(stdout, "CRL present:\t%s\n", (PSTR)pCert->pCrl ? "yes" : "no");

        VMAFD_SAFE_FREE_MEMORY(pszCN);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszCN);
    return dwError;

error :
    goto cleanup;
}

DWORD
VmAfdSaveCACertificateAndCrlToFile(
    PVMAFD_CA_CERT pCert,
    PCSTR pszCertFile,
    PCSTR pszCrlFile
    )
{
    DWORD dwError = 0;

    if (!pCert || (!pszCertFile && !pszCrlFile))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pszCertFile)
    {
        dwError = VmAfdSaveStringToFile(pszCertFile, (PSTR)pCert->pCert);
        BAIL_ON_VMAFD_ERROR(dwError);    }

    if (pszCrlFile)
    {
        if (pCert->pCrl)
        {
            dwError = VmAfdSaveStringToFile(pszCrlFile, (PSTR)pCert->pCrl);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_NOT_FOUND;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error :
    goto cleanup;
}

