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

DWORD
VmDirComputeMessageDigest(
    const EVP_MD*           digestMethod,
    const unsigned char*    pData,
    size_t                  dataSize,
    unsigned char**         ppMD,
    size_t*                 pMDSize
    )
{
    DWORD   dwError = 0;
    EVP_MD_CTX  mdCtx = {0};
    unsigned char   md[EVP_MAX_MD_SIZE] = {0};
    unsigned int    mdSize = 0;
    unsigned char*  pMD = NULL;

    if (!digestMethod || !pData || !ppMD || !pMDSize)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    EVP_MD_CTX_init(&mdCtx);

    if (EVP_DigestInit_ex(&mdCtx, digestMethod, NULL) == 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: EVP_DigestInit_ex returned 0", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    if (EVP_DigestUpdate(&mdCtx, pData, dataSize) == 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: EVP_DigestUpdate returned 0", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    if (EVP_DigestFinal_ex(&mdCtx, md, &mdSize) == 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: EVP_DigestFinal_ex returned 0", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    dwError = VmDirAllocateAndCopyMemory(md, mdSize, (PVOID*)&pMD);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMD = pMD;
    *pMDSize = mdSize;

cleanup:
    EVP_MD_CTX_cleanup(&mdCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed with error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pMD);
    goto cleanup;
}

DWORD
VmDirConvertPEMToPublicKey(
    PCSTR       pszPEM,
    EVP_PKEY**  ppPubKey
    )
{
    DWORD   dwError = 0;
    int     retVal = 0;
    BIO*    bio = NULL;
    RSA*    rsa = NULL;
    EVP_PKEY*   pPubKey = NULL;

    if (IsNullOrEmptyString(pszPEM) || !ppPubKey)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    bio = BIO_new(BIO_s_mem());
    if (!bio)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BIO_new returned NULL", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    retVal = BIO_puts(bio, pszPEM);
    if (retVal <= 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BIO_puts returned %d", __FUNCTION__, retVal);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL);
    if (!rsa)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: PEM_read_RSA_PUBKEY returned NULL", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    pPubKey = EVP_PKEY_new();
    if (!pPubKey)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: EVP_PKEY_new returned NULL", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    retVal = EVP_PKEY_assign_RSA(pPubKey, rsa);
    if (retVal != 1)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: EVP_PKEY_assign_RSA returned %d", __FUNCTION__, retVal);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    *ppPubKey = pPubKey;

cleanup:
    BIO_free(bio);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed with error (%d)",
            __FUNCTION__,
            dwError);

    RSA_free(rsa);
    EVP_PKEY_free(pPubKey);
    goto cleanup;
}

DWORD
VmDirVerifyRSASignature(
    EVP_PKEY*               pPubKey,
    const EVP_MD*           digestMethod,
    const unsigned char*    pData,
    size_t                  dataSize,
    const unsigned char*    pSignature,
    size_t                  signatureSize,
    PBOOLEAN                pVerified
    )
{
    DWORD   dwError = 0;
    int     retVal = 0;
    unsigned char*  pMd = NULL;
    size_t          mdSize = 0;
    EVP_PKEY_CTX*   pPubKeyCtx = NULL;

    if (!pPubKey || !digestMethod || !pData || !pData || !pVerified)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirComputeMessageDigest(digestMethod, pData, dataSize, &pMd, &mdSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    pPubKeyCtx = EVP_PKEY_CTX_new(pPubKey, NULL);
    if (!pPubKeyCtx)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: EVP_PKEY_CTX_new returned NULL",
                __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    retVal = EVP_PKEY_verify_init(pPubKeyCtx);
    if (retVal <= 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: EVP_PKEY_verify_init returned %d",
                __FUNCTION__,
                retVal);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    retVal = EVP_PKEY_CTX_set_rsa_padding(pPubKeyCtx, RSA_PKCS1_PADDING);
    if (retVal <= 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: EVP_PKEY_CTX_set_rsa_padding returned %d",
                __FUNCTION__,
                retVal);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    retVal = EVP_PKEY_CTX_set_signature_md(pPubKeyCtx, digestMethod);
    if (retVal <= 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: EVP_PKEY_CTX_set_signature_md returned %d",
                __FUNCTION__,
                retVal);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

    retVal = EVP_PKEY_verify(pPubKeyCtx, pSignature, signatureSize, pMd, mdSize);
    if (retVal == 1)
    {
        *pVerified = TRUE;
    }
    else if (retVal == 0)
    {
        *pVerified = FALSE;
    }
    else
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: EVP_PKEY_verify returned %d",
                __FUNCTION__,
                retVal);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SSL);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pMd);
    EVP_PKEY_CTX_free(pPubKeyCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed with error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
