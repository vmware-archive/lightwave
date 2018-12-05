#include "CuTest.h"
#include "../server/common/includes.h"
#include "vmdnscommon.h"

void
TestRecordListCreate(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pList = NULL;
    dwError = VmDnsRecordListCreate(&pList);
    CuAssert(tc, "TestRecordListCreate: Error code check.", !dwError);
    CuAssert(tc, "TestRecordListCreate: Error code check.", !!pList);
    CuAssert(tc, "TestRecordListCreate: Zero entries.", pList->dwCurrentSize == 0);
    CuAssert(tc, "TestRecordListCreate: Max Size set.", pList->dwMaxSize == 10);
    CuAssert(tc, "TestRecordListCreate: Records Allocated.", pList->ppRecords != NULL);
    VmDnsRecordListRelease(pList);
    CuAssert(tc, "VmDnsLruFree succeeded.", TRUE);
}

void
TestRecordListAddObject(
    CuTest* tc
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pList = NULL;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_OBJECT pRecordObject = NULL;
    PVMDNS_RECORD_OBJECT pListObject = NULL;

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (PVOID*)&pRecord);
    CuAssert(tc, "VmDnsAllocateMemory: Error code check.", !dwError);

    dwError = VmDnsRecordObjectCreate(pRecord, &pRecordObject);
    CuAssert(tc, "VmDnsRecordObjectCreate: Error code check.", !dwError);

    dwError = VmDnsRecordListCreate(&pList);
    CuAssert(tc, "VmDnsRecordListCreate: Error code check.", !dwError);

    dwError = VmDnsRecordListAdd(pList, pRecordObject);

    CuAssert(tc, "VmDnsRecordListCreate: Error code check.", !dwError);
    CuAssert(tc, "TestRecordListCreate: Zero entries.", pList->dwCurrentSize == 1);

    pListObject = VmDnsRecordListGetRecord(pList, 0);
    CuAssert(tc, "TestRecordListCreate: Zero entries.", pListObject == pRecordObject);
    CuAssert(tc, "TestRecordListCreate: Zero entries.", pRecordObject->nRefCount == 2);

    VmDnsRecordListRelease(pList);
    CuAssert(tc, "TestRecordListCreate: Zero entries.", pRecordObject->nRefCount == 1);

    VmDnsRecordObjectRelease(pRecordObject);
    CuAssert(tc, "VmDnsLruFree succeeded.", TRUE);
}

void
TestRecordListRemoveObject(
    CuTest* tc
    )
{
    PVMDNS_RECORD_LIST pList = NULL;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_OBJECT pRecordObject = NULL;

    VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (PVOID*)&pRecord);
    VmDnsRecordObjectCreate(pRecord, &pRecordObject);
    VmDnsRecordListCreate(&pList);
    VmDnsRecordListAdd(pList, pRecordObject);

    VmDnsRecordListRemove(pList, pRecordObject);
    CuAssert(tc, "TestRecordListRemoveObject: Zero entries.", pList->dwCurrentSize == 0);
    CuAssert(tc, "TestRecordListRemoveObject: Released Object.", pRecordObject->nRefCount == 1);

    VmDnsRecordListRelease(pList);
    VmDnsRecordObjectRelease(pRecordObject);
}

void
TestRecordListAddObject100(
    CuTest* tc
    )
{
    DWORD i = 0;
    PVMDNS_RECORD_LIST pList = NULL;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_OBJECT pRecordObject = NULL;

    VmDnsRecordListCreate(&pList);

    for (i = 0; i < 100; ++i)
    {
        VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (PVOID*)&pRecord);
        VmDnsRecordObjectCreate(pRecord, &pRecordObject);
        VmDnsRecordListAdd(pList, pRecordObject);
        VmDnsRecordObjectRelease(pRecordObject);
    }

    CuAssert(tc, "TestRecordListRemoveObject: Zero entries.", pList->dwCurrentSize == 100);

    VmDnsRecordListRelease(pList);
}

CuSuite* CuGetRecordListSuite(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestRecordListCreate);
    SUITE_ADD_TEST(suite, TestRecordListAddObject);
    SUITE_ADD_TEST(suite, TestRecordListRemoveObject);
    SUITE_ADD_TEST(suite, TestRecordListAddObject100);

    return suite;
}
