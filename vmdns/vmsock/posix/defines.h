/*
 * Copyright (c) VMware Inc.  All rights Reserved.
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
