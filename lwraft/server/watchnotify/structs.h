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
 */

typedef enum
{
    VDIR_EVENT_MOD = 0,
    VDIR_EVENT_ADD,
    VDIR_EVENT_DEL
}VDIR_EVENT_TYPE;

typedef struct _VDIR_EVENT
{
    BOOL            bIsEventReady;
    BOOL            bIsGroupUpdate;
    DWORD           revision;
    PVDIR_ENTRY     pCurEntry;
    PVDIR_ENTRY     pPrevEntry;
    VDIR_EVENT_TYPE eventType;
}VDIR_EVENT, *PVDIR_EVENT;

typedef struct _VDIR_EVENT_NODE
{
    DWORD                       refCount;
    PVMDIR_MUTEX                pMutex;
    PVDIR_EVENT                 pEvent;
    struct _VDIR_EVENT_LIST*    pEventList;
    struct _VDIR_EVENT_NODE*    pNext;
}VDIR_EVENT_NODE, *PVDIR_EVENT_NODE;

typedef struct _VDIR_EVENT_LIST
{
    PVDIR_EVENT_NODE    pHead;
    PVDIR_EVENT_NODE    pTail;
    PVMDIR_MUTEX        pMutex;
}VDIR_EVENT_LIST, *PVDIR_EVENT_LIST;
