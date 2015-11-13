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


typedef enum _VMKDC_SERVICE_PORT_TYPE
{
    VMKDC_SERVICE_PORT_UDP = 1,
    VMKDC_SERVICE_PORT_TCP,
} VMKDC_SERVICE_PORT_TYPE;


DWORD
VmKdcSrvOpenServicePort(
    PVMKDC_GLOBALS pGlobals,
    VMKDC_SERVICE_PORT_TYPE portType);

DWORD
VmKdcSrvServicePortListen(
    PVMKDC_GLOBALS pGlobals);

DWORD
VmKdcInitConnAcceptThread(
    PVMKDC_GLOBALS pGlobals);

DWORD
VmKdcSendTcp(
    int sock,
    unsigned char *msg,
    int msgLen);

void
VmKdcSrvCloseSocketAcceptFd(
    VOID);

void
VmKdcSrvIncrementThreadCount(
    PVMKDC_GLOBALS pGlobals);

void
VmKdcSrvDecrementThreadCount(
    PVMKDC_GLOBALS pGlobals);

void
VmKdcSrvGetThreadCount(
    PVMKDC_GLOBALS pGlobals,
    PDWORD pWorkerThreadCount);
