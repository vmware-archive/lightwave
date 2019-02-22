/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
    PENQUEUE_TEST_CONTEXT   pEnqueueContext = NULL;

    dwError = _VmDirTestQueueEnqueueInit(&pEnqueueContext);
    assert_int_equal(dwError, 0);

    dwError = VmDirQueueEnqueue(
            bInLock, pEnqueueContext->pExpectedQueue, pEnqueueContext->pElement);
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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(
            bInLock, pDequeueContext->pInputQueue, pDequeueContext->pElement);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDequeueContext->iTimeoutMs = 0;

    *state = pDequeueContext;

cleanup:
    return dwError;

error:
    goto cleanup;
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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(
            bInLock, pDequeueContext->pInputQueue, pDequeueContext->pElement);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDequeueContext->iTimeoutMs = 100;

    *state = pDequeueContext;

cleanup:
    return dwError;

error:
    goto cleanup;
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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA("input1", &pszString1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA("input2", &pszString2);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString2);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pRightQueue, (PVOID)pszString1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pRightQueue, (PVOID)pszString2);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCompareContext->bExpected = TRUE;

    *state = pCompareContext;

cleanup:
    return dwError;

error:
    goto cleanup;
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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA("input1", &pszString1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA("input2", &pszString2);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pLeftQueue, (PVOID)pszString2);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueEnqueue(bInLock, pCompareContext->pRightQueue, (PVOID)pszString1);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCompareContext->bExpected = FALSE;

    *state = pCompareContext;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
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

    assert_int_equal(pEnqueueContext->pInputQueue->iSize, iSizeBefore+1);
    assert_true(VmDirQueueCompare(pEnqueueContext->pInputQueue, pEnqueueContext->pExpectedQueue));

    return;
}

VOID
VmDirTestQueueTestDequeue(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    PVOID                   pElement = NULL;
    DWORD                   dwError = 0;
    int64_t                 iSizeBefore = 0;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL;

    pDequeueContext = (PDEQUEUE_TEST_CONTEXT) *state;

    iSizeBefore = pDequeueContext->pInputQueue->iSize;

    dwError = VmDirQueueDequeue(
            bInLock, pDequeueContext->pInputQueue, pDequeueContext->iTimeoutMs, &pElement);
    assert_int_equal(dwError, 0);

    assert_int_equal(pDequeueContext->pInputQueue->iSize, iSizeBefore-1);
    assert_true(
            VmDirQueueCompare(pDequeueContext->pInputQueue, pDequeueContext->pExpectedQueue));
    assert_true((pElement == pDequeueContext->pElement));

    return;
}


VOID
VmDirTestQueueTestCompare(
    VOID    **state
    )
{
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

    return;
}

int
VmDirTestQueueTearDownEnqueue(
    VOID    **state
    )
{
    BOOL                    bInLock = FALSE;
    PVOID                   pElement = NULL;
    DWORD                   dwError = 0;
    PENQUEUE_TEST_CONTEXT   pEnqueueContext = NULL;

    pEnqueueContext = (PENQUEUE_TEST_CONTEXT) *state;

    while (!dwError)
    {
        dwError = VmDirQueueDequeue(bInLock, pEnqueueContext->pInputQueue, 0, &pElement);
        VMDIR_SAFE_FREE_MEMORY(pElement);
    }

    if (dwError == VMDIR_ERROR_QUEUE_EMPTY)
    {
        dwError = 0;
    }

    VmDirQueueFree(pEnqueueContext->pInputQueue);
    VmDirQueueFree(pEnqueueContext->pExpectedQueue);
    VMDIR_SAFE_FREE_MEMORY(pEnqueueContext);

    return dwError;
}

int
VmDirTestQueueTearDownDequeue(
    VOID    **state
    )
{
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL;

    pDequeueContext = (PDEQUEUE_TEST_CONTEXT) *state;

    VMDIR_SAFE_FREE_MEMORY(pDequeueContext->pElement);
    VmDirQueueFree(pDequeueContext->pInputQueue);
    VmDirQueueFree(pDequeueContext->pExpectedQueue);
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
        dwError = VmDirQueueDequeue(bInLock, pCompareContext->pLeftQueue, 0, (PVOID*)&pszString);
        VMDIR_SAFE_FREE_MEMORY(pszString);
    }

    if (dwError == VMDIR_ERROR_QUEUE_EMPTY)
    {
        dwError = 0;
    }

    VmDirQueueFree(pCompareContext->pLeftQueue);
    VmDirQueueFree(pCompareContext->pRightQueue);
    VMDIR_SAFE_FREE_MEMORY(pCompareContext);

    return dwError;
}

static
DWORD
_VmDirTestQueueDequeueInit(
    PDEQUEUE_TEST_CONTEXT*  ppDequeueContext
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszInputStr = NULL;
    PDEQUEUE_TEST_CONTEXT   pDequeueContext = NULL;

    dwError = VmDirAllocateMemory(sizeof(DEQUEUE_TEST_CONTEXT), (PVOID*)&pDequeueContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pDequeueContext->pInputQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pDequeueContext->pExpectedQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA("input1", &pszInputStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDequeueContext->pElement = (PVOID)pszInputStr;
    pszInputStr = NULL;

    *ppDequeueContext = pDequeueContext;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirTestQueueEnqueueInit(
    PENQUEUE_TEST_CONTEXT*  ppEnqueueContext
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszInputStr = NULL;
    PENQUEUE_TEST_CONTEXT   pEnqueueContext = NULL;

    dwError = VmDirAllocateMemory(sizeof(ENQUEUE_TEST_CONTEXT), (PVOID*)&pEnqueueContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pEnqueueContext->pInputQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pEnqueueContext->pExpectedQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA("input1", &pszInputStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEnqueueContext->pElement = (PVOID)pszInputStr;
    pszInputStr = NULL;

    *ppEnqueueContext = pEnqueueContext;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirTestQueueCompareInit(
    PQUEUE_COMPARE_TEST_CONTEXT*    ppCompareContext
    )
{
    DWORD                       dwError = 0;
    PQUEUE_COMPARE_TEST_CONTEXT pCompareContext = NULL;

    dwError = VmDirAllocateMemory(sizeof(QUEUE_COMPARE_TEST_CONTEXT), (PVOID*)&pCompareContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pCompareContext->pLeftQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pCompareContext->pRightQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCompareContext = pCompareContext;

cleanup:
    return dwError;

error:
    goto cleanup;
}
