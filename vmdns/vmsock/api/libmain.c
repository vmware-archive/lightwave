/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

DWORD
VmwSockInitialize(
    VOID
    )
{
    DWORD dwError = 0;

    if (!gpVmSockPackage)
    {
#ifdef _WIN32
        dwError = VmWinSockInitialize(&gpVmSockPackage);
#else
        dwError = VmSockPosixInitialize(&gpVmSockPackage);
#endif
    }

    return dwError;
}

VOID
VmwSockShutdown(
    VOID
    )
{
    if (gpVmSockPackage)
    {
#ifdef _WIN32
        VmWinSockShutdown(gpVmSockPackage);
#else
        VmSockPosixShutdown(gpVmSockPackage);
#endif
    }
}
