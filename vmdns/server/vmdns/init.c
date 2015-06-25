/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
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
    DWORD   dwError = 0;

#ifndef _WIN32
    dwError = InitializeResouceLimit();
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = VmDnsAllocateMutex(&gVmdnsGlobals.pMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCoreInit();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockInitialize();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStartDirectorySync();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcServerInit();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsInitProtocolServer();
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
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
