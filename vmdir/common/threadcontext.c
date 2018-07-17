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

PVMDIR_THREAD_CONTEXT   pThreadContext = NULL;

static
VOID
_VmDirInitThreadContextOnce(VOID);

// Protects key from being created more than once
static
VOID
_VmDirInitThreadContextOnce(VOID)
{
    DWORD dwError = 0;

    dwError = pthread_key_create(&pThreadContext->threadLogContext, NULL);
    if (dwError)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL, "_VmDirInitThreadContextOnce failed (%d)", dwError);
        VMDIR_SAFE_FREE_MEMORY(pThreadContext);
    }
}

VOID
VmDirFreeThreadContext(VOID)
{
    VMDIR_SAFE_FREE_MEMORY(pThreadContext);
}

VOID
VmDirFreeThreadLogContext(
    PVMDIR_THREAD_LOG_CONTEXT pThreadLogContext
    )
{
    if (pThreadLogContext)
    {
        VMDIR_SAFE_FREE_MEMORY(pThreadLogContext->pszRequestId);
        VMDIR_SAFE_FREE_MEMORY(pThreadLogContext->pszSessionId);
        VMDIR_SAFE_FREE_MEMORY(pThreadLogContext->pszUserId);
        VMDIR_SAFE_FREE_MEMORY(pThreadLogContext);
    }
}

DWORD
VmDirInitThreadContext(VOID)
{
    DWORD dwError = 0;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_THREAD_CONTEXT),
            (PVOID*)&pThreadContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    pThreadContext->threadContextOnce = PTHREAD_ONCE_INIT;

    dwError = pthread_once(&pThreadContext->threadContextOnce, _VmDirInitThreadContextOnce);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirGetThreadContext(
    PVMDIR_THREAD_CONTEXT*  ppThreadContext
    )
{
    DWORD dwError = 0;

    if (!ppThreadContext)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppThreadContext = pThreadContext;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirGetThreadLogContextValue(
    PVMDIR_THREAD_LOG_CONTEXT*  ppThreadLogContext
    )
{
    DWORD dwError = 0;
    PVMDIR_THREAD_LOG_CONTEXT   pThreadLogContext = NULL;

    if (!ppThreadLogContext)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pThreadContext)
    {
        pThreadLogContext = pthread_getspecific(pThreadContext->threadLogContext);
    }

    *ppThreadLogContext = pThreadLogContext;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL, "VmDirGetThreadLogContextValue failed (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirSetThreadLogContextValue(
    PVMDIR_THREAD_LOG_CONTEXT  pThreadLogContext
    )
{
    DWORD dwError = 0;

    assert(pThreadContext);

    dwError = pthread_setspecific(
            pThreadContext->threadLogContext,
            (PVOID)pThreadLogContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL, "VmDirSetThreadLogContextValue failed (%d)", dwError);
    goto cleanup;
}

VOID
VmDirUnsetAndFreeThrLogCtx(
    PVMDIR_THREAD_LOG_CONTEXT   pThrLogCtx
    )
{
    PVMDIR_THREAD_LOG_CONTEXT pLocalLogCtx = NULL;

    if (pThrLogCtx)
    {
        VmDirGetThreadLogContextValue(&pLocalLogCtx);
        if (pLocalLogCtx)
        {
            if (pLocalLogCtx == pThrLogCtx)
            {
                VmDirSetThreadLogContextValue(NULL);
            }
            else
            {
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "%s thrlogctx mismatch. curr ctx (%p), parm ctx (%p)",
                    __FUNCTION__,
                    pLocalLogCtx,
                    pThrLogCtx);
            }
        }

        VmDirFreeThreadLogContext(pThrLogCtx);
    }
}

DWORD
VmDirAllocAndSetThrLogCtx(
    PVMDIR_THREAD_LOG_CONTEXT*  ppThrLogCtx
    )
{
    DWORD   dwError = 0;
    PVMDIR_THREAD_LOG_CONTEXT pCurrLogCtx = NULL;
    PVMDIR_THREAD_LOG_CONTEXT pLocalLogCtx = NULL;

    if (!ppThrLogCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirGetThreadLogContextValue(&pCurrLogCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pCurrLogCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_THREAD_LOG_CONTEXT), (PVOID)&pLocalLogCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetThreadLogContextValue(pLocalLogCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppThrLogCtx = pLocalLogCtx;
    pLocalLogCtx = NULL;

cleanup:
    return dwError;

error:
    VmDirUnsetAndFreeThrLogCtx(pLocalLogCtx);
    goto cleanup;
}
