/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : defines.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            dns main
 *
 *            Local definitions
 *
 */

#ifndef _WIN32

#define VMDNS_OPTION_LOGGING_LEVEL 'l'
#define VMDNS_OPTION_LOG_FILE_NAME 'L'
#define VMDNS_OPTION_PORT 'p'
#define VMDNS_OPTION_ENABLE_SYSLOG 's'
#define VMDNS_OPTIONS_VALID "l:L:p:s"

/*
 * Table to define and initialize VMDNS configuration data.
 *
 * To add a new configuration key,
 * 1. define its name in vmdnscommon.h
 * 2. define its entry in the table below and init default/cfg Value
 *
 * VMDNS_CONFIG_VALUE_TYPE_STRING <-> REG_SZ
 * VMDNS_CONFIG_VALUE_TYPE_DWORD  <-> REG_DWORD
 * VMDNS_CONFIG_VALUE_TYPE_BOOLEAN <-> REG_DWORD
 *
 */

#define VMDNS_CONFIG_INIT_TABLE_INITIALIZER               \
{                                                         \
    {                                                     \
        .pszName        = VMDNS_REG_KEY_PORT,             \
        .Type           = VMDNS_CONFIG_VALUE_TYPE_DWORD,  \
        .RegDataType    = REG_DWORD,                      \
        .dwMin          = 0,                              \
        .dwMax          = UINT32_MAX,                     \
        .defaultValue.dwDefault = VMDNS_PORT, \
        .cfgValue.dwValue = 0                             \
    }                                                     \
};

#define VMDNS_MAX_CONFIG_VALUE_LENGTH   2048
#define VMDNS_REG_KEY_PORT              "Port"

#define VMDNS_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMDNS_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMDNS_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#else // #ifndef _WIN32
#define VMDNS_OPTION_LOGGING_LEVEL  "-l"
#define VMDNS_OPTION_LOG_FILE_NAME  "-L"
#define VMDNS_OPTION_PORT           "-p"
#define VMDNS_OPTION_ENABLE_SYSLOG  "-s"
#define VMDNS_OPTION_CONSOLE_MODE   "-C"

#define VMDNS_NT_SERVICE_NAME _T("VMWareDomainNameService")

#define VMDNS_CLOSE_HANDLE(handle) \
    {                              \
        if ((handle) != NULL)      \
        {                          \
            CloseHandle((handle)); \
            (handle) = NULL;       \
        }                          \
    }

#define VMDNS_CLOSE_SERVICE_HANDLE(hServiceHandle) \
    {                                              \
         if ( (hServiceHandle) != NULL )           \
         {                                         \
             CloseServiceHandle((hServiceHandle)); \
             (hServiceHandle) = NULL;              \
         }                                         \
    }

#define VMDNS_MAX_CONFIG_VALUE_LENGTH   2048

#define VMDNS_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMDNS_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMDNS_ADDR_INFO_ADDR( ai ) ai->ai_addr

#endif

#define VMDNS_KEY_VALUE_LOG_PATH "LogPath"
#define VMDNS_KEY_VALUE_LOG_CAP  "MaxLogFileCount"
#define VMDNS_KEY_VALUE_LOG_SIZE "MaxLogSize"

#define VMDNS_DEFAULT_LOG_CAP 5
#define VMDNS_DEFAULT_LOG_SIZE 10*(1<<6)

#define VMDNS_DEFAULT_REFRESH_INTERVAL  3600
#define VMDNS_DEFAULT_RETRY_INTERVAL    600
#define VMDNS_DEFAULT_EXPIRE            86400
#define VMDNS_DEFAULT_TTL               3600
#define VMDNS_DEFAULT_LDAP_PORT         389
