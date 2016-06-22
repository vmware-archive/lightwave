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
 *        defines.h
 *
 * Abstract:
 *      
 *        Identity Manager - Active Directory Integration
 *
 *        Definitions
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#ifndef _WIN32
typedef SIZE_T size_t;

#ifndef IDM_ERROR_USER_INVALID_CREDENTIAL
#define IDM_ERROR_USER_INVALID_CREDENTIAL 9234
#endif

#else

#define pthread_mutex_lock(pCriticalSection)    EnterCriticalSection(pCriticalSection)
#define pthread_mutex_unlock(pCriticalSection)  LeaveCriticalSection(pCriticalSection)
#define pthread_mutex_init(mutex, defaults)     InitializeCriticalSection((mutex))
#define pthread_mutex_destroy(mutex)            DeleteCriticalSection((mutex))

#endif

#define BAIL_ON_ERROR(dwError) \
    if (dwError) goto error;

#define BAIL_ON_SECURITY_ERROR(secError, dwError)  \
    do {                                           \
        if ((secError) < 0)                        \
        {                                          \
            dwError = (DWORD) (secError);          \
            goto error;                            \
        }                                          \
    } while (0)

#define BAIL_ON_GSSAPI_ERROR(err, maj, min)                      \
    do {                                                         \
        if (err) { maj = GSS_S_FAILURE; min = err; goto error; } \
    } while (0)

#ifndef _WIN32

#define BAIL_ON_KERBEROS_ERROR(pCtx, krbErr, dwError) \
	do {                                      \
		if (krbErr) {                         \
			(dwError) = LwTranslateKrb5Error( \
							pCtx,             \
							krbErr,           \
							__FUNCTION__,     \
							__FILE__,         \
							__LINE__);        \
			goto error;                       \
		}                                     \
	} while (0)

#define IDM_RWMUTEX_LOCK_SHARED(mutex, locked, dwError) \
    do { \
        if ((mutex) && !(locked)) { \
            DWORD dwErrorLock = pthread_rwlock_rdlock(mutex); \
            if (dwErrorLock) { \
                (dwError) = LwErrnoToWin32Error(dwErrorLock); \
            } else { \
                (locked) = TRUE; \
                (dwError) = 0; \
            } \
        } \
    } while(0)

#define IDM_RWMUTEX_LOCK_EXCLUSIVE(mutex, locked, dwError) \
    do { \
        if ((mutex) && !(locked)) { \
            DWORD dwErrorLock = pthread_rwlock_wrlock(mutex); \
            if (dwErrorLock) { \
                (dwError) = LwErrnoToWin32Error(dwErrorLock); \
            } else { \
                (locked) = TRUE; \
                (dwError) = 0; \
            } \
        } \
    } while(0)

#define IDM_RWMUTEX_UNLOCK(mutex, locked, dwError) \
    do { \
        if ((mutex) && locked) { \
            DWORD dwErrorLock = pthread_rwlock_unlock(mutex); \
            if (dwErrorLock) { \
                (dwError) = LwErrnoToWin32Error(dwErrorLock); \
            } else { \
                (locked) = FALSE; \
                (dwError) = 0; \
            } \
        } \
    } while(0)

#define IDM_MUTEX_LOCK(mutex, locked, dwError) \
    do { \
        if ((mutex) && !(locked)) { \
            DWORD dwErrorLock = pthread_mutex_lock(mutex); \
            if (dwErrorLock) { \
                (dwError) = LwErrnoToWin32Error(dwErrorLock); \
            } else { \
                (locked) = TRUE; \
                (dwError) = 0; \
            } \
        } \
    } while(0)

#define IDM_MUTEX_UNLOCK(mutex, locked, dwError) \
    do { \
        if ((mutex) && locked) { \
            DWORD dwErrorLock = pthread_mutex_unlock(mutex); \
            if (dwErrorLock) { \
                (dwError) = LwErrnoToWin32Error(dwErrorLock); \
            } else { \
                (locked) = FALSE; \
                (dwError) = 0; \
            } \
        } \
    } while(0)

#endif /* !_WIN32 */

#define SEC_SUCCESS(Status) ((Status) >= 0)

#define IDM_SAFE_FREE_MEMORY(PTR)         \
    do {                                  \
        if ((PTR)) {                      \
            IDMFreeMemory(PTR);           \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)
