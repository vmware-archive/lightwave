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
#define VMKDC_CONFIG_BASE_DIR "C:\\Program Files\\VMware\\cis\\config\\"
#define VMKDC_LOG_FILE        "C:\\Documents and Settings\\All Users\\Application Data\\VMware\\cis\\logs\\vmdird\\vmkdc.log"
#else
#define VMKDC_CONFIG_BASE_DIR "/opt/vmware/share/config/"
#define VMKDC_LOG_FILE        "/var/log/vmware/vmkdc/vmkdcd.log"
#endif

#define VMKDC_KERBEROS_PORT         88
#define VMKDC_DEFAULT_REALM         "VSPHERE.LOCAL"
#define VMKDC_CLOCK_SKEW            300
#define VMKDC_MAX_LIFE              28800
#define VMKDC_MAX_RENEWABLE_LIFE    604800

#define VMKDC_TCP_READ_BUFSIZ 4096
#define VMKDC_UDP_READ_BUFSIZ 2048
#define MAX_KRB_REQ_LEN (1024 * 16) // 16K

#ifndef _WIN32
#define VMKDC_RPC_SERVER_NOTAVAIL 1225
#else
#define VMKDC_RPC_SERVER_NOTAVAIL 1727
#endif
#define VMKDC_DIRECTORY_NOTREADY 2103
#define VMKDC_PRINCIPAL_NOTFOUND 2003
#define VMKDC_DIRECTORY_POLL_SECS 10

#ifndef _WIN32

#define VMKDC_OPTION_LOGGING_LEVEL 'l'
#define VMKDC_OPTION_ENABLE_SYSLOG 's'
#define VMKDC_OPTIONS_VALID "l:s"

#define VMKDC_MAX_CONFIG_VALUE_LENGTH       2048
#define VMKDC_CONFIG_PARAMETER_KEY_PATH     "Services\\vmdir\\Parameters"

#define VMKDC_REG_KEY_KERBEROS_PORT         "KerberosPort"
#define VMKDC_REG_KEY_DEFAULT_REALM         "DefaultRealm"
#define VMKDC_REG_KEY_CLOCK_SKEW            "KdcClockSkew"
#define VMKDC_REG_KEY_MAX_LIFE              "KdcMaxLife"
#define VMKDC_REG_KEY_MAX_RENEWABLE_LIFE    "KdcMaxRenewableLife"
#define VMKDC_REG_KEY_LOG_FILE              "KdcLogFile"

#define VMKDC_ADDR_INFO_NEXT( ifa ) ifa->ifa_next
#define VMKDC_ADDR_INFO_FLAGS( ifa ) ifa->ifa_flags
#define VMKDC_ADDR_INFO_ADDR( ifa ) ifa->ifa_addr

#else // #ifndef _WIN32

#define VMKDC_OPTION_LOGGING_LEVEL "-l"
#define VMKDC_OPTION_ENABLE_SYSLOG "-s"
#define VMKDC_OPTION_ENABLE_CONSOLE "-c"
#define VMKDC_OPTION_ENABLE_CONSOLE_LONG "--console-debug"

#define VMKDC_NT_SERVICE_NAME _T("VMwareKdcService")

#define VMKDC_CLOSE_HANDLE(handle) \
    {                              \
        if ((handle) != NULL)      \
        {                          \
            CloseHandle((handle)); \
            (handle) = NULL;       \
        }                          \
    }

#define VMKDC_CLOSE_SERVICE_HANDLE(hServiceHandle) \
    {                                              \
         if ( (hServiceHandle) != NULL )           \
         {                                         \
             CloseServiceHandle((hServiceHandle)); \
             (hServiceHandle) = NULL;              \
         }                                         \
    }

#define VMKDC_MAX_CONFIG_VALUE_LENGTH       2048
#define VMKDC_CONFIG_PARAMETER_KEY_PATH     _T("SYSTEM\\CurrentControlSet\\Services\\LightwaveRaftService\\Parameters")

#define VMKDC_REG_KEY_KERBEROS_PORT         _T("KerberosPort")
#define VMKDC_REG_KEY_DEFAULT_REALM         _T("DefaultRealm")
#define VMKDC_REG_KEY_CLOCK_SKEW            _T("ClockSkew")
#define VMKDC_REG_KEY_MAX_LIFE              _T("MaxLife")
#define VMKDC_REG_KEY_MAX_RENEWABLE_LIFE    _T("MaxRenewableLife")
#define VMKDC_REG_KEY_LOG_FILE              _T("KdcLogFile")

#define VMKDC_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMKDC_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMKDC_ADDR_INFO_ADDR( ai ) ai->ai_addr

#define tcp_close( s )	(shutdown( s, SD_BOTH ), closesocket( s ))

#endif

/*
 * Table to define and initialize VMKDC configuration data.
 *
 * To add a new configuration key,
 * 1. define its name in vmkdccommon.h
 * 2. define its entry in the table below and init default/cfg Value
 *
 * VMKDC_CONFIG_VALUE_TYPE_STRING <-> REG_SZ
 * VMKDC_CONFIG_VALUE_TYPE_DWORD  <-> REG_DWORD
 * VMKDC_CONFIG_VALUE_TYPE_BOOLEAN <-> REG_DWORD
 *
 */

#define VMKDC_CONFIG_INIT_TABLE_INITIALIZER   \
{                                             \
    {                                         \
        VMKDC_REG_KEY_KERBEROS_PORT,          \
        VMKDC_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMKDC_KERBEROS_PORT, NULL},          \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMKDC_REG_KEY_CLOCK_SKEW,             \
        VMKDC_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMKDC_CLOCK_SKEW, NULL},             \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMKDC_REG_KEY_MAX_LIFE,               \
        VMKDC_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMKDC_MAX_LIFE, NULL},               \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMKDC_REG_KEY_MAX_RENEWABLE_LIFE,     \
        VMKDC_CONFIG_VALUE_TYPE_DWORD,        \
        REG_DWORD,                            \
        0,                                    \
        UINT32_MAX,                           \
        {VMKDC_MAX_RENEWABLE_LIFE, NULL},     \
        {0, NULL},                            \
    },                                        \
    {                                         \
        VMKDC_REG_KEY_LOG_FILE,               \
        VMKDC_CONFIG_VALUE_TYPE_STRING,       \
        REG_SZ,                               \
        0,                                    \
        0,                                    \
        {0, VMKDC_LOG_FILE},                  \
        {0, NULL},                            \
    },                                        \
}
