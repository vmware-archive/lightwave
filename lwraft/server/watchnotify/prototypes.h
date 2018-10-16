/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

/* event.c
 */

DWORD
VmDirEventInit(
    PVDIR_EVENT*   ppEventNode
    );

VOID
VmDirEventFree(
    PVDIR_EVENT    pEventNode
    );

/* eventlist.c
 */

DWORD
VmDirEventListInit(
    PVDIR_EVENT_LIST*   ppEventList
    );

DWORD
VmDirEventListGetNext(
    PVDIR_EVENT_LIST    pEventList,
    PVDIR_EVENT_NODE    pCurEvent,
    PVDIR_EVENT_NODE*   ppNextEvent
    );

DWORD
VmDirEventListAddNode(
    PVDIR_EVENT_LIST    pEventList,
    PVDIR_EVENT_NODE    pEventNode,
    BOOL                bInLock
    );

DWORD
VmDirEventListFreeHead(
    PVDIR_EVENT_LIST    pEventList,
    BOOL                bInLock
    );

VOID
VmDirEventListFree(
    PVDIR_EVENT_LIST    pEventList
    );

/* eventnode.c
 */

DWORD
VmDirEventNodeInit(
    PVDIR_EVENT         pEvent,
    PVDIR_EVENT_NODE*   ppEventNode
    );

DWORD
VmDirEventNodeRelease(
    PVDIR_EVENT_NODE   pEventNode
    );

DWORD
VmDirEventNodeAcquire(
    PVDIR_EVENT_NODE   pEventNode
    );

VOID
VmDirEventNodeFree(
    PVDIR_EVENT_NODE    pEventNode
    );
