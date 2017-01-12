#include "CuTest.h"
#include "../server/common/includes.h"
#include "vmdnscommon.h"

void
TestNameEntryCreate(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PVMDNS_NAME_ENTRY pEntry = NULL;
    dwError = VmDnsNameEntryCreate("vsphere.local", &pEntry);
    CuAssert(tc, "TestNameEntryCreate: Error code check.", !dwError);
    CuAssert(tc, "TestNameEntryCreate: Entry allocated.", !!pEntry);
    CuAssert(tc, "TestNameEntryCreate: Name copied", !!pEntry->pszName);
    VmDnsNameEntryDelete(pEntry);
    CuAssert(tc, "TestNameEntryCreate succeeded.", TRUE);
}



CuSuite* CuGetNameEntrySuite(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestNameEntryCreate);
    return suite;
}
