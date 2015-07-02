/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
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
    VmDnsStopDirectorySync();
    VmDnsCoreCleanup();
    VmwSockShutdown();
    VmDnsCleanupGlobals();
}

static
VOID
VmDnsCleanupGlobals(
    VOID
    )
{
    VMDNS_SAFE_FREE_MEMORY(gVmdnsGlobals.pszLogFile);
    VmDnsFreeMutex( gVmdnsGlobals.pMutex);
}
