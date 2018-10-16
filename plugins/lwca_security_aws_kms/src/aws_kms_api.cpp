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
#include "aws_kms_includes.h"

extern "C"
DWORD
LwAwsKmsInitialize(
    PAWS_KMS_API_CONTEXT *ppContext
    )
{
    DWORD dwError = 0;
    PAWS_KMS_API_CONTEXT pContext = NULL;

    if (!ppContext)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pContext = new AWS_KMS_API_CONTEXT;
    if (!pContext)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    /* TODO: read this in from config */
    /* pContext->cmkId = CMK_ID; */
    pContext->keySpec = Aws::KMS::Model::DataKeySpec::AES_256;

    Aws::InitAPI(pContext->sdkOptions);

    *ppContext = pContext;

cleanup:
    return dwError;

error:
    LwAwsKmsShutdown(pContext);
    goto cleanup;
}

static
DWORD
_HasData(
    PLWCA_BINARY_DATA pData
    )
{
    return pData && pData->pData && pData->dwLength > 0;
}

static
DWORD
_CombineEncryptedData(
    LwKMSEncryptedData& encryptedData,
    PLWCA_BINARY_DATA *ppEncryptedData
    )
{
    DWORD dwError = 0;
    PLWCA_BINARY_DATA pEncryptedData = NULL;
    LWCA_KMS_RAW_ENCRYPTED_MATERIALS materials = {0};
    int nTotalLength = 0;
    int nOffset = 0;

    materials.arLengths[ENCRYPTED_MATERIAL_HEADER] = ENCRYPTED_MATERIAL_COUNT;
    materials.arLengths[ENCRYPTED_MATERIAL_DATA] =
        encryptedData.GetEncryptedData().GetLength();
    materials.arLengths[ENCRYPTED_MATERIAL_KEY] =
        encryptedData.GetEncryptedKey().GetLength();
    materials.arLengths[ENCRYPTED_MATERIAL_IV] =
        encryptedData.GetIV().GetLength();

    nTotalLength = materials.arLengths[ENCRYPTED_MATERIAL_DATA] +
                   materials.arLengths[ENCRYPTED_MATERIAL_KEY] +
                   materials.arLengths[ENCRYPTED_MATERIAL_IV] +
                   sizeof(materials.arLengths);

    if (nTotalLength > MAX_ENCRYPTED_DATA_LENGTH)
    {
        dwError = LWCA_SECURITY_AWS_KMS_ENCRYPTED_DATA_TOO_BIG;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pEncryptedData = new LWCA_BINARY_DATA;
    if (!pEncryptedData)
    {
        dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pEncryptedData->pData = new BYTE[nTotalLength];
    pEncryptedData->dwLength = nTotalLength;

    nOffset = sizeof(materials.arLengths);
    /* copy the length of all materials */
    memcpy(pEncryptedData->pData, &materials, nOffset);

    /* copy encrypted data material */
    memcpy(pEncryptedData->pData + nOffset,
           encryptedData.GetEncryptedData().GetUnderlyingData(),
           materials.arLengths[ENCRYPTED_MATERIAL_DATA]);

    nOffset += materials.arLengths[ENCRYPTED_MATERIAL_DATA];
    /* copy encrypted key */
    memcpy(pEncryptedData->pData + nOffset,
           encryptedData.GetEncryptedKey().GetUnderlyingData(),
           materials.arLengths[ENCRYPTED_MATERIAL_KEY]);

    nOffset += materials.arLengths[ENCRYPTED_MATERIAL_KEY];
    /* copy iv */
    memcpy(pEncryptedData->pData + nOffset,
           encryptedData.GetIV().GetUnderlyingData(),
           materials.arLengths[ENCRYPTED_MATERIAL_IV]);

    *ppEncryptedData = pEncryptedData;
cleanup:
    return dwError;

error:
    LwAwsKmsFreeBinaryData(pEncryptedData);
    goto cleanup;
}

static
DWORD
_Encrypt(
    PAWS_KMS_API_CONTEXT pContext,
    PLWCA_BINARY_DATA pData,
    PLWCA_BINARY_DATA *ppEncryptedData
    )
{
    DWORD dwError = 0;
    PLWCA_BINARY_DATA pEncryptedData = NULL;
    CryptoBuffer data(pData->pData, pData->dwLength);

    LwKMSEncryptedData encryptedData;
    LwKMSCryptoConfig cryptoConfig(
                        pContext->clientConfig,
                        pContext->cmkId,
                        pContext->keySpec);
    LwKMSCryptoHelper cryptoHelper(cryptoConfig);

    if (!cryptoHelper.Encrypt(data, encryptedData))
    {
        dwError = _CombineEncryptedData(encryptedData, &pEncryptedData);
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    *ppEncryptedData = pEncryptedData;

error:
    return dwError;
}

extern "C"
DWORD
LwAwsKmsEncrypt(
    PAWS_KMS_API_CONTEXT pContext,
    PLWCA_BINARY_DATA pData,
    PLWCA_BINARY_DATA *ppEncryptedData
    )
{
    DWORD dwError = 0;
    PLWCA_BINARY_DATA pEncryptedData = NULL;

    if (!pContext || !pData || !ppEncryptedData)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    if (!_HasData(pData))
    {
        dwError = LWCA_SECURITY_AWS_KMS_ZERO_ENCRYPT_DATA;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _Encrypt(pContext, pData, &pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    *ppEncryptedData = pEncryptedData;

cleanup:
    return dwError;

error:
    LwAwsKmsFreeBinaryData(pEncryptedData);
    goto cleanup;
}

static
DWORD
_Decrypt(
    PAWS_KMS_API_CONTEXT pContext,
    PLWCA_KMS_RAW_ENCRYPTED_MATERIALS pEncryptedMaterials,
    PLWCA_BINARY_DATA *ppDecryptedData
    )
{
    DWORD dwError = 0;
    int nOffset = 0;
    CryptoBuffer decryptedData;
    PLWCA_BINARY_DATA pDecryptedData = NULL;

    int nLength = pEncryptedMaterials->arLengths[ENCRYPTED_MATERIAL_DATA];
    CryptoBuffer data(pEncryptedMaterials->pData, nLength);

    nOffset += nLength;
    nLength = pEncryptedMaterials->arLengths[ENCRYPTED_MATERIAL_KEY];
    CryptoBuffer key(pEncryptedMaterials->pData + nOffset, nLength);

    nOffset += nLength;
    nLength = pEncryptedMaterials->arLengths[ENCRYPTED_MATERIAL_IV];
    CryptoBuffer iv(pEncryptedMaterials->pData + nOffset, nLength);

    LwKMSEncryptedData encryptedData(data, key, iv);

    LwKMSCryptoConfig cryptoConfig(
                        pContext->clientConfig,
                        pContext->cmkId,
                        pContext->keySpec);

    LwKMSCryptoHelper cryptoHelper(cryptoConfig);
    if (!cryptoHelper.Decrypt(encryptedData, decryptedData))
    {
        pDecryptedData = new LWCA_BINARY_DATA;
        if (!pDecryptedData)
        {
            dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
            BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
        }
        pDecryptedData->pData = new BYTE[decryptedData.GetLength()];
        if (!pDecryptedData->pData)
        {
            dwError = LWCA_SECURITY_AWS_KMS_NOMEM;
            BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
        }
        memcpy(pDecryptedData->pData,
               decryptedData.GetUnderlyingData(),
               decryptedData.GetLength());
        pDecryptedData->dwLength = decryptedData.GetLength();
    }

    *ppDecryptedData = pDecryptedData;

cleanup:
    return dwError;

error:
    LwAwsKmsFreeBinaryData(pDecryptedData);
    goto cleanup;
}

extern "C"
VOID
LwAwsKmsFreeBinaryData(
    PLWCA_BINARY_DATA pBinaryData
    )
{
    if (pBinaryData)
    {
        memset(pBinaryData->pData, 0, pBinaryData->dwLength);
        delete pBinaryData->pData;
        delete pBinaryData;
    }
}

static
DWORD
_GetRawEncryptedMaterials(
    PLWCA_BINARY_DATA pEncryptedData,
    PLWCA_KMS_RAW_ENCRYPTED_MATERIALS pMaterials
    )
{
    DWORD dwError = 0;
    DWORD dwBinaryLength = 0;
    PLWCA_KMS_RAW_ENCRYPTED_MATERIALS pLocal = NULL;
    size_t headerLength = sizeof(pLocal->arLengths);

    if (!_HasData(pEncryptedData))
    {
        dwError = LWCA_SECURITY_AWS_KMS_ZERO_ENCRYPT_DATA;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    pLocal = (PLWCA_KMS_RAW_ENCRYPTED_MATERIALS)pEncryptedData->pData;

    if (pLocal->arLengths[ENCRYPTED_MATERIAL_HEADER] != ENCRYPTED_MATERIAL_COUNT)
    {
        dwError = LWCA_SECURITY_AWS_KMS_ENCRYPTED_MATERIAL_ERROR;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    /* this is the total length of the binary data */
    dwBinaryLength = pLocal->arLengths[ENCRYPTED_MATERIAL_DATA] +
                     pLocal->arLengths[ENCRYPTED_MATERIAL_KEY] +
                     pLocal->arLengths[ENCRYPTED_MATERIAL_IV];

    /* expect encrypted data length to be sum of binary data length
     * and header length
    */
    if (pEncryptedData->dwLength != headerLength + dwBinaryLength)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_ENCRYPTED_DATA;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    memcpy(pMaterials->arLengths, pLocal->arLengths, headerLength);
    pMaterials->pData = pEncryptedData->pData + headerLength;

error:
    return dwError;
}

extern "C"
DWORD
LwAwsKmsDecrypt(
    PAWS_KMS_API_CONTEXT pContext,
    PLWCA_BINARY_DATA pEncryptedData,
    PLWCA_BINARY_DATA *ppDecryptedData
    )
{
    DWORD dwError = 0;
    PLWCA_BINARY_DATA pDecryptedData = NULL;
    LWCA_KMS_RAW_ENCRYPTED_MATERIALS encryptedMaterials = {0};

    if (!pContext || !pEncryptedData || !ppDecryptedData)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _GetRawEncryptedMaterials(pEncryptedData, &encryptedMaterials);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = _Decrypt(pContext, &encryptedMaterials, &pDecryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    *ppDecryptedData = pDecryptedData;

cleanup:
    return dwError;

error:
    LwAwsKmsFreeBinaryData(pDecryptedData);
    goto cleanup;
}


extern "C"
VOID
LwAwsKmsShutdown(
    PAWS_KMS_API_CONTEXT pContext
    )
{
    if (pContext)
    {
        Aws::ShutdownAPI(pContext->sdkOptions);
        delete pContext;
        pContext = NULL;
    }
}
