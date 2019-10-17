/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef __VM_COMMON_DEFINE_H__
#define __VM_COMMON_DEFINE_H__
/*
#define VM_SIZE_8192                    8192
#define VM_SIZE_4096                    4096
#define VM_SIZE_2048                    2048
#define VM_SIZE_1024                    1024
#define VM_SIZE_512                     512
#define VM_SIZE_256                     256
#define VM_SIZE_128                     128
#define VM_SIZE_64                      64
#define VM_SIZE_32                      32
#define VM_SIZE_16                      16
#define VM_SIZE_8                       8
#define VM_SIZE_4                       4
#define VM_SIZE_2                       2
#define VM_SIZE_1                       1

#define NSECS_PER_SEC                   1000000000
#define NSECS_PER_MSEC                  1000000
#define MSECS_PER_SEC                   1000
*/

#define BUFFER_SIZE                     1024
#define VM_COMMON_MAX_TIME_BYTES        128
#define VM_COMMON_TIME_FORMAT_RFC_1233      "%a, %d %b %Y %H:%M:%S GMT"
#define VM_COMMON_HTTP_CONTENT_TYPE_KEY     "Content-Type"
#define VM_COMMON_HTTP_CONTENT_TYPE_JSON    "application/json"
#define VM_COMMON_HTTP_DATE                 "Date"

#define VM_MAX_GWTPWR_BUF_LENGTH        16384

#define VM_LIGHTWAVE_USER               "lightwave"
#define VM_LIGHTWAVE_GROUP              "lightwave"

#define HEADER_BEARER_AUTH "Authorization: Bearer %s"
#define HEADER_HOTK_PK_AUTH "Authorization: hotk-pk %s"

#define VM_COMMON_SAFE_FREE_MEMORY(PTR)       \
    do {                                       \
        if ((PTR)) {                           \
            VmFreeMemory(PTR);          \
            (PTR) = NULL;                      \
        }                                      \
    } while(0)

#define VM_COMMON_SAFE_FREE_CURL_MEMORY(PTR)   \
    do {                                       \
        if ((PTR)) {                           \
            curl_free(PTR);          \
            (PTR) = NULL;                      \
        }                                      \
    } while(0)

#define BAIL_ON_VM_COMMON_ERROR(dwError)       \
    if (dwError)                               \
    {                                          \
        goto error;                            \
    }

#define BAIL_WITH_VM_COMMON_ERROR(dwError, ERROR_CODE)  \
    dwError = ERROR_CODE;                               \
    BAIL_ON_VM_COMMON_ERROR(dwError);

#define BAIL_ON_VM_COMMON_INVALID_PARAMETER(ptr, dwError)                           \
    if ((ptr) == NULL)                                                              \
    {                                                                               \
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);      \
    }

#define BAIL_ON_VM_COMMON_INVALID_STR_PARAMETER(ptr, dwError)                       \
    if (IsNullOrEmptyString((ptr)))                                                 \
    {                                                                               \
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);      \
    }

#define VM_COMMON_SAFE_FREE_STRINGA(PTR)    \
    do {                                    \
        if ((PTR)) {                        \
            VmFreeStringA(PTR);             \
            (PTR) = NULL;                   \
        }                                   \
    } while(0)

#define VM_COMMON_SAFE_STRING(str) ((str) ? (str) : "")


#define VM_COMMON_ASCII_SPACE(c) \
    ( (c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' )

#define BAIL_AND_LOG_ON_VM_COMMON_ERROR(x, str) \
    if ((x) != 0)                                            \
    {                                                        \
        fprintf(stderr, "error [%u] in file [%s] function [%s] at line [%d] with message [%s]\n", x, __FILE__, __FUNCTION__, __LINE__, VM_COMMON_SAFE_STRING(str)); \
        goto error;                                          \
    }

#define BAIL_AND_LOG_ON_VM_COMMON_CURL_ERROR(x, curlCode) \
    if (curlCode != CURLE_OK)                             \
    {                                                     \
        x = VM_COMMON_ERROR_CURL_FAILURE;                 \
        BAIL_AND_LOG_ON_VM_COMMON_ERROR(x, curl_easy_strerror(curlCode)); \
    }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#define VM_COMMON_GET_BASE64_ENCODE_LEN(x) ((x / 3 + 1) * 4 + 1)

#define VM_COMMON_SAFE_FREE_MUTEX(mutex)      \
    do {                                  \
        if ((mutex)) {                    \
            VmFreeMutex(mutex);           \
            (mutex) = NULL;               \
        }                                 \
    } while(0)

#define VM_COMMON_SAFE_FREE_RWLOCK(lock)      \
    do {                                  \
        if ((lock)) {                     \
            VmFreeRWLock(lock);           \
            (lock) = NULL;                \
        }                                 \
    } while(0)

#define VM_COMMON_SAFE_FREE_CONDITION(cond)   \
    do {                                  \
        if ((cond)) {                     \
            VmFreeCondition(cond);        \
            (cond) = NULL;                \
        }                                 \
    } while(0)

#define VM_LOCK_MUTEX(bInLock, mutex)           \
    do {                                        \
        if (!(bInLock))                         \
        {                                       \
            if (VmLockMutex(mutex) == 0)        \
            {                                   \
                (bInLock) = TRUE;               \
            }                                   \
        }                                       \
    } while (0)

#define VM_UNLOCK_MUTEX(bInLock, mutex)         \
    do {                                        \
        if ((bInLock))                          \
        {                                       \
            if (VmUnLockMutex(mutex) == 0)      \
            {                                   \
                (bInLock) = FALSE;              \
            }                                   \
        }                                       \
    } while (0)

#define VM_RWLOCK_READLOCK(bInLock, lock, dwMilliSec)           \
    do {                                                        \
        if (!(bInLock))                                         \
        {                                                       \
            if (VmRWLockReadLock(lock, dwMilliSec) == 0)        \
            {                                                   \
                (bInLock) = TRUE;                               \
            }                                                   \
        }                                                       \
    } while (0)

#define VM_RWLOCK_WRITELOCK(bInLock, lock, dwMilliSec)          \
    do {                                                        \
        if (!(bInLock))                                         \
        {                                                       \
            if (VmRWLockWriteLock(lock, dwMilliSec) == 0)       \
            {                                                   \
                (bInLock) = TRUE;                               \
            }                                                   \
        }                                                       \
    } while (0)

#define VM_RWLOCK_UNLOCK(bInLock, lock)             \
    do {                                            \
        if ((bInLock))                              \
        {                                           \
            if (VmRWLockUnlock(lock) == 0)          \
            {                                       \
                (bInLock) = FALSE;                  \
            }                                       \
        }                                           \
    } while (0)


#endif /* __VM_COMMON_DEFINE_H__ */
