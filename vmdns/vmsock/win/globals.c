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
    .pfnSetTimeOut = &VmSockWinSetTimeOut,
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
