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

typedef struct _LWCA_AUTHZ_CONTEXT
{
    BOOLEAN                         bInitialized;
    BOOLEAN                         bPluginLoaded;
    pthread_rwlock_t                pMutex;
    PSTR                            pszPluginPath;
    PLWCA_AUTHZ_FUNCTION_TABLE      pFT;
    PLWCA_PLUGIN_HANDLE             pPluginHandle;
} LWCA_AUTHZ_CONTEXT, *PLWCA_AUTHZ_CONTEXT;
