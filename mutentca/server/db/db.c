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
 * To initialize db context, the Mutent CA config file must provide db plugin with absolute path
 * Example:
 * {
      "dbPlugin": "/usr/lib/libdbplugin.so"
 * }
 */
DWORD
LwCADbInitCtx(
    PLWCA_JSON_OBJECT pConfig
    )
{
    DWORD dwError = 0;
    PSTR pszPlugin = NULL;

    if (gpDbCtx != NULL)
    {
        dwError = LWCA_DB_ALREADY_INITIALIZED;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!pConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetStringFromKey(pConfig, CONFIG_DB_PLUGIN_KEY_NAME, &pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CONTEXT), (PVOID*)&gpDbCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_FUNCTION_TABLE), (PVOID*)&gpDbCtx->pFt);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(pszPlugin, &gpDbCtx->pszPlugin);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPluginInitialize(pszPlugin, gpDbCtx->pFt, &gpDbCtx->pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = gpDbCtx->pFt->pFnInit(&gpDbCtx->pDbHandle);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPlugin);
    return dwError;

error:
    LwCADbFreeCtx();
    goto cleanup;
}

VOID
LwCADbFreeFunctionTable(
    PLWCA_DB_FUNCTION_TABLE pFt
    )
{
    if (pFt)
    {
        LWCA_SAFE_FREE_MEMORY(pFt);
    }
}

VOID
LwCADbFreeCtx(
   VOID
   )
{
    if (gpDbCtx)
    {
        if(gpDbCtx->pFt)
        {
            if (gpDbCtx->pDbHandle)
            {
                gpDbCtx->pFt->pFnFreeHandle(gpDbCtx->pDbHandle);
            }
            LwCADbFreeFunctionTable(gpDbCtx->pFt);
        }
        if (gpDbCtx->pPluginHandle)
        {
            LwCAPluginDeinitialize(gpDbCtx->pPluginHandle);
        }
        LWCA_SAFE_FREE_MEMORY(gpDbCtx->pszPlugin);
        LWCA_SAFE_FREE_MEMORY(gpDbCtx);
    }
}
