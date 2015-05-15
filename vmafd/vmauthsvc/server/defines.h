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



#ifndef _WIN32

#define VMAUTHSVC_OPTION_LOGGING_LEVEL 'l'
#define VMAUTHSVC_OPTION_LOG_FILE_NAME 'L'
#define VMAUTHSVC_OPTION_PORT 'p'
#define VMAUTHSVC_OPTION_ENABLE_SYSLOG 's'
#define VMAUTHSVC_OPTIONS_VALID "l:L:p:s"

/*
 * Table to define and initialize VMAUTHSVC configuration data.
 *
 * To add a new configuration key,
 * 1. define its name in vmauthsvccommon.h
 * 2. define its entry in the table below and init default/cfg Value
 *
 * VMAUTHSVC_CONFIG_VALUE_TYPE_STRING <-> REG_SZ
 * VMAUTHSVC_CONFIG_VALUE_TYPE_DWORD  <-> REG_DWORD
 * VMAUTHSVC_CONFIG_VALUE_TYPE_BOOLEAN <-> REG_DWORD
 *
 */

#define VMAUTHSVC_CONFIG_INIT_TABLE_INITIALIZER               \
{                                                         \
    {                                                     \
        .pszName        = VMAUTHSVC_REG_KEY_PORT,             \
        .Type           = VMAUTHSVC_CONFIG_VALUE_TYPE_DWORD,  \
        .RegDataType    = REG_DWORD,                      \
        .dwMin          = 0,                              \
        .dwMax          = UINT32_MAX,                     \
        .defaultValue.dwDefault = VMAUTHSVC_PORT, \
        .cfgValue.dwValue = 0                             \
    }                                                     \
};

#define VMAUTHSVC_MAX_CONFIG_VALUE_LENGTH   2048
#define VMAUTHSVC_CONFIG_PARAMETER_KEY_PATH "Services\\vmauthsvc\\Parameters"
#define VMAUTHSVC_CONFIG_CREDS_KEY_PATH     "Services\\vmauthsvc\\Parameters\\Credentials"
#define VMAUTHSVC_REG_KEY_PORT              "Port"
#define VMAUTHSVC_REG_KEY_ADMIN_DN     "AdministratorDN"
#define VMAUTHSVC_REG_KEY_ADMIN_PASSWD "AdministratorPassword"

#define VMAUTHSVC_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMAUTHSVC_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMAUTHSVC_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#else // #ifndef _WIN32
#define VMAUTHSVC_OPTION_LOGGING_LEVEL "l"
#define VMAUTHSVC_OPTION_LOG_FILE_NAME "L"
#define VMAUTHSVC_OPTION_PORT          "p"
#define VMAUTHSVC_OPTION_ENABLE_SYSLOG "s"

#define VMAUTHSVC_NT_SERVICE_NAME _T("VMwareAuthsvcService")

#define VMAUTHSVC_CLOSE_HANDLE(handle) \
    {                              \
        if ((handle) != NULL)      \
        {                          \
            CloseHandle((handle)); \
            (handle) = NULL;       \
        }                          \
    }

#define VMAUTHSVC_CLOSE_SERVICE_HANDLE(hServiceHandle) \
    {                                              \
         if ( (hServiceHandle) != NULL )           \
         {                                         \
             CloseServiceHandle((hServiceHandle)); \
             (hServiceHandle) = NULL;              \
         }                                         \
    }

#define VMAUTHSVC_MAX_CONFIG_VALUE_LENGTH   2048
#define VMAUTHSVC_CONFIG_PARAMETER_KEY_PATH _T("SYSTEM\\CurrentControlSet\\services\\VMwareAuthsvcService\\Parameters")
#define VMAUTHSVC_CONFIG_CREDS_KEY_PATH     _T("SYSTEM\\CurrentControlSet\\services\\VMwareAuthsvcService\\Parameters\\Credentials")
#define VMAUTHSVC_REG_KEY_PORT         _T("Port")
#define VMAUTHSVC_REG_KEY_ADMIN_DN     _T("AdministratorDN")
#define VMAUTHSVC_REG_KEY_ADMIN_PASSWD _T("AdministratorPassword")

#define VMAUTHSVC_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMAUTHSVC_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMAUTHSVC_ADDR_INFO_ADDR( ai ) ai->ai_addr

#define tcp_close( s )	(shutdown( s, SD_BOTH ), closesocket( s ))

#endif
