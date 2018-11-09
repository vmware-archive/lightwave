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
    PVDIR_EVENT*   ppEvent
    );

DWORD
VmDirEventRelease(
    PVDIR_EVENT   pEvent
    );

DWORD
VmDirEventAcquire(
    PVDIR_EVENT   pEvent
    );

VOID
VmDirEventFree(
    PVDIR_EVENT    pEvent
    );

/* eventrepo.c
 */

DWORD
VmDirEventRepoInit(
    PVDIR_EVENT_REPO*   ppEventRepo
    );

DWORD
VmDirEventRepoGetNextReadyEvent(
    PVDIR_EVENT_REPO            pEventRepo,
    PVDIR_EVENT_REPO_COOKIE*    ppEventRepoCookie,
    PVDIR_EVENT*                ppNextEvent
    );

DWORD
VmDirEventRepoAddPendingEvent(
    PVDIR_EVENT_REPO    pEventRepo,
    PVDIR_EVENT         pEvent,
    BOOLEAN             bInLock
    );

DWORD
VmDirEventRepoSync(
    PVDIR_EVENT_REPO    pEventRepo,
    int64_t             iTimeoutMs
    );

VOID
VmDirEventRepoFree(
    PVDIR_EVENT_REPO    pEventRepo
    );
