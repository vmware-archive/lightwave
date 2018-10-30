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
VOID
LwCAAuthZFunctionTableFree(
    PLWCA_AUTHZ_FUNCTION_TABLE      pFT
    );

static
BOOLEAN
LwCAAuthZIsValidFunctionTable(
    PLWCA_AUTHZ_FUNCTION_TABLE      pFT
    );


DWORD
LwCAAuthZInitialize(
    PLWCA_JSON_OBJECT               pJsonConfig         // IN
    )
{
    DWORD                           dwError = 0;
    PSTR                            pszPluginPath = NULL;
    PLWCA_PLUGIN_HANDLE             pPluginHandle = NULL;
    PLWCA_AUTHZ_FUNCTION_TABLE      pFT = NULL;
    BOOLEAN                         bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gAuthZCtx.pMutex, bLocked);

    if (gAuthZCtx.bInitialized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_INITIALIZED);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_AUTHZ_FUNCTION_TABLE), (PVOID *)&pFT);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!pJsonConfig)
    {
        gAuthZCtx.bPluginLoaded = FALSE;

        LWCA_LOG_INFO("No AuthZ plugin specified... Using default Lightwave rules");
    }
    else
    {
        dwError = LwCAJsonGetStringFromKey(pJsonConfig, FALSE, LWCA_AUTHZ_PLUGIN_PATH_KEY, &pszPluginPath);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAPluginInitialize(pszPluginPath, pFT, &pPluginHandle);
        BAIL_ON_LWCA_ERROR(dwError);

        if (!LwCAAuthZIsValidFunctionTable(pFT))
        {
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_INVALID_PLUGIN);
        }

        gAuthZCtx.bPluginLoaded = TRUE;

        dwError = LwCAAllocateStringA(pszPluginPath, &gAuthZCtx.pszPluginPath);
        BAIL_ON_LWCA_ERROR(dwError);

        gAuthZCtx.pFT = pFT;
        gAuthZCtx.pPluginHandle = pPluginHandle;

        LWCA_LOG_INFO("Loaded %s AuthZ Plugin", gAuthZCtx.pFT->pfnAuthZGetVersion());
    }

    gAuthZCtx.bInitialized = TRUE;



cleanup:

    LWCA_SAFE_FREE_STRINGA(pszPluginPath);

    LWCA_LOCK_MUTEX_UNLOCK(&gAuthZCtx.pMutex, bLocked);

    return dwError;

error:

    LWCA_LOCK_MUTEX_UNLOCK(&gAuthZCtx.pMutex, bLocked);

    if (dwError != LWCA_ERROR_AUTHZ_INITIALIZED)
    {
        LWCA_SAFE_FREE_MEMORY(pFT);
        LwCAAuthZDestroy();
    }

    LWCA_LOG_ERROR("[%s] Failed to initialize AuthZ context. Error (%d)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
LwCAAuthZCheckAccess(
    PLWCA_REQ_CONTEXT               pReqCtx,                // IN
    X509_REQ                        *pX509Request,          // IN
    LWCA_AUTHZ_API_PERMISSION       apiPermissions,         // IN
    PBOOLEAN                        pbAuthorized            // OUT
    )
{
    DWORD                   dwError = 0;

    // TODO: Implement this

    return dwError;
}

VOID
LwCAAuthZDestroy(
    VOID
    )
{
    BOOLEAN     bLocked = FALSE;

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gAuthZCtx.pMutex, bLocked);

    gAuthZCtx.bInitialized = FALSE;
    gAuthZCtx.bPluginLoaded = FALSE;
    LWCA_SAFE_FREE_STRINGA(gAuthZCtx.pszPluginPath);
    LwCAAuthZFunctionTableFree(gAuthZCtx.pFT);
    LwCAPluginDeinitialize(gAuthZCtx.pPluginHandle);

    LWCA_LOCK_MUTEX_UNLOCK(&gAuthZCtx.pMutex, bLocked);
}


static
VOID
LwCAAuthZFunctionTableFree(
    PLWCA_AUTHZ_FUNCTION_TABLE      pFT
    )
{
    if (pFT)
    {
        pFT->pfnAuthZGetVersion = NULL;
        pFT->pfnAuthZErrorToString = NULL;
        pFT->pfnAuthZCheckAccess = NULL;
        LWCA_SAFE_FREE_MEMORY(pFT);
    }
}

static
BOOLEAN
LwCAAuthZIsValidFunctionTable(
    PLWCA_AUTHZ_FUNCTION_TABLE      pFT
    )
{
    return (pFT                         &&
            pFT->pfnAuthZGetVersion     &&
            pFT->pfnAuthZErrorToString  &&
            pFT->pfnAuthZCheckAccess);
}
