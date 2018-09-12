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

DWORD
VMCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PVOID                   pPluginVTable,
    PVMCA_PLUGIN_HANDLE     *ppPluginHandle
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszDLError = NULL;
    PVMCA_PLUGIN_HANDLE     pPluginHandle = NULL;
    PFN_VMCA_PLUGIN_LOAD    pfnPluginLoad = NULL;

    if (IsNullOrEmptyString(pcszPluginPath) || !pPluginVTable || !ppPluginHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCALoadLibrary(pcszPluginPath, &pPluginHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    pfnPluginLoad = (PFN_VMCA_PLUGIN_LOAD)VMCAGetLibSym(pPluginHandle, VMCA_PLUGIN_LOAD_FUNC);
    pszDLError = VMCA_SAFE_STRING(dlerror());
    if (!IsNullOrEmptyString(pszDLError))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Failed to lookup load function (%s) from plugin (%s). Error (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_PLUGIN_LOAD_FUNC,
            pcszPluginPath,
            VMCA_SAFE_STRING(dlerror()));
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
    PFN_VMCA_PLUGIN_UNLOAD      pfnPluginUnload = NULL;

    if (pPluginHandle)
    {
        pfnPluginUnload = (PFN_VMCA_PLUGIN_UNLOAD)VMCAGetLibSym(pPluginHandle, VMCA_PLUGIN_UNLOAD_FUNC);
        if (!IsNullOrEmptyString(VMCA_SAFE_STRING(dlerror())) && pfnPluginUnload)
        {
            pfnPluginUnload();
        }

        VMCACloseLibrary(pPluginHandle);
    }
}
