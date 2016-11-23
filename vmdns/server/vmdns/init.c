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
 * Module Name:  init.c
 *
 * Abstract: VMware Domain Name Service.
 *
 */

#include "includes.h"

static
DWORD
InitializeResouceLimit(
    VOID
    );

/*
 * Initialize vmdnsd components
 */
DWORD
VmDnsInit()
{
    DWORD dwError = 0;
    DWORD dwEnableDNS = 0;

#ifndef _WIN32
    dwError = InitializeResouceLimit();
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = VmDnsAllocateMutex(&gVmdnsGlobals.pMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvInitialize(TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockInitialize();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcServerInit();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsConfigGetDword(VMDNS_KEY_VALUE_ENABLE_PROTOCOL, &dwEnableDNS);
    if (dwError)
    {
        gVmdnsGlobals.bEnableDNSProtocol = TRUE;
    }
    else
    {
        gVmdnsGlobals.bEnableDNSProtocol = (dwEnableDNS != 0);
    }

    if (gVmdnsGlobals.bEnableDNSProtocol)
    {
        dwError = VmDnsInitProtocolServer();
        BAIL_ON_VMDNS_ERROR(dwError);
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}


/*
 * Set process resource limits
 */
static
DWORD
InitializeResouceLimit(
    VOID
    )
{
    DWORD           dwError = 0;
    BAIL_ON_VMDNS_ERROR(dwError);

#ifndef _WIN32
    struct rlimit   VMLimit = {0};

    // unlimited virtual memory
    VMLimit.rlim_cur = RLIM_INFINITY;
    VMLimit.rlim_max = RLIM_INFINITY;

    dwError = setrlimit(RLIMIT_AS, &VMLimit);
    if (dwError != 0)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

#endif

error:

    return dwError;
}
