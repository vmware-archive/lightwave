/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
VmAuthsvcCleanupGlobals(
    VOID
    );

/*
 * Server shutdown
 */
VOID
VmAuthsvcShutdown(
    VOID)
{
    VmAuthsvcRpcServerShutdown();

    VmAuthsvcCleanupGlobals();
}

static
VOID
VmAuthsvcCleanupGlobals(
    VOID
    )
{
    // Free Server global 'gVmauthsvcServerGlobals' upon shutdown
    VMAUTHSVC_SAFE_FREE_MEMORY(gVmauthsvcGlobals.pszLogFile);
    pthread_mutex_destroy( &gVmauthsvcGlobals.mutex );
}
