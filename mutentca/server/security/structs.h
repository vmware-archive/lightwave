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

#ifndef _LWCA_MUTENTCA_SECURITY_STRUCTS_H_
#define _LWCA_MUTENTCA_SECURITY_STRUCTS_H_

typedef struct _LWCA_SECURITY_CONTEXT
{
    pthread_rwlock_t                securityMutex;
    PSTR                            pszPlugin;
    PLWCA_SECURITY_INTERFACE        pInterface;
    PLWCA_PLUGIN_HANDLE             pPluginHandle;
    PLWCA_SECURITY_HANDLE           pHandle;
    BOOLEAN                         isInitialized;
    LWCA_SECURITY_CAP_OVERRIDE      capOverride;
    pthread_mutex_t                 storageMutex;
    PLW_HASHMAP                     pStorageMap;
} LWCA_SECURITY_CONTEXT, *PLWCA_SECURITY_CONTEXT;

#endif /* _LWCA_MUTENTCA_SECURITY_STRUCTS_H_ */
