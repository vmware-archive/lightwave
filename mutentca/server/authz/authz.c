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
    PSTR                            pszPluginConfigPath = NULL;
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

        pFT->pfnAuthZCheckAccess = &LwCAAuthZLWCheckAccess;

        LWCA_LOG_INFO("No AuthZ plugin specified... Using default Lightwave rules");
    }
    else
    {
        dwError = LwCAJsonGetStringFromKey(pJsonConfig, FALSE, LWCA_AUTHZ_PLUGIN_PATH_KEY, &pszPluginPath);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonGetStringFromKey(pJsonConfig, TRUE, LWCA_AUTHZ_PLUGIN_CONFIG_PATH_KEY, &pszPluginConfigPath);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAPluginInitialize(pszPluginPath, pFT, &pPluginHandle);
        BAIL_ON_LWCA_ERROR(dwError);

        if (!LwCAAuthZIsValidFunctionTable(pFT))
        {
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_INVALID_PLUGIN);
        }

        if (pFT->pfnAuthZPluginInit)
        {
            dwError = pFT->pfnAuthZPluginInit(pszPluginConfigPath);
            if (gAuthZCtx.bPluginLoaded)
            {
                LWCA_LOG_ERROR(
                        "[%s,%d] (%s) AuthZ plugin Init failed with (%s)",
                        __FUNCTION__,
                        __LINE__,
                        gAuthZCtx.pFT->pfnAuthZGetVersion(),
                        gAuthZCtx.pFT->pfnAuthZErrorToString(dwError));
                BAIL_WITH_LWCA_ERROR(dwError, LWCA_PLUGIN_FAILURE);
            }
        }

        gAuthZCtx.bPluginLoaded = TRUE;

        dwError = LwCAAllocateStringA(pszPluginPath, &gAuthZCtx.pszPluginPath);
        BAIL_ON_LWCA_ERROR(dwError);

        gAuthZCtx.pPluginHandle = pPluginHandle;

        LWCA_LOG_INFO("Loaded %s AuthZ Plugin", pFT->pfnAuthZGetVersion());
    }

    gAuthZCtx.pFT = pFT;
    gAuthZCtx.bInitialized = TRUE;


cleanup:

    LWCA_SAFE_FREE_STRINGA(pszPluginPath);
    LWCA_SAFE_FREE_STRINGA(pszPluginConfigPath);

    LWCA_LOCK_MUTEX_UNLOCK(&gAuthZCtx.pMutex, bLocked);

    return dwError;

error:

    LWCA_LOCK_MUTEX_UNLOCK(&gAuthZCtx.pMutex, bLocked);

    if (dwError != LWCA_ERROR_AUTHZ_INITIALIZED)
    {
        LwCAAuthZFunctionTableFree(pFT);
        LwCAAuthZDestroy();
    }

    LWCA_LOG_ERROR("[%s] Failed to initialize AuthZ context. Error (%d)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
LwCAAuthZCheckAccess(
    PLWCA_REQ_CONTEXT               pReqCtx,                // IN
    PCSTR                           pcszCAId,               // IN
    X509_REQ                        *pX509Request,          // IN
    LWCA_AUTHZ_API_PERMISSION       apiPermissions,         // IN
    PBOOLEAN                        pbAuthorized            // OUT
    )
{
    DWORD                           dwError = 0;
    BOOLEAN                         bLocked = FALSE;
    BOOLEAN                         bAuthorized = FALSE;

    if (!pReqCtx || IsNullOrEmptyString(pcszCAId) || !pX509Request || !pbAuthorized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    LWCA_LOCK_MUTEX_EXCLUSIVE(&gAuthZCtx.pMutex, bLocked);

    if (!gAuthZCtx.bInitialized)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_AUTHZ_UNINITIALIZED);
    }

    dwError = gAuthZCtx.pFT->pfnAuthZCheckAccess(
                             pReqCtx,
                             pcszCAId,
                             pX509Request,
                             apiPermissions,
                             &bAuthorized);
    if (dwError)
    {
        if (gAuthZCtx.bPluginLoaded)
        {
            LWCA_LOG_ERROR(
                    "[%s,%d] (%s) AuthZ plugin CheckAccess failed with (%s)",
                    __FUNCTION__,
                    __LINE__,
                    gAuthZCtx.pFT->pfnAuthZGetVersion(),
                    gAuthZCtx.pFT->pfnAuthZErrorToString(dwError));
            BAIL_WITH_LWCA_ERROR(dwError, LWCA_PLUGIN_FAILURE);
        }
        BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Lightwave AuthZ failed");
    }

    if (!bAuthorized)
    {
        LWCA_LOG_ALERT(
                "Unauthorized Request! UPN (%s) API (%d)",
                pReqCtx->pszBindUPN,
                apiPermissions);
    }

    *pbAuthorized = bAuthorized;


cleanup:

    LWCA_LOCK_MUTEX_UNLOCK(&gAuthZCtx.pMutex, bLocked);

    return dwError;

error:

    if (pbAuthorized)
    {
        *pbAuthorized = FALSE;
    }

    goto cleanup;
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
    gAuthZCtx.pszPluginPath = NULL;
    LwCAAuthZFunctionTableFree(gAuthZCtx.pFT);
    gAuthZCtx.pFT = NULL;
    LwCAPluginDeinitialize(gAuthZCtx.pPluginHandle);
    gAuthZCtx.pPluginHandle = NULL;

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
