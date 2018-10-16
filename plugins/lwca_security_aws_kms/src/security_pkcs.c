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
_MakePrivateKeyPem(
    EVP_PKEY *pPrivateKey,
    PSTR *ppszPrivateKey
    )
{
    DWORD dwError = 0;
    BUF_MEM *pBuffMem = NULL;
    BIO* pBioMem = NULL;
    PKCS8_PRIV_KEY_INFO *pPkcs8FormatKey = NULL;
    PSTR pszPrivateKey = NULL;

    if(pPrivateKey == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_INVALID_LENGTH;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pPkcs8FormatKey = EVP_PKEY2PKCS8_broken(pPrivateKey, PKCS8_OK);
    if (pPkcs8FormatKey == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (!PEM_write_bio_PKCS8_PRIV_KEY_INFO(pBioMem, pPkcs8FormatKey))
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_CONVERSION_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);
    if (!pBuffMem)
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_CONVERSION_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = VmAllocateMemory(pBuffMem->length, (PVOID*)&pszPrivateKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    memcpy(pszPrivateKey, pBuffMem->data, pBuffMem->length - 1);

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

error :
    LwSecurityAwsKmsSecureFreeString(pszPrivateKey);
    goto cleanup;
}

static
DWORD
_MakePublicKeyPem(
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
        dwError = LWCA_SECURITY_AWS_KMS_KEY_INVALID_LENGTH;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pBioMem = BIO_new(BIO_s_mem());
    if (pBioMem == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (!PEM_write_bio_PUBKEY(pBioMem, pPrivateKey))
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_CONVERSION_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    BIO_get_mem_ptr(pBioMem, &pBuffMem);
    if (!pBuffMem)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = VmAllocateMemory(pBuffMem->length, (PVOID*)&pszPublicKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    memcpy(pszPublicKey, pBuffMem->data, pBuffMem->length - 1);

    *ppszPublicKey = pszPublicKey;

cleanup:
    if (pBioMem != NULL)
    {
        BIO_free(pBioMem);
    }
    return dwError;

error :
    LwSecurityAwsKmsFreeMemory(pszPublicKey);
    goto cleanup;
}

static
DWORD
_CreateKeyPair(
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
        dwError = LWCA_SECURITY_AWS_KMS_KEY_INVALID_LENGTH;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pPrivateKey = EVP_PKEY_new();
    if (!pPrivateKey)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pRSA = RSA_new();
    if ( pRSA == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pBigNum = BN_new();
    if ( pBigNum == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (!BN_set_word(pBigNum, RSA_F4))
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (RSA_generate_key_ex(pRSA, nKeyLength, pBigNum, NULL) <= 0)
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_CREATION_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (!EVP_PKEY_assign_RSA(pPrivateKey, pRSA))
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_CREATION_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pRSA = NULL; /* pPrivateKey free will free pRSA */

    dwError = _MakePrivateKeyPem(pPrivateKey, &pszPrivateKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = _MakePublicKeyPem(pPrivateKey, &pszPublicKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

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
    LwSecurityAwsKmsSecureFreeString(pszPrivateKey);
    LwSecurityAwsKmsFreeMemory(pszPublicKey);
    goto cleanup;
}

DWORD
LwCreateKeyPair(
    PCSTR pszPassPhrase, /* optional */
    size_t nKeyLength,
    PSTR *ppszPrivateKey,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;
    PSTR pszPrivateKey = NULL;
    PSTR pszPublicKey = NULL;

    if (!ppszPrivateKey || !ppszPublicKey)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _CreateKeyPair(nKeyLength, &pszPrivateKey, &pszPublicKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    *ppszPrivateKey = pszPrivateKey;
    *ppszPublicKey = pszPublicKey;

cleanup:
    return dwError;

error:
    LwSecurityAwsKmsSecureFreeString(pszPrivateKey);
    LwSecurityAwsKmsFreeMemory(pszPublicKey);
    goto cleanup;
}

static
DWORD
_MakePrivateKeyFromPEM(
    PLWCA_BINARY_DATA pKeyData,
    EVP_PKEY **ppPrivateKey
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    RSA *pRSA = NULL;
    EVP_PKEY *pPrivateKey = NULL;
    PSTR pszKey = NULL;

    pPrivateKey = EVP_PKEY_new();
    if (!pPrivateKey)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = VmAllocateMemory(pKeyData->dwLength + 1, (PVOID *)&pszKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    memcpy(pszKey, pKeyData->pData, pKeyData->dwLength);

    pBioMem = BIO_new_mem_buf((PVOID)pszKey, -1);
    if ( pBioMem == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pRSA = PEM_read_bio_RSAPrivateKey(pBioMem, NULL, NULL, NULL);
    if (pRSA == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (!EVP_PKEY_assign_RSA(pPrivateKey, pRSA))
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_CREATION_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pRSA = NULL; /* pPrivateKey free will free pRSA */

    *ppPrivateKey = pPrivateKey;

cleanup:
    LwSecurityAwsKmsSecureFreeString(pszKey);
    if (pBioMem)
    {
        BIO_free(pBioMem);
    }
    if (pRSA)
    {
        RSA_free(pRSA);
    }
    return dwError;

error:
    if (pPrivateKey != NULL)
    {
        EVP_PKEY_free(pPrivateKey);
    }
    goto cleanup;
}

DWORD
LwX509Sign(
    X509 *pX509,
    PLWCA_BINARY_DATA pKeyData,
    LWCA_SECURITY_MESSAGE_DIGEST md
    )
{
    DWORD dwError = 0;
    EVP_PKEY *pPrivateKey = NULL;
    const EVP_MD *pmd = NULL;

    switch(md)
    {
        case LWCA_SECURITY_MESSAGE_DIGEST_SHA256:
            pmd = EVP_sha256();
            break;
        case LWCA_SECURITY_MESSAGE_DIGEST_SHA512:
            pmd = EVP_sha512();
            break;
        default:
            dwError = LWCA_SECURITY_AWS_KMS_INVALID_MESSAGE_DIGEST;
            BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _MakePrivateKeyFromPEM(pKeyData, &pPrivateKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    if (!X509_sign(pX509, pPrivateKey, pmd))
    {
        dwError = LWCA_SECURITY_AWS_KMS_SIGN_FAILED;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

error:
    if (pPrivateKey)
    {
        EVP_PKEY_free(pPrivateKey);
    }
    return dwError;
}

static
DWORD
_MakeX509FromPEM(
    PCSTR pszCertificate,
    X509 **ppX509
    )
{
    DWORD dwError = 0;
    BIO *pBioMem = NULL;
    X509 *pX509 = NULL;

    pBioMem = BIO_new_mem_buf(pszCertificate, -1);
    if (pBioMem == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pX509 = PEM_read_bio_X509(pBioMem, NULL, NULL, NULL);
    if (pX509 == NULL)
    {
        dwError = LWCA_SECURITY_AWS_KMS_READ_PEM_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    *ppX509 = pX509;

cleanup:
    if(pBioMem)
    {
        BIO_free(pBioMem);
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
LwX509Verify(
    PCSTR pszCertificate,
    PLWCA_BINARY_DATA pKeyData,
    PBOOLEAN pbValid
    )
{
    DWORD dwError = 0;
    int nVerifyResult = -1;
    EVP_PKEY *pPrivateKey = NULL;
    X509 *pX509 = NULL;

    dwError = _MakeX509FromPEM(pszCertificate, &pX509);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = _MakePrivateKeyFromPEM(pKeyData, &pPrivateKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    nVerifyResult = X509_verify(pX509, pPrivateKey);
    if (nVerifyResult < 0)
    {
        dwError = LWCA_SECURITY_AWS_KMS_VERIFY_FAILED;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    *pbValid = nVerifyResult ? TRUE : FALSE;

cleanup:
    if (pX509)
    {
        X509_free(pX509);
    }
    if (pPrivateKey)
    {
        EVP_PKEY_free(pPrivateKey);
    }
    return dwError;
error:
    if (pbValid)
    {
        *pbValid = FALSE;
    }
    goto cleanup;
}
