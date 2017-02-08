/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : logging.c
 *
 * Abstract :
 *
 *            VMware Directory Service
 *
 *            Common Utilities (Client & Server)
 *
 *            Threading
 *
 */
#include "..\includes.h"

#ifdef _WIN32

DWORD
VMCAAllocateMutex(
    PVMCA_MUTEX* ppMutex
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMCA_MUTEX pVMCAMutex = NULL;

    if ( ppMutex == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory( sizeof(VMCA_MUTEX), ((PVOID*)&pVMCAMutex));
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAInitializeMutexContent( pVMCAMutex );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppMutex = pVMCAMutex;
    pVMCAMutex = NULL;

error:

    VMCA_SAFE_FREE_MUTEX( pVMCAMutex );

    return dwError;
}

DWORD
VMCAInitializeMutexContent(
    PVMCA_MUTEX pMutex
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pMutex == NULL ) || ( pMutex->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    memset( &(pMutex->critSect), 0, sizeof(CRITICAL_SECTION) );
    
    InitializeCriticalSection( &(pMutex->critSect) );
    pMutex->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VMCAFreeMutex(
    PVMCA_MUTEX pMutex
)
{
    VMCAFreeMutexContent( pMutex );
    VMCA_SAFE_FREE_MEMORY( pMutex );
}

VOID
VMCAFreeMutexContent(
    PVMCA_MUTEX pMutex
)
{
    if ( ( pMutex != NULL ) &&  ( pMutex->bInitialized != FALSE ) )
    {
        DeleteCriticalSection(&(pMutex->critSect));
        pMutex->bInitialized = FALSE;
    }
}

DWORD
VMCALockMutex(
    PVMCA_MUTEX pMutex
)
{
    DWORD dwError = ERROR_SUCCESS;

    if (
        ( pMutex == NULL )
        ||
        (pMutex->bInitialized == FALSE)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    EnterCriticalSection( &(pMutex->critSect) );

error:

    return dwError;
}

DWORD
VMCAUnLockMutex(
    PVMCA_MUTEX pMutex
)
{
    DWORD dwError = ERROR_SUCCESS;

    if (
        ( pMutex == NULL )
        ||
        (pMutex->bInitialized == FALSE)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    LeaveCriticalSection( &(pMutex->critSect) );

error:

    return dwError;
}

BOOLEAN
VMCAIsMutexInitialized(
    PVMCA_MUTEX pMutex
)
{
    return ( pMutex != NULL ) &&
           ( (pMutex->bInitialized) != FALSE );
}

DWORD
VMCAAllocateCondition(
    PVMCA_COND* ppCondition
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMCA_COND pVMCACond = NULL;

    if ( ppCondition == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory( sizeof(VMCA_COND), ((PVOID*)&pVMCACond));
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAInitializeConditionContent(pVMCACond);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppCondition = pVMCACond;
    pVMCACond = NULL;

error:

    VMCA_SAFE_FREE_CONDITION( pVMCACond );

    return dwError;
}

DWORD
VMCAInitializeConditionContent(
    PVMCA_COND pCondition
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL ) || ( pCondition->bInitialized != FALSE ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    memset( &(pCondition->cond), 0, sizeof(CONDITION_VARIABLE) );

    InitializeConditionVariable( &(pCondition->cond) );
    pCondition->bInitialized = TRUE;

error:

    return dwError;
}

VOID
VMCAFreeCondition(
    PVMCA_COND pCondition
)
{
    VMCAFreeConditionContent(pCondition);
    VMCA_SAFE_FREE_MEMORY( pCondition );
}

VOID
VMCAFreeConditionContent(
    PVMCA_COND pCondition
)
{
    if ( ( pCondition != NULL ) &&  ( pCondition->bInitialized != FALSE ) )
    {
        pCondition->bInitialized = FALSE;
    }
}

DWORD
VMCAConditionWait(
    PVMCA_COND pCondition,
    PVMCA_MUTEX pMutex
)
{
    return VMCAConditionTimedWait(
        pCondition, pMutex, INFINITE
    );
}

DWORD
VMCAConditionTimedWait(
    PVMCA_COND pCondition,
    PVMCA_MUTEX pMutex,
    DWORD dwMilliseconds
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
         ||
         ( pMutex == NULL )
         ||
         ( pMutex->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if ( SleepConditionVariableCS(
            &(pCondition->cond),
            &(pMutex->critSect),
            dwMilliseconds) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VMCAConditionSignal(
    PVMCA_COND pCondition
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( ( pCondition == NULL )
         ||
         ( pCondition->bInitialized == FALSE )
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    WakeConditionVariable( &(pCondition->cond) );

error:

    return dwError;
}

static
unsigned int
__stdcall
ThreadFunction(
  PVOID pArgs
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMCA_START_ROUTINE pThreadStart = NULL;
    PVOID pThreadArgs = NULL;

    if( pArgs == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pThreadStart = ((PVMCA_THREAD_START_INFO)pArgs)->pStartRoutine;
    pThreadArgs = ((PVMCA_THREAD_START_INFO)pArgs)->pArgs;

    if( pThreadStart == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_SAFE_FREE_MEMORY( pArgs );

    dwError = pThreadStart( pThreadArgs );
    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_SAFE_FREE_MEMORY( pArgs );

    return dwError;
}

DWORD
VMCACreateThread(
    PVMCA_THREAD pThread,
    BOOLEAN bDetached,
    PVMCA_START_ROUTINE pStartRoutine,
    PVOID pArgs
)
{
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hThread = NULL;
    PVMCA_THREAD_START_INFO pThreadStartInfo = NULL;

    if ( ( pThread == NULL ) || ( pStartRoutine == NULL ) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
        sizeof(VMCA_THREAD_START_INFO),
        ((PVOID*)&pThreadStartInfo)
    );
    BAIL_ON_VMCA_ERROR(dwError);

    pThreadStartInfo->pStartRoutine = pStartRoutine;
    pThreadStartInfo->pArgs = pArgs;

    hThread = (HANDLE)_beginthreadex(
        NULL, 0, ThreadFunction, pThreadStartInfo, 0, NULL
    );
    if(hThread == NULL)
    {
        dwError = errno;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    else
    {
        // we started thread, so pThreadStartInfo
        // is now owned by ThreadFunction ...
        pThreadStartInfo = NULL;
    }
    // note: for win32, there is no notion of detached ...
    // the only thing to do - is not return the thread handle, so
    // nobody will be able to join the thread ...
    if ( bDetached == FALSE )
    {
        *pThread = hThread;
        hThread = NULL;
    }
    else
    {
        *pThread = NULL;
    }

error:

    if(hThread != NULL)
    {
         CloseHandle(hThread);
         hThread = NULL;
    }

    VMCA_SAFE_FREE_MEMORY(pThreadStartInfo);

    return dwError;
}

DWORD
VMCAThreadJoin(
    PVMCA_THREAD pThread,
    PDWORD pRetVal
)
{
    DWORD dwError = ERROR_SUCCESS;

    if(pThread == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = WaitForSingleObject( *pThread, INFINITE );
    if( dwError == WAIT_OBJECT_0 )
    {
        dwError = ERROR_SUCCESS;
    }
    else if ( dwError == WAIT_FAILED )
    {
        dwError = GetLastError();
    }
    BAIL_ON_VMCA_ERROR(dwError);

    if( pRetVal != NULL )
    {
        if( GetExitCodeThread( *pThread, pRetVal ) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

error:

    return dwError;
}

VOID
VMCAFreeVMCAThread(
    PVMCA_THREAD pThread
)
{
    if ( ( pThread != NULL ) && ( (*pThread) != NULL) )
    {
        CloseHandle((*pThread));
        (*pThread) = NULL;
    }
}

#endif