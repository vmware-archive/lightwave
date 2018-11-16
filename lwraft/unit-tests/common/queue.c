/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

typedef struct _DEQUEUE_TEST_CONTEXT{
    PVDIR_QUEUE pInputQueue;
    int64_t     iTimeoutMs;
    PVOID       pElement;
    PVDIR_QUEUE pExpectedQueue;
}DEQUEUE_TEST_CONTEXT, *PDEQUEUE_TEST_CONTEXT;

typedef struct _ENQUEUE_TEST_CONTEXT{
    PVDIR_QUEUE pInputQueue;
    PVOID       pElement;
    PVDIR_QUEUE pExpectedQueue;
}ENQUEUE_TEST_CONTEXT, *PENQUEUE_TEST_CONTEXT;

typedef struct _QUEUE_COMPARE_TEST_CONTEXT{
    PVDIR_QUEUE pLeftQueue;
    PVDIR_QUEUE pRightQueue;
    BOOL        bExpected;
}QUEUE_COMPARE_TEST_CONTEXT, *PQUEUE_COMPARE_TEST_CONTEXT;

static
DWORD
_VmDirTestQueueDequeueInit(
    PDEQUEUE_TEST_CONTEXT*  ppDequeueContext
    );

static
DWORD
_VmDirTestQueueEnqueueInit(
    PENQUEUE_TEST_CONTEXT*  ppEnqueueContext
    );

static
DWORD
_VmDirTestQueueCompareInit(
    PQUEUE_COMPARE_TEST_CONTEXT*    ppCompareContext
    );

int
VmDirTestQueueSetupEnqueue_EmptyQueue(
    VOID    **state
    )
{
    DWORD                   dwError = 0;
    BOOL                    bInLock = FALSE;
    DEQUEUE_TEST_CONTEXT    pEnqueueContext = NULL;

    dwError = _VmDirTestQueueEnqueueInit(&pEnqueueContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(
            bInLock, pEnqueueContext->pExpectedQueue, (PVOID)pEnqueueContext->pszString);
    assert_int_equal(dwError, 0);

    *state = pEnqueueContext;

    return 0;
}

int
VmDirTestQueueSetupDequeue_Nonblocking(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    DWORD                   dwError = 0;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL;

    dwError = _VmDirTestQueueDequeueInit(&pDequeueContext);
    assert_int_equal(dwError,0);

    dwError = VmDirQueueEnqueue(
            bInLock, pDequeueContext->pInputQueue, (PVOID)pDequeueContext->pszString);
    assert_int_equal(dwError, 0);

    pDequeueContext->iTimeoutMs = 0;

    *state = pDequeueContext;

    return 0;
}

int
VmDirTestQueueSetupDequeue_Waiting(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    DWORD                   dwError = 0;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL;

    dwError = _VmDirTestQueueDequeueInit(&pDequeueContext);
    assert_int_equal(dwError,0);

    dwError = VmDirQueueEnqueue(
            bInLock, pDequeueContext->pInputQueue, (PVOID)pDequeueContext->pszString);
    assert_int_equal(dwError, 0);

    pDequeueContext->iTimeoutMs = 100;

    *state = pDequeueContext;

    return 0;
}

int
VmDirTestQueueSetupCompare_Equal(
    VOID    **state
    )
{
    BOOL                        bInLock = FALSE;
    PSTR                        pszString1 = NULL;
    PSTR                        pszString2 = NULL;
    DWORD                       dwError = 0;
    PQUEUE_COMPARE_TEST_CONTEXT pCompareContext = NULL;

    dwError = _VmDirTestQueueCompareInit(&pCompareContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("input1", &pszString1);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("input2", &pszString2);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString1);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString2);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pRightQueue, (PVOID)pszString1);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pRightQueue, (PVOID)pszString2);
    assert_int_equal(dwError, 0);

    pCompareContext->bExpected = TRUE;

    *state = pCompareContext;

    return 0;
}

int
VmDirTestQueueSetupCompare_NotEqual(
    VOID    **state
    )
{
    BOOL                        bInLock = FALSE;
    PSTR                        pszString1 = NULL;
    PSTR                        pszString2 = NULL;
    DWORD                       dwError = 0;
    PQUEUE_COMPARE_TEST_CONTEXT pCompareContext = NULL;

    dwError = _VmDirTestQueueCompareInit(&pCompareContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("input1", &pszString1);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("input2", &pszString2);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString1);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString2);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pRightQueue, (PVOID)pszString1);
    assert_int_equal(dwError, 0);

    pCompareContext->bExpected = FALSE;

    *state = pCompareContext;

    return 0;
}

int
VmDirTestQueueTestEnqueue(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    DWORD                   dwError = 0;
    int64_t                 iSizeBefore = 0;
    PENQUEUE_TEST_CONTEXT   pEnqueueContext = NULL;

    pEnqueueContext = (PENQUEUE_TEST_CONTEXT) *state;

    iSizeBefore = pEnqueueContext->pInputQueue->iSize;

    dwError = VmDirQueueEnqueue(bInLock, pEnqueueContext->pInputQueue, pEnqueueContext->pElement);
    assert_int_equal(dwError, 0);

    assert_int_equal(pInputQueue->iSize, iSizeBefore+1);
    assert_true(VmDirQueueCompare(pEnqueueContext->pInputQueue, pEnqueueContext->pExpectedQueue));

    return 0;
}

int
VmDirTestQueueTestDequeue(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    PSTR                    pszString = NULL;
    DWORD                   dwError = 0;
    int64_t                 iSizeBefore = 0;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL;

    pDequeueContext = (PDEQUEUE_TEST_CONTEXT) *state;

    iSizeBefore = pDequeueContext->pInputQueue->iSize;

    dwError = VmDirQueueDequeue(bInLock, pInputQueue, pDequeueContext->iTimeoutMs, &pszString);
    assert_int_equal(dwError, 0);

    assert_int_equal(pInputQueue->iSize, iSizeBefore-1);
    assert_int_equal(
            VmDirQueueCompare(pDequeueContext->pInputQueue, pDequeueContext->pExpectedQueue));
    assert_true((pszString == pDequeueContext->pszString));

    return 0;
}

int
VmDirTestQueueTestCompare(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PQUEUE_COMPARE_TEST_CONTEXT pCompareContext = NULL;

    pCompareContext = (PQUEUE_COMPARE_TEST_CONTEXT) *state;

    if (pCompareContext->bExpected)
    {
        assert_true(VmDirQueueCompare(pCompareContext->pLeftQueue, pCompareContext->pRightQueue));
    }
    else
    {
        assert_false(VmDirQueueCompare(pCompareContext->pLeftQueue, pCompareContext->pRightQueue));
    }

    return 0;
}

int
VmDirTestQueueTearDownEnqueue(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    PSTR                    pszString = NULL;
    DWORD                   dwError = 0;
    PENQUEUE_TEST_CONTEXT   pEnqueueContext = NULL;

    pEnqueueContext = (PENQUEUE_TEST_CONTEXT) *state;

    while (!dwError)
    {
        VmDirQueueDequeue(bInLock, pEnqueueContext->pInputQueue, 0, (PVOID*)&pszString);
        VMDIR_SAFE_FREE_MEMORY(pszString);
    }

    VmDirQueueFree(pEnqueueContext->pInputQueue);
    VmDirQueueFree(pEnqueueContext->pExpectedQueue);
    VMDIR_SAFE_FREE_MEMORY(pEnqueueContext);

    return 0;
}

int
VmDirTestQueueTearDownDequeue(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    PSTR                    pszString = NULL;
    DWORD                   dwError = 0;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL

    pDequeueContext = (PDEQUEUE_TEST_CONTEXT) *state;

    VMDIR_SAFE_FREE_MEMORY(pDequeueContext->pszString);
    VmDirQueueFree(pEnqueueContext->pInputQueue);
    VmDirQueueFree(pEnqueueContext->pExpectedQueue);
    VMDIR_SAFE_FREE_MEMORY(pDequeueContext);

    return 0;
}

int
VmDirTestQueueTearDownCompare(
    VOID    **state
    )
{
    BOOL                        bInLock = FALSE;
    PSTR                        pszString = NULL;
    DWORD                       dwError = 0;
    PQUEUE_COMPARE_TEST_CONTEXT pCompareContext = NULL;

    pCompareContext = (PQUEUE_COMPARE_TEST_CONTEXT) *state;

    while (!dwError)
    {
        VmDirQueueDequeue(bInLock, pCompareContext->pLeftQueue, 0, (PVOID*)&pszString);
        VMDIR_SAFE_FREE_MEMORY(pszString);
    }

    VmDirQueueFree(pCompareContext->pLeftQueue);
    VmDirQueueFree(pCompareContext->pRightQueue);
    VMDIR_SAFE_FREE_MEMORY(pCompareContext);

    return 0;
}

static
DWORD
_VmDirTestQueueDequeueInit(
    PDEQUEUE_TEST_CONTEXT*  ppDequeueContext
    )
{
    DWORD                   dwError = 0;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = 0;

    dwError = VmDirAllocateMemory(sizeof(DEQUEUE_TEST_CONTEXT), (PVOID*)&pDequeueContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueInit(&pDequeueContext->pInputQueue);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueInit(&pDequeueContext->pExpectedQueue);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("input1", &pDequeueContext->pszString);
    assert_int_equal(dwError, 0);

    *ppDequeueContext = pDequeueContext;

    return 0;
}

static
DWORD
_VmDirTestQueueEnqueueInit(
    PENQUEUE_TEST_CONTEXT*  ppEnqueueContext
    )
{
    DWORD                   dwError = 0;
    PENQUEUE_TEST_CONTEXT   pEnqueueContext = NULL;

    dwError = VmDirAllocateMemory(sizeof(ENQUEUE_TEST_CONTEXT), (PVOID*)&pEnqueueContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueInit(&pEnqueueContext->pInputQueue);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueInit(&pEnqueueContext->pExpectedQueue);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("input1", &pEnqueueContext->pszString);
    assert_int_equal(dwError, 0);

    *ppEnqueueContext = pEnqueueContext;

    return 0;
}

static
DWORD
_VmDirTestQueueCompareInit(
    PQUEUE_COMPARE_TEST_CONTEXT*    ppCompareContext
    )
{
    DWORD                       dwError = 0;
    PQUEUE_COMPARE_TEST_CONTEXT pCompareContext = NULL;

    dwError = VmDirAllocateMemory(sizeof(QUEUE_COMPARE_TEST_CONTEXT), (PVOID*)pCompareContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueInit(&pCompareContext->pLeftQueue);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueInit(&pCompareContext->pRightQueue);
    assert_int_equal(dwError, 0);

    *ppCompareContext = pCompareContext;

    return 0;
}
