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
const EVP_MD*
SSOGetEvpMethod(
    SSO_DIGEST_METHOD digestMethod)
{
    return digestMethod == SSO_DIGEST_METHOD_MD5 ? EVP_md5() : EVP_sha256();
}

static
SSOERROR
SSOConvertPEMKeyToPrivateKey(
    PCSTRING pszPEMKey,
    EVP_PKEY** ppPrivateKey)
{
    SSOERROR e = SSOERROR_NONE;
    BIO* bio = NULL;
    EVP_PKEY* pPrivateKey = NULL;

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (BIO_puts(bio, pszPEMKey) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    PEM_read_bio_PrivateKey(bio, &pPrivateKey, NULL, NULL);
    if (pPrivateKey == NULL)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppPrivateKey = pPrivateKey;

    error:

    if (e != SSOERROR_NONE)
    {
        EVP_PKEY_free(pPrivateKey);
    }

    // cleanup
    BIO_free_all(bio);

    return e;
}

static
SSOERROR
SSOConvertPEMCertificateToPublicKey(
    PCSTRING pszPEMCertificate,
    EVP_PKEY** ppPublicKey)
{
    SSOERROR e = SSOERROR_NONE;
    BIO* bio = NULL;
    X509* x509 = NULL;
    EVP_PKEY* pPublicKey = NULL;

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (BIO_puts(bio, pszPEMCertificate) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    PEM_read_bio_X509(bio, &x509, NULL, NULL);
    if (x509 == NULL)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    pPublicKey = X509_get_pubkey(x509);
    if (pPublicKey == NULL)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppPublicKey = pPublicKey;

    error:

    if (e != SSOERROR_NONE)
    {
        EVP_PKEY_free(pPublicKey);
    }

    // cleanup
    X509_free(x509);
    BIO_free_all(bio);

    return e;
}

SSOERROR
SSOComputeMessageDigest(
    SSO_DIGEST_METHOD digestMethod,
    const unsigned char* pData,
    size_t dataSize,
    unsigned char** ppMD,
    size_t* pMDSize)
{
    SSOERROR e = SSOERROR_NONE;
    unsigned char* pMD = NULL;
    EVP_MD_CTX mdctx = { 0 };
    unsigned char md[EVP_MAX_MD_SIZE] = { 0 };
    unsigned int mdSize = 0;

    if (pData == NULL || ppMD == NULL || pMDSize == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    EVP_MD_CTX_init(&mdctx);

    if (EVP_DigestInit_ex(&mdctx, SSOGetEvpMethod(digestMethod), NULL) == 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_DigestUpdate(&mdctx, pData, dataSize) == 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_DigestFinal_ex(&mdctx, md, &mdSize) == 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(mdSize, (void**) &pMD);
    BAIL_ON_ERROR(e);

    SSOMemoryCopy(pMD, md, mdSize);

    *ppMD = pMD;
    *pMDSize = mdSize;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pMD, mdSize);
    }

    // cleanup
    EVP_MD_CTX_cleanup(&mdctx);

    return e;
}

SSOERROR
SSOComputeRSASignature(
    SSO_DIGEST_METHOD digestMethod,
    const unsigned char* pData,
    size_t dataSize,
    PCSTRING pszRSAPrivateKeyPEM,
    unsigned char** ppRSASignature,
    size_t* pRSASignatureSize)
{
    SSOERROR e = SSOERROR_NONE;
    unsigned char* pMd = NULL;
    size_t mdSize = 0;
    EVP_PKEY* pPrivateKey = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    unsigned char* pRSASignature = NULL;
    size_t rsaSignatureSize = 0;

    if (pData == NULL || IS_NULL_OR_EMPTY_STRING(pszRSAPrivateKeyPEM) || ppRSASignature == NULL || pRSASignatureSize == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    // compute message digest.
    e = SSOComputeMessageDigest(digestMethod, pData, dataSize, &pMd, &mdSize);
    BAIL_ON_ERROR(e);

    e = SSOConvertPEMKeyToPrivateKey(pszRSAPrivateKeyPEM, &pPrivateKey);
    BAIL_ON_ERROR(e);

    // setup context
    ctx = EVP_PKEY_CTX_new(pPrivateKey, NULL /* no engine */);

    if (!ctx)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_sign_init(ctx) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_CTX_set_signature_md(ctx, SSOGetEvpMethod(digestMethod)) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_sign(ctx, NULL, &rsaSignatureSize, pMd, mdSize) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(rsaSignatureSize, (void**) &pRSASignature);
    BAIL_ON_ERROR(e);

    if (EVP_PKEY_sign(ctx, pRSASignature, &rsaSignatureSize, pMd, mdSize) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppRSASignature = pRSASignature;
    *pRSASignatureSize = rsaSignatureSize;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pRSASignature, rsaSignatureSize);
    }

    // cleanup
    EVP_PKEY_free(pPrivateKey);
    EVP_PKEY_CTX_free(ctx);
    SSOMemoryFree(pMd, mdSize);

    return e;
}

SSOERROR
SSOVerifyRSASignature(
    SSO_DIGEST_METHOD digestMethod,
    const unsigned char* pData,
    size_t dataSize,
    const unsigned char* pRSASignature,
    size_t rsaSignatureSize,
    PCSTRING pszRSACertificatePEM,
    bool* verifySuccess)
{
    SSOERROR e = SSOERROR_NONE;
    unsigned char* pMd = NULL;
    size_t mdSize = 0;
    EVP_PKEY* pPublicKey = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    int ret;

    if (pData == NULL || pRSASignature == NULL || IS_NULL_OR_EMPTY_STRING(pszRSACertificatePEM))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    // compute message digest.
    e = SSOComputeMessageDigest(digestMethod, pData, dataSize, &pMd, &mdSize);
    BAIL_ON_ERROR(e);

    e = SSOConvertPEMCertificateToPublicKey(pszRSACertificatePEM, &pPublicKey);
    BAIL_ON_ERROR(e);

    // setup context
    ctx = EVP_PKEY_CTX_new(pPublicKey, NULL /* no engine */);

    if (!ctx)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_verify_init(ctx) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (EVP_PKEY_CTX_set_signature_md(ctx, SSOGetEvpMethod(digestMethod)) <= 0)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    ret = EVP_PKEY_verify(ctx, pRSASignature, rsaSignatureSize, pMd, mdSize);
    if (ret == 1)
    {
        *verifySuccess = true;
    } else if (ret == 0)
    {
        *verifySuccess = false;
    } else
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    error:

    // cleanup
    EVP_PKEY_free(pPublicKey);
    EVP_PKEY_CTX_free(ctx);
    SSOMemoryFree(pMd, mdSize);

    return e;
}
