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


/*
 * Module Name:  shutdown.c
 *
 * Abstract: VMware Domain Name Service.
 *
 * Server shutdown sequence
 */

#include "includes.h"

static
VOID
VmDnsCleanupGlobals(
    VOID
    );

/*
 * Server shutdown
 */
VOID
VmDnsShutdown(
    VOID)
{
    VmDnsRpcServerShutdown();
    VmDnsShutdownProtocolServer();
    VmDnsSrvCleanup();
    VmDnsSockShutdown();
    VmDnsCleanupGlobals();
    VmDnsRESTServerShutdown();
    VmMetricsDestroy(gVmDnsMetricsContext);
}

static
VOID
VmDnsCleanupGlobals(
    VOID
    )
{
    VMDNS_SAFE_FREE_MEMORY(gVmdnsGlobals.pszLogFile);
    VmDnsFreeMutex( gVmdnsGlobals.pMutex);
    VMDNS_SAFE_FREE_MEMORY(gVmdnsGlobals.pszRestListenPort);
}
