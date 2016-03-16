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

static VMAFD_SSL_GLOBALS gVmAfdSSLGlobals = {0};

static
VOID
VmAfdOpenSSLLockingCallback(
    int mode,
    int index,
    const char* file,
    int line
    );

static
DWORD
VecsAllocateStringAFromFile(
    PCSTR pszPath,
    PSTR* ppszFileContent
    );

VOID
VmAfdCleanupSSLMutexes(VOID)
{
    if (gVmAfdSSLGlobals.pMutexes)
    {
        VmAfdFreeMutexesArray(
                            gVmAfdSSLGlobals.pMutexes,
                            gVmAfdSSLGlobals.dwNumMutexes
                            );

        gVmAfdSSLGlobals.pMutexes = NULL;
        gVmAfdSSLGlobals.dwNumMutexes = 0;
    }
}

DWORD
VmAfdOpenSSLInit(VOID)
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    pthread_mutex_t *pMutexes = NULL;
    BOOL bInitMutexes = FALSE;

    if (!gVmAfdSSLGlobals.pMutexes)
    {
        DWORD dwNumLocks = CRYPTO_num_locks();

        gVmAfdSSLGlobals.dwNumMutexes = 0;
        bInitMutexes = TRUE;

        dwError = VmAfdAllocateMemory(
                                      dwNumLocks*sizeof(pthread_mutex_t),
                                      (PVOID*)&pMutexes
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);


        for (; dwIndex<dwNumLocks; ++dwIndex)
        {
            if (pthread_mutex_init(&(pMutexes[dwIndex]), NULL))
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                BAIL_ON_VMAFD_ERROR (dwError);
            }
        }

        gVmAfdSSLGlobals.pMutexes = pMutexes;
        pMutexes = NULL;
        gVmAfdSSLGlobals.dwNumMutexes = dwNumLocks;
        CRYPTO_set_locking_callback (VmAfdOpenSSLLockingCallback);

    }

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

cleanup:
    return dwError;

error:

    if (pMutexes)
    {
      VmAfdFreeMutexesArray(pMutexes, dwIndex);
    }
    if (bInitMutexes)
    {
        VmAfdCleanupSSLMutexes();
    }
    goto cleanup;
}

VOID
VmAfdOpenSSLShutdown(VOID)
{
    CRYPTO_set_locking_callback(NULL);

    VmAfdCleanupSSLMutexes();
}

DWORD
VecsPEMToX509Stack(
    PSTR pCertificate,
    STACK_OF(X509) **pskX509certs
    )
{
    DWORD dwError = 0;
    STACK_OF(X509) *skX509certs = sk_X509_new_null();
    BIO *pBioMem = NULL;
    X509 *pCert = NULL;

    if (skX509certs == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID) pCertificate, -1);
    if (pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    while ((pCert = PEM_read_bio_X509( pBioMem, NULL, NULL, NULL)) != NULL)
    {
      sk_X509_push(skX509certs, pCert);
    }

    if (!sk_X509_num(skX509certs))
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *pskX509certs = skX509certs;

cleanup:
    if (pBioMem)
    {
        BIO_free (pBioMem);
    }

    return dwError;

error:
    if (pskX509certs)
    {
        *pskX509certs = NULL;
    }

    sk_X509_pop_free(skX509certs, X509_free);

    goto cleanup;
}

DWORD
VecsPEMToX509(
    PSTR pCertificate,
    X509 **ppX509Cert
    )
{
    DWORD dwError = 0;
    X509 *pCert = NULL;
    BIO *pBioMem = NULL;

    if (
        IsNullOrEmptyString(pCertificate) ||
        !ppX509Cert
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID) pCertificate, -1);
    if ( pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCert  = PEM_read_bio_X509(pBioMem, NULL, NULL, NULL);
    if (pCert  == NULL)
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppX509Cert = pCert;

cleanup :

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error :

    if (ppX509Cert)
    {
      *ppX509Cert = NULL;
    }

    if (pCert != NULL)
    {
       X509_free(pCert);
    }

    goto cleanup;
}

DWORD
VecsPEMToRSA (
      PCSTR pszKey,
      RSA **ppKey
      )
{
    DWORD dwError = 0;
    PSTR psazKey = NULL;
    RSA *pKey = NULL;
    BIO *pBioMemBuff = NULL;

    if (IsNullOrEmptyString (pszKey) ||
        !ppKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMemBuff = BIO_new_mem_buf((PVOID)pszKey, -1);

    if (!pBioMemBuff)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pKey = PEM_read_bio_RSAPrivateKey (pBioMemBuff, NULL, 0, NULL);

    if (!pKey)
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppKey = pKey;

cleanup:
    if (pBioMemBuff)
    {
      BIO_free (pBioMemBuff);
    }
    VMAFD_SAFE_FREE_MEMORY (psazKey);

    return dwError;

error:

    if (ppKey)
    {
        *ppKey = NULL;
    }

    if (pKey)
    {
      RSA_free (pKey);
    }

    goto cleanup;
}

DWORD
VecsPEMToEVPKey (
      PCSTR pszKey,
      PCSTR pszPassword,
      EVP_PKEY **ppKey
      )
{

    DWORD dwError = 0;
    EVP_PKEY *pKey = NULL;
    BIO *pBioMemBuff = NULL;

    if (IsNullOrEmptyString (pszKey) ||
        !ppKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMemBuff = BIO_new_mem_buf((PVOID)pszKey, -1);

    if (!pBioMemBuff)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pKey = PEM_read_bio_PrivateKey (pBioMemBuff, NULL, 0,(void *) pszPassword);

    if (!pKey)
    {
        if (pszPassword)
        {
            /*XXX: Making an educative guess that passwword is wrong*/
            dwError = ERROR_WRONG_PASSWORD;
        }
        else
        {
            dwError = ERROR_BAD_FORMAT;
        }
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppKey = pKey;

cleanup:
    if (pBioMemBuff)
    {
      BIO_free (pBioMemBuff);
    }

    return dwError;

error:

    if (ppKey)
    {
        *ppKey = NULL;
    }

    if (pKey)
    {
      EVP_PKEY_free (pKey);
    }

    goto cleanup;
}

DWORD
VecsPEMToX509Crl(
    PSTR        pCrl,
    X509_CRL**  ppX509Crl
    )
{
    DWORD   dwError = 0;
    BIO*    pBioMem = NULL;
    X509_CRL *pX509Crl = NULL;

    if (!pCrl || !ppX509Crl)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID) pCrl, -1);
    if ( pBioMem == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pX509Crl  = PEM_read_bio_X509_CRL(pBioMem, NULL, NULL, NULL);
    if (pX509Crl  == NULL)
    {
        dwError = VECS_CERT_IO_FAILURE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppX509Crl = pX509Crl;

cleanup :

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error :

    *ppX509Crl = NULL;

    if (pX509Crl != NULL)
    {
       X509_CRL_free(pX509Crl);
    }

    goto cleanup;
}


DWORD
VecsPEMFiletoX509(
    PCSTR   pszPath,
    X509**  ppX509Cert
    )
{
    DWORD dwError = 0;
    PSTR  pszFileContent = NULL;
    X509* pCert = NULL;

    dwError = VecsAllocateStringAFromFile(pszPath, &pszFileContent);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsPEMToX509(pszFileContent, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppX509Cert = pCert;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszFileContent);
    return dwError;

error:

    if (ppX509Cert)
    {
        *ppX509Cert = NULL;
    }

    goto cleanup;
}

DWORD
VecsPEMFiletoX509Stack(
    PCSTR   pszPath,
    STACK_OF(X509) **pskX509certs
    )
{
    DWORD dwError = 0;
    PSTR  pszFileContent = NULL;
    STACK_OF(X509) *x509Certs = NULL;

    dwError = VecsAllocateStringAFromFile(pszPath, &pszFileContent);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsPEMToX509Stack(pszFileContent, &x509Certs);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pskX509certs = x509Certs;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszFileContent);
    return dwError;

error:

    if (pskX509certs)
    {
        *pskX509certs = NULL;
    }

    goto cleanup;
}

DWORD
VecsPEMFiletoX509Crl(
    PCSTR       pszPath,
    X509_CRL**  ppX509Crl
    )
{
    DWORD       dwError = 0;
    PSTR        pszFileContent = NULL;
    X509_CRL*   pCrl = NULL;

    dwError = VecsAllocateStringAFromFile(pszPath, &pszFileContent);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsPEMToX509Crl(pszFileContent, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppX509Crl = pCrl;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszFileContent);
    return dwError;

error:

    if (ppX509Crl)
    {
        *ppX509Crl = NULL;
    }

    goto cleanup;
}

DWORD
VecsHashToAlias(
    unsigned long nSubjectHash,
    PSTR* ppszAlias
    )
{
    DWORD dwError = 0;
    BIO *b64Bio = NULL;
    BIO *bMem = NULL;
    BUF_MEM *bPtr = NULL;
    char buff[8] = { 0 };
    PSTR pszAlias = NULL;

    if (!ppszAlias)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    b64Bio = BIO_new(BIO_f_base64());
    if ( b64Bio == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bMem = BIO_new(BIO_s_mem());
    if ( bMem == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    b64Bio = BIO_push(b64Bio, bMem);
    if ( b64Bio == NULL)
    {
        dwError = VECS_OUT_OF_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    BIO_write(b64Bio, (PVOID)&nSubjectHash, sizeof(nSubjectHash));
    dwError = BIO_flush(b64Bio);
    if ( dwError != 1)
    {
        dwError = VECS_CERT_IO_FAILURE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    dwError = 0;

    BIO_get_mem_ptr(b64Bio, &bPtr);

    if (((bPtr->length  - 1) < sizeof(buff)))
    {
        memcpy(buff, bPtr->data, bPtr->length - 1);

        dwError = VmAfdAllocateStringA(buff, &pszAlias);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszAlias = pszAlias;

cleanup:

    if (b64Bio)
    {
        BIO_free_all(b64Bio);
    }

    return dwError;

error:

    if (ppszAlias)
    {
      *ppszAlias = NULL;
    }

    goto cleanup;
}

DWORD
VecsComputeCertAliasFile(
    PCSTR pszPath,
    PSTR* ppszAlias
    )
{
    DWORD dwError = 0;
    X509* pCert = NULL;
    PSTR pszAlias = NULL;

    if (IsNullOrEmptyString(pszPath) || !ppszAlias)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsPEMFiletoX509(pszPath, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsComputeCertFingerPrint(
                    pCert,
                    VECS_ENCRYPTION_ALGORITHM_SHA_1,
                    &pszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAlias = pszAlias;

cleanup:

    if(pCert)
    {
        X509_free(pCert);
    }

    return dwError;

error :

    if (ppszAlias)
    {
        *ppszAlias = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszAlias);

    goto cleanup;
}

DWORD
VecsComputeCrlAliasFromFile(
    PCSTR pszPath,
    PSTR* ppszAlias
    )
{
    DWORD dwError = 0;
    X509_CRL* pCrl = NULL;
    PSTR pszAlias = NULL;

    if (IsNullOrEmptyString(pszPath) || !ppszAlias)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VecsPEMFiletoX509Crl(pszPath, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsComputeCrlFingerPrint(
                    pCrl,
                    VECS_ENCRYPTION_ALGORITHM_SHA_1,
                    &pszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAlias = pszAlias;

cleanup:

    if(pCrl)
    {
        X509_CRL_free(pCrl);
    }

    return dwError;

error :

    if (ppszAlias)
    {
        *ppszAlias = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszAlias);

    goto cleanup;
}


DWORD
VecsComputeCertAliasA(
    PSTR pszCertificate,
    PSTR* ppszAlias
    )
{
    DWORD dwError = 0;
    X509* pCert = NULL;
    PSTR  pszAlias = NULL;

    if (!pszCertificate || !ppszAlias)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsComputeCertFingerPrint(
                    pCert,
                    VECS_ENCRYPTION_ALGORITHM_SHA_1,
                    &pszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAlias = pszAlias;

cleanup:

    if(pCert)
    {
        X509_free(pCert);
    }
    return dwError;

error :

    if (ppszAlias)
    {
        *ppszAlias = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszAlias);

    goto cleanup;
}

DWORD
VecsComputeCrlAliasA(
    PSTR   pszCrl,
    PSTR*  ppszAlias
    )
{
    DWORD       dwError = 0;
    X509_CRL*   pCrl = NULL;
    PSTR        pszAlias = NULL;

    if (!pszCrl || !ppszAlias)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509Crl(pszCrl, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VecsComputeCrlFingerPrint(
                    pCrl,
                    VECS_ENCRYPTION_ALGORITHM_SHA_1,
                    &pszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAlias = pszAlias;

cleanup:

    if(pCrl)
    {
        X509_CRL_free(pCrl);
    }
    return dwError;

error :

    if (ppszAlias)
    {
        *ppszAlias = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszAlias);

    goto cleanup;
}

DWORD
VecsComputeCrlAliasW(
    PWSTR  pwszCrl,
    PWSTR* ppwszAlias
    )
{
    DWORD       dwError = 0;
    PSTR        pszCrl = NULL;
    PSTR        pszAlias = NULL;
    PWSTR       pwszAlias = NULL;

    if (!pwszCrl || !ppwszAlias)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW (
                                          pwszCrl,
                                          &pszCrl
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsComputeCrlAliasA(
                    pszCrl,
                    &pszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszAlias, &pwszAlias);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszAlias = pwszAlias;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszAlias);
    VMAFD_SAFE_FREE_MEMORY(pszCrl);
    return dwError;

error :

    if (ppwszAlias)
    {
        *ppwszAlias = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszAlias);

    goto cleanup;
}

DWORD
VecsComputeCertAliasW(
    PWSTR pszCertificate,
    PWSTR *ppszHash
    )
{
    DWORD dwError = 0;
    PSTR psazCertificate = NULL;
    PSTR psazHash = NULL;
    PWSTR pszHash = NULL;

    if (IsNullOrEmptyString (pszCertificate) ||
        !ppszHash
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW (
                                          pszCertificate,
                                          &psazCertificate
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsComputeCertAliasA(
                                    psazCertificate,
                                    &psazHash
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringWFromA(
                                          psazHash,
                                          &pszHash
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszHash = pszHash;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (psazCertificate);
    VMAFD_SAFE_FREE_MEMORY (psazHash);

    return dwError;
error:
    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszHash);

    goto cleanup;
}


DWORD
VecsComputeCertHash_MD5(
    PSTR  pszCertificate,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    X509* pCert = NULL;
    PSTR pszHash = NULL;

    if (!pszCertificate || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCertSubjectHash(pCert, TRUE, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    if(pCert)
    {
        X509_free(pCert);
    }
    return dwError;

error :

    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}

DWORD
VecsComputeCertHash_SHA_1(
    PSTR pszCertificate,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    X509* pCert = NULL;
    PSTR  pszHash = NULL;

    if (!pszCertificate || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509(pszCertificate, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCertSubjectHash(pCert, FALSE, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    if(pCert)
    {
        X509_free(pCert);
    }
    return dwError;

error :

    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}

DWORD
VecsComputeCrlAuthorityHash_MD5(
    PSTR pszCrl,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    X509_CRL* pCrl = NULL;
    PSTR pszHash = NULL;

    if (!pszCrl || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509Crl(pszCrl, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCrlAuthorityHash(pCrl, TRUE, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    if(pCrl)
    {
        X509_CRL_free(pCrl);
    }
    return dwError;

error :

    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}

DWORD
VecsComputeCrlAuthorityHash_SHA_1(
    PSTR pszCrl,
    PSTR* ppszHash
    )
{
    DWORD dwError = 0;
    X509_CRL* pCrl = NULL;
    PSTR  pszHash = NULL;

    if (!pszCrl || !ppszHash)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  VecsPEMToX509Crl(pszCrl, &pCrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCrlAuthorityHash(pCrl, FALSE, &pszHash);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszHash = pszHash;

cleanup:

    if(pCrl)
    {
        X509_CRL_free(pCrl);
    }
    return dwError;

error :

    if (ppszHash)
    {
        *ppszHash = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszHash);

    goto cleanup;
}


DWORD
VecsValidateCertificate (
                          PCWSTR pszCertificate
                        )
{
    DWORD dwError = 0;
    PSTR psazCertificate = NULL;
    X509 *pCert = NULL;
    BIO *pBioMemBuff = NULL;

    if (IsNullOrEmptyString (pszCertificate))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW (
                                          pszCertificate,
                                          &psazCertificate
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pBioMemBuff = BIO_new_mem_buf((PVOID)psazCertificate, -1);

    if (!pBioMemBuff)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pCert = PEM_read_bio_X509 (pBioMemBuff, NULL, 0, NULL);

    if (!pCert)
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:
    if (pBioMemBuff)
    {
      BIO_free (pBioMemBuff);
    }
    if (pCert)
    {
      X509_free (pCert);
    }

    VMAFD_SAFE_FREE_MEMORY (psazCertificate);

    return dwError;

error:
    goto cleanup;
}

DWORD
VecsValidateKey (
                 PCWSTR pszKey
                )
{
    DWORD dwError = 0;
    PSTR psazKey = NULL;
    RSA *pKey = NULL;
    BIO *pBioMemBuff = NULL;

    if (IsNullOrEmptyString (pszKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW (
                                          pszKey,
                                          &psazKey
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pBioMemBuff = BIO_new_mem_buf((PVOID)psazKey, -1);

    if (!pBioMemBuff)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pKey = PEM_read_bio_RSAPrivateKey (pBioMemBuff, NULL, 0, NULL);

    if (!pKey)
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:
    if (pBioMemBuff)
    {
      BIO_free (pBioMemBuff);
    }
    if (pKey)
    {
      RSA_free (pKey);
    }

    VMAFD_SAFE_FREE_MEMORY (psazKey);

    return dwError;

error:
    goto cleanup;
}

DWORD
VecsCertStackToPEM(
    STACK_OF(X509) *skX509certs,
    PSTR* ppszCertificate
    )
{
    DWORD dwError = 0;
    PSTR pszBuffer = NULL;
    DWORD dwIndex = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    X509 *certificate = NULL;
    DWORD dwNumCerts = 0;

    if (!skX509certs ||
        !ppszCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwNumCerts = sk_X509_num(skX509certs);

    for (; dwIndex <dwNumCerts; dwIndex++)
    {
        certificate = sk_X509_value(skX509certs, dwIndex);
        if (!certificate)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (!PEM_write_bio_X509(pBioMem, certificate))
        {
            dwError = ERROR_BAD_FORMAT;
            BAIL_ON_VMAFD_ERROR (dwError);
        }
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VmAfdAllocateMemory(
                        (DWORD)pBuffMem->length +1,
                        (PVOID*)&pszBuffer
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length-1);

    pszBuffer[pBuffMem->length-1] = '\n';

    *ppszCertificate = pszBuffer;


cleanup:
    if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
     if (ppszCertificate)
     {
        *ppszCertificate = NULL;
     }
     VMAFD_SAFE_FREE_MEMORY (pszBuffer);
     goto cleanup;

}

DWORD
VecsCertToPEM(
    X509* pCertificate,
    PSTR* ppszCertificate
    )
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;

    if (
         !pCertificate ||
         !ppszCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!PEM_write_bio_X509(pBioMem,pCertificate))
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VmAfdAllocateMemory(
                      (DWORD)pBuffMem->length + 1,
                      (PVOID*)&pszBuffer
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    pszBuffer[pBuffMem->length-1] = '\n';

    *ppszCertificate = pszBuffer;

cleanup:
    if ( pBioMem != NULL) {
        BIO_free(pBioMem);
    }


    return dwError;
error :
     if (ppszCertificate)
     {
        *ppszCertificate = NULL;
     }
     VMAFD_SAFE_FREE_MEMORY (pszBuffer);
     goto cleanup;
}

DWORD
VecsX509CRLToPEM(
    X509_CRL*   pCrl,
    PSTR*       ppszCrl
    )
{
    DWORD       dwError = ERROR_SUCCESS;
    PSTR        pszBuffer = NULL;
    BIO*        pBioMem = NULL;
    BUF_MEM*    pBuffMem = NULL;


    if (!pCrl || !ppszCrl)
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

    if (!PEM_write_bio_X509_CRL(pBioMem, pCrl))
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);
    dwError = VmAfdAllocateMemory((DWORD)pBuffMem->length, (PVOID*)&pszBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    *ppszCrl = pszBuffer;

cleanup:
    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error :
    if (ppszCrl)
    {
        *ppszCrl = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pszBuffer);
    goto cleanup;
}

DWORD
VecsKeyToPEM(
    RSA* pKey,
    PSTR pszPassword,
    PSTR* ppszKey
    )
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;
    EVP_PKEY *pPKCS8Key = NULL;

    if ( !pKey ||
         !ppszKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pPKCS8Key = EVP_PKEY_new();

    if (pPKCS8Key == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!EVP_PKEY_set1_RSA(pPKCS8Key, pKey))
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    if (!PEM_write_bio_PrivateKey(
                      pBioMem,
                      pPKCS8Key,
                      IsNullOrEmptyString(pszPassword)? NULL:EVP_des_ede3_cbc(),
                      NULL,
                      0,
                      NULL,
                      pszPassword
                      )
       )
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VmAfdAllocateMemory(
                    (DWORD)pBuffMem->length + 1,
                    (PVOID*)&pszBuffer
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    pszBuffer[pBuffMem->length-1] = '\n';

    *ppszKey = pszBuffer;

cleanup:
    if (pBioMem != NULL) {
        BIO_free(pBioMem);
    }
    if (pPKCS8Key)
    {
        EVP_PKEY_free (pPKCS8Key);
    }

    return dwError;
error :
     if (ppszKey)
     {
        *ppszKey = NULL;
     }
     VMAFD_SAFE_FREE_MEMORY (pszBuffer);
     goto cleanup;
}

DWORD
VecsEVPKeyToPEM(
    EVP_PKEY* pKey,
    PSTR* ppszKey
    )
{
    PSTR pszBuffer = NULL;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    DWORD dwError = 0;

    if ( !pKey ||
         !ppszKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if ( pBioMem == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!PEM_write_bio_PrivateKey(
                          pBioMem,
                          pKey,
                          NULL,
                          NULL,
                          0,
                          NULL,
                          NULL
                          )
      )
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);

    dwError = VmAfdAllocateMemory(
                    (DWORD)pBuffMem->length + 1,
                    (PVOID*)&pszBuffer
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    memcpy(pszBuffer, pBuffMem->data, pBuffMem->length - 1);

    pszBuffer[pBuffMem->length-1] = '\n';

    *ppszKey = pszBuffer;

cleanup:
    if (pBioMem != NULL) {
        BIO_free(pBioMem);
    }

    return dwError;
error :
     if (ppszKey)
     {
        *ppszKey = NULL;
     }
     VMAFD_SAFE_FREE_MEMORY (pszBuffer);
     goto cleanup;
}

DWORD
VecsValidateAndFormatCert(
    PCWSTR pszCertificate,
    PWSTR *ppszPEMCertificate
    )
{
    DWORD dwError = 0;
    PWSTR pwszPEMCertificate = NULL;
    PSTR pszPEMCertificate = NULL;
    PSTR paszCertificate = NULL;
    STACK_OF(X509) *x509Certs = NULL;

    if (IsNullOrEmptyString(pszCertificate) ||
        !ppszPEMCertificate
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                        pszCertificate,
                                        &paszCertificate
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsPEMToX509Stack(
                              paszCertificate,
                              &x509Certs
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsCertStackToPEM(
                              x509Certs,
                              &pszPEMCertificate
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringWFromA(
                                        pszPEMCertificate,
                                        &pwszPEMCertificate
                                       );

    *ppszPEMCertificate = pwszPEMCertificate;

cleanup:
    if (x509Certs)
    {
        sk_X509_pop_free(x509Certs, X509_free);
    }
    VMAFD_SAFE_FREE_MEMORY (paszCertificate);
    VMAFD_SAFE_FREE_MEMORY (pszPEMCertificate);

    return dwError;

error:
    if (ppszPEMCertificate)
    {
        *ppszPEMCertificate = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pwszPEMCertificate);

    goto cleanup;
}

DWORD
VecsValidateAndFormatCrl(
    PCWSTR pwszCrl,
    PWSTR *ppwszPEMCrl
    )
{
    DWORD dwError = 0;
    PWSTR pwszPEMCrl = NULL;
    PSTR pszPEMCrl = NULL;
    PSTR paszCrl = NULL;
    X509_CRL *x509Crl = NULL;

    if (IsNullOrEmptyString(pwszCrl) ||
        !ppwszPEMCrl
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                        pwszCrl,
                                        &paszCrl
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsPEMToX509Crl (
                             paszCrl,
                             &x509Crl
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsX509CRLToPEM(
                              x509Crl,
                              &pszPEMCrl
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringWFromA(
                                        pszPEMCrl,
                                        &pwszPEMCrl
                                       );

    *ppwszPEMCrl = pwszPEMCrl;

cleanup:
    if (x509Crl)
    {
        X509_CRL_free(x509Crl);
    }
    VMAFD_SAFE_FREE_MEMORY (paszCrl);
    VMAFD_SAFE_FREE_MEMORY (pszPEMCrl);

    return dwError;

error:
    if (ppwszPEMCrl)
    {
        *ppwszPEMCrl = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pwszPEMCrl);

    goto cleanup;
}

DWORD
VecsValidateAndFormatKey(
    PCWSTR pszKey,
    PCWSTR pszPassword,
    PWSTR *ppszPEMKey
    )
{

    DWORD dwError = 0;
    PSTR pszPEMKey = NULL;
    PWSTR pwszPEMKey = NULL;
    PSTR paszKey = NULL;
    PSTR paszPassword = NULL;
    RSA *rsaKey = NULL;

    if (IsNullOrEmptyString(pszKey) ||
        !ppszPEMKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                        pszKey,
                                        &paszKey
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);
    if (!IsNullOrEmptyString(pszPassword))
    {
        dwError = VmAfdAllocateStringAFromW(
                                        pszPassword,
                                        &paszPassword
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

    }

    dwError = VecsPEMToRSA (
                         paszKey,
                         &rsaKey
                         );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsKeyToPEM(
                         rsaKey,
                         paszPassword,
                         &pszPEMKey
                         );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringWFromA (
                                          pszPEMKey,
                                          &pwszPEMKey
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszPEMKey = pwszPEMKey;

cleanup:
    if (rsaKey)
    {
        RSA_free (rsaKey);
    }
    VMAFD_SAFE_FREE_MEMORY (paszKey);
    VMAFD_SAFE_FREE_MEMORY (pszPEMKey);

    return dwError;

error:
    if (ppszPEMKey)
    {
        *ppszPEMKey = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pwszPEMKey);

    goto cleanup;
}

DWORD
VecsDecryptAndFormatKey(
    PCWSTR pszKey,
    PCWSTR pszPassword,
    PWSTR *ppszPEMKey
    )
{

    DWORD dwError = 0;
    PSTR pszPEMKey = NULL;
    PWSTR pwszPEMKey = NULL;
    PSTR paszKey = NULL;
    PSTR paszPassword = NULL;
    EVP_PKEY *pPKCS8Key = NULL;

    if (IsNullOrEmptyString(pszKey) ||
        !ppszPEMKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                        pszKey,
                                        &paszKey
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!IsNullOrEmptyString(pszPassword))
    {
        dwError = VmAfdAllocateStringAFromW(
                                        pszPassword,
                                        &paszPassword
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

    }

    dwError = VecsPEMToEVPKey(
                         paszKey,
                         paszPassword,
                         &pPKCS8Key
                         );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsEVPKeyToPEM(
                         pPKCS8Key,
                         &pszPEMKey
                         );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringWFromA (
                                          pszPEMKey,
                                          &pwszPEMKey
                                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszPEMKey = pwszPEMKey;

cleanup:
    if (pPKCS8Key)
    {
      EVP_PKEY_free (pPKCS8Key);
    }

    VMAFD_SAFE_FREE_MEMORY (paszKey);
    VMAFD_SAFE_FREE_MEMORY (paszPassword);
    VMAFD_SAFE_FREE_MEMORY (pszPEMKey);

    return dwError;

error:
    if (ppszPEMKey)
    {
        *ppszPEMKey = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pwszPEMKey);

    goto cleanup;

}

DWORD
VecsValidateCertKeyPair(
    PCWSTR pszCertificate,
    PCWSTR pszPrivateKey
    )
{

    DWORD dwError = 0;
    PSTR paszKey = NULL;
    PSTR paszCert = NULL;
    EVP_PKEY *pPKCS8Key = NULL;
    X509 *pCert = NULL;

    if (IsNullOrEmptyString(pszCertificate) ||
        !pszPrivateKey
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                        pszPrivateKey,
                                        &paszKey
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsPEMToEVPKey(
                         paszKey,
                         NULL,
                         &pPKCS8Key
                         );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateStringAFromW(
                                        pszCertificate,
                                        &paszCert
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsPEMToX509(paszCert, &pCert);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = X509_check_private_key(pCert, pPKCS8Key);
    if (dwError == 1)
    {
        dwError = ERROR_SUCCESS;
    }
    else
    {
        dwError = VECS_PRIVATE_KEY_MISMATCH;
    }
    BAIL_ON_VMAFD_ERROR(dwError);


cleanup:
    if (pPKCS8Key)
    {
      EVP_PKEY_free (pPKCS8Key);
    }
    if(pCert)
    {
        X509_free(pCert);
    }

    VMAFD_SAFE_FREE_MEMORY (paszKey);
    VMAFD_SAFE_FREE_MEMORY (paszCert);

    return dwError;

error:

    goto cleanup;

}

static
VOID
VmAfdOpenSSLLockingCallback(
    int mode,
    int index,
    const char* file,
    int line
    )
{
    if (gVmAfdSSLGlobals.pMutexes && gVmAfdSSLGlobals.dwNumMutexes > index)
    {
        if (mode & CRYPTO_LOCK)
        {
            pthread_mutex_lock(&(gVmAfdSSLGlobals.pMutexes[index]));
        }
        else
        {
            pthread_mutex_unlock(&(gVmAfdSSLGlobals.pMutexes[index]));
        }
    }
}

static
DWORD
VecsAllocateStringAFromFile(
    PCSTR pszPath,
    PSTR* ppszFileContent
    )
{
    DWORD dwError = 0;
    FILE* fp = NULL;
    size_t fileSize = 0;
    PSTR   pszContent = NULL;
    PSTR   pszCursor = NULL;
    size_t bytesToRead = 0;

    if (IsNullOrEmptyString(pszPath) || !ppszFileContent)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetFileSize(pszPath, &fileSize);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (fileSize == 0)
    {
        dwError = ERROR_FILE_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // TODO : How to limit the max file size?

    dwError = VmAfdAllocateMemory(fileSize + 1, (PVOID*)&pszContent);
    BAIL_ON_VMAFD_ERROR(dwError);

    if ((fp = fopen(pszPath, "rb")) == NULL)
    {
#ifndef _WIN32
        dwError = VmAfdGetWin32ErrorCode(errno);
#else
        dwError = GetLastError();
#endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bytesToRead = fileSize;
    pszCursor = pszContent;

    while (!feof(fp) && bytesToRead > 0)
    {
        size_t bytesRead = 0;

        if ((bytesRead = fread(pszCursor, 1, bytesToRead, fp)) == 0)
        {
#ifndef _WIN32
            dwError = VmAfdGetWin32ErrorCode(errno);
#else
            dwError = GetLastError();
#endif
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszCursor += bytesRead;
        bytesToRead -= bytesRead;
    }

    *ppszFileContent = pszContent;

cleanup:

    if (fp)
    {
        fclose(fp);
    }

    return dwError;

error:

    VMAFD_SAFE_FREE_MEMORY(pszContent);

    if (ppszFileContent)
    {
        *ppszFileContent = NULL;
    }

    goto cleanup;
}
