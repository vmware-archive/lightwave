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
DWORD
testEmpty(PDEQUE pDeque)
{
    DWORD           dwError = 0;
    PTEST_ELEMENT   pElement = NULL;
    //Test case: pop from empty queue
    printf("\nTest empty deque...\n");
    if (!dequeIsEmpty(pDeque))
    {
        printf("deque is not NULL.\n");
        goto error;
    }

    dwError = dequePopLeft(pDeque, (PVOID*)&pElement);
    if (dwError != ERROR_NO_MORE_ITEMS)
    {
        printf("PopLeft is not NULL from empty queue.\n");
        goto error;
    }
    else
    {
        dwError = 0;
    }

cleanup:
    printf("Test empty finished.\n");
    return dwError;
error:
    goto cleanup;
}

DWORD
testQueue(PDEQUE pDeque)
{
    DWORD           dwError = 0;
    PTEST_ELEMENT   pElement = NULL;
    int             i=0;

    printf("\nTesting Queue...\n");
    for (i=0; i<arrLen; i++)
    {
        dwError = dequePush(pDeque, &arrTestData[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i<arrLen; i++)
    {
        dwError = dequePopLeft(pDeque, (PVOID*)&pElement);
        BAIL_ON_VMDIR_ERROR(dwError);
        if(pElement != &arrTestData[i])
        {
            printf("test failed.\n");
            dwError = 1;
            goto error;
        }
        printf("Name: %s, Age: %d\n", pElement->name, pElement->age);
    }

cleanup:
    printf("testQueue finished.\n");
    return dwError;
error:
    goto cleanup;
}

DWORD
testStack(PDEQUE pDeque)
{
    DWORD           dwError = 0;
    PTEST_ELEMENT   pElement = NULL;
    int             i=0;

    printf("\nTesting Stack...\n");
    for (i=0; i<arrLen; i++)
    {
        dwError = dequePush(pDeque, &arrTestData[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i<arrLen; i++)
    {
        dwError = dequePop(pDeque, (PVOID*)&pElement);
        BAIL_ON_VMDIR_ERROR(dwError);
        if(pElement != &arrTestData[arrLen-i-1])
        {
            printf("test failed.\n");
            dwError = 1;
            goto error;
        }
        printf("Name: %s, Age: %d\n", pElement->name, pElement->age);
    }

cleanup:
    printf("testStack finished.\n");
    return dwError;
error:
    goto cleanup;
}

int
main(int argc, char* argv[])
{
    DWORD           dwError = 0;
    PDEQUE          pDeque = NULL;

    dwError = dequeCreate(&pDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = testEmpty(pDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = testQueue(pDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = testStack(pDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = testEmpty(pDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pDeque)
    {
        dequeFree(pDeque);
    }

    return dwError;
error:

    goto cleanup;
}
