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
_LwCAInitializeStorage(
    VOID
    );

static
BOOLEAN
IsValidInterface(
    PLWCA_SECURITY_INTERFACE pInterface
    )
{
    return (pInterface &&
            pInterface->pFnInitialize &&
            pInterface->pFnGetCaps &&
            pInterface->pFnCapOverride &&
            pInterface->pFnAddKeyPair &&
            pInterface->pFnCreateKeyPair &&
            pInterface->pFnSign &&
            pInterface->pFnVerify &&
            pInterface->pFnGetErrorString &&
            pInterface->pFnCloseHandle &&
            pInterface->pFnFreeMemory);
}

/*
 * To initialize security context, the Mutent CA config file must provide
 * security plugin with absolute path
 * Example:
 * {
      "securityPlugin": "/usr/lib/libsecurityplugin.so"
      "securityPluginConfig": "/etc/casecurityplugin/config.json"
 * }
 */
DWORD
LwCASecurityInitCtx(
    PLWCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    PSTR pszPlugin = NULL;
    PSTR pszConfigFile = NULL;
    BOOLEAN bLocked = FALSE;

    if (!pConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (gSecurityCtx.isInitialized)
    {
        dwError = LWCA_SECURITY_ALREADY_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    /* read the plugin file path */
    dwError = LwCAJsonGetStringFromKey(
                  pConfig,
                  FALSE,
                  MUTENTCA_SECURITY_PLUGIN_NAME,
                  &pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    /* read the plugin config file path. required to initialize. */
    dwError = LwCAJsonGetStringFromKey(
                  pConfig,
                  FALSE,
                  MUTENTCA_SECURITY_PLUGIN_CONFIG,
                  &pszConfigFile);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszPlugin, &gSecurityCtx.pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPluginInitializeCustom(
                  pszPlugin,
                  LWCA_FN_NAME_SECURITY_LOAD_INTERFACE,
                  &gSecurityCtx.pInterface,
                  &gSecurityCtx.pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gSecurityCtx.pInterface->pFnInitialize(
                  pszConfigFile,
                  &gSecurityCtx.pHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!IsValidInterface(gSecurityCtx.pInterface))
    {
        dwError = LWCA_SECURITY_INVALID_PLUGIN;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = _LwCAInitializeStorage();
    BAIL_ON_LWCA_ERROR(dwError);

    gSecurityCtx.isInitialized = TRUE;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);

    LWCA_SAFE_FREE_STRINGA(pszPlugin);
    LWCA_SAFE_FREE_STRINGA(pszConfigFile);
    return dwError;

error:
    if (dwError != LWCA_SECURITY_ALREADY_INITIALIZED)
    {
        LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
        LwCASecurityFreeCtx();
    }
    goto cleanup;
}

DWORD
LwCASecurityAddKeyPair(
    PCSTR pszKeyId,
    PCSTR pszPrivateKey
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pszKeyId) || IsNullOrEmptyString(pszPrivateKey))
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = LWCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = gSecurityCtx.pInterface->pFnAddKeyPair(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pszKeyId,
                  pszPrivateKey);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCASecurityCreateKeyPair(
    PCSTR pszKeyId,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;
    PSTR pszPublicKey = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pszKeyId) || !ppszPublicKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = LWCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = gSecurityCtx.pInterface->pFnCreateKeyPair(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pszKeyId,
                  LWCA_MIN_CA_CERT_PRIV_KEY_LENGTH,
                  &pszPublicKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszPublicKey = pszPublicKey;

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    VmFreeMemory(pszPublicKey);
    goto cleanup;
}

DWORD
LwCASecuritySignX509Cert(
    PCSTR pcszKeyId,
    X509 *pCert
    )
{
    DWORD dwError = 0;
    LWCA_SECURITY_SIGN_DATA signData = {0};
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszKeyId) || !pCert)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = LWCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    signData.signType = LWCA_SECURITY_SIGN_CERT;
    signData.signData.pX509Cert = pCert;

    dwError = gSecurityCtx.pInterface->pFnSign(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pcszKeyId,
                  &signData,
                  LWCA_SECURITY_MESSAGE_DIGEST_SHA256);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCASecuritySignX509Request(
    PCSTR    pcszKeyId,
    X509_REQ *pReq
    )
{
    DWORD dwError = 0;
    LWCA_SECURITY_SIGN_DATA signData = {0};
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszKeyId) || !pReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = LWCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    signData.signType = LWCA_SECURITY_SIGN_REQ;
    signData.signData.pX509Req = pReq;

    dwError = gSecurityCtx.pInterface->pFnSign(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pcszKeyId,
                  &signData,
                  LWCA_SECURITY_MESSAGE_DIGEST_SHA256);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
LwCASecuritySignX509Crl(
    PCSTR    pcszKeyId,
    X509_CRL *pCrl
    )
{
    DWORD dwError = 0;
    LWCA_SECURITY_SIGN_DATA signData = {0};
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pcszKeyId) || !pCrl)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = LWCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    signData.signType = LWCA_SECURITY_SIGN_CRL;
    signData.signData.pX509Crl = pCrl;

    dwError = gSecurityCtx.pInterface->pFnSign(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pcszKeyId,
                  &signData,
                  LWCA_SECURITY_MESSAGE_DIGEST_SHA256);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

/*
 * set up cap override for storage
 * set up cache for caId to encrypted data
*/
static
DWORD
_LwCAInitializeStorage(
    VOID
    )
{
    DWORD dwError = 0;
    BOOLEAN bStorageLocked = FALSE;

    /* plugin implementation requires storage to be provided */
    gSecurityCtx.capOverride.pFnStoragePut = LwCASecurityStoragePut;
    gSecurityCtx.capOverride.pFnStorageGet = LwCASecurityStorageGet;

    dwError = gSecurityCtx.pInterface->pFnCapOverride(
                  gSecurityCtx.pHandle,
                  &gSecurityCtx.capOverride);
    BAIL_ON_LWCA_ERROR(dwError);

    LWCA_LOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);

    dwError = LwRtlCreateHashMap(
                  &gSecurityCtx.pStorageMap,
                  LwRtlHashDigestPstrCaseless,
                  LwRtlHashEqualPstrCaseless,
                  NULL);
    BAIL_ON_LWCA_ERROR(dwError);

error:
    LWCA_UNLOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);
    return dwError;
}

static
VOID
_LwCASecurityFreeStorageMapPair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
    LwCASecurityFreeBinaryData(pPair->pValue);
}

VOID
LwCASecurityFreeBinaryData(
    PLWCA_BINARY_DATA pBinaryData
    )
{
    if (pBinaryData)
    {
        VmFreeMemory(pBinaryData->pData);
        VmFreeMemory(pBinaryData);
    }
}

VOID
LwCASecurityFreeCtx(
   VOID
   )
{
    BOOLEAN bLocked = FALSE;
    BOOLEAN bStorageLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    LWCA_LOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);

    if (gSecurityCtx.pStorageMap)
    {
        LwRtlHashMapClear(
            gSecurityCtx.pStorageMap,
            _LwCASecurityFreeStorageMapPair,
            NULL);
        LwRtlFreeHashMap(&gSecurityCtx.pStorageMap);
    }

    LWCA_UNLOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);

    if (gSecurityCtx.pHandle)
    {
        gSecurityCtx.pInterface->pFnCloseHandle(gSecurityCtx.pHandle);
    }
    if (gSecurityCtx.pPluginHandle)
    {
        LwCAPluginDeinitializeCustom(
            gSecurityCtx.pPluginHandle,
            LWCA_FN_NAME_SECURITY_UNLOAD_INTERFACE);
    }
    LWCA_SAFE_FREE_MEMORY(gSecurityCtx.pszPlugin);

    gSecurityCtx.isInitialized = FALSE;

    LWCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
}
