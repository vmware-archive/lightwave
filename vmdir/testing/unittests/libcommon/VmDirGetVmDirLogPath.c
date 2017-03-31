/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#define LOG_SUBDIRECTORY "UnitTests"

#ifndef _WIN32
VOID
_Test_VmDirGetVmDirLogPathSucceeds(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szPath[MAX_PATH] = {0};

    dwError = VmDirGetVmDirLogPath(szPath, LOG_SUBDIRECTORY);
    TestAssertEquals(dwError, 0);
    TestAssert(strlen(VMDIR_LOG_DIR) != 0);
    TestAssertStrEquals(szPath, VMDIR_LOG_DIR LOG_SUBDIRECTORY);
}
#endif

VOID
TestVmDirGetVmDirLogPath(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirGetVmDirLogPath ...");

#ifndef _WIN32
    _Test_VmDirGetVmDirLogPathSucceeds(pState);
#endif

    printf(" PASSED\n");
}
