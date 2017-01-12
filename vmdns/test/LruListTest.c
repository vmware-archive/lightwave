#include "CuTest.h"
#include "../server/common/includes.h"
#include "vmdnscommon.h"

void 
TestLruInitialize(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PVMDNS_LRU_LIST pList = NULL;
    dwError = VmDnsLruInitialize(&pList);
    CuAssert(tc, "VmDnsLruInitialize: Error code check.", !dwError);
    CuAssert(tc, "VmDnsLruInitialize: Error code check.", !!pList);
    CuAssert(tc, "VmDnsLruInitialize: Zero entries.", pList->dwCurrentCount == 0);
    CuAssert(tc, "VmDnsLruInitialize: Lower threshold set.", pList->dwLowerThreshold != 0);
    CuAssert(tc, "VmDnsLruInitialize: Upper threshold set.", pList->dwUpperThreshold != 0);
    CuAssert(tc, "VmDnsLruInitialize: Max size set.", pList->dwMaxCount != 0);
    VmDnsLruFree(pList);
    CuAssert(tc, "VmDnsLruFree succeeded.", TRUE);
}

void
TestLruAddRemoveList(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PVMDNS_LRU_LIST pList = NULL;
    VMDNS_NAME_ENTRY ne1 = { { NULL, NULL }, "vsphere.local", NULL };

    dwError = VmDnsLruInitialize(&pList);
    CuAssert(tc, "VmDnsLruInitialize: Allocation check.", !!pList);

    dwError = VmDnsLruAddNameEntry(pList, &ne1);
    CuAssert(tc, "VmDnsLruInitialize: Allocation check.", !dwError);
    CuAssert(tc, "VmDnsLruInitialize: Size incremented.", pList->dwCurrentCount == 1);
    dwError = VmDnsLruRemoveNameEntry(pList, &ne1);
    CuAssert(tc, "VmDnsLruInitialize: Allocation check.", !dwError);
    CuAssert(tc, "VmDnsLruInitialize: Size incremented.", pList->dwCurrentCount == 0);

    VmDnsLruFree(pList);
    CuAssert(tc, "VmDnsLruFree succeeded.", TRUE);
}

CuSuite* CuGetLruSuite(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestLruInitialize);
    SUITE_ADD_TEST(suite, TestLruAddRemoveList);

    return suite;
}
