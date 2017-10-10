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


#ifndef _WIN32
VOID
_Test_VmDirReadStringSucceeds(
    PVMDIR_TEST_STATE pState
    )
{
    CHAR wszString[10] =  {0};
    FILE *pfOldStdin = NULL;
    PSTR pszData = "This is a test string";

    pfOldStdin = stdin;
    stdin = fmemopen(pszData, strlen(pszData), "r");
    VmDirReadString("", wszString, sizeof(wszString), TRUE);
    TestAssert(strncmp(wszString, pszData, sizeof(wszString) - 1) == 0);

    stdin = pfOldStdin;
}
#endif


VOID
TestVmDirReadString(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirReadString ...");

#ifndef _WIN32
    _Test_VmDirReadStringSucceeds(pState);
#endif

    printf(" PASSED\n");
}
