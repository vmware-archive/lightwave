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

typedef struct _DEQUEUE_TEST_ELEMENT
{
    PCSTR   name;
    int     age;
} DEQUEUE_TEST_ELEMENT, *PDEQUEUE_TEST_ELEMENT;

DEQUEUE_TEST_ELEMENT arrDequeTestData[] = {
    {"user1", 1},
    {"user2", 2},
    {"user3", 3},
    {"user4", 4},
    {"user5", 5}
};

VOID
testEmpty(
    PVMDIR_TEST_STATE pState,
    PDEQUE pDeque
    )
{
    DWORD dwError = 0;
    PDEQUEUE_TEST_ELEMENT pElement = NULL;
    BOOLEAN fEmpty = FALSE;

    //Test case: pop from empty queue
    fEmpty = dequeIsEmpty(pDeque);
    TestAssertEquals(fEmpty, TRUE);

    dwError = dequePopLeft(pDeque, (PVOID*)&pElement);
    TestAssertEquals(dwError, ERROR_NO_MORE_ITEMS);
}

VOID
testQueue(
    PVMDIR_TEST_STATE pState,
    PDEQUE pDeque
    )
{
    DWORD dwError = 0;
    PDEQUEUE_TEST_ELEMENT pElement = NULL;
    int i = 0;

    for (i = 0; i < VMDIR_ARRAY_SIZE(arrDequeTestData); i++)
    {
        dwError = dequePush(pDeque, &arrDequeTestData[i]);
        TestAssertEquals(dwError, ERROR_SUCCESS);
    }

    for (i = 0; i < VMDIR_ARRAY_SIZE(arrDequeTestData); i++)
    {
        dwError = dequePopLeft(pDeque, (PVOID*)&pElement);
        TestAssertEquals(dwError, ERROR_SUCCESS);
        TestAssertPtrEquals(pElement, &arrDequeTestData[i]);
    }
}

VOID
testStack(
    PVMDIR_TEST_STATE pState,
    PDEQUE pDeque
    )
{
    DWORD dwError = 0;
    PDEQUEUE_TEST_ELEMENT pElement = NULL;
    int i = 0;

    for (i = 0; i < VMDIR_ARRAY_SIZE(arrDequeTestData); i++)
    {
        dwError = dequePush(pDeque, &arrDequeTestData[i]);
        TestAssertEquals(dwError, ERROR_SUCCESS);
    }

    for (i = 0; i < VMDIR_ARRAY_SIZE(arrDequeTestData); i++)
    {
        dwError = dequePop(pDeque, (PVOID*)&pElement);
        TestAssertEquals(dwError, ERROR_SUCCESS);
        TestAssertPtrEquals(pElement, &arrDequeTestData[VMDIR_ARRAY_SIZE(arrDequeTestData)-i-1]);
    }
}

VOID
TestDequeCode(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PDEQUE pDeque = NULL;

    printf("Testing deque code ...");

    dwError = dequeCreate(&pDeque);
    TestAssertEquals(dwError, ERROR_SUCCESS);

    testEmpty(pState, pDeque);
    testQueue(pState, pDeque);
    testStack(pState, pDeque);
    testEmpty(pState, pDeque);

    dequeFree(pDeque);

    printf(" PASSED\n");
}
