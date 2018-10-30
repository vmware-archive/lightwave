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

#ifndef _LWCA_SRV_PLUGIN_H_
#define _LWCA_SRV_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef LWCA_LIB_HANDLE     PLWCA_PLUGIN_HANDLE;

DWORD
LwCAPluginInitialize(
    PCSTR                   pcszPluginPath,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    );

DWORD
LwCAPluginInitializeCustom(
    PCSTR                   pcszPluginPath,
    PCSTR                   pcszLoadFnName,
    PVOID                   pPluginVTable,
    PVOID                   *ppPluginHandle
    );

VOID
LwCAPluginDeinitializeCustom(
    PLWCA_PLUGIN_HANDLE     pPluginHandle,
    PCSTR                   pcszUnloadFnName
    );

VOID
LwCAPluginDeinitialize(
    PLWCA_PLUGIN_HANDLE     pPluginHandle
    );

#ifdef __cplusplus
}
#endif

#endif /* _LWCA_SRV_PLUGIN_H_ */
