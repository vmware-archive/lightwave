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




#ifdef _WIN32
#define VMAFD_CONFIG_BASE_DIR "C:\\Program Files\\VMware\\cis\\config\\"
#define VMAFD_LOG_FILE        "C:\\ProgramData\\VMware\\cis\\logs\\vmafdd\\vmafdd.log"
#define VMAFD_KRB5_CONF       "C:\\ProgramData\\MIT\\Kerberos5\\krb5.ini"
#define VMAFD_KEYTAB_PATH     "C:\\ProgramData\\VMware\\cis\\cfg\\vmafdd\\krb5.keytab"
#define VMAFD_TRUSTED_ROOT_DOWNLOAD_PATH     "%VMWARE_CFG_DIR%\\vmware-vpx\\docRoot\\certs"
#define VMAFD_MACHINE_SSL_PATH               "%VMWARE_CFG_DIR%\\vmafdd"
#else
#include "vmafd-server-defines.h"
#endif
#define VMAFD_MAX_OLD_LOGS    5
#define VMAFD_MAX_LOG_SIZE    1000

#ifndef _WIN32

#define VMAFD_OPTION_LOGGING_LEVEL 'l'
#define VMAFD_OPTION_LOG_FILE_NAME 'L'
#define VMAFD_OPTION_ENABLE_SYSLOG 's'
#define VMAFD_OPTIONS_VALID "f:l:L:p:s"
#define VMAFD_CERT_DB VMAFD_DB_DIR "/afd.db"
#define VMAFD_OLD_CERT_DB VMAFD_DB_DIR "/vecs/afd.db"


#define VMAFD_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMAFD_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMAFD_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr
#define VMAFD_ERRNO VmAfdGetWin32ErrorCode(errno)

#else // #ifndef _WIN32
#define VMAFD_OPTION_LOGGING_LEVEL       "-l"
#define VMAFD_OPTION_ENABLE_SYSLOG       "-s"
#define VMAFD_OPTION_ENABLE_CONSOLE      "-c"
#define VMAFD_OPTION_ENABLE_CONSOLE_LONG "--console-debug"
#define VMAFD_OPTION_LOG_FILE_NAME       "-L"
#define VMAFD_CERT_DB_FILE  "afd.db"


#define VMAFD_NT_SERVICE_NAME _T("VMWareAfdService")

#define VMAFD_CLOSE_HANDLE(handle) \
    {                              \
        if ((handle) != NULL)      \
        {                          \
            CloseHandle((handle)); \
            (handle) = NULL;       \
        }                          \
    }

#define VMAFD_CLOSE_SERVICE_HANDLE(hServiceHandle) \
    {                                              \
         if ( (hServiceHandle) != NULL )           \
         {                                         \
             CloseServiceHandle((hServiceHandle)); \
             (hServiceHandle) = NULL;              \
         }                                         \
    }

#define VMAFD_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMAFD_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMAFD_ADDR_INFO_ADDR( ai ) ai->ai_addr
#define VMAFD_ERRNO GetLastError()

#endif

#define VMAFD_REG_PATH             "SOFTWARE\\VMware, Inc.\\VMware Afd Services"
#define VMAFD_REG_KEY_LOG_FILE     "LogFile"
#define VMAFD_REG_KEY_KRB5_CONF    "Krb5Conf"
#define VMAFD_REG_KEY_KEYTAB_PATH  "KeytabPath"
#define VMAFD_REG_KEY_DB_PATH      "DataPath"
#define VMAFD_REG_KEY_MAX_OLD_LOGS "MaximumOldLogs"
#define VMAFD_REG_KEY_MAX_LOG_SIZE "MaximumLogSize"

#define ATTR_AUTOREFRESH           "AutoRefresh"

// This is the prefix in flush store for marking left over cert files.
#define VECS_DEL_FILE_PREFIX       "delete_"
#define VECS_DEL_RETRY_INTV        1000
#define VECS_DEL_RETRY_MAX         10
#define VECS_MACHINE_SSL_STORE_ID  1
#define VECS_TRUSTED_ROOT_STORE_ID 2
#define VECS_CRL_STORE_ID          3

#define DEFAULT_VECS_CERT_SEC       60 // default value at which certs are pulled down by VECS, in Seconds
#define VECS_STOREHASH_MAP_SIZE     64
#define VMAFD_FILE_COPY_BUFSZ       1024

/*
 * Table to define and initialize VMAFD configuration data.
 *
 * To add a new configuration key,
 * 1. define its name in defines.h ( @ Line : 90 of this file)
 * 2. define its entry in the table below and init default/cfg Value
 *
 * VMAFD_CONFIG_VALUE_TYPE_STRING <-> REG_SZ
 * VMAFD_CONFIG_VALUE_TYPE_DWORD  <-> REG_DWORD
 * VMAFD_CONFIG_VALUE_TYPE_BOOLEAN <-> REG_DWORD
 *
 */

#define VMAFD_CONFIG_INIT_TABLE_INITIALIZER   \
{                                             \
    {                                         \
        VMAFD_REG_KEY_DOMAIN_STATE,           \
        VMAFD_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMAFD_DOMAIN_STATE_NONE, NULL},      \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_LOG_FILE,               \
        VMAFD_CONFIG_VALUE_TYPE_STRING,       \
        REG_SZ,                               \
        0,                                    \
        0,                                    \
        {0, VMAFD_LOG_FILE},                  \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_MAX_OLD_LOGS,           \
        VMAFD_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMAFD_MAX_OLD_LOGS, NULL},           \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_MAX_LOG_SIZE,           \
        VMAFD_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMAFD_MAX_LOG_SIZE, NULL},           \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_KRB5_CONF,              \
        VMAFD_CONFIG_VALUE_TYPE_STRING,       \
        REG_SZ,                               \
        0,                                    \
        0,                                    \
        {0, VMAFD_KRB5_CONF},                 \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_KEYTAB_PATH,            \
        VMAFD_CONFIG_VALUE_TYPE_STRING,       \
        REG_SZ,                               \
        0,                                    \
        0,                                    \
        {0, VMAFD_KEYTAB_PATH},               \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_CERT_SEC,               \
        VMAFD_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {DEFAULT_VECS_CERT_SEC, NULL},        \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMAFD_REG_KEY_RPC_CONFIG,             \
        VMAFD_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {1, NULL},                            \
        {1, NULL},                            \
    }                                         \
}

#define VMAFD_FIRST_REPL_CYCLE_MODE_COPY_DB 1
#define VMAFD_FIRST_REPL_CYCLE_MODE_BY_OBJECT 3

#define VMAFD_MAX_KDC_SERVERS 10
#define VMAFD_MACHINE_CERT "__MachineCert"

#define VMAFD_RPC_FLAG_ALLOW_NCALRPC         0x01
#define VMAFD_RPC_FLAG_ALLOW_TCPIP           0x02
#define VMAFD_RPC_FLAG_REQUIRE_AUTH_NCALRPC  0x04
#define VMAFD_RPC_FLAG_REQUIRE_AUTH_TCPIP    0x08
#define VMAFD_RPC_FLAG_REQUIRE_AUTHZ         0x10

#define VMAFD_HEARTBEAT_TABLE_COUNT          16

#define VMAFD_LOCK_MUTEX_EXCLUSIVE(pmutex, bLocked) \
if (! (bLocked) ) \
{ \
  pthread_rwlock_wrlock (pmutex); \
  (bLocked) = TRUE; \
}

#define VMAFD_LOCK_MUTEX_SHARED(pmutex, bLocked) \
if (! (bLocked) ) \
{ \
  pthread_rwlock_rdlock (pmutex); \
  (bLocked) = TRUE; \
}

#define VMAFD_LOCK_MUTEX_UNLOCK(pmutex, bLocked) \
if (bLocked) \
{ \
  pthread_rwlock_unlock (pmutex); \
  (bLocked) = FALSE; \
}

#define IPV4_LINKLOCAL_PREFIX_BYTE_1 0xA9
#define IPV4_LINKLOCAL_PREFIX_BYTE_2 0xFE
#define IPV6_LINKLOCAL_PREFIX_BYTE_1 0xFE
#define IPV6_LINKLOCAL_PREFIX_BYTE_2 0x80
#define IPV6_ULA_PREFIX_BYTE_1 0xFC
#define IPV6_ULA_PREFIX_BYTE_2 0x00

#define IS_IPV4_LINKLOCAL(a) \
    (((a)[0] == IPV4_LINKLOCAL_PREFIX_BYTE_1) && ((a)[1] == IPV4_LINKLOCAL_PREFIX_BYTE_2))
#define IS_IPV6_LINKLOCAL(a) \
    (((a)[0] == IPV6_LINKLOCAL_PREFIX_BYTE_1) && (((a)[1] & 0xC0)== IPV6_LINKLOCAL_PREFIX_BYTE_2))
#define IS_IPV6_ULA(a) \
    (((a)[0] == IPV6_ULA_PREFIX_BYTE_1) && (((a)[1] & 0xC0)== IPV6_ULA_PREFIX_BYTE_2))

#define VMDNS_DEFAULT_LDAP_PORT 389



#define NSECS_PER_SEC       1000000000
#define NSECS_PER_MSEC      1000000
#define MSECS_PER_SEC       1000
#define CDC_DISABLE_OFFSITE 1

#define SSO_HA_MIN_DOMAIN_LEVEL_MAJOR   2
#define SSO_HA_MIN_DOMAIN_LEVEL_MINOR   0
