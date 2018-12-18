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

#ifndef _VMCA_SRV_PLUGIN_H_
#define _VMCA_SRV_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef VMCA_LIB_HANDLE     PVMCA_PLUGIN_HANDLE;

DWORD
VMCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    );

VOID
VMCAPluginDeinitialize(
    PVMCA_PLUGIN_HANDLE     pPluginHandle
    );

DWORD
VMCAPluginInitializeCustom(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    );

VOID
VMCAPluginDeinitializeCustom(
    PVMCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    );

#ifdef __cplusplus
}
#endif

#endif /* _VMCA_SRV_PLUGIN_H_ */
