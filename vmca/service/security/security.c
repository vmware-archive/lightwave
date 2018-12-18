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
_VMCAInitializeStorage(
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
 * To initialize security context, VMCA config file must provide
 * security plugin with absolute path
 * Example:
 * "security":
 * {
      "securityPlugin": "/usr/lib/libsecurityplugin.so"
      "securityPluginConfig": "/etc/casecurityplugin/config.json"
 * }
 */
DWORD
VMCASecurityInitCtx(
    PVMCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    PSTR pszPlugin = NULL;
    PSTR pszConfigFile = NULL;
    BOOLEAN bLocked = FALSE;
    PVMCA_JSON_OBJECT pSecurityConfig = NULL;

    if (!pConfig)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (gSecurityCtx.isInitialized)
    {
        dwError = VMCA_SECURITY_ALREADY_INITIALIZED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    /* read the plugin file path */
    dwError = VMCAJsonGetObjectFromKey(
                  pConfig,
                  VMCA_SECURITY_PLUGIN_SECTION,
                  &pSecurityConfig);
    BAIL_ON_VMCA_ERROR(dwError);

    /* read the plugin file path */
    dwError = VMCAJsonGetStringFromKey(
                  pSecurityConfig,
                  VMCA_SECURITY_PLUGIN_NAME,
                  &pszPlugin);
    BAIL_ON_VMCA_ERROR(dwError);

    /* read the plugin config file path. required to initialize. */
    dwError = VMCAJsonGetStringFromKey(
                  pSecurityConfig,
                  VMCA_SECURITY_PLUGIN_CONFIG,
                  &pszConfigFile);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(pszPlugin, &gSecurityCtx.pszPlugin);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAPluginInitializeCustom(
                  pszPlugin,
                  LWCA_FN_NAME_SECURITY_LOAD_INTERFACE,
                  &gSecurityCtx.pInterface,
                  &gSecurityCtx.pPluginHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = gSecurityCtx.pInterface->pFnInitialize(
                  pszConfigFile,
                  &gSecurityCtx.pHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!IsValidInterface(gSecurityCtx.pInterface))
    {
        dwError = VMCA_SECURITY_INVALID_PLUGIN;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = _VMCAInitializeStorage();
    BAIL_ON_VMCA_ERROR(dwError);

    gSecurityCtx.isInitialized = TRUE;

cleanup:
    VMCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);

    VMCA_SAFE_FREE_STRINGA(pszPlugin);
    VMCA_SAFE_FREE_STRINGA(pszConfigFile);
    return dwError;

error:
    if (dwError != VMCA_SECURITY_ALREADY_INITIALIZED)
    {
        VMCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
        VMCASecurityFreeCtx();
    }
    goto cleanup;
}

DWORD
VMCASecurityAddKeyPair(
    PCSTR pszKeyId,
    PCSTR pszPrivateKey
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pszKeyId) || IsNullOrEmptyString(pszPrivateKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = VMCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = gSecurityCtx.pInterface->pFnAddKeyPair(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pszKeyId,
                  pszPrivateKey);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
VMCASecurityCreateKeyPair(
    PCSTR pszKeyId,
    PSTR *ppszPublicKey
    )
{
    DWORD dwError = 0;
    PSTR pszPublicKey = NULL;
    BOOLEAN bLocked = FALSE;

    if (IsNullOrEmptyString(pszKeyId) || !ppszPublicKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    if (!gSecurityCtx.isInitialized)
    {
        dwError = VMCA_SECURITY_NOT_INITIALIZED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = gSecurityCtx.pInterface->pFnCreateKeyPair(
                  gSecurityCtx.pHandle,
                  NULL, /* user data */
                  pszKeyId,
                  VMCA_MIN_CA_CERT_PRIV_KEY_LENGTH,
                  &pszPublicKey);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszPublicKey = pszPublicKey;

cleanup:
    VMCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
    return dwError;

error:
    VmFreeMemory(pszPublicKey);
    goto cleanup;
}

/*
 * set up cap override for storage
 * set up cache for caId to encrypted data
*/
static
DWORD
_VMCAInitializeStorage(
    VOID
    )
{
    DWORD dwError = 0;
    BOOLEAN bStorageLocked = FALSE;

    /* plugin implementation requires storage to be provided */
    //gSecurityCtx.capOverride.pFnStoragePut = VMCASecurityStoragePut;
    //gSecurityCtx.capOverride.pFnStorageGet = VMCASecurityStorageGet;

    dwError = gSecurityCtx.pInterface->pFnCapOverride(
                  gSecurityCtx.pHandle,
                  &gSecurityCtx.capOverride);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);

    dwError = LwRtlCreateHashMap(
                  &gSecurityCtx.pStorageMap,
                  LwRtlHashDigestPstrCaseless,
                  LwRtlHashEqualPstrCaseless,
                  NULL);
    BAIL_ON_VMCA_ERROR(dwError);

error:
    VMCA_UNLOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);
    return dwError;
}

static
VOID
_VMCASecurityFreeStorageMapPair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
    VMCASecurityFreeBinaryData(pPair->pValue);
}

VOID
VMCASecurityFreeBinaryData(
    PLWCA_BINARY_DATA pBinaryData
    )
{
    if (pBinaryData)
    {
        VMCA_SAFE_FREE_MEMORY(pBinaryData->pData);
        VmFreeMemory(pBinaryData);
    }
}

VOID
VMCASecurityFreeCtx(
   VOID
   )
{
    BOOLEAN bLocked = FALSE;
    BOOLEAN bStorageLocked = FALSE;

    VMCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

    VMCA_LOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);

    if (gSecurityCtx.pStorageMap)
    {
        LwRtlHashMapClear(
            gSecurityCtx.pStorageMap,
            _VMCASecurityFreeStorageMapPair,
            NULL);
        LwRtlFreeHashMap(&gSecurityCtx.pStorageMap);
    }

    VMCA_UNLOCK_MUTEX(bStorageLocked, &gSecurityCtx.storageMutex);

    if (gSecurityCtx.pHandle)
    {
        gSecurityCtx.pInterface->pFnCloseHandle(gSecurityCtx.pHandle);
    }
    if (gSecurityCtx.pPluginHandle)
    {
        VMCAPluginDeinitializeCustom(
            gSecurityCtx.pPluginHandle,
            LWCA_FN_NAME_SECURITY_UNLOAD_INTERFACE);
    }
    VMCA_SAFE_FREE_MEMORY(gSecurityCtx.pszPlugin);

    gSecurityCtx.isInitialized = FALSE;

    VMCA_LOCK_MUTEX_UNLOCK(&gSecurityCtx.securityMutex, bLocked);
}
