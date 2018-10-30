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

VOID
LwCASecurityFreeCtx(
   VOID
   )
{
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gSecurityCtx.securityMutex, bLocked);

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
