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

LWCA_SERVER_GLOBALS gLwCAServerGlobals =
{
    // NOTE: order of fields MUST stay in sync with struct definition...
    LWCA_SF_INIT(.mutex, PTHREAD_MUTEX_INITIALIZER),
    LWCA_SF_INIT(.svcMutex, PTHREAD_RWLOCK_INITIALIZER),
    LWCA_SF_INIT(.fLwCALog, NULL),
    LWCA_SF_INIT(.mutentcadState, LWCAD_STARTUP),
    LWCA_SF_INIT(.dwFuncLevel, LWCA_FUNC_LEVEL_INITIAL),
    LWCA_SF_INIT(.pDirSyncParams, NULL),
    LWCA_SF_INIT(.pDirSyncThr, NULL),
    LWCA_SF_INIT(.gpEventLog, NULL),
    LWCA_SF_INIT(.pSslCtx, NULL)
};
