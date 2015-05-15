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

#define VMKDC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VMKDC_MAX(a, b) ((a) > (b) ? (a) : (b))

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

#ifndef VMKDC_SAFE_SPACE_STRING
#define VMKDC_SAFE_SPACE_STRING(str) ((str) ? (str) : " ")
#endif

#define VMKDC_OFFSET_TO_POINTER(Pointer, Offset) \
    ((char*)(Pointer) + Offset)

#define VMKDC_FIELD_OFFSET(Type, Field) offsetof(Type, Field)

#define VMKDC_FIELD_SIZE(Type, Field) \
    (sizeof(((Type*)(0))->Field))

#define VMKDC_STRUCT_FROM_FIELD(Pointer, Type, Field) \
    ((Type*)VMKDC_OFFSET_TO_POINTER(Pointer, -((ssize_t)VMKDC_FIELD_OFFSET(Type, Field))))

#define VMKDC_SID_REVISION 1
#define VMKDC_SID_MAX_SUB_AUTHORITIES 15

#define VMKDC_SID_MIN_SIZE \
    (VMKDC_FIELD_OFFSET(SID, SubAuthority))

#define _VMKDC_SID_GET_SIZE_REQUIRED(SubAuthorityCount) \
    (VMKDC_SID_MIN_SIZE + (VMKDC_FIELD_SIZE(SID, SubAuthority[0]) * (SubAuthorityCount)))

#define VMKDC_SID_MAX_SIZE \
    _VMKDC_SID_GET_SIZE_REQUIRED(VMKDC_SID_MAX_SUB_AUTHORITIES)

#define VMKDC_ACL_HEADER_SIZE 8
// TODO-Perhaps this should be rounded to nearest ULONG size
#define VMKDC_ACL_MAX_SIZE ((USHORT)-1)

#define VMKDC_SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE (5 * sizeof(PVOID))
#define VMKDC_SECURITY_DESCRIPTOR_RELATIVE_MIN_SIZE (5 * sizeof(ULONG))
// Maximum for a revision 1 security descriptor
#define VMKDC_SECURITY_DESCRIPTOR_RELATIVE_MAX_SIZE \
    (VMKDC_SECURITY_DESCRIPTOR_RELATIVE_MIN_SIZE + 2 * VMKDC_SID_MAX_SIZE + 2 * VMKDC_ACL_MAX_SIZE)

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

#define VMKDC_ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define VMKDC_ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define VMKDC_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )
#define VMKDC_PASSWD_SP_CHAR(c) ( (c) == '@' ||   \
                                  (c) == '#' ||   \
                                  (c) == '$' ||   \
                                  (c) == '%' ||   \
                                  (c) == '^' ||   \
                                  (c) == '&' ||   \
                                  (c) == '*' )

#define VMKDC_MAX_HOSTNAME_LEN         100
#define VMKDC_MAX_LDAP_URI_LEN         256 /* e.g. ldap://192.168.122.65 */
#define VMKDC_DEFAULT_REPL_LAST_USN_PROCESSED       "0"

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
