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
_VMCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PVMCA_PLUGIN_HANDLE     *ppPluginHandle
    );

static
VOID
_VMCAPluginDeinitialize(
    PVMCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    );

DWORD
VMCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PVOID                   pPluginVTable,
    PVMCA_PLUGIN_HANDLE     *ppPluginHandle
    )
{
    return _VMCAPluginInitialize(
               pcszPluginPath,
               VMCA_PLUGIN_LOAD_FUNC,
               pPluginVTable,
               ppPluginHandle);
}

static
DWORD
_VMCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PVMCA_PLUGIN_HANDLE     *ppPluginHandle
    )
{
    DWORD                   dwError = 0;
    PCSTR                    pszDLError = NULL;
    PVMCA_PLUGIN_HANDLE     pPluginHandle = NULL;
    PFN_VMCA_PLUGIN_LOAD    pfnPluginLoad = NULL;

    if (IsNullOrEmptyString(pcszPluginPath) || !pPluginVTable || !ppPluginHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCALoadLibrary(pcszPluginPath, &pPluginHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    pfnPluginLoad = (PFN_VMCA_PLUGIN_LOAD)VMCAGetLibSym(pPluginHandle, pcszLoadFnName);

    pszDLError = VMCAGetLibError();
    if (!IsNullOrEmptyString(pszDLError))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Failed to lookup load function (%s) from plugin (%s). Error (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_PLUGIN_LOAD_FUNC,
            pcszPluginPath,
            pszDLError);
        dwError = VMCA_ERROR_DLL_SYMBOL_NOTFOUND;
    }

    if (!pfnPluginLoad)
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Retrieved NULL load function (%s) from plugin (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_PLUGIN_LOAD_FUNC,
            pcszPluginPath);
        dwError = VMCA_ERROR_DLL_SYMBOL_NOTFOUND;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = pfnPluginLoad(pPluginVTable);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppPluginHandle = pPluginHandle;

cleanup:

    return dwError;

error:

    VMCACloseLibrary(pPluginHandle);
    if (ppPluginHandle)
    {
        *ppPluginHandle = NULL;
    }

    goto cleanup;
}

VOID
VMCAPluginDeinitialize(
    PVMCA_PLUGIN_HANDLE         pPluginHandle
    )
{
    return _VMCAPluginDeinitialize(pPluginHandle, VMCA_PLUGIN_UNLOAD_FUNC);
}

static
VOID
_VMCAPluginDeinitialize(
    PVMCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    )
{
    PFN_VMCA_PLUGIN_UNLOAD      pfnPluginUnload = NULL;

    if (pPluginHandle)
    {
        pfnPluginUnload = (PFN_VMCA_PLUGIN_UNLOAD)VMCAGetLibSym(pPluginHandle, pcszUnloadFnName);
        if (!IsNullOrEmptyString(VMCAGetLibError()) && pfnPluginUnload)
        {
            pfnPluginUnload();
        }

        VMCACloseLibrary(pPluginHandle);
    }
}

DWORD
VMCAPluginInitializeCustom(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    )
{
    return _VMCAPluginInitialize(
               pcszPluginPath,
               pcszLoadFnName,
               pPluginVTable,
               ppPluginHandle);
}

VOID
VMCAPluginDeinitializeCustom(
    PVMCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    )
{
    return _VMCAPluginDeinitialize(pPluginHandle, pcszUnloadFnName);
}
