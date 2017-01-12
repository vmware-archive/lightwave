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
 * Module Name: VMDNS
 *
 * Filename: vmdnsdefines.h
 *
 * Abstract:
 *
 * Common macros
 */

#ifndef __VMDNSDEFINES_H__
#define __VMDNSDEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define BOOLEAN BOOL
    #define PBOOLEAN PBOOL
    #define PCVOID const PVOID
    #define ssize_t SSIZE_T
#endif

#define VMDNS_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VMDNS_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef _WIN32
#define VMDNS_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMDNS_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMDNS_SAFE_FREE_STRINGA(PTR)      \
    do {                                  \
        if ((PTR)) {                      \
            VmDnsFreeStringA(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMDNS_SAFE_FREE_MEMORY(PTR)       \
    do {                                  \
        if ((PTR)) {                      \
            VmDnsFreeMemory(PTR);         \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMDNS_SAFE_FREE_MUTEX(mutex)      \
    do {                                  \
        if ((mutex)) {                    \
            VmDnsFreeMutex(mutex);        \
            (mutex) = NULL;               \
        }                                 \
    } while(0)

#define VMDNS_SAFE_FREE_CONDITION(cond)   \
    do {                                  \
        if ((cond)) {                     \
            VmDnsFreeCondition(cond);     \
            (cond) = NULL;                \
        }                                 \
    } while(0)

#define VMDNS_LOCK_MUTEX(bInLock, mutex) \
    do {                                 \
        if (!(bInLock))                  \
        {                                \
            VmDnsLockMutex(mutex);       \
            (bInLock) = TRUE;            \
        }                                \
    } while (0)

#define VMDNS_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            VmDnsUnLockMutex(mutex);      \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)

#define BAIL_ON_VMDNS_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        VmDnsLog( VMDNS_LOG_LEVEL_DEBUG, "[%s,%s,%d,%u]",__FILE__, __FUNCTION__, __LINE__, dwError); \
        goto error;                                                \
    }

#define BAIL_AND_LOG_ON_VMDNS_ERROR(dwError, level) \
    if (dwError)                                                   \
    {                                                              \
        VmDnsLog( level, "[%s,%s,%d,%u]",__FILE__, __FUNCTION__, __LINE__, dwError); \
        goto error;                                                \
    }

#define BAIL_AND_LOG_MESSAGE_ON_VMDNS_ERROR(dwError, level, message) \
    if (dwError)                                                   \
    {                                                              \
        VmDnsLog( level, "[%s,%s,%d,%u] %s",__FILE__, __FUNCTION__, __LINE__, dwError, message); \
        goto error;                                                \
    }

#define BAIL_ON_VMDNS_ERROR_IF(condition) \
    if (condition)                                                 \
    {                                                              \
        VmDnsLog( VMDNS_LOG_LEVEL_DEBUG, "[%s,%s,%d,%u]",__FILE__, __FUNCTION__, __LINE__, dwError); \
        goto error;                                                \
    }

#define BAIL_ON_VMDNS_INVALID_POINTER(p, errCode)   \
        if (p == NULL) {                            \
            errCode = ERROR_INVALID_PARAMETER;      \
            BAIL_ON_VMDNS_ERROR(errCode);           \
        }

#define BAIL_ON_INVALID_ACCESSINFO(pAccessInfo, errCode)     \
        if (pAccessInfo == NULL || VmDnsIsFailedAccessInfo(pAccessInfo)) {  \
           errCode = ERROR_ACCESS_DENIED;         \
           BAIL_ON_VMDNS_ERROR(errCode);          \
        }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#define BAIL_ON_VMDNS_EMPTY_STRING(p, errCode)  \
    if (IsNullOrEmptyString(p)) {               \
        errCode = ERROR_INVALID_PARAMETER;      \
        BAIL_ON_VMDNS_ERROR(errCode);           \
    }


#ifndef VMDNS_SAFE_STRING
#define VMDNS_SAFE_STRING(str) ((str) ? (str) : "")
#endif


#ifndef VMDNS_SAFE_SPACE_STRING
#define VMDNS_SAFE_SPACE_STRING(str) ((str) ? (str) : " ")
#endif

#define VMDNS_OFFSET_TO_POINTER(Pointer, Offset) \
    ((char*)(Pointer) + Offset)

#define VMDNS_FIELD_OFFSET(Type, Field) offsetof(Type, Field)

#define VMDNS_FIELD_SIZE(Type, Field) \
    (sizeof(((Type*)(0))->Field))

#define VMDNS_STRUCT_FROM_FIELD(Pointer, Type, Field) \
    ((Type*)VMDNS_OFFSET_TO_POINTER(Pointer, -((ssize_t)VMDNS_FIELD_OFFSET(Type, Field))))

// Logging
#define MAX_LOG_MESSAGE_LEN    4096
#define VMDNS_PATH_MAX         512

#define VMDNS_ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define VMDNS_ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define VMDNS_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )
#define VMDNS_PASSWD_SP_CHAR(c) ( (c) == '@' ||   \
                                  (c) == '#' ||   \
                                  (c) == '$' ||   \
                                  (c) == '%' ||   \
                                  (c) == '^' ||   \
                                  (c) == '&' ||   \
                                  (c) == '*' )

#define VMDNS_MAX_HOSTNAME_LEN         100

#ifdef _WIN32

#define VMDNS_PATH_SEPARATOR_STR "\\"
#define VMDNS_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMDNS_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMDNS_ADDR_INFO_ADDR( ai ) ai->ai_addr

#else

#define VMDNS_PATH_SEPARATOR_STR "/"
#define VMDNS_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMDNS_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMDNS_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#endif

#ifndef _WIN32
#define InterlockedExchange __sync_lock_test_and_set
#endif

#define VMDNS_DEFAULT_REFRESH_INTERVAL  3600
#define VMDNS_DEFAULT_RETRY_INTERVAL    600
#define VMDNS_DEFAULT_EXPIRE            86400
#define VMDNS_DEFAULT_TTL               3600
#define VMDNS_DEFAULT_LDAP_PORT         389
#define VMDNS_DEFAULT_KDC_PORT          88

#ifdef __cplusplus
}
#endif

#endif /* __VMDNSDEFINES_H__ */
