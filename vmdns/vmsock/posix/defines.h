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


#define VM_SOCK_POSIX_DEFAULT_LISTEN_QUEUE_SIZE (5)

#define VM_SOCK_POSIX_DEFAULT_QUEUE_SIZE        (64)

#define BAIL_ON_POSIX_SOCK_ERROR(dwError) \
        if (dwError) \
            goto error;

#ifndef PopEntryList
#define PopEntryList(ListHead) \
    (ListHead)->Next;\
        {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
                }                             \
        }
#endif

#ifndef PushEntryList
#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)
#endif

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (ULONG_PTR)(&((type *)0)->field)))

#endif

typedef enum
{
    VM_SOCK_POSIX_EVENT_STATE_UNKNOWN = 0,
    VM_SOCK_POSIX_EVENT_STATE_WAIT,
    VM_SOCK_POSIX_EVENT_STATE_PROCESS
} VM_SOCK_POSIX_EVENT_STATE;
