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

// For now only client socket part is portable

VM_SOCK_PACKAGE gVmDnsSockWinPackage =
{
    .pfnOpenClientSocket = &VmDnsSockWinOpenClient,
    .pfnOpenServerSocket = &VmDnsSockWinOpenServer,
    .pfnCreateEventQueue = &VmDnsSockWinCreateEventQueue,
    .pfnAddEventQueue = &VmDnsSockWinEventQueueAdd,
    .pfnWaitForEvent = &VmDnsSockWinWaitForEvent,
    .pfnStartListening = &VmDnsSockWinStartListening,
    .pfnShutdownEventQueue = &VmDnsSockWinCloseEventQueue,
    .pfnSetNonBlocking = &VmDnsSockWinSetNonBlocking,
    .pfnSetTimeOut = &VmDnsSockWinSetTimeOut,
    .pfnGetProtocol = &VmDnsSockWinGetProtocol,
    .pfnSetData = &VmDnsSockWinSetData,
    .pfnGetData = &VmDnsSockWinGetData,
    .pfnRead = &VmDnsSockWinRead,
    .pfnWrite = &VmDnsSockWinWrite,
    .pfnAcquireSocket = &VmDnsSockWinAcquire,
    .pfnReleaseSocket = &VmDnsSockWinRelease,
    .pfnCloseSocket = &VmDnsSockWinClose,
    .pfnGetAddress = &VmDnsSockWinGetAddress,
    .pfnAllocateIoBuffer = &VmDnsSockWinAllocateIoBuffer,
    .pfnReleaseIoBuffer = &VmDnsSockWinFreeIoBuffer,
    .pfnCreateTimerSocket = &VmDnsSockWinCreateTimerSocket,
};

PVM_SOCK_PACKAGE gpVmWinSockPackage = &gVmDnsSockWinPackage;
