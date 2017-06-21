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

#ifndef _VMNETEVENT_H_
#define _VMNETEVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __VMNETEVENT_HANDLE* PVMNETEVENT_HANDLE;

#ifndef _PFN_VMNETEVENT_CALLBACK
#define _PFN_VMNETEVENT_CALLBACK 1
typedef DWORD (*PFN_VMNETEVENT_CALLBACK) (VOID);
#endif

typedef enum
{
    VMNET_EVENT_TYPE_UNDEFINED = 0,
    VMNET_EVENT_TYPE_IPCHANGE,
    VMNET_EVENT_TYPE_MAX
} VMNET_EVENT_TYPE, *PVMNETEVENT_TYPE;

DWORD
VmNetEventRegister(
    VMNET_EVENT_TYPE vmEventType,
    PFN_VMNETEVENT_CALLBACK pfnCallBack,
    PVMNETEVENT_HANDLE* ppEventHandle
    );

VOID
VmNetEventUnregister(
    PVMNETEVENT_HANDLE pEventHandle
    );

#ifdef __cplusplus
}
#endif

#endif /* _VMNETEVENT_H_ */
