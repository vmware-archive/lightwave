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

#include "CuTest.h"
#include "../server/common/includes.h"
#include "vmdnscommon.h"

void TestTrimDomainName(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PSTR pszHost = NULL;
    PCSTR pszDomainName = "vsphere.local";
    PCSTR pszFQDN = "vsphere.local.";
    PSTR pszHostNames[] = { "www", "www.dev", "www.backend.dev" };
    DWORD idx = 0;

    for (; idx < sizeof(pszHostNames)/sizeof(PSTR); ++idx)
    {
        dwError = VmDnsAllocateStringPrintfA(
                    &pszHost,
                    "%s.%s",
                    pszHostNames[idx],
                    pszDomainName);
        CuAssertTrue(tc, !dwError);

        VmDnsTrimDomainNameSuffix(pszHost, pszDomainName);
        CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, pszHostNames[idx], FALSE));
        VMDNS_SAFE_FREE_STRINGA(pszHost);

        dwError = VmDnsAllocateStringPrintfA(
                    &pszHost,
                    "%s.%s",
                    pszHostNames[idx],
                    pszFQDN);
        CuAssertTrue(tc, !dwError);

        VmDnsTrimDomainNameSuffix(pszHost, pszDomainName);
        CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, pszHostNames[idx], FALSE));
        VMDNS_SAFE_FREE_STRINGA(pszHost);
    }

    for (idx = 0; idx < sizeof(pszHostNames)/sizeof(PSTR); ++idx)
    {
        dwError = VmDnsAllocateStringA(pszHostNames[idx], &pszHost);
        CuAssertTrue(tc, !dwError);

        VmDnsTrimDomainNameSuffix(pszHost, pszDomainName);
        CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, pszHostNames[idx], FALSE));
        VMDNS_SAFE_FREE_STRINGA(pszHost);
    }
    VmDnsTrimDomainNameSuffix(pszHost, "");
    CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, pszHost, FALSE));
    VmDnsTrimDomainNameSuffix(pszHost, NULL);
    CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, pszHost, FALSE));
    VmDnsTrimDomainNameSuffix(NULL, NULL);
    CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, NULL, FALSE));
    VmDnsTrimDomainNameSuffix(NULL, "vsphere.local");
    CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, NULL, FALSE));

    dwError = VmDnsAllocateStringA(pszDomainName, &pszHost);
    CuAssertTrue(tc, !dwError);
    VmDnsTrimDomainNameSuffix(pszHost, pszHost);
    CuAssertTrue(tc, !VmDnsStringCompareA(pszHost, pszDomainName, FALSE));

    VmDnsTrimDomainNameSuffix("", "vsphere.local");
}

typedef struct _PtrNameTestData
{
    PSTR  pszNetworkId;
    PSTR  pszZoneName;
    DWORD dwError;
} PtrNameTestData;

void TestPtrName(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    PSTR  pszZoneName = NULL;
    PtrNameTestData ptrTestData[] = {
        { "192.168.1.0/24", "1.168.192.in-addr.arpa",   0},
        { "192.168.1.0/16", "168.192.in-addr.arpa",     0},
        { "10.118.0.0/8",   "10.in-addr.arpa",          0},
        { "10.118.0.0/19",   "10.in-addr.arpa",         ERROR_INVALID_PARAMETER},
        { "fc00:10:118:97:8152:a429:635d:1284/64", "7.9.0.0.8.1.1.0.0.1.0.0.0.0.c.f.ip6.arpa", 0 },
        { "fc00:10:118:97:8152:a429:635d:1284/56", "0.0.8.1.1.0.0.1.0.0.0.0.c.f.ip6.arpa", 0},
        { "fc00:10:118:97:8152:a429:635d:1284/48", "8.1.1.0.0.1.0.0.0.0.c.f.ip6.arpa", 0},
        { "fc00:10:118:97:8152:a429:635d:1284/40", "1.0.0.1.0.0.0.0.c.f.ip6.arpa", 0 },
        { "fc00:10:118:97:8152:a429:635d:1284/32", "0.1.0.0.0.0.c.f.ip6.arpa", 0 }
    };

    for (; idx < sizeof(ptrTestData)/sizeof(PtrNameTestData); ++idx)
    {
        VmDnsAllocateStringA(ptrTestData[idx].pszNetworkId, &ptrTestData[idx].pszNetworkId);
        VmDnsAllocateStringA(ptrTestData[idx].pszZoneName, &ptrTestData[idx].pszZoneName);
        dwError = VmDnsGenerateReversZoneNameFromNetworkId(
                    ptrTestData[idx].pszNetworkId,
                    &pszZoneName);
        printf("pszZoneName: %s\n", pszZoneName);
        CuAssertTrue(tc, dwError == ptrTestData[idx].dwError);
        if (ptrTestData[idx].dwError == 0)
        {
            CuAssertTrue(tc,!VmDnsStringCompareA(pszZoneName, ptrTestData[idx].pszZoneName, FALSE));
            VMDNS_SAFE_FREE_STRINGA(pszZoneName);
        }
        VMDNS_SAFE_FREE_STRINGA(ptrTestData[idx].pszNetworkId);
        VMDNS_SAFE_FREE_STRINGA(ptrTestData[idx].pszZoneName);
    }
}

void TestReverseZoneName(CuTest* tc)
{
    DWORD dwError = 0;
    DWORD idx = 0;
    int family = AF_INET;
    PSTR pszPtrName = NULL;
    PCSTR pszNetworkIds[] = {
        "192.168.1.1",
        "127.0.0.1",
        "fc00:10:20:116:41d1:2e00:1760:cd2b",
        "fc00:10:20::41d1:2e00:1760:cd2b",
        "fc00:10:20::2e00:1760:cd2b",
        "fe80::20c:29ff:fe35:1e05",
        "::1"
    };

    for (; idx < sizeof(pszNetworkIds)/sizeof(PCSTR); ++idx)
    {
        dwError = VmDnsGeneratePtrNameFromIp(pszNetworkIds[idx], &family, &pszPtrName);
        CuAssert(tc, "Generating PTR name from ip address should succeed.", !dwError);
    }
}

typedef struct _StringTrimTestData
{
    PSTR pszStr;
    PSTR pszSearch;
    PSTR pszResult;
} StringTrimTestData;

void VmDnsTestTrimString(
    CuTest* tc
    )
{
    DWORD idx = 0;
    StringTrimTestData strTrimTestData[] = {
        { NULL,                     ".vsphere.local.",  NULL},
        { "host1.vsphere.local.",   "",                 "host1.vsphere.local."},
        { "host1.vsphere.local.",   NULL,               "host1.vsphere.local."},
        { NULL,                     NULL,               NULL},
        { "host1.vsphere.local.",   ".",                "host1.vsphere.local"},
        { "host1.vsphere.local.",   ".vsphere.local.",  "host1"},
        { "host1.vsphere.local.",   ".vsphere.local",   "host1.vsphere.local."},
        { "host1.vsphere.local.",   ".vsphere.",        "host1.vsphere.local."},
    };

    for (; idx < sizeof(strTrimTestData)/sizeof(PtrNameTestData); ++idx)
    {
        VmDnsAllocateStringA(
            strTrimTestData[idx].pszStr,
            &strTrimTestData[idx].pszStr);
        VmDnsStringTrimA(
            strTrimTestData[idx].pszStr,
            strTrimTestData[idx].pszSearch,
            FALSE);
        CuAssertTrue(tc,
            VmDnsStringCompareA(
                strTrimTestData[idx].pszStr,
                strTrimTestData[idx].pszResult,
                FALSE) == 0);
        VMDNS_SAFE_FREE_STRINGA(strTrimTestData[idx].pszStr);
    }
}

CuSuite* CuGetUtilSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, TestTrimDomainName);
    SUITE_ADD_TEST(suite, TestPtrName);
    SUITE_ADD_TEST(suite, TestReverseZoneName);
	SUITE_ADD_TEST(suite, VmDnsTestTrimString);

	return suite;
}

