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

VM_SOCK_PACKAGE gVmDnsSockPosixPackage =
{
    .pfnOpenClientSocket = &VmDnsSockPosixOpenClient,
    .pfnOpenServerSocket = &VmDnsSockPosixOpenServer,
    .pfnCreateEventQueue = &VmDnsSockPosixCreateEventQueue,
    .pfnAddEventQueue = &VmDnsSockPosixEventQueueAdd,
    .pfnRemoveEventQueue = &VmDnsSockPosixEventQueueRemove,
    .pfnWaitForEvent = &VmDnsSockPosixWaitForEvent,
    .pfnShutdownEventQueue = &VmDnsSockPosixShutdownEventQueue,
    .pfnFreeEventQueue = &VmDnsSockPosixFreeEventQueue,
    .pfnSetNonBlocking = &VmDnsSockPosixSetNonBlocking,
    .pfnSetTimeOut = &VmDnsSockPosixSetTimeOut,
    .pfnGetProtocol = &VmDnsSockPosixGetProtocol,
    .pfnSetData = &VmDnsSockPosixSetData,
    .pfnGetData = &VmDnsSockPosixGetData,
    .pfnRead = &VmDnsSockPosixRead,
    .pfnWrite = &VmDnsSockPosixWrite,
    .pfnAcquireSocket = &VmDnsSockPosixAcquireSocket,
    .pfnReleaseSocket = &VmDnsSockPosixReleaseSocket,
    .pfnCloseSocket = &VmDnsSockPosixCloseSocket,
    .pfnGetAddress = &VmDnsSockPosixGetAddress,
    .pfnAllocateIoBuffer = &VmDnsSockPosixAllocateIoBuffer,
    .pfnSetEventContext = &VmDnsSockPosixSetEventContext,
    .pfnGetEventContext = &VmDnsSockPosixGetEventContext,
    .pfnReleaseIoBuffer = &VmDnsSockPosixFreeIoBuffer
};

PVM_SOCK_PACKAGE gpVmDnsSockPosixPackage = &gVmDnsSockPosixPackage;
