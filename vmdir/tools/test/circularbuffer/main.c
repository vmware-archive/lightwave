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

typedef struct _TEST_ELEMENT
{
    PCSTR   name;
    int     age;
} TEST_ELEMENT, *PTEST_ELEMENT;
int             arrLen = 5;
TEST_ELEMENT    arrTestData[] = {
                            {"user1", 1},
                            {"user2", 2},
                            {"user3", 3},
                            {"user4", 4},
                            {"user5", 5}};

void FillBuffer(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    UINT Count
    )
{
    UINT i = 0;

    for (; i < Count; ++i)
    {
        PTEST_ELEMENT Destination = VmDirCircularBufferGetNextEntry(pCircularBuffer);
        PTEST_ELEMENT Source;

        Source = &arrTestData[i % 5];
        Destination->name = Source->name;
        Destination->age = Source->age;
    }
}

BOOLEAN Callback(PVOID Element, PVOID Context)
{
    PTEST_ELEMENT TestElement = (PTEST_ELEMENT)Element;
    PTEST_ELEMENT ReferenceElement = (PTEST_ELEMENT)Context;

    ASSERT(TestElement->age == ReferenceElement->age);
    ASSERT(strcmp(TestElement->name, ReferenceElement->name) == 0);

    return TRUE;
}

void TestCleanupOfValidCircularBuffer()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
    DWORD dwError = 0;

    dwError = VmDirCircularBufferCreate(4, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    VmDirCircularBufferFree(pCircularBuffer);
}

void TestCleanupOfNullCircularBuffer()
{
    VmDirCircularBufferFree(NULL);
}

void TestSingleElement()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
    DWORD dwError = 0;
    PTEST_ELEMENT Element = NULL;

    printf("TestSingleElement() ...\n");

    dwError = VmDirCircularBufferCreate(4, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrTestData[0].name;
    Element->age = arrTestData[0].age;

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 1, Callback, &arrTestData[0]);
    ASSERT(dwError == 0);

    VmDirCircularBufferFree(pCircularBuffer);
}

void TestWrap()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer;
    PTEST_ELEMENT Element;
    DWORD dwError = 0;

    printf("TestWrap() ...\n");

    dwError = VmDirCircularBufferCreate(3, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrTestData[0].name;
    Element->age = arrTestData[0].age;

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrTestData[1].name;
    Element->age = arrTestData[1].age;

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrTestData[2].name;
    Element->age = arrTestData[2].age;

    Element = VmDirCircularBufferGetNextEntry(pCircularBuffer);
    Element->name = arrTestData[3].name;
    Element->age = arrTestData[3].age;

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 1, Callback, &arrTestData[1]);
    ASSERT(dwError == 0);

    VmDirCircularBufferFree(pCircularBuffer);
}

void TestZeroSizedBufferShouldFail()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    printf("TestZeroSizedBufferShouldFail() ...\n");

    dwError = VmDirCircularBufferCreate(0, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError != 0);
}

void TestOverflowSizedBufferShouldFail()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    printf("TestOverFlowSizedBufferShouldFail() ...\n");

    dwError = VmDirCircularBufferCreate((0XFFFFFFFF / sizeof(TEST_ELEMENT)) + 2, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError != 0);
}

void TestMakeCapacityBiggerShouldSucceed()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    printf("TestMakeCapacityBiggerShouldSucceed() ...\n");

    dwError = VmDirCircularBufferCreate(2, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    dwError = VmDirCircularBufferSetCapacity(pCircularBuffer, 4);
    ASSERT(dwError == 0);
}

void TestMakeCapacitySmallerShouldSucceed()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;

    printf("TestMakeCapacitySmallerShouldSucceed() ...\n");

    dwError = VmDirCircularBufferCreate(4, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    FillBuffer(pCircularBuffer, 4);

    dwError = VmDirCircularBufferSetCapacity(pCircularBuffer, 2);
    ASSERT(pCircularBuffer->dwCapacity == 2);
    ASSERT(dwError == 0);
    ASSERT(pCircularBuffer->dwHead < pCircularBuffer->dwCapacity);
}

BOOLEAN Callback2(PVOID Element, PVOID Context)
{
    PDWORD pdwCount = (PDWORD)Context;

    switch (*pdwCount)
    {
        case 0:
        ASSERT(memcmp(Element, &arrTestData[3], sizeof(TEST_ELEMENT)) == 0);
        break;

        case 1:
        ASSERT(memcmp(Element, &arrTestData[4], sizeof(TEST_ELEMENT)) == 0);
        break;

        case 2:
        ASSERT(memcmp(Element, &arrTestData[0], sizeof(TEST_ELEMENT)) == 0);
        break;

        case 3:
        ASSERT(memcmp(Element, &arrTestData[1], sizeof(TEST_ELEMENT)) == 0);
        break;
    }

    *pdwCount += 1;
    return TRUE;
}

void TestSelectReturnsCorrectElementsInCorrectOrder()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwCount = 0;

    printf("TestSelectReturnsCorrectElementsInCorrectOrder() ...\n");

    dwError = VmDirCircularBufferCreate(4, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    FillBuffer(pCircularBuffer, 7);

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 4, Callback2, &dwCount);
    ASSERT(dwError == 0);
}

BOOLEAN CountingCallback(PVOID Element, PVOID Context)
{
    PDWORD pdwCount = (PDWORD)Context;
    PTEST_ELEMENT TestElement = (PTEST_ELEMENT)Element;

    if (TestElement->age == 3)
    {
        return FALSE;
    }

    *pdwCount += 1;
    return TRUE;
}

void TestSelectReturnsWhenCallbackReturnsFalse()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwCount = 0;

    printf("TestSelectReturnsWhenCallbackReturnsFalse() ...\n");

    dwError = VmDirCircularBufferCreate(4, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    FillBuffer(pCircularBuffer, 4);

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 4, CountingCallback, &dwCount);
    ASSERT(dwError == 0);
    ASSERT(dwCount == 2);
}

void TestSelectTooManyElementsQuietlySucceeds()
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwCount = 0;

    printf("TestSelectTooManyElementsQuietlySucceeds() ...\n");

    dwError = VmDirCircularBufferCreate(4, sizeof(TEST_ELEMENT), &pCircularBuffer);
    ASSERT(dwError == 0);

    FillBuffer(pCircularBuffer, 4);

    dwError = VmDirCircularBufferSelectElements(pCircularBuffer, 20, CountingCallback, &dwCount);
    ASSERT(dwError == 0);
}

int
main(int argc, char* argv[])
{
    TestSingleElement();
    TestWrap();

    TestZeroSizedBufferShouldFail();
    TestOverflowSizedBufferShouldFail();
    TestMakeCapacityBiggerShouldSucceed();
    TestMakeCapacitySmallerShouldSucceed();
    TestSelectReturnsCorrectElementsInCorrectOrder();
    TestSelectReturnsWhenCallbackReturnsFalse();
    TestSelectTooManyElementsQuietlySucceeds();
    TestCleanupOfValidCircularBuffer();
    TestCleanupOfNullCircularBuffer();

    return 0;
}
