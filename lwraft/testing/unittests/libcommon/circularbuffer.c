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

typedef struct _CIRCULAR_BUFFER_TEST_ELEMENT
{
    PVMDIR_TEST_STATE pState;
    PCSTR   name;
    int     age;
} CIRCULAR_BUFFER_TEST_ELEMENT, *PCIRCULAR_BUFFER_TEST_ELEMENT;

CIRCULAR_BUFFER_TEST_ELEMENT arrCircularBufferTestData[] = {
                            {NULL, "user1", 1},
                            {NULL, "user2", 2},
                            {NULL, "user3", 3},
                            {NULL, "user4", 4},
                            {NULL, "user5", 5}};

VOID
FillBuffer(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    UINT Count
    )
{
    UINT i = 0;

    for (; i < Count; ++i)
    {
        PCIRCULAR_BUFFER_TEST_ELEMENT Destination = VmDirCircularBufferGetNextEntry(pCircularBuffer);
        PCIRCULAR_BUFFER_TEST_ELEMENT Source;

        Source = &arrCircularBufferTestData[i % 5];
        Destination->name = Source->name;
        Destination->age = Source->age;
    }
}

BOOLEAN
Callback(
    PVOID Element,
    PVOID Context
    )
{
    PCIRCULAR_BUFFER_TEST_ELEMENT TestElement = (PCIRCULAR_BUFFER_TEST_ELEMENT)Element;
    PCIRCULAR_BUFFER_TEST_ELEMENT ReferenceElement = (PCIRCULAR_BUFFER_TEST_ELEMENT)Context;
    PVMDIR_TEST_STATE pState = NULL;

    pState = TestElement->pState;
    TestAssertEquals(TestElement->age, ReferenceElement->age);
    TestAssertStrEquals(TestElement->name, ReferenceElement->name);

    return TRUE;
}

VOID
TestCleanupOfValidCircularBuffer(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate(4, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    VmDirCircularBufferFree(pCircularBuffer);
}

VOID
TestCleanupOfNullCircularBuffer(
    PVMDIR_TEST_STATE pState
    )
{
    VmDirCircularBufferFree(NULL);
}

VOID
TestSingleElement(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
    DWORD dwError = 0;
    PCIRCULAR_BUFFER_TEST_ELEMENT Element = NULL;

    dwError = VmDirCircularBufferCreate(4, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrCircularBufferTestData[0].name;
    Element->age = arrCircularBufferTestData[0].age;
    Element->pState = pState;

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 1, Callback, &arrCircularBufferTestData[0]);
    TestAssertEquals(dwError, 0);

    VmDirCircularBufferFree(pCircularBuffer);
}

VOID
TestWrap(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
    PCIRCULAR_BUFFER_TEST_ELEMENT Element;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate(3, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrCircularBufferTestData[0].name;
    Element->age = arrCircularBufferTestData[0].age;

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrCircularBufferTestData[1].name;
    Element->age = arrCircularBufferTestData[1].age;

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrCircularBufferTestData[2].name;
    Element->age = arrCircularBufferTestData[2].age;

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrCircularBufferTestData[3].name;
    Element->age = arrCircularBufferTestData[3].age;

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 1, Callback, &arrCircularBufferTestData[1]);
    TestAssertEquals(dwError, 0);

    VmDirCircularBufferFree(pCircularBuffer);
}

VOID
TestZeroSizedBufferShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate(0, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertNotEquals(dwError, 0);
}

VOID
TestOverflowSizedBufferShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate((0XFFFFFFFF / sizeof(CIRCULAR_BUFFER_TEST_ELEMENT)) + 2, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertNotEquals(dwError, 0);
}

VOID
TestMakeCapacityBiggerShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate(2, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    dwError = VmDirCircularBufferSetCapacity(pCircularBuffer, 4);
    TestAssertEquals(dwError, 0);
}

VOID
TestMakeCapacitySmallerShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate(4, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    FillBuffer(pCircularBuffer, 4);

    dwError = VmDirCircularBufferSetCapacity(pCircularBuffer, 2);
    TestAssertEquals(pCircularBuffer->dwCapacity, 2);
    TestAssertEquals(dwError, 0);
    TestAssert(pCircularBuffer->dwHead < pCircularBuffer->dwCapacity);
}

BOOLEAN
Callback2(
    PVOID Element,
    PVOID Context
    )
{
    PCIRCULAR_BUFFER_TEST_ELEMENT TestElement = (PCIRCULAR_BUFFER_TEST_ELEMENT)Element;
    PDWORD pdwCount = (PDWORD)Context;
    PVMDIR_TEST_STATE pState = NULL;

    pState = TestElement->pState;

    switch (*pdwCount)
    {
        case 0:
        TestAssert(memcmp(TestElement, &arrCircularBufferTestData[3], sizeof(CIRCULAR_BUFFER_TEST_ELEMENT)) == 0);
        break;

        case 1:
        TestAssert(memcmp(TestElement, &arrCircularBufferTestData[4], sizeof(CIRCULAR_BUFFER_TEST_ELEMENT)) == 0);
        break;

        case 2:
        TestAssert(memcmp(TestElement, &arrCircularBufferTestData[0], sizeof(CIRCULAR_BUFFER_TEST_ELEMENT)) == 0);
        break;

        case 3:
        TestAssert(memcmp(TestElement, &arrCircularBufferTestData[1], sizeof(CIRCULAR_BUFFER_TEST_ELEMENT)) == 0);
        break;
    }

    *pdwCount += 1;
    return TRUE;
}

VOID
TestSelectReturnsCorrectElementsInCorrectOrder(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwCount = 0;

    dwError = VmDirCircularBufferCreate(4, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    FillBuffer(pCircularBuffer, 7);

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 4, Callback2, &dwCount);
    TestAssertEquals(dwError, 0);
}

BOOLEAN
CountingCallback(
    PVOID Element,
    PVOID Context
    )
{
    PDWORD pdwCount = (PDWORD)Context;
    PCIRCULAR_BUFFER_TEST_ELEMENT TestElement = (PCIRCULAR_BUFFER_TEST_ELEMENT)Element;

    if (TestElement->age == 3)
    {
        return FALSE;
    }

    *pdwCount += 1;
    return TRUE;
}

VOID
TestSelectReturnsWhenCallbackReturnsFalse(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwCount = 0;

    dwError = VmDirCircularBufferCreate(4, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    FillBuffer(pCircularBuffer, 4);

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 4, CountingCallback, &dwCount);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCount, 2);
}

VOID
TestSelectTooManyElementsQuietlySucceeds(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwCount = 0;

    dwError = VmDirCircularBufferCreate(4, sizeof(CIRCULAR_BUFFER_TEST_ELEMENT), &pCircularBuffer);
    TestAssertEquals(dwError, 0);

    FillBuffer(pCircularBuffer, 4);

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 20, CountingCallback, &dwCount);
    TestAssertEquals(dwError, 0);
}

VOID
TestCircularBufferCode(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing circular buffer code ...");

    TestSingleElement(pState);
    TestWrap(pState);

    TestZeroSizedBufferShouldFail(pState);
    TestOverflowSizedBufferShouldFail(pState);
    TestMakeCapacityBiggerShouldSucceed(pState);
    TestMakeCapacitySmallerShouldSucceed(pState);
    TestSelectReturnsCorrectElementsInCorrectOrder(pState);
    TestSelectReturnsWhenCallbackReturnsFalse(pState);
    TestSelectTooManyElementsQuietlySucceeds(pState);
    TestCleanupOfValidCircularBuffer(pState);
    TestCleanupOfNullCircularBuffer(pState);

    printf(" PASSED\n");
}
