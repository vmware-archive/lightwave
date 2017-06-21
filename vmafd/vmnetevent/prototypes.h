/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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



#pragma once


DWORD
VmNetEventOpenConnection(
    VMNET_EVENT_TYPE eventType,
    PVMNETEVENT_FD pEventFD
    );

DWORD
VmNetEventWaitOnEvent(
    VMNETEVENT_FD EventFd,
    PFN_VMNETEVENT_CALLBACK pfnCallBack,
    pthread_t* pEventThread
    );

VOID
VmNetEventCloseConnection(
    VMNETEVENT_FD FD
    );

/*linux_api.c*/

DWORD
VmLinuxOpenConnection(
    DWORD dwEventType,
    PVMNETEVENT_FD pFD
    );

DWORD
VmLinuxWaitOnEvent(
    VMNETEVENT_FD FD,
    PFN_VMNETEVENT_CALLBACK pCallback,
    pthread_t* pEventThread
    );

VOID
VmLinuxCloseConnection(
    VMNETEVENT_FD FD
    );

