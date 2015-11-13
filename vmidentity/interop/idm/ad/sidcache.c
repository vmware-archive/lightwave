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

/*
 * Module Name:
 *
 *        sidcache.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        SID Cache
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#include "includes.h"
#include "idmsiddefault.h"

#ifndef _WIN32

#define EqualSid RtlEqualSid

#else

typedef struct _pthread_once_t
{
    volatile int initialized;
    HANDLE mtx;
} pthread_once_t;
#define PTHREAD_ONCE_INIT {0, ((void *) 0)}

#endif

static pthread_once_t g_initialized = PTHREAD_ONCE_INIT;
static SID_CACHE g_WellKnownSidsCache;

PSID_CACHE
IDMGetBuiltinSidCache(VOID)
{
    return &g_WellKnownSidsCache;
}

DWORD
AddSidCacheEntryWithLock(
    PSID_CACHE pSidCache,
    PSID pSid,
    PWSTR pszName
    );

static
int
initialize_sid_cache_in_lock(PSID_CACHE pSidCache)
{
    DWORD dwError = 0;
    int i = 0;
    PWSTR pwszSidName = NULL;
    PSID pSid = NULL;
    NTSTATUS status = 0;

    /*
     * Populate SID cache with list of well-known SIDS
     */
    for (i=0; i<sizeof(WellKnownSids)/sizeof(WellKnownSids[0]); i++)
    {
        IDMFreeSid(pSid);
        pSid = NULL;
        IDM_SAFE_FREE_MEMORY(pwszSidName);
#ifdef _WIN32
        dwError = IDMAllocateStringWFromA(
                      WellKnownSids[i].CommonName,
                      &pwszSidName);
        BAIL_ON_ERROR(dwError);

        if (!ConvertStringSidToSidA(WellKnownSids[i].Sid, &pSid))
        {
            dwError = GetLastError();
            if (dwError == ERROR_INVALID_SID)
            {
                continue;
            }
            BAIL_ON_ERROR(dwError);
        }
#else
        dwError = LwRtlWC16StringAllocateFromCString(
                      &pwszSidName,
                      WellKnownSids[i].CommonName);
        BAIL_ON_ERROR(dwError);

        status = RtlAllocateSidFromCString(
                     &pSid,
                     WellKnownSids[i].Sid);
        dwError = LwNtStatusToWin32Error(status);
        BAIL_ON_ERROR(dwError);
#endif

        dwError = AddSidCacheEntryWithLock(
                      pSidCache,
                      pSid,
                      pwszSidName);
        BAIL_ON_ERROR(dwError);

#if defined(_WIN32)
        /*
         * Force thread to give up time slice. This reveals thread locking
         * issues during initialization should more than one thread
         * try to enter this code before the previous thread has finished
         * initializing the complete well-known SID cache.
         * This has no runtime performance penalty, as this function is
         * intended to be called only once.
         */
        Sleep(0);
#endif
    }

    /*
     * Handle well-known SIDs that are "prefix" values, and processing beyond
     * a string equality check.
     */
    for (i=0; i<sizeof(SpecialSidPrefixes)/sizeof(SpecialSidPrefixes[0]); i++)
    {
    }

    pSidCache->bInitialized = TRUE;

error:
    IDMFreeSid(pSid);
    IDM_SAFE_FREE_MEMORY(pwszSidName);

    return dwError;
}

/*
 * Structures which must be initialized once in a thread-safe manner
 */
static void
InitializeSidCacheStruct(
    PSID_CACHE  pSidCache
    )
{
    if (!pSidCache->bInitialized)
    {
        pthread_mutex_init(&pSidCache->cs, NULL);
        pSidCache->pStartEntry = NULL;
    }
}

/*
 * The prototype for this function is dictated by pthread_once()
 */
static void
InitializeOnceFunc(void)
{
    InitializeSidCacheStruct(IDMGetBuiltinSidCache());
}


/*
 * Implementation of pthread_once() for _WIN32, otherwise
 * use pthread_once() on POSIX platforms.
 *
 * Note: This function goes away when this project is converted
 * to using pthreads-win32.
 */
#ifdef _WIN32
int
pthread_once(
  pthread_once_t *oc,
  void (*init_routine)(void))
{
    int sts = ERROR_INVALID_PARAMETER;
    if (!oc || !init_routine)
    {
        BAIL_ON_ERROR(sts);
    }

    if (!oc->initialized)
    {
        if (!oc->mtx)
        {
            HANDLE newMtx = CreateMutex(NULL, 0, NULL);
            if (InterlockedCompareExchangePointer(&oc->mtx, newMtx, NULL))
            {
                CloseHandle(newMtx);
            }
        }

        WaitForSingleObject(oc->mtx, INFINITE);
        if (!oc->initialized)
        {
            init_routine();
            oc->initialized = 1;
        }
        ReleaseMutex(oc->mtx);
    }
    sts = 0;

error:
    return sts;
}
#endif

DWORD
IDMInitializeSidCache(VOID)
{
    DWORD dwError = 0;

    pthread_once(&g_initialized, InitializeOnceFunc);
    dwError = InitializeSidCache(IDMGetBuiltinSidCache());
    return dwError;
}


DWORD
IDMDestroySidCache(VOID)
{
    DWORD dwError = 0;

    dwError = DestroySidCache(IDMGetBuiltinSidCache());
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

DWORD
InitializeSidCache(
    PSID_CACHE  pSidCache
    )
{
    DWORD dwError = 0;

    /*
     * bInitialized must be tested with the mutex held. This prevents
     * another thread from entering initialize_sid_cache_in_lock()
     * before the cache initialization is complete, which sets this boolean
     * to true.
     */
    pthread_mutex_lock(&pSidCache->cs);
    if (!pSidCache->bInitialized)
    {
        dwError = initialize_sid_cache_in_lock(pSidCache);
    }
    pthread_mutex_unlock(&pSidCache->cs);
    return dwError;
}

DWORD
DestroySidCache(
    PSID_CACHE  pSidCache)
{
    DWORD dwError = 0;
    PSID_ENTRY pSidEntry = NULL;
    PSID_ENTRY pSidEntryNext = NULL;
    BOOL bLock = FALSE;

    if (!pSidCache)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pthread_mutex_lock(&pSidCache->cs);
    bLock = TRUE;

    pSidEntry = pSidCache->pStartEntry;
    while (pSidEntry)
    {
        pSidEntryNext = pSidEntry->pNext;
        FreeSidEntry(pSidEntry);
        pSidEntry = pSidEntryNext;
    }

error:
    if (bLock)
    {
        pthread_mutex_unlock(&pSidCache->cs);
    }
    pthread_mutex_destroy(&pSidCache->cs);
    memset(pSidCache, 0, sizeof(*pSidCache));
    return dwError;
}


DWORD
AddSidCacheEntry(
    PSID_CACHE pSidCache,
    PSID pSid,
    PWSTR pszName
    )
{
    DWORD dwError = 0;

    pthread_mutex_lock(&pSidCache->cs);
    dwError = AddSidCacheEntryWithLock(pSidCache, pSid, pszName);
    pthread_mutex_unlock(&pSidCache->cs);

    return dwError;
}

DWORD
AddSidCacheEntryWithLock(
    PSID_CACHE pSidCache,
    PSID pSid,
    PWSTR pszName
    )
{
    DWORD dwError = 0;
    PSID_ENTRY pSidEntry = NULL;

    dwError = FindSidCacheEntryWithLock(
                    pSidCache,
                    pSid,
                    NULL,
                    NULL);

    /* Success means entry exists, so don't add again */
    if (dwError == ERROR_SUCCESS)
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_ERROR(dwError);
    }

    /* Failed in some abnormal way */
    if (dwError != ERROR_NOT_FOUND)
    {
        BAIL_ON_ERROR(dwError);
    }

    dwError = CreateSidCacheEntry(
                    pSid,
                    pszName,
                    &pSidEntry
                    );
    BAIL_ON_ERROR(dwError);

    pSidEntry->pNext = pSidCache->pStartEntry;
    pSidCache->pStartEntry = pSidEntry;

error:
    return dwError;
}

DWORD
FindSidCacheEntry(
    PSID_CACHE pSidCache,
    PSID pSid,
    PWSTR * ppszName
    )
{
    DWORD dwError = 0;

    pthread_mutex_lock(&pSidCache->cs);

    dwError = FindSidCacheEntryWithLock(
                    pSidCache,
                    pSid,
                    ppszName,
                    NULL
                    );

    pthread_mutex_unlock(&pSidCache->cs);

    return dwError;
}

DWORD
FindSidCacheEntryWithLock(
    PSID_CACHE pSidCache,
    PSID pSid,
    PWSTR * ppszName,  // Optional
    PSID_ENTRY *ppPrevSidEntry // Optional
    )
{
    DWORD dwError = 0;
    PWSTR pszName = NULL;
    BOOL bRet = FALSE;
    PSID_ENTRY pSidEntry = NULL;
    PSID_ENTRY pPrevSidEntry = NULL;

    if (!pSidCache || !pSid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pSidEntry = pSidCache->pStartEntry;
    while (pSidEntry)
    {
        bRet = EqualSid(
                    pSidEntry->pSid,
                    pSid);
        if (bRet)
        {
            if (ppszName)
            {
                dwError = IDMAllocateString(
                                pSidEntry->pszName,
                                &pszName);
                BAIL_ON_ERROR(dwError);
                *ppszName = pszName;
            }
            if (ppPrevSidEntry)
            {
                *ppPrevSidEntry = pPrevSidEntry;
            }
            return dwError;
        }
        pPrevSidEntry = pSidEntry;
        pSidEntry = pSidEntry->pNext;
    }

error:
    if (dwError)
    {
        // Recover from error
        IDM_SAFE_FREE_MEMORY(pszName);
    }
    else
    {
        dwError = ERROR_NOT_FOUND;
    }
    return(dwError);
}


DWORD
DeleteSidCacheEntry(
    PSID_CACHE pSidCache,
    PSID pSid
    )
{
    DWORD dwError = 0;
    PSID_ENTRY pPrevSidEntry = NULL;
    PSID_ENTRY pSidEntry = NULL;
    BOOL bLocked = FALSE;

    if (!pSidCache)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    pthread_mutex_lock(&pSidCache->cs);
    bLocked = TRUE;

    dwError = FindSidCacheEntryWithLock( pSidCache,
                  pSid,
                  NULL,
                  &pPrevSidEntry);
    BAIL_ON_ERROR(dwError);

    if (pPrevSidEntry)
    {
        /* Get pointer to current entry, and unlink it from the list */
        pSidEntry = pPrevSidEntry->pNext;
        pPrevSidEntry->pNext = pSidEntry->pNext;
        FreeSidEntry(pSidEntry);
    }
    else
    {
        /* SID is head of list, so there is no previous node. Use start entry */
        pSidEntry = pSidCache->pStartEntry;
        pSidCache->pStartEntry = pSidEntry->pNext;
        FreeSidEntry(pSidEntry);
    }

error:
    if (bLocked)
    {
        pthread_mutex_unlock(&pSidCache->cs);
    }
    return dwError;
}

DWORD
CreateSidCacheEntry(
    PSID pSid,
    PWSTR pszName,
    PSID_ENTRY * ppSidEntry
    )
{
    DWORD dwError = 0;
    PSID pNewSid = NULL;
    PWSTR pszNewName = NULL;
    PSID_ENTRY pSidEntry = NULL;

    dwError = IDMCloneSid(
                 pSid,
                 &pNewSid
                 );
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateString(
                    pszName,
                    &pszNewName
                    );
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                    sizeof(SID_ENTRY),
                    (PVOID) &pSidEntry
                    );
    BAIL_ON_ERROR(dwError);

    pSidEntry->pSid = pNewSid;
    pSidEntry->pszName = pszNewName;

    *ppSidEntry = pSidEntry;

    return dwError;

error:

    if (pszName) {
        IDMFreeString(pszNewName);
    }
    if (pNewSid) {
        IDMFreeSid(pNewSid);
    }
    if (pSidEntry) {
        FreeSidEntry(pSidEntry);
    }

    return dwError;
}

VOID
FreeSidEntry(
    PSID_ENTRY pSidEntry
    )
{
    if (pSidEntry)
    {
        IDM_SAFE_FREE_MEMORY(pSidEntry->pszName);
        IDM_SAFE_FREE_MEMORY(pSidEntry->pSid);
        IDM_SAFE_FREE_MEMORY(pSidEntry);
    }
    return;
}

