/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"
#include "vmregconfigincludes.h"

VOID
VmRegConfigConstructorKVFree(
    PVM_CONSTRUCT_KV    pConstructor
    )
{
    if (pConstructor)
    {
        VM_COMMON_SAFE_FREE_STRINGA(pConstructor->pszValue);
        VM_COMMON_SAFE_FREE_MEMORY(pConstructor);
    }
}

VOID
VmRegConfigKVFree(
    PVM_REGCONFIG_LIST_KV   pKV
    )
{
    if (pKV)
    {
        VM_COMMON_SAFE_FREE_STRINGA(pKV->pszKey);
        VM_COMMON_SAFE_FREE_STRINGA(pKV->pszValue);
        VM_COMMON_SAFE_FREE_MEMORY(pKV);
    }
}

VOID
VmRegConfigListKVFree(
    PVM_REGCONFIG_LIST_KV   pListKV
    )
{
    PVM_REGCONFIG_LIST_KV pCurr = NULL;
    PVM_REGCONFIG_LIST_KV pNext = NULL;

    for (pCurr = pListKV; pCurr; pCurr = pNext)
    {
        pNext = pCurr->pNext;

        VmRegConfigKVFree(pCurr);
    }
}

VOID
VmRegConfigListEntryFree(
    PVM_REGCONFIG_LIST_ENTRY    pListEntry
    )
{
    PVM_REGCONFIG_LIST_ENTRY pCurr = NULL;
    PVM_REGCONFIG_LIST_ENTRY pNext = NULL;

    if (pListEntry)
    {
        for (pCurr = pListEntry; pCurr; pCurr = pNext)
        {
            pNext = pCurr->pNext;

            VmRegConfigListKVFree(pCurr->pListKV);

            VM_COMMON_SAFE_FREE_STRINGA(pCurr->pszFileName);
            VM_COMMON_SAFE_FREE_STRINGA(pCurr->pszLockFileName);
            VM_COMMON_SAFE_FREE_STRINGA(pCurr->pszTopKey);
            VM_COMMON_SAFE_FREE_MUTEX(pCurr->pMutex);

            VM_COMMON_SAFE_FREE_MEMORY(pCurr);
        };
    }
}
