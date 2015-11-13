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

#include <stdio.h>

#include "CuTest.h"

int  vmdns_syslog_level = 0;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

CuSuite* CuGetZoneSuite();
CuSuite* CuGetUtilSuite();

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuite* zoneSuite = CuGetZoneSuite();
    CuSuiteAddSuite(suite, zoneSuite);
    CuSuiteDelete(zoneSuite);

    CuSuite* utilSuite = CuGetUtilSuite();
    CuSuiteAddSuite(suite, utilSuite);
    CuSuiteDelete(utilSuite);

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    CuSuiteDelete(suite);
    CuStringDelete(output);
}

int main(void)
{
    RunAllTests();
        return 0;
}
