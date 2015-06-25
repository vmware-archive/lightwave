/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

VM_SOCK_PACKAGE gVmSockPosixPackage =
{
    .pfnOpenClientSocket = &VmSockPosixOpenClient,
    .pfnOpenServerSocket = &VmSockPosixOpenServer,
    .pfnCreateEventQueue = &VmSockPosixCreateEventQueue,
    .pfnAddEventQueue = &VmSockPosixEventQueueAdd,
    .pfnWaitForEvent = &VmSockPosixWaitForEvent,
    .pfnCloseEventQueue = &VmSockPosixCloseEventQueue,
    .pfnSetNonBlocking = &VmSockPosixSetNonBlocking,
    .pfnGetProtocol = &VmSockPosixGetProtocol,
    .pfnSetData = &VmSockPosixSetData,
    .pfnGetData = &VmSockPosixGetData,
    .pfnRead = &VmSockPosixRead,
    .pfnWrite = &VmSockPosixWrite,
    .pfnAcquireSocket = &VmSockPosixAcquireSocket,
    .pfnReleaseSocket = &VmSockPosixReleaseSocket,
    .pfnCloseSocket = &VmSockPosixCloseSocket,
    .pfnGetAddress = &VmSockPosixGetAddress,
    .pfnAllocateIoBuffer = &VmSockPosixAllocateIoBuffer,
    .pfnReleaseIoBuffer = &VmSockPosixFreeIoBuffer
};

PVM_SOCK_PACKAGE gpVmSockPosixPackage = &gVmSockPosixPackage;
