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
_LwCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PLWCA_PLUGIN_HANDLE     *ppPluginHandle
    );

static
VOID
_LwCAPluginDeinitialize(
    PLWCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    );

DWORD
LwCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    )
{
    return _LwCAPluginInitialize(
               pcszPluginPath,
               LWCA_PLUGIN_LOAD_FUNC,
               pPluginVTable,
               ppPluginHandle);
}

VOID
LwCAPluginDeinitialize(
    PLWCA_PLUGIN_HANDLE     pPluginHandle
    )
{
    return _LwCAPluginDeinitialize(pPluginHandle, LWCA_PLUGIN_UNLOAD_FUNC);
}

DWORD
LwCAPluginInitializeCustom(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    )
{
    return _LwCAPluginInitialize(
               pcszPluginPath,
               pcszLoadFnName,
               pPluginVTable,
               ppPluginHandle);
}

VOID
LwCAPluginDeinitializeCustom(
    PLWCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    )
{
    return _LwCAPluginDeinitialize(pPluginHandle, pcszUnloadFnName);
}

static
DWORD
_LwCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PLWCA_PLUGIN_HANDLE     *ppPluginHandle
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszDLError = NULL;
    PLWCA_PLUGIN_HANDLE     pPluginHandle = NULL;
    PFN_LWCA_PLUGIN_LOAD    pfnPluginLoad = NULL;

    if (IsNullOrEmptyString(pcszPluginPath) ||
        IsNullOrEmptyString(pcszLoadFnName) ||
        !pPluginVTable ||
        !ppPluginHandle)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCALoadLibrary(pcszPluginPath, &pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    pfnPluginLoad = (PFN_LWCA_PLUGIN_LOAD)LwCAGetLibSym(pPluginHandle, pcszLoadFnName);
    pszDLError = LWCA_SAFE_STRING(dlerror());
    if (!IsNullOrEmptyString(pszDLError))
    {
        LWCA_LOG_ERROR(
            "[%s,%d] Failed to lookup load function (%s) from plugin (%s). Error (%s)",
            __FUNCTION__,
            __LINE__,
            LWCA_PLUGIN_LOAD_FUNC,
            pcszPluginPath,
            LWCA_SAFE_STRING(dlerror()));
        dwError = LWCA_ERROR_DLL_SYMBOL_NOTFOUND;
    }
    if (!pfnPluginLoad)
    {
        LWCA_LOG_ERROR(
            "[%s,%d] Retrieved NULL load function (%s) from plugin (%s)",
            __FUNCTION__,
            __LINE__,
            LWCA_PLUGIN_LOAD_FUNC,
            pcszPluginPath);
        dwError = LWCA_ERROR_DLL_SYMBOL_NOTFOUND;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = pfnPluginLoad(pPluginVTable);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppPluginHandle = pPluginHandle;

cleanup:

    return dwError;

error:

    LwCACloseLibrary(pPluginHandle);
    if (ppPluginHandle)
    {
        *ppPluginHandle = NULL;
    }

    goto cleanup;
}

static
VOID
_LwCAPluginDeinitialize(
    PLWCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    )
{
    PFN_LWCA_PLUGIN_UNLOAD      pfnPluginUnload = NULL;

    if (pPluginHandle && !IsNullOrEmptyString(pcszUnloadFnName))
    {
        pfnPluginUnload = (PFN_LWCA_PLUGIN_UNLOAD)LwCAGetLibSym(pPluginHandle, pcszUnloadFnName);
        if (!IsNullOrEmptyString(LWCA_SAFE_STRING(dlerror())) && pfnPluginUnload)
        {
            pfnPluginUnload();
        }

        LwCACloseLibrary(pPluginHandle);
    }
}
