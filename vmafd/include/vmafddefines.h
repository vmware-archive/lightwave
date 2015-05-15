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
 * Module Name: VMAFD
 *
 * Filename: vmafddefines.h
 *
 * Abstract:
 *
 * Common macros
 *
 *
 */

#ifndef __VMAFDDEFINES_H__
#define __VMAFDDEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Logging stuff
#define MAX_LOG_MESSAGE_LEN    4096
#define VMAFD_PATH_MAX         512

#ifndef VMAFD_DEBUG_ANY
#define VMAFD_DEBUG_ANY (-2)
#endif

#ifndef VMAFD_DEBUG_DEBUG
#define VMAFD_DEBUG_DEBUG (-3)
#endif

#ifndef VMAFD_DEBUG_ERROR
#define VMAFD_DEBUG_ERROR (1)
#endif

#ifndef VMAFD_DEBUG_TRACE
#define VMAFD_DEBUG_TRACE (-1)
#endif

#ifdef _WIN32
    #define BOOLEAN BOOL
    #define PBOOLEAN PBOOL
    #define PCVOID const PVOID
    #define ssize_t SSIZE_T
#endif

#define VMAFD_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VMAFD_MAX(a, b) ((a) > (b) ? (a) : (b))

#ifndef _WIN32
#define VMAFD_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMAFD_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMAFD_SAFE_FREE_STRINGA(PTR)      \
    do {                                  \
        if ((PTR)) {                      \
            VmAfdFreeStringA(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMAFD_SAFE_FREE_STRINGW(PTR)      \
    do {                                  \
        if ((PTR)) {                      \
            VmAfdFreeStringW(PTR);        \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMAFD_SAFE_FREE_MEMORY(PTR)       \
    do {                                  \
        if ((PTR)) {                      \
            VmAfdFreeMemory(PTR);         \
            (PTR) = NULL;                 \
        }                                 \
    } while(0)

#define VMAFD_SAFE_FREE_MUTEX(mutex)      \
    do {                                  \
        if ((mutex)) {                    \
            VmAfdFreeMutex(mutex);        \
            (mutex) = NULL;               \
        }                                 \
    } while(0)

#define VMAFD_SAFE_FREE_CONDITION(cond)   \
    do {                                  \
        if ((cond)) {                     \
            VmAfdFreeCondition(cond);     \
            (cond) = NULL;                \
        }                                 \
    } while(0)

#define VMAFD_LOCK_MUTEX(bInLock, mutex) \
    do {                                 \
        if (!(bInLock))                  \
        {                                \
            pthread_mutex_lock(mutex);   \
            (bInLock) = TRUE;            \
        }                                \
    } while (0)

#define VMAFD_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            pthread_mutex_unlock(mutex);  \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)

#define BAIL_ON_VMAFD_ERROR(dwError) \
    if (dwError)                                                   \
    {                                                              \
        VmAfdLog( VMAFD_DEBUG_TRACE,                               \
         "%s failed. Error code [%d]. [%s,%d]",                    \
            __FUNCTION__,dwError,__FILE__, __LINE__);              \
        goto error;                                                \
    }

#define BAIL_ON_VMAFD_ERROR_IF(condition) \
    if (condition)                                                 \
    {                                                              \
        VmAfdLog( VMAFD_DEBUG_TRACE,                               \
         "%s failed. Error code [%d]. [%s,%d]",                    \
            __FUNCTION__,dwError,__FILE__, __LINE__);              \
        goto error;                                                \
    }

#define BAIL_ON_VMAFD_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMAFD_ERROR(errCode);          \
        }

#define BAIL_ON_INVALID_ACCESSINFO(pAccessInfo, errCode)     \
        if (pAccessInfo == NULL || VmAfdIsFailedAccessInfo(pAccessInfo)) {  \
           errCode = ERROR_ACCESS_DENIED;         \
           BAIL_ON_VMAFD_ERROR(errCode);          \
        }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#ifndef VMAFD_SAFE_STRING
#define VMAFD_SAFE_STRING(str) ((str) ? (str) : "")
#endif


#ifndef VMAFD_SAFE_SPACE_STRING
#define VMAFD_SAFE_SPACE_STRING(str) ((str) ? (str) : " ")
#endif

#define VMAFD_OFFSET_TO_POINTER(Pointer, Offset) \
    ((char*)(Pointer) + Offset)

#define VMAFD_FIELD_OFFSET(Type, Field) offsetof(Type, Field)

#define VMAFD_FIELD_SIZE(Type, Field) \
    (sizeof(((Type*)(0))->Field))

#define VMAFD_STRUCT_FROM_FIELD(Pointer, Type, Field) \
    ((Type*)VMAFD_OFFSET_TO_POINTER(Pointer, -((ssize_t)VMAFD_FIELD_OFFSET(Type, Field))))

#ifndef VMAFD_DEBUG_ANY
#define VMAFD_DEBUG_ANY (-1)
#endif

#ifndef VMAFD_DEBUG_TRACE
#define VMAFD_DEBUG_TRACE (1)
#endif

#define VMAFD_ASCII_aTof(c)     ( (c) >= 'a' && (c) <= 'f' )
#define VMAFD_ASCII_AToF(c)     ( (c) >= 'A' && (c) <= 'F' )
#define VMAFD_ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define VMAFD_ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define VMAFD_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )

#define VMAFD_ASCII_LOWER_TO_UPPER(c)   \
if ( VMAFD_ASCII_LOWER(c) )             \
{                                       \
    (c) = ((c) - 32);                   \
}

#define VMAFD_ASCII_UPPER_TO_LOWER(c)   \
if ( VMAFD_ASCII_UPPER(c) )             \
{                                       \
    (c) = ((c) + 32);                   \
}

#define VMAFD_MAX_HOSTNAME_LEN         100

#ifdef _WIN32

#define VMAFD_PATH_SEPARATOR_STR "\\"
#define VMAFD_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMAFD_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMAFD_ADDR_INFO_ADDR( ai ) ai->ai_addr

#else

#define VMAFD_PATH_SEPARATOR_STR "/"
#define VMAFD_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMAFD_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMAFD_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#endif

#ifdef __cplusplus
}
#endif

//API_TYPES

#define VECS_IPC_CREATE_CERTSTORE     0
#define VECS_IPC_OPEN_CERTSTORE       1
#define VECS_IPC_ADD_ENTRY            2
#define VECS_IPC_GET_ENTRY_TYPE       3
#define VECS_IPC_GET_KEY              4
#define VECS_IPC_DELETE_ENTRY         5
#define VECS_IPC_DELETE_CERTSTORE     6
#define VECS_IPC_SET_PERMISSION       7
#define VECS_IPC_REVOKE_PERMISSION    8
#define VECS_IPC_GET_PERMISSIONS      9
#define VECS_IPC_CHANGE_OWNER        10
#define VECS_IPC_CLOSE_CERTSTORE     11
#define VECS_IPC_BEGIN_ENUM_ENTRIES  12
#define VECS_IPC_GET_ENTRY_BY_ALIAS  13
#define VECS_IPC_GET_KEY_BY_ALIAS    14
#define VECS_IPC_ENUM_STORES         15
#define VECS_IPC_ENUM_ENTRIES        16
#define VECS_IPC_GET_ENTRY_COUNT     17
#define VECS_IPC_END_ENUM_ENTRIES    18

#define VMAFD_IPC_GET_STATUS                 19
#define VMAFD_IPC_GET_DOMAIN_NAME            20
#define VMAFD_IPC_SET_DOMAIN_NAME            21
#define VMAFD_IPC_GET_DOMAIN_STATE           22
#define VMAFD_IPC_GET_LDU                    23
#define VMAFD_IPC_SET_LDU                    24
#define VMAFD_IPC_SET_RHTTPPROXY_PORT        25
#define VMAFD_IPC_SET_DC_PORT                26
#define VMAFD_IPC_GET_CM_LOCATION            27
#define VMAFD_IPC_GET_LS_LOCATION            28
#define VMAFD_IPC_GET_DC_NAME                29
#define VMAFD_IPC_SET_DC_NAME                30
#define VMAFD_IPC_GET_MACHINE_ACCOUNT_INFO   31
#define VMAFD_IPC_GET_SITE_GUID              32
#define VMAFD_IPC_GET_MACHINE_ID             33
#define VMAFD_IPC_SET_MACHINE_ID             34
#define VMAFD_IPC_PROMOTE_VMDIR              35
#define VMAFD_IPC_DEMOTE_VMDIR               36
#define VMAFD_IPC_JOIN_VMDIR                 37
#define VMAFD_IPC_LEAVE_VMDIR                38
#define VMAFD_IPC_JOIN_AD                    39
#define VMAFD_IPC_LEAVE_AD                   40
#define VMAFD_IPC_QUERY_AD                   41
#define VMAFD_IPC_FORCE_REPLICATION          42
#define VMAFD_IPC_GET_PNID                   43
#define VMAFD_IPC_SET_PNID                   44
#define VMAFD_IPC_GET_CA_PATH                45
#define VMAFD_IPC_SET_CA_PATH                46
#define VMAFD_IPC_TRIGGER_ROOT_CERTS_REFRESH 47

typedef enum
{
  VM_AFD_CONTEXT_TYPE_UNKNOWN = 0,
  VM_AFD_CONTEXT_TYPE_ROOT,
  VM_AFD_CONTEXT_TYPE_EVERYONE
} VM_AFD_CONTEXT_TYPE, *PVM_AFD_CONTEXT_TYPE;

#define GROUP_EVERYONE_W {'E','V','E','R','Y','O','N','E',0}
#define GROUP_EVERYONE "EVERYONE";

//VERSIONS
#define VER1_INPUT 0
#define VER1_OUTPUT 1

//PERMISSIONS
#define VMW_IPC_PERMISSIONS_SET 1
#define VMW_IPC_PERMISSIONS_ADD 2

//VECS
#define STORE_LABEL_MAX_LENGTH 128

typedef enum
{
    VECS_ENCRYPTION_ALGORITHM_UNKNOWN = 0,
    VECS_ENCRYPTION_ALGORITHM_MD5,
    VECS_ENCRYPTION_ALGORITHM_SHA_1,

} VECS_ENCRYPTION_ALGORITHM, *PVECS_ENCRYPTION_ALGORITHM;

#ifndef _WIN32
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#define VMAFD_CONFIG_CREDS_KEY_PATH     "Services\\services\\Parameters\\Credentials"
#define VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#else
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters"
#define VMAFD_CONFIG_CREDS_KEY_PATH     "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters\\Credentials"
#define VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#endif

#define VMAFD_REG_KEY_DOMAIN_NAME     "DomainName"
#define VMAFD_REG_KEY_DOMAIN_STATE    "DomainState"
#define VMAFD_REG_KEY_LDU             "LDU"
#define VMAFD_REG_KEY_RHTTPPROXY_PORT "RHTTPProxyPort"
#define VMAFD_REG_KEY_DC_PORT         "DCPort"
#define VMAFD_REG_KEY_DC_NAME         "DCName"
#define VMAFD_REG_KEY_PNID            "PNID"
#define VMAFD_REG_KEY_CA_PATH         "CAPath"
#define VMAFD_REG_KEY_CERT_SEC        "CertificateSyncInterval"
#define VMAFD_REG_KEY_RPC_CONFIG      "EnableDCERPC"
#define VMAFD_REG_KEY_MACHINE_ID      "MachineID"

#define VMAFD_REG_KEY_DC_ACCOUNT      "dcAccount"
#define VMAFD_REG_KEY_DC_ACCOUNT_DN   "dcAccountDN"
#define VMAFD_REG_KEY_DC_PASSWORD     "dcAccountPassword"
#define VMAFD_REG_KEY_DC_OLD_PASSWORD "dcAccountOldPassword"
#define VMAFD_REG_KEY_MACHINE_GUID    "MachineGuid"

#endif /* __VMAFDDEFINES_H__ */
