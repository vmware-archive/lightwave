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
 * Module Name: VMKDC
 *
 * Filename: vmkdcdefines.h
 *
 * Abstract:
 *
 * Common macros
 *
 *
 */

#ifndef __VMKDCDEFINES_H__
#define __VMKDCDEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#define HAVE_MDB_H
#define PSECURITY_DESCRIPTOR_ABSOLUTE PSECURITY_DESCRIPTOR
#define PSECURITY_DESCRIPTOR_RELATIVE PSECURITY_DESCRIPTOR
#define BOOLEAN BOOL
#define PBOOLEAN PBOOL
#define PCVOID const PVOID
#define ssize_t SSIZE_T

#endif

#ifndef _WIN32
#define VMKDC_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMKDC_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMKDC_SAFE_FREE_STRINGA(PTR)      \
    do {                                  \
        if ((PTR)) {                      \
            VmKdcFreeStringA(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMKDC_SAFE_FREE_MEMORY(PTR)       \
    do {                                  \
        if ((PTR)) {                      \
            VmKdcFreeMemory(PTR);         \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define BAIL_ON_VMKDC_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        goto error;                                                \
    }

#define BAIL_ON_VMKDC_ERROR_IF(condition) \
    if (condition)                                                 \
    {                                                              \
        goto error;                                                \
    }

#define BAIL_ON_VMKDC_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMKDC_ERROR(errCode);          \
        }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#ifndef VMKDC_SAFE_STRING
#define VMKDC_SAFE_STRING(str) ((str) ? (str) : "")
#endif

#ifndef VMKDC_DEBUG_ANY
#define VMKDC_DEBUG_ANY (-1)
#endif

#ifndef VMKDC_DEBUG_DEBUG
#define VMKDC_DEBUG_DEBUG (-2)
#endif

#ifndef VMKDC_DEBUG_ERROR
#define VMKDC_DEBUG_ERROR (-3)
#endif

#ifndef VMKDC_DEBUG_TRACE
#define VMKDC_DEBUG_TRACE (1)
#endif

#ifdef _WIN32

#define VMKDC_PATH_SEPARATOR_STR "\\"
#define VMKDC_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMKDC_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMKDC_ADDR_INFO_ADDR( ai ) ai->ai_addr

#else

#define VMKDC_PATH_SEPARATOR_STR "/"
#define VMKDC_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMKDC_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMKDC_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#endif

#define VMKDC_LOCK_MUTEX(bInLock, mutex) \
    do {                                 \
        if (!(bInLock))                  \
        {                                \
            VmKdcLockMutex(mutex);       \
            (bInLock) = TRUE;            \
        }                                \
    } while (0)

#define VMKDC_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            VmKdcUnLockMutex(mutex);      \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __VMKDCDEFINES_H__ */
