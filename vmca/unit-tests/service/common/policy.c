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

// Unit Test Helper Functions

//int
//VMCAPolicyArrayTestSetup(
//    VOID                **state
//    )
//{
//    DWORD               dwError = 0;
//    PVMCA_POLICY        *ppPolicies = NULL;
//
//    *state = ppPolicies;
//
//    return 0;
//}

//int
//VMCAPolicyArrayTestFree(
//    VOID    **state
//    )
//{
//    VMCAPolicyArrayFree((PVMCA_POLICY *)*state);
//
//    return 0;
//}

// Unit Tests

VOID
VMCAPolicyInit_ValidInput(
    VOID            **state
    )
{
    DWORD           dwError = 0;
    DWORD           dwIdx = 0;
    PVMCA_POLICY    *ppPolicies = NULL;

    dwError = VMCAPolicyInit(
                    SNPOLICY_VALID_CONFIG_PATH,
                    &ppPolicies
                    );
    assert_int_equal(dwError, 0);
    assert_non_null(ppPolicies);

    for (; dwIdx < 1; ++dwIdx)
    {
        assert_non_null(ppPolicies[dwIdx]);
    }

    VMCAPolicyArrayFree(ppPolicies);
}
