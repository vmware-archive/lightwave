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
_VmSignatureGetEvpMethod(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const EVP_MD            **ppDigestMethod
    );

static
DWORD
_VmSignatureConvertPEMKeyToPrivateKey(
    PCSTR       pszPEMKey,
    EVP_PKEY    **ppPrivateKey
    );

DWORD
VmSignatureComputeMessageDigest(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const unsigned char     *pData,
    size_t                  dataSize,
    unsigned char           **ppMD,
    size_t                  *pMDSize
    )
{
    DWORD           dwError = 0;
    unsigned char   *pMD = NULL;
    EVP_MD_CTX      mdctx = {0};
    unsigned char   md[EVP_MAX_MD_SIZE] = {0};
    unsigned int    mdSize = 0;
    const EVP_MD    *pDigestMethod = NULL;

    if (!pData || !ppMD || !pMDSize)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    EVP_MD_CTX_init(&mdctx);

    dwError = _VmSignatureGetEvpMethod(digestMethod, &pDigestMethod);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (!EVP_DigestInit_ex(&mdctx, pDigestMethod, NULL))
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (!EVP_DigestUpdate(&mdctx, pData, dataSize))
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (!EVP_DigestFinal_ex(&mdctx, md, &mdSize))
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateMemory(mdSize, (void **)&pMD);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmCopyMemory(pMD, mdSize, md, mdSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppMD = pMD;
    *pMDSize = mdSize;

cleanup:
    EVP_MD_CTX_cleanup(&mdctx);
    return dwError;

error:
    VmFreeMemory(pMD);
    if (ppMD)
    {
        *ppMD = NULL;
    }

    goto cleanup;
}

DWORD
VmSignatureEncodeHex(
    const unsigned char     data[],
    const size_t            length,
    PSTR                    *ppHex
    )
{
    DWORD   dwError = 0;
    PSTR    pHex = NULL;
    int     i = 0;

    dwError = VmAllocateMemory(length * 2 + 1, (void**) &pHex);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (i = 0; i < length; ++i)
    {
        sprintf(&pHex[i * 2], "%02x", data[i]);
    }

    *ppHex = pHex;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_STRINGA(pHex);
    if (ppHex)
    {
        *ppHex = NULL;
    }

    goto cleanup;
}

DWORD
VmSignatureComputeRSASignature(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const unsigned char     *pData,
    size_t                  dataSize,
    PCSTR                   pszRSAPrivateKeyPEM,
    unsigned char**         ppRSASignature,
    size_t*                 pRSASignatureSize
    )
{
    DWORD           dwError = 0;
    unsigned char   *pMd = NULL;
    size_t          mdSize = 0;
    EVP_PKEY        *pPrivateKey = NULL;
    EVP_PKEY_CTX    *ctx = NULL;
    unsigned char   *pRSASignature = NULL;
    size_t          rsaSignatureSize = 0;
    const EVP_MD    *pDigestMethod = NULL;

    if (!pData ||
        IsNullOrEmptyString(pszRSAPrivateKeyPEM) ||
        !ppRSASignature ||
        !pRSASignatureSize
        )
    {
        dwError = VM_COMMON_ERROR_INVALID_ARGUMENT;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmSignatureComputeMessageDigest(digestMethod,
                                              pData,
                                              dataSize,
                                              &pMd,
                                              &mdSize
                                              );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmSignatureConvertPEMKeyToPrivateKey(pszRSAPrivateKeyPEM,
                                                    &pPrivateKey
                                                    );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    // setup context
    ctx = EVP_PKEY_CTX_new(pPrivateKey, NULL);

    if (!ctx)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (EVP_PKEY_sign_init(ctx) <= 0)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _VmSignatureGetEvpMethod(digestMethod, &pDigestMethod);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (EVP_PKEY_CTX_set_signature_md(ctx, pDigestMethod) <= 0)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (EVP_PKEY_sign(ctx, NULL, &rsaSignatureSize, pMd, mdSize) <= 0)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateMemory(rsaSignatureSize, (void**) &pRSASignature);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (EVP_PKEY_sign(ctx, pRSASignature, &rsaSignatureSize, pMd, mdSize) <= 0)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppRSASignature = pRSASignature;
    *pRSASignatureSize = rsaSignatureSize;

cleanup:
    EVP_PKEY_free(pPrivateKey);
    EVP_PKEY_CTX_free(ctx);
    VmFreeMemory(pMd);
    return dwError;

error:
    VM_COMMON_SAFE_FREE_STRINGA(pRSASignature);
    if (ppRSASignature)
    {
        *ppRSASignature = NULL;
    }
    if (pRSASignatureSize)
    {
        *pRSASignatureSize = 0;
    }
    goto cleanup;
}

static
DWORD
_VmSignatureGetEvpMethod(
    VMSIGN_DIGEST_METHOD    digestMethod,
    const EVP_MD            **ppDigestMethod
    )
{
    DWORD           dwError = 0;
    const EVP_MD    *pDigestMethod = NULL;

    switch (digestMethod)
    {
        case VMSIGN_DIGEST_METHOD_MD5:
            pDigestMethod = EVP_md5();
            break;

        case VMSIGN_DIGEST_METHOD_SHA256:
            pDigestMethod = EVP_sha256();
            break;

        default:
            dwError = VM_COMMON_INVALID_EVP_METHOD;
            BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppDigestMethod = pDigestMethod;

cleanup:
    return dwError;

error:
    pDigestMethod = NULL;
    if (ppDigestMethod)
    {
        *ppDigestMethod = NULL;
    }
    goto cleanup;
}

static
DWORD
_VmSignatureConvertPEMKeyToPrivateKey(
    PCSTR       pszPEMKey,
    EVP_PKEY    **ppPrivateKey
    )
{
    DWORD       dwError = 0;
    BIO         *bio = NULL;
    EVP_PKEY    *pPrivateKey = NULL;

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (BIO_puts(bio, pszPEMKey) <= 0)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    PEM_read_bio_PrivateKey(bio, &pPrivateKey, NULL, NULL);
    if (pPrivateKey == NULL)
    {
        dwError = VM_COMMON_ERROR_OPENSSL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppPrivateKey = pPrivateKey;

cleanup:
    BIO_free_all(bio);
    return dwError;

error:
    if (pPrivateKey)
    {
        EVP_PKEY_free(pPrivateKey);
    }
    if (ppPrivateKey)
    {
        *ppPrivateKey = NULL;
    }

    goto cleanup;
}