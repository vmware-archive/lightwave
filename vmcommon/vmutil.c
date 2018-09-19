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

#include "includes.h"

/*
 * when hash map does not own key and value pair.
 */
VOID
VmNoopHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUnused
    )
{
    return;
}

/*
 * when hash map can use simple free function for both key and value.
 */
VOID
VmSimpleHashMapPairFree(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pKey);
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pValue);
}

/*
 * when hash map can use simple free function for key only.
 */
VOID
VmSimpleHashMapPairFreeKeyOnly(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pKey);
}

/*
 * when hash map can use simple free function for value only.
 */
VOID
VmSimpleHashMapPairFreeValOnly(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VM_COMMON_SAFE_FREE_MEMORY(pPair->pValue);
}
