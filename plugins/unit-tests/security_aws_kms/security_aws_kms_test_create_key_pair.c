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

LWCA_SECURITY_CAP_OVERRIDE capOverride = {0};

LWCA_BINARY_DATA _gEncryptedData = {0};

static
DWORD
_Security_Storage_Put(
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_BINARY_DATA pEncryptedData
    )
{
    VmFreeMemory(_gEncryptedData.pData);
    VmAllocateMemory(
        pEncryptedData->dwLength,
        &_gEncryptedData.pData);

    memcpy(_gEncryptedData.pData, pEncryptedData->pData, pEncryptedData->dwLength);
    _gEncryptedData.dwLength = pEncryptedData->dwLength;

    return 0;
}

static
DWORD
_Security_Storage_Get(
    PVOID pUserData,
    PCSTR pszKeyId,
    PLWCA_BINARY_DATA *ppEncryptedData
    )
{
    PLWCA_BINARY_DATA pEncryptedData = NULL;

    VmAllocateMemory(sizeof(*pEncryptedData), (PVOID *)&pEncryptedData);
    VmAllocateMemory(_gEncryptedData.dwLength, (PVOID *)&pEncryptedData->pData);

    pEncryptedData->dwLength = _gEncryptedData.dwLength;
    memcpy(pEncryptedData->pData, _gEncryptedData.pData, _gEncryptedData.dwLength);

    *ppEncryptedData = pEncryptedData;
    return 0;
}

int
Security_Aws_Kms_Tests_Create_Key_Pair_Setup(
    void **state
    )
{
    DWORD dwError = 0;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    /* plugin implementation requires storage to be provided */
    capOverride.pFnStoragePut = _Security_Storage_Put;
    capOverride.pFnStorageGet = _Security_Storage_Get;

    dwError = Security_Aws_Kms_Tests_Load_Interface(state);
    if (dwError)
    {
        goto error;
    }

    dwError = pState->pInterface->pFnInitialize(NULL, &pState->pHandle);
    if (dwError)
    {
        goto error;
    }

    dwError = pState->pInterface->pFnCapOverride(
                  pState->pHandle,
                  &capOverride);
    if (dwError)
    {
        goto error;
    }

error:
    return dwError;
}

VOID
Security_Aws_Kms_Tests_Create_Key_Pair (
    void **state
    )
{
    DWORD dwError = 0;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;
    PCSTR pszKeyId = "key1";
    PSTR pszPublicKey = NULL;
    X509 *pX509 = NULL;
    PSTR pszCertificate = NULL;

    dwError = pState->pInterface->pFnCreateKeyPair(
                  pState->pHandle,
                  NULL,
                  pszKeyId,
                  NULL,
                  2048,
                  &pszPublicKey);
    if (dwError)
    {
        fail_msg("Error: %d @ %s\n", dwError, __FUNCTION__);
        goto error;
    }

    /* TODO: supply pX509. this will fail validation for pX509 now */
    dwError = pState->pInterface->pFnSignCertificate(
                  pState->pHandle,
                  NULL,
                  pX509,
                  LWCA_SECURITY_MESSAGE_DIGEST_SHA256,
                  pszKeyId,
                  NULL,
                  &pszCertificate);
    if (dwError)
    {
        fail_msg("Error: %d @ %s\n", dwError, __FUNCTION__);
    }

error:
    return;
}
