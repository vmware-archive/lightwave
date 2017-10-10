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

#include "includes.h"

DWORD
TestSetup(
    PVMDIR_TEST_STATE pState
    )
{
    return 0;
}

DWORD
TestCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    return 0;
}

DWORD
TestRunner(
    PVMDIR_TEST_STATE pState
    )
{
    TestCircularBufferCode(pState);
    TestDequeCode(pState);
    TestRegistryCode(pState);
    TestVmDirParseArguments(pState);
    TestVmDirAllocateStringA(pState);
    TestVmDirAllocateStringOfLenA(pState);
    TestVmDirAllocateStringPrintf(pState);
    TestVmDirGetVmDirLogPath(pState);
    TestVmDirStringList(pState);
    TestVmDirReadString(pState);
    TestVmDirStringCpyA(pState);
    TestVmDirStringNCpyA(pState);
    TestVmDirStringCatA(pState);
    TestVmDirStringNCatA(pState);
    TestVmDirStringToTokenList(pState);
    return 0;
}
