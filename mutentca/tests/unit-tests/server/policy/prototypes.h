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

#ifdef __cplusplus
extern "C" {
#endif

// util.c

VOID
Test_LwCAPolicyCfgObjInit(
    VOID **state
    );

VOID
Test_LwCAPolicyCfgObjArrayInit(
    VOID **state
    );

VOID
Test_LwCAPoliciesInit(
    VOID **state
    );

VOID
Test_LwCAPoliciesInit_Empty(
    VOID **state
    );

VOID
Test_LwCAPolicyCfgObjCopy(
    VOID **state
    );

VOID
Test_LwCAPolicyCfgObjArrayCopy(
    VOID **state
    );

// parse.c

VOID
Test_LwCAPolicyInitCtx_Valid_AllCombinations(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Valid_OnlyCertPolicy(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Valid_OnlyCAPolicy(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Invalid_Type(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Invalid_Match(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Invalid_TypeMatchCombo(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Invalid_MissingValueForTypeMatchCombo(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Invalid_KeyUsage(
    VOID **state
    );

VOID
Test_LwCAPolicyInitCtx_Invalid_MissingTypeMatchValue(
    VOID **state
    );

#ifdef __cplusplus
}
#endif
