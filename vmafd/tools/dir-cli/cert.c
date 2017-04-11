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
DWORD
DirCliGetCertContent(
    PCSTR pszCertPath,
    PSTR* ppszContent
    );

static
DWORD
DirCliGetCertSubjectName(
    X509* pCert,
    PSTR* ppszSubjectName
    );

DWORD
DirCliCreateCert(
    PCSTR          pszCertPath,
    PDIR_CLI_CERT* ppCert
    )
{
    DWORD  dwError = 0;
    PSTR   pszCert = NULL;
    PDIR_CLI_CERT pCert = NULL;

    dwError = VmAfdAllocateMemory(sizeof(DIR_CLI_CERT), (PVOID*)&pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliGetCertContent(pszCertPath, &pszCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliPEMToX509(pszCert, &pCert->pX509Cert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliGetCertSubjectName(
                    pCert->pX509Cert,
                    &pCert->pszSubjectName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsComputeCertFingerPrint(
                    pCert->pX509Cert,
                    VECS_ENCRYPTION_ALGORITHM_SHA_1,
                    &pCert->pszFingerPrint);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppCert = pCert;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszCert);

    return dwError;

error:

    *ppCert = NULL;

    if (pCert)
    {
        DirCliFreeCert(pCert);
    }

    goto cleanup;
}

VOID
DirCliFreeCert(
    PDIR_CLI_CERT pCert
    )
{
    VMAFD_SAFE_FREE_MEMORY(pCert->pszSubjectName);
    VMAFD_SAFE_FREE_MEMORY(pCert->pszFingerPrint);
    if (pCert->pX509Cert)
    {
        X509_free(pCert->pX509Cert);
    }
    VmAfdFreeMemory(pCert);
}

static
DWORD
DirCliGetCertContent(
    PCSTR pszCertPath,
    PSTR* ppszContent
    )
{
    DWORD  dwError = 0;
    size_t fileSize = 0;
    FILE*  fp = NULL;
    size_t bytesToRead = 0;
    PSTR   pszContent = NULL;
    PSTR   pszCursor = NULL;

    if (IsNullOrEmptyString(pszCertPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetFileSize(pszCertPath, &fileSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (fileSize <= 0)
    {
        dwError = ERROR_FILE_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(fileSize + 1, (PVOID*)&pszContent);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOpenFilePath(pszCertPath, "rb", &fp, 0);
    BAIL_ON_VMAFD_ERROR(dwError);

    bytesToRead = fileSize;
    pszCursor = pszContent;

    while (!feof(fp) && bytesToRead > 0)
    {
        size_t bytesRead = 0;

        if ((bytesRead = fread(pszCursor, 1, bytesToRead, fp)) == 0)
        {
#ifndef _WIN32
            dwError = LwErrnoToWin32Error(errno);
#else
            dwError = GetLastError();
#endif
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszCursor += bytesRead;
        bytesToRead -= bytesRead;
    }

    *ppszContent = pszContent;

cleanup:

    if (fp)
    {
        fclose(fp);
    }

    return dwError;

error:

    *ppszContent = NULL;

    VMAFD_SAFE_FREE_MEMORY(pszContent);

    goto cleanup;
}

DWORD
DirCliDERToX509(
    PBYTE pCertBytes,
    DWORD dwLength,
    X509** ppCert
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    X509* pCert = NULL;

    pBioMem = BIO_new_mem_buf(pCertBytes, dwLength);
    if ( pBioMem == NULL)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCert = d2i_X509_bio(pBioMem, NULL);
    if (pCert  == NULL)
    {
        dwError = ERROR_OPEN_FAILED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppCert = pCert;

cleanup:

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:

    *ppCert = NULL;

    goto cleanup;
}

DWORD
DirCliX509ToDER(
    X509* pCert,
    PBYTE* ppCertBytes,
    PDWORD pdwLength
    )
{
    DWORD dwError = 0;
    PBYTE pBytes_openssl = NULL;
    PBYTE pCertBytes = NULL;
    int len = 0;

    len = i2d_X509(pCert, &pBytes_openssl);
    if (len  < 0)
    {
        dwError = ERROR_ENCRYPTION_FAILED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(len, (PVOID)&pCertBytes);
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pCertBytes, pBytes_openssl, len);

    *ppCertBytes = pCertBytes;
    *pdwLength = len;

cleanup:

    if (pBytes_openssl)
    {
        OPENSSL_free(pBytes_openssl);
    }

    return dwError;

error:

    *ppCertBytes = NULL;
    *pdwLength = 0;

    goto cleanup;
}

DWORD
DirCliPEMToX509(
    PCSTR  pszCert,
    X509** ppCert
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    X509* pCert = NULL;

    pBioMem = BIO_new_mem_buf((PVOID) pszCert, -1);
    if ( pBioMem == NULL)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCert  = PEM_read_bio_X509(pBioMem, NULL, NULL, NULL);
    if (pCert  == NULL)
    {
        dwError = ERROR_OPEN_FAILED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppCert = pCert;

cleanup:

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:

    *ppCert = NULL;

    goto cleanup;
}

DWORD
DirCliGetX509Name(
    X509_NAME *pCertName,
    DWORD dwFlags,
    PSTR* ppszSubjectDN
    )
{
    DWORD dwError = 0;
    size_t len = 0;
    BIO*  pBioMem = NULL;
    PSTR  pszSubjectName = NULL;

    pBioMem = BIO_new(BIO_s_mem());
    if (!pBioMem)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    X509_NAME_print_ex(pBioMem, pCertName, 0, dwFlags);

    len = BIO_pending(pBioMem);

    if (len <= 0)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(len + 1, (PVOID*)&pszSubjectName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (BIO_read(pBioMem, pszSubjectName, len) != len)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszSubjectDN = pszSubjectName;

cleanup:

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:

    *ppszSubjectDN = NULL;

    goto cleanup;
}


static
DWORD
DirCliGetCertSubjectName(
    X509* pCert,
    PSTR* ppszSubjectName
    )
{
    return DirCliGetX509Name(X509_get_subject_name(pCert),
            XN_FLAG_RFC2253, ppszSubjectName);
}

// TODO: move this to common
static
DWORD SslErrorToVmAfdError(
    DWORD dwSslError,
    DWORD dwNewError)
{
    if (dwSslError == 0)                                                       \
    {
        return dwNewError;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

DWORD
DirCliCertToPEM(
    X509* pCertificate,
    PSTR* ppszCertificate
)
// This function takes a Certificate and returns a PEM Encoded String.
//
// Arguments :
//          pCertificate      : OpenSSL X509 Certificate
//          ppCertificate     : PEM encoded Certificate String.
//
// Returns :
//      Error Code
{
    DWORD       dwError = 0;
    PSTR        pszBuffer = NULL;
    BUF_MEM*    pBuffMem = NULL;
    BIO*        pBioMem = NULL;

    if (!pCertificate || *ppszCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (!pBioMem)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = PEM_write_bio_X509(
                  pBioMem,
                  pCertificate);
    dwError = SslErrorToVmAfdError(dwError, ERROR_INVALID_DATA);
    BAIL_ON_VMAFD_ERROR(dwError);

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VmAfdAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    *ppszCertificate = pszBuffer;

cleanup:
    if ( pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error :
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pszBuffer);
    goto cleanup;
}

DWORD
DirCliReadCertStringFromFile(
    PCSTR pszFileName,
    PSTR* ppszCertificate
    )
{
    DWORD dwError = ERROR_SUCCESS;
    X509* pCert = NULL;

    if (IsNullOrEmptyString(pszFileName) || !ppszCertificate)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsPEMFiletoX509(pszFileName, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DirCliCertToPEM(pCert, ppszCertificate);
    BAIL_ON_VMAFD_ERROR(dwError);

error :
    if (ppszCertificate)
    {
        *ppszCertificate = NULL;
    }
    if (pCert)
    {
        X509_free(pCert);
    }

    return dwError;
}

DWORD
DirCliReadCrlStringFromFile(
    PCSTR pszFileName,
    PSTR* ppszCrl
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pszCrl = NULL;
    X509_CRL* pCrl = NULL;

    if (IsNullOrEmptyString(pszFileName) || !ppszCrl)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsPEMFiletoX509Crl(pszFileName, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsX509CRLToPEM(pCrl, &pszCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszCrl = pszCrl;

cleanup:
    return dwError;

error:
    if(pCrl)
    {
        X509_CRL_free(pCrl);
    }
    if (ppszCrl)
    {
        *ppszCrl = NULL;
    }

    goto cleanup;
}
