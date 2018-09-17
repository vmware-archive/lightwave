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

VOID
LwCASrvSetState(
    LWCA_SERVER_STATE state
    )
{
    pthread_mutex_lock(&gLwCAServerGlobals.mutex);

    gLwCAServerGlobals.mutentcadState = state;

    pthread_mutex_unlock(&gLwCAServerGlobals.mutex);
}

LWCA_SERVER_STATE
LwCASrvGetState(
    VOID
    )
{
    LWCA_SERVER_STATE rtnState;

    pthread_mutex_lock(&gLwCAServerGlobals.mutex);

    rtnState = gLwCAServerGlobals.mutentcadState;

    pthread_mutex_unlock(&gLwCAServerGlobals.mutex);

    return rtnState;
}

VOID
LwCASrvSetFuncLevel(
    LWCA_FUNC_LEVEL dwFuncLevel
    )
{
    pthread_mutex_lock(&gLwCAServerGlobals.mutex);

    gLwCAServerGlobals.dwFuncLevel = dwFuncLevel;

    pthread_mutex_unlock(&gLwCAServerGlobals.mutex);
}

LWCA_FUNC_LEVEL
LwCASrvGetFuncLevel(
    VOID
    )
{
    LWCA_FUNC_LEVEL dwFuncLevel;

    pthread_mutex_lock(&gLwCAServerGlobals.mutex);

    dwFuncLevel = gLwCAServerGlobals.dwFuncLevel;

    pthread_mutex_unlock(&gLwCAServerGlobals.mutex);

    return dwFuncLevel;
}

VOID
LwCASrvCleanupGlobalState(
    VOID
    )
{
    BOOLEAN bLocked = FALSE;

    LWCA_LOCK_MUTEX(bLocked, &gLwCAServerGlobals.mutex);

    // Do server state cleanup here

    LWCA_UNLOCK_MUTEX(bLocked, &gLwCAServerGlobals.mutex);
}

