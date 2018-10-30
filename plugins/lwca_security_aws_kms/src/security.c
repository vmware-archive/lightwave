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

/*
 * this is the ca security implementation providing key security using aws kms.
 * this implementation fulfills capabilities for
 * 1. LWCA_SECURITY_CAP_CRYPT
 * this implementation requires external capability overrides for
 * 1. LWCA_SECURITY_CAP_SIGN_VERIFY
 * 2. LWCA_SECURITY_CAP_STORAGE
 * Please see README for configuration details
 * This implementation assumes openssl init is done and does not
 * include openssl init code. Make sure openssl is initialized before
 * using this.
*/

LWCA_SECURITY_INTERFACE _interface = {0};

PCSTR
LwCASecurityGetVersion(
    VOID
    )
{
    return LWCA_SECURITY_VERSION_MAJOR"."\
           LWCA_SECURITY_VERSION_MINOR"."\
           LWCA_SECURITY_VERSION_RELEASE;
}

DWORD
LwSecurityAwsKmsInitialize(
    PCSTR pszConfigFile,
    PLWCA_SECURITY_HANDLE *ppHandle
    )
{
    DWORD dwError = 0;
    PLWCA_SECURITY_HANDLE pHandle = NULL;
    PLWCA_SECURITY_CONFIG pConfig = NULL;

    if (IsNullOrEmptyString(pszConfigFile) || !ppHandle)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = VmAllocateMemory(sizeof(*pHandle), (PVOID *)&pHandle);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = LwCASecurityReadConfigFile(pszConfigFile, &pConfig);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = LwAwsKmsInitialize(pConfig, &pHandle->pContext);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    *ppHandle = pHandle;

cleanup:
    LwAwsKmsFreeConfig(pConfig);
    return dwError;

error:
    if (pHandle)
    {
        LwAwsKmsShutdown(pHandle->pContext);
    }
    goto cleanup;
}

/* this implementation only provides encryption and decryption */
DWORD
LwSecurityAwsKmsGetCaps(
    PLWCA_SECURITY_HANDLE pHandle,
    LWCA_SECURITY_CAP *pnCaps
    )
{
    DWORD dwError = 0;
    LWCA_SECURITY_CAP nCaps = LWCA_SECURITY_CAP_CRYPT |
                              LWCA_SECURITY_CAP_SIGN_VERIFY;
    *pnCaps = nCaps;

    return dwError;
}

/* validate required cap overrides */
static
DWORD
_CapOverrideValidate(
    PLWCA_SECURITY_CAP_OVERRIDE pOverride
    )
{
    DWORD dwError = 0;

    /* This implementation does not provide storage
     * However, it requires storage to work.
     * Therefore, it expects storage to be set as a cap override.
    */
    if (!pOverride->pFnStoragePut || !pOverride->pFnStorageGet)
    {
        dwError = LWCA_SECURITY_AWS_KMS_MISSING_CAPS;
    }

    return dwError;
}

/*
 * cap override is required for
 * storage, sign and verify
*/
DWORD
LwSecurityAwsKmsCapOverride(
    PLWCA_SECURITY_HANDLE pHandle,
    PLWCA_SECURITY_CAP_OVERRIDE pOverride
    )
{
    DWORD dwError = 0;

    if (!pHandle || !pOverride)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _CapOverrideValidate(pOverride);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    pHandle->pCapOverride = pOverride;

error:
    return dwError;
}

/*
 * adding a key pair has the following operations
 * 1. encrypt the private key (this implementation handles it)
 * 2. encrypt the pass phrase if there is one (this is not implemented)
 * 3. store encrypted key and encryption details using key id passed in
 *    - (this implementation expects a cap override to implement this)
*/
DWORD
LwSecurityAwsKmsAddKeyPair(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszKeyId,
    PCSTR pszPrivateKey
    )
{
    DWORD dwError = 0;
    LWCA_BINARY_DATA dataToEncrypt = {0};
    PLWCA_BINARY_DATA pEncryptedData = NULL;

    if (!pHandle || !pHandle->pCapOverride)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _CapOverrideValidate(pHandle->pCapOverride);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dataToEncrypt.pData = (PBYTE)pszPrivateKey;
    dataToEncrypt.dwLength = VmStringLenA(pszPrivateKey);

    dwError = LwAwsKmsEncrypt(
                  pHandle->pContext,
                  &dataToEncrypt,
                  &pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = pHandle->pCapOverride->pFnStoragePut(
                  pUserData,
                  pszKeyId,
                  pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

error:
    LwAwsKmsFreeBinaryData(pEncryptedData);
    return dwError;
}

/*
 * use openssl apis to create key pair
 * encrypt the private key using data key from aws kms
 * encrypt the data key using aws kms apis
 * this plugin does not implement storage so call override to store
 * encrypted data.
 * return public key
*/
DWORD
LwSecurityAwsKmsCreateKeyPair(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszKeyId,
    size_t nKeyLength,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;
    LWCA_BINARY_DATA dataToEncrypt = {0};
    PLWCA_BINARY_DATA pEncryptedData = NULL;

    PSTR pszPrivateKey = NULL;
    PSTR pszPublicKey = NULL;

    if (!pHandle ||
        !pHandle->pCapOverride ||
        IsNullOrEmptyString(pszKeyId) ||
        !ppszPublicKey)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _CapOverrideValidate(pHandle->pCapOverride);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    /* create key pair */
    dwError = LwCreateKeyPair(
                  nKeyLength,
                  &pszPrivateKey,
                  &pszPublicKey);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dataToEncrypt.pData = (PBYTE)pszPrivateKey;
    dataToEncrypt.dwLength = VmStringLenA(pszPrivateKey);

    dwError = LwAwsKmsEncrypt(
                  pHandle->pContext,
                  &dataToEncrypt,
                  &pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = pHandle->pCapOverride->pFnStoragePut(
                  pUserData,
                  pszKeyId,
                  pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    *ppszPublicKey = pszPublicKey;

cleanup:
    LwSecurityAwsKmsSecureFreeString(pszPrivateKey);
    LwAwsKmsFreeBinaryData(pEncryptedData);
    return dwError;

error:
    LwSecurityAwsKmsFreeMemory(pszPublicKey);
    goto cleanup;
}

DWORD
LwSecurityAwsKmsSign(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszKeyId,
    LWCA_SECURITY_SIGN_DATA *pSignData,
    LWCA_SECURITY_MESSAGE_DIGEST md
    )
{
    DWORD dwError = 0;
    PLWCA_BINARY_DATA pDecryptedData = NULL;
    PLWCA_BINARY_DATA pEncryptedData = NULL;

    if (!pHandle ||
        !pHandle->pCapOverride ||
        IsNullOrEmptyString(pszKeyId) ||
        !pSignData)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _CapOverrideValidate(pHandle->pCapOverride);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = pHandle->pCapOverride->pFnStorageGet(
                  pUserData,
                  pszKeyId,
                  &pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = LwAwsKmsDecrypt(
                  pHandle->pContext,
                  pEncryptedData,
                  &pDecryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = LwX509Sign(pSignData, pDecryptedData, md);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

error:
    /* TODO: properly free pEncryptedData as it is allocated by cap override */
    LwAwsKmsFreeBinaryData(pDecryptedData);
    return dwError;
}

DWORD
LwSecurityAwsKmsVerify(
    PLWCA_SECURITY_HANDLE pHandle,
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_SECURITY_SIGN_DATA pSignData,
    BOOLEAN *pbValid
    )
{
    DWORD dwError = 0;
    PLWCA_BINARY_DATA pDecryptedData = NULL;
    PLWCA_BINARY_DATA pEncryptedData = NULL;
    BOOLEAN bValid = 0;

    if (!pHandle ||
        !pHandle->pCapOverride ||
        IsNullOrEmptyString(pszKeyId) ||
        !pSignData ||
        !pbValid)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = _CapOverrideValidate(pHandle->pCapOverride);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = pHandle->pCapOverride->pFnStorageGet(
                  pUserData,
                  pszKeyId,
                  &pEncryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = LwAwsKmsDecrypt(
                  pHandle->pContext,
                  pEncryptedData,
                  &pDecryptedData);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = LwX509Verify(pSignData, pDecryptedData, &bValid);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    *pbValid = bValid;

error:
    /* TODO: properly free pEncryptedData as it is allocated by cap override */
    LwAwsKmsFreeBinaryData(pDecryptedData);
    return dwError;
}

DWORD
LwSecurityAwsKmsGetErrorString(
    DWORD dwErrorCode,
    PSTR *ppszError
    )
{
    DWORD dwError = 0;

    return dwError;
}

VOID
LwSecurityAwsKmsCloseHandle(
    PLWCA_SECURITY_HANDLE pHandle
    )
{
    if (pHandle)
    {
        LwAwsKmsShutdown(pHandle->pContext);
        pHandle->pContext = NULL;
    }
}

VOID
LwSecurityAwsKmsFreeMemory(
    PVOID pMemory
    )
{
    VmFreeMemory(pMemory);
}

VOID
LwSecurityAwsKmsSecureFreeString(
    PSTR pszData
    )
{
    if (pszData)
    {
        memset(pszData, 0, strlen(pszData));
        VmFreeMemory(pszData);
    }
}

DWORD
LwCASecurityLoadInterface(
    PLWCA_SECURITY_INTERFACE *ppInterface
    )
{
    DWORD dwError = 0;

    _interface.pFnInitialize = LwSecurityAwsKmsInitialize;
    _interface.pFnGetCaps = LwSecurityAwsKmsGetCaps;
    _interface.pFnCapOverride = LwSecurityAwsKmsCapOverride;
    _interface.pFnAddKeyPair = LwSecurityAwsKmsAddKeyPair;
    _interface.pFnCreateKeyPair = LwSecurityAwsKmsCreateKeyPair;
    _interface.pFnSign = LwSecurityAwsKmsSign;
    _interface.pFnVerify = LwSecurityAwsKmsVerify;
    _interface.pFnGetErrorString = LwSecurityAwsKmsGetErrorString;
    _interface.pFnCloseHandle = LwSecurityAwsKmsCloseHandle;
    _interface.pFnFreeMemory = LwSecurityAwsKmsFreeMemory;

    *ppInterface = &_interface;

    return dwError;
}

DWORD
LwCASecurityUnloadInterface(
    PLWCA_SECURITY_INTERFACE pInterface
    )
{
    DWORD dwError = 0;

    if (pInterface)
    {
        pInterface->pFnInitialize = NULL;
        pInterface->pFnGetCaps = NULL;
        pInterface->pFnCapOverride = NULL;
        pInterface->pFnAddKeyPair = NULL;
        pInterface->pFnCreateKeyPair = NULL;
        pInterface->pFnSign = NULL;
        pInterface->pFnVerify = NULL;
        pInterface->pFnGetErrorString = NULL;
        pInterface->pFnCloseHandle = NULL;
        pInterface->pFnFreeMemory = NULL;
    }

    return dwError;
}
