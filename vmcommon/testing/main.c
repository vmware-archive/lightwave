/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
#include "vmregconfigtest.h"
#include "vmmetricstest.h"
#include "vmhttpclienttest.h"
#include "vmsignaturetest.h"
#include "vmjsonresulttest.h"
#include "vmjsonresultobjectmaptest.h"
#include "vmjsonresultfromobjectmaptest.h"

int main(int argc, char *argv[])
{
    DWORD dwError = 0;

    dwError += VmRegConfigTest();
    dwError += VmMetricsCounterTest();
    dwError += VmMetricsGaugeTest();
    dwError += VmMetricsHistogramTest();
    dwError += VmMetricsMixedTest();
    dwError += VmHttpClientTest();
    dwError += VmSignatureTest();
    dwError += VmJsonResultTest();
    dwError += VmJsonResultObjectMapTest();
    dwError += VmJsonResultFromObjectMapTest();

    if (dwError == 0)
    {
        printf("\nTEST PASSED\n");
    }
    else
    {
        printf("\nTEST FAILED. %d test failures\n", dwError);
    }
    return dwError;
}
