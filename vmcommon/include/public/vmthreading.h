/*
 * Copyright ©  2019 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef _VM_COMMON_THREADING_H__
#define _VM_COMMON_THREADING_H__

typedef struct _VM_MUTEX*    PVM_MUTEX;
typedef struct _VM_RWLOCK*   PVM_RWLOCK;
typedef struct _VM_COND*     PVM_COND;

DWORD
VmAllocateMutex(
    PVM_MUTEX*  ppMutex
    );

DWORD
VmInitializeMutexContent(
    PVM_MUTEX   pMutex
    );

VOID
VmFreeMutex(
    PVM_MUTEX   pMutex
    );

VOID
VmFreeMutexContent(
    PVM_MUTEX   pMutex
    );

DWORD
VmLockMutex(
    PVM_MUTEX   pMutex
    );

DWORD
VmUnLockMutex(
    PVM_MUTEX   pMutex
    );

BOOLEAN
VmIsMutexInitialized(
    PVM_MUTEX   pMutex
    );

DWORD
VmAllocateRWLock(
    PVM_RWLOCK*  ppLock
    );

DWORD
VmInitializeRWLockContent(
    PVM_RWLOCK   pLock
    );

VOID
VmFreeRWLock(
    PVM_RWLOCK   pLock
    );

VOID
VmFreeRWLockContent(
    PVM_RWLOCK   pLock
    );

DWORD
VmRWLockReadLock(
    PVM_RWLOCK   pLock,
    DWORD       dwMilliSec
    );

DWORD
VmRWLockWriteLock(
    PVM_RWLOCK   pLock,
    DWORD       dwMilliSec
    );

DWORD
VmRWLockUnlock(
    PVM_RWLOCK   pLock
    );


BOOLEAN
VmIsRWLockInitialized(
    PVM_RWLOCK   pLock
    );

DWORD
VmAllocateCondition(
    PVM_COND*   ppCondition
    );

DWORD
VmInitializeConditionContent(
    PVM_COND    pCondition
    );

VOID
VmFreeCondition(
    PVM_COND    pCondition
    );

VOID
VmFreeConditionContent(
    PVM_COND    pCondition
    );

DWORD
VmConditionWait(
    PVM_COND    pCondition,
    PVM_MUTEX   pMutex
    );

DWORD
VmConditionTimedWait(
    PVM_COND    pCondition,
    PVM_MUTEX   pMutex,
    DWORD       dwMilliseconds
    );

DWORD
VmConditionSignal(
    PVM_COND    pCondition
    );

DWORD
VmConditionBroadcast(
    PVM_COND    pCondition
    );

#endif /* __VM_COMMON_THREADING_H__ */
