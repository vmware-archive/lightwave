/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

// For now only client socket part is portable

VM_SOCK_PACKAGE gVmSockWinPackage =
{
    .pfnOpenClientSocket = &VmSockWinOpenClient,
    .pfnOpenServerSocket = &VmSockWinOpenServer,
    .pfnCreateEventQueue = &VmSockWinCreateEventQueue,
    .pfnAddEventQueue = &VmSockWinEventQueueAdd,
    .pfnWaitForEvent = &VmSockWinWaitForEvent,
    .pfnStartListening = &VmSockWinStartListening,
    .pfnCloseEventQueue = &VmSockWinCloseEventQueue,
    .pfnSetNonBlocking = &VmSockWinSetNonBlocking,
    .pfnGetProtocol = &VmSockWinGetProtocol,
    .pfnSetData = &VmSockWinSetData,
    .pfnGetData = &VmSockWinGetData,
    .pfnRead = &VmSockWinRead,
    .pfnWrite = &VmSockWinWrite,
    .pfnAcquireSocket = &VmSockWinAcquire,
    .pfnReleaseSocket = &VmSockWinRelease,
    .pfnCloseSocket = &VmSockWinClose,
    .pfnGetAddress = &VmSockWinGetAddress,
    .pfnAllocateIoBuffer = &VmSockWinAllocateIoBuffer,
    .pfnReleaseIoBuffer = &VmSockWinFreeIoBuffer
};

PVM_SOCK_PACKAGE gpVmWinSockPackage = &gVmSockWinPackage;
