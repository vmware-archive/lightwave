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
 * Module Name: VMAUTHSVC
 *
 * Filename: vmauthsvcdefines.h
 *
 * Abstract:
 *
 * Common macros
 *
 *
 */

#ifndef __VMAUTHSVCDEFINES_H__
#define __VMAUTHSVCDEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define BOOLEAN BOOL
    #define PBOOLEAN PBOOL
    #define PCVOID const PVOID
    #define ssize_t SSIZE_T
#endif

#define VMAUTHSVC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VMAUTHSVC_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef _WIN32
#define VMAUTHSVC_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMAUTHSVC_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMAUTHSVC_SAFE_FREE_STRINGA(PTR)      \
    do {                                  \
        if ((PTR)) {                      \
            VmAuthsvcFreeStringA(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMAUTHSVC_SAFE_FREE_MEMORY(PTR)       \
    do {                                  \
        if ((PTR)) {                      \
            VmAuthsvcFreeMemory(PTR);         \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMAUTHSVC_SAFE_FREE_MUTEX(mutex)      \
    do {                                  \
        if ((mutex)) {                    \
            VmAuthsvcFreeMutex(mutex);        \
            (mutex) = NULL;               \
        }                                 \
    } while(0)

#define VMAUTHSVC_SAFE_FREE_CONDITION(cond)   \
    do {                                  \
        if ((cond)) {                     \
            VmAuthsvcFreeCondition(cond);     \
            (cond) = NULL;                \
        }                                 \
    } while(0)

#define VMAUTHSVC_LOCK_MUTEX(bInLock, mutex) \
    do {                                 \
        if (!(bInLock))                  \
        {                                \
            VmAuthsvcLockMutex(mutex);       \
            (bInLock) = TRUE;            \
        }                                \
    } while (0)

#define VMAUTHSVC_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            VmAuthsvcUnLockMutex(mutex);      \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)

#define BAIL_ON_VMAUTHSVC_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        VmAuthsvcLog( VMAUTHSVC_DEBUG_TRACE, "[%s,%d]",__FILE__, __LINE__); \
        goto error;                                                \
    }

#define BAIL_ON_VMAUTHSVC_ERROR_IF(condition) \
    if (condition)                                                 \
    {                                                              \
        VmAuthsvcLog( VMAUTHSVC_DEBUG_TRACE, "[%s,%d]",__FILE__, __LINE__); \
        goto error;                                                \
    }

#define BAIL_ON_VMAUTHSVC_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMAUTHSVC_ERROR(errCode);          \
        }

#define BAIL_ON_INVALID_ACCESSINFO(pAccessInfo, errCode)     \
        if (pAccessInfo == NULL || VmAuthsvcIsFailedAccessInfo(pAccessInfo)) {  \
           errCode = ERROR_ACCESS_DENIED;         \
           BAIL_ON_VMAUTHSVC_ERROR(errCode);          \
        }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#ifndef VMAUTHSVC_SAFE_STRING
#define VMAUTHSVC_SAFE_STRING(str) ((str) ? (str) : "")
#endif


#ifndef VMAUTHSVC_SAFE_SPACE_STRING
#define VMAUTHSVC_SAFE_SPACE_STRING(str) ((str) ? (str) : " ")
#endif

#define VMAUTHSVC_OFFSET_TO_POINTER(Pointer, Offset) \
    ((char*)(Pointer) + Offset)

#define VMAUTHSVC_FIELD_OFFSET(Type, Field) offsetof(Type, Field)

#define VMAUTHSVC_FIELD_SIZE(Type, Field) \
    (sizeof(((Type*)(0))->Field))

#define VMAUTHSVC_STRUCT_FROM_FIELD(Pointer, Type, Field) \
    ((Type*)VMAUTHSVC_OFFSET_TO_POINTER(Pointer, -((ssize_t)VMAUTHSVC_FIELD_OFFSET(Type, Field))))

#ifndef VMAUTHSVC_DEBUG_ANY
#define VMAUTHSVC_DEBUG_ANY (-1)
#endif

#ifndef VMAUTHSVC_DEBUG_TRACE
#define VMAUTHSVC_DEBUG_TRACE (1)
#endif

#define VMAUTHSVC_ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define VMAUTHSVC_ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define VMAUTHSVC_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )

#define VMAUTHSVC_ASCII_LOWER_TO_UPPER(c)   \
if ( VMAUTHSVC_ASCII_LOWER(c) )             \
{                                       \
    (c) = ((c) - 32);                   \
}

#define VMAUTHSVC_ASCII_UPPER_TO_LOWER(c)   \
if ( VMAUTHSVC_ASCII_UPPER(c) )             \
{                                       \
    (c) = ((c) + 32);                   \
}

#define VMAUTHSVC_MAX_HOSTNAME_LEN         100

#ifdef _WIN32

#define VMAUTHSVC_PATH_SEPARATOR_STR "\\"
#define VMAUTHSVC_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMAUTHSVC_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMAUTHSVC_ADDR_INFO_ADDR( ai ) ai->ai_addr

#else

#define VMAUTHSVC_PATH_SEPARATOR_STR "/"
#define VMAUTHSVC_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMAUTHSVC_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMAUTHSVC_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#endif

#ifdef __cplusplus
}
#endif

#endif /* __VMAUTHSVCDEFINES_H__ */
