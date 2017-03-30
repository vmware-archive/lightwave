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

VM_SOCK_PACKAGE gVmSockPosixPackage =
{
    .pfnOpenClientSocket = &VmSockPosixOpenClient,
    .pfnOpenServerSocket = &VmSockPosixOpenServer,
    .pfnCreateEventQueue = &VmSockPosixCreateEventQueue,
    .pfnAddEventQueue = &VmSockPosixEventQueueAdd,
    .pfnWaitForEvent = &VmSockPosixWaitForEvent,
    .pfnCloseEventQueue = &VmSockPosixCloseEventQueue,
    .pfnSetNonBlocking = &VmSockPosixSetNonBlocking,
    .pfnSetTimeOut = &VmSockPosixSetTimeOut,
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
