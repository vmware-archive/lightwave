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
#define VMAFD_DEBUG_ANY (1)
#endif

#ifndef VMAFD_DEBUG_ERROR
#define VMAFD_DEBUG_ERROR (2)
#endif

#ifndef VMAFD_DEBUG_WARNING
#define VMAFD_DEBUG_WARNING (3)
#endif

#ifndef VMAFD_DEBUG_INFO
#define VMAFD_DEBUG_INFO (4)
#endif

#ifndef VMAFD_DEBUG_TRACE
#define VMAFD_DEBUG_TRACE (5)
#endif

#ifndef VMAFD_DEBUG_DEBUG
#define VMAFD_DEBUG_DEBUG (6)
#endif

#ifdef _WIN32
    #define BOOLEAN BOOL
    #define PBOOLEAN PBOOL
    #define PCVOID const PVOID
    #define ssize_t SSIZE_T
#endif

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
        VmAfdLog( VMAFD_DEBUG_ERROR, "[Error - %d, %s:%d]", dwError, __FILE__, __LINE__); \
        goto error;                                                \
    }

#define BAIL_ON_VMAFD_ERROR_NO_LOG(dwError) \
    if (dwError)                                                   \
    {                                                              \
        goto error;                                                \
    }

#define BAIL_ON_VMAFD_ERROR_IF(condition) \
    if (condition)                                                 \
    {                                                              \
        VmAfdLog( VMAFD_DEBUG_ERROR, "[%s,%d]",__FILE__, __LINE__); \
        goto error;                                                \
    }

#define BAIL_ON_VMAFD_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMAFD_ERROR(errCode);          \
        }

#define BAIL_ON_VMAFD_EMPTY_STRING(p, errCode) \
        if (IsNullOrEmptyString(p)) {             \
            errCode = ERROR_INVALID_PARAMETER;   \
            BAIL_ON_VMAFD_ERROR(errCode);        \
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

#define IsFlagSet(mask, bits) (((mask) != 0) && (((mask) & (bits)) == (bits)))

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

#ifndef _WIN32
#define InterlockedExchange __sync_lock_test_and_set
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
#define VMAFD_IPC_GET_SITE_NAME              48
#define VMAFD_IPC_GET_RHTTPPROXY_PORT        49
#define VMAFD_IPC_JOIN_VMDIR_2               50
#define VMAFD_IPC_REFRESH_SITE_NAME          51
#define VMAFD_IPC_CONFIGURE_DNS              52
#define VMAFD_IPC_JOIN_VALIDATE_CREDENTIALS  53
#define VMAFD_IPC_GET_DC_LIST                54
#define VMAFD_IPC_CHANGE_PNID                55
#define VMAFD_IPC_CREATE_COMPUTER_ACCOUNT    56

#define CDC_IPC_ENABLE_DEFAULT_HA            60
#define CDC_IPC_ENABLE_LEGACY_HA             61
#define CDC_IPC_GET_DC_NAME                  62
#define CDC_IPC_GET_CDC_STATE                63
#define CDC_IPC_ENUM_DC_ENTRIES              64
#define CDC_IPC_GET_DC_STATUS_INFO           65

#define VMAFD_IPC_POST_HEARTBEAT             70
#define VMAFD_IPC_GET_HEARBEAT_STATUS        71

#define VMAFD_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VMAFD_MAX(a, b) ((a) > (b) ? (a) : (b))

#define RPC_PING_TIMEOUT 5

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

//CDC

#define CDC_DEFAULT_SYNC_INTERVAL     1*60
#define CDC_DEFAULT_HEARTBEAT         30

typedef enum
{
    CDC_DB_ENTRY_STATUS_UNDEFINED = 0,
    CDC_DB_ENTRY_STATUS_NEW,
    CDC_DB_ENTRY_STATUS_SITE_UPDATE,
    CDC_DB_ENTRY_STATUS_UPDATE,
    CDC_DB_ENTRY_STATUS_EXISTING
} CDC_DB_ENTRY_STATUS, *PCDC_DB_ENTRY_STATUS;

typedef struct _CDC_DB_ENTRY_W
{
    PWSTR pszDCName;
    PWSTR pszSiteName;
    PWSTR pszDomainName;
    DWORD dwPingTime;
    DWORD dwLastPing;
    DWORD dwLastError;
    BOOL  bIsAlive;
    CDC_DB_ENTRY_STATUS cdcEntryStatus;
} CDC_DB_ENTRY_W, *PCDC_DB_ENTRY_W;

typedef struct _CDC_DB_ENTRY_ARRAY
{
    PCDC_DB_ENTRY_W *pCdcDbEntries;
    DWORD dwCount;
} CDC_DB_ENTRY_ARRAY, *PCDC_DB_ENTRY_ARRAY;

typedef struct _VMAFD_CRED_CONTEXT_W
{
    PWSTR pwszDCName;
    PWSTR pwszUPN;
    PWSTR pwszPassword;
} VMAFD_CRED_CONTEXT_W, *PVMAFD_CRED_CONTEXT_W;


//Heartbeat
#define VMAFD_HEARTBEAT_INTERVAL 10

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
#define VMAFD_REG_KEY_SITE_NAME       "SiteName"
#define VMAFD_REG_KEY_DOMAIN_STATE    "DomainState"
#define VMAFD_REG_KEY_LDU             "LDU"
#define VMAFD_REG_KEY_RHTTPPROXY_PORT "RHTTPProxyPort"
#define VMAFD_REG_KEY_DC_PORT         "DCPort"
#define VMAFD_REG_KEY_DC_NAME         "DCName"
#define VMAFD_REG_KEY_DC_NAME_HA      "DCNameHA"
#define VMAFD_REG_KEY_DC_ENTRIES      "DCEntries"
#define VMAFD_REG_KEY_PNID            "PNID"
#define VMAFD_REG_KEY_CA_PATH         "CAPath"
#define VMAFD_REG_KEY_CERT_SEC        "CertificateSyncInterval"
#define VMAFD_REG_KEY_DCCACHE_SYNC    "DcCacheSyncInterval"
#define VMAFD_REG_KEY_DC_HEARTBEAT    "DcCacheHeartBeat"
#define VMAFD_REG_KEY_RPC_CONFIG      "EnableDCERPC"
#define VMAFD_REG_KEY_MACHINE_ID      "MachineID"

#define VMAFD_REG_KEY_DC_ACCOUNT      "dcAccount"
#define VMAFD_REG_KEY_DC_ACCOUNT_DN   "dcAccountDN"
#define VMAFD_REG_KEY_DC_PASSWORD     "dcAccountPassword"
#define VMAFD_REG_KEY_DC_OLD_PASSWORD "dcAccountOldPassword"
#define VMAFD_REG_KEY_MACHINE_GUID    "MachineGuid"

#define VMAFD_REG_VALUE_HA_CONFIG     "LegacyModeHA"
#define VMAFD_REG_VALUE_SITE          "Site"
#define VMAFD_REG_VALUE_LAST_PING     "LastPing"
#define VMAFD_REG_VALUE_PING_TIME     "PingTime"
#define VMAFD_REG_KEY_ENABLE_DNS      "EnableDnsUpdates"
#define VMAFD_REG_KEY_HEARTBEAT       "HeartbeatInterval"

//domainJoinFlag
#define VMAFD_DOMAIN_LEAVE_FLAGS_FORCE 0x00000001

#endif /* __VMAFDDEFINES_H__ */
