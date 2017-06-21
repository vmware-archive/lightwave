/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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



typedef struct __VMNET_EVENT_FD
{
    DWORD dwNetlinkFD;
} VMNETEVENT_FD, *PVMNETEVENT_FD;

#ifndef _PFN_VMNETEVENT_CALLBACK
#define _PFN_VMNETEVENT_CALLBACK 1
typedef DWORD (*PFN_VMNETEVENT_CALLBACK) (VOID);
#endif
typedef struct __VMNETEVENT_DATA
{
    VMNETEVENT_FD eventFd;
    PFN_VMNETEVENT_CALLBACK pfnCallBack;
}VMNETEVENT_DATA, *PVMNETEVENT_DATA;

typedef struct __VMNETEVENT_HANDLE
{
     pthread_t eventThread;
     pthread_t* peventThread;
     VMNETEVENT_FD  fd;
     DWORD dwRefCount;
} VMNETEVENT_HANDLE;


typedef DWORD (*PFN_VMNETEVENT_OPEN_CONNECTION) (DWORD dwEventType, PVMNETEVENT_FD pFD);
typedef DWORD (*PFN_VMNETEVENT_WAIT_EVENT) (VMNETEVENT_FD EventFd, PFN_VMNETEVENT_CALLBACK pfnCallback, pthread_t* pEventThread);
typedef VOID (*PFN_VMNETEVET_CLOSE_CONNECTION) (VMNETEVENT_FD FD);

typedef struct _VMNET_EVENT_VTABLE
{
    PFN_VMNETEVENT_OPEN_CONNECTION pfnOpenConnection;
    PFN_VMNETEVENT_WAIT_EVENT pfnWaitEvent;
    PFN_VMNETEVET_CLOSE_CONNECTION pfnCloseConnection;
} VMNET_EVENT_VTABLE, *PVMNET_EVENT_TABLE;

