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

#define ASSERT(a) if (!(a)) { \
                    printf("Assertion failed ==> %s (%s:%d)\n", #a, __FILE__, __LINE__); \
                    exit(0); \
                  }

void TestDwordRoundTrip()
{
    DWORD dwTestValue = 0;
    DWORD dwComparisonValue = 0;
    DWORD dwError = 0;

    printf("TestDwordRoundTrip() ...\n");

    dwTestValue = 42;
    dwError = VmDirSetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, "TestValue", dwTestValue);
    ASSERT(dwError == 0);

    dwError = VmDirGetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, "TestValue", &dwComparisonValue, 0);
    ASSERT(dwError == 0);

    ASSERT(dwTestValue == dwComparisonValue);
}

void TestDwordDefaultValue()
{
    DWORD dwTestValue = 0;
    DWORD dwComparisonValue = 0;
    DWORD dwError = 0;

    printf("TestDwordDefaultValue() ...\n");

    dwTestValue = 42;
    dwError = VmDirGetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, "TestValueDoesNotExist", &dwComparisonValue, dwTestValue);
    ASSERT(dwError != 0);

    ASSERT(dwTestValue == dwComparisonValue);
}

void TestMaxDwordValueRoundTrip()
{
    DWORD dwTestValue = 0;
    DWORD dwComparisonValue = 0;
    DWORD dwError = 0;

    printf("TestMaxDwordValueRoundTrip() ...\n");

    dwTestValue = 0xFFFFFFFF; // Biggest possible DWORD
    dwError = VmDirSetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, "TestMaxValue", dwTestValue);
    ASSERT(dwError == 0);

    VmDirGetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_KEY_PATH, "TestMaxValue", &dwComparisonValue, 0);
    ASSERT(dwError == 0);

    ASSERT(dwTestValue == dwComparisonValue);
}


int
main(int argc, char* argv[])
{
    TestDwordRoundTrip();
    TestDwordDefaultValue();
    TestMaxDwordValueRoundTrip();

    return 0;
}
