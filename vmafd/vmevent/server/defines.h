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
 * Module Name: ThinAppRepoService
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Definitions/Macros
 *
 */

#define    GetLastError() errno

#define  __RPC_USER

#define EVENTLOG_LOG_FILE                "/opt/thinapp/repo/Repo.log"

#define EVENTLOG_DEFAULT_ICON_PATH       "/opt/vmware/tam/share/config/default.png"

#define EVENTLOG_SERVICE_MAX_SPN_SIZE    500
#define EVENTLOG_SERVICE_SPN_FIRST_PART  L"RPC/RepoService"

#define EVENTLOG_ASSERT(x) assert( (x) )

#define EVENTLOG_MAX(x, y) ((x) > (y) ? (x) : (y))

#define _PTHREAD_FUNCTION_RTN_ASSERT(Function, ...)       \
    do {                                                  \
        int error = Function(__VA_ARGS__);                \
        EVENTLOG_ASSERT(!error);                              \
    } while (0)

#define INITIALIZE_SRW_LOCK(pRWLock, pRWLockAttr) \
    _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_init, pRWLock, pRWLockAttr)

#define ENTER_READERS_SRW_LOCK(bHasLock, pRWLock)                 \
    do {                                                          \
    assert (!bHasLock);                                           \
    _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_rdlock, pRWLock); \
    bHasLock = true;                                              \
    } while(0)

#define LEAVE_READERS_SRW_LOCK(bHasLock, pRWLock)                 \
    do {                                                          \
    if (bHasLock) {                                               \
    _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_unlock, pRWLock); \
    bHasLock = false;                                             \
    }                                                             \
    } while(0)

#define ENTER_WRITER_SRW_LOCK(bHasLock, pRWLock)                  \
    do {                                                          \
    assert (!bHasLock);                                           \
    _PTHREAD_FUNCTION_RTN_ASSERT(pthread_rwlock_wrlock, pRWLock); \
    bHasLock = true;                                              \
    } while(0)

#define LEAVE_WRITER_SRW_LOCK(bHasLock, pSRWLock)  LEAVE_READERS_SRW_LOCK(bHasLock, pSRWLock)


#define SQL_BUFFER_SIZE          1024


#define EVENTLOG_RPC_SAFE_FREE_MEMORY(mem) \
    if ((mem) != NULL) \
    { \
        EventLogRpcFreeMemory(mem); \
    }

#define IMAGE_DOS_MAGIC        0x5A4D     /* MZ */
#define IMAGE_NT_SIGNATURE     0x00004550 /* PE00 */
#define IMAGE_PNG_SIGNATURE    0x474E5089

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16
#define IMAGE_DIRECTORY_ENTRY_RESOURCE      2
#define IMAGE_SIZEOF_SHORT_NAME             8

typedef enum
{
    IMAGE_ORIENTATION_UNKNOWN = 0,
    IMAGE_ORIENTATION_TOP_DOWN,
    IMAGE_ORIENTATION_BOTTOM_UP
} IMAGE_ORIENTATION;


#define THINAPP_PKG_SIGNATURE     { 't', 'h', 'i', 'n' }
#define THINAPP_PKG_APP_ID   { 'A', 'p', 'p', 'I', 'D', 0 }
#define THINAPP_PKG_VERSION_ID   { 'V', 'e', 'r', 's', 'i', 'o', 'n', 'I', 'D', 0 }
#define THINAPP_PKG_INVENTORY_ICON \
        {'I', 'n', 'v', 'e', 'n', 't', 'o', 'r', 'y', 'I', 'c', 'o', 'n', 0 }
#define THINAPP_PKG_SHORTCUT_ICON {'I', 'c', 'o', 'n', 0 }
#define THINAPP_PKG_INVENTORY_NAME \
        {'I', 'n', 'v', 'e', 'n', 't', 'o', 'r', 'y', 'N', 'a', 'm', 'e', 0 }
#define THINAPP_PKG_READONLYDATA \
        {'R','e','a','d','O','n','l','y','D','a','t','a', 0 }
#define THINAPP_PKG_SHORTCUT \
        {'S','h','o','r','t','c','u','t', 0}

#define UNKNOWN_STRING "UNKNOWN"

#define EVENTLOG_ERROR_HZN_TABLE_INITIALIZER \
{ \
    { -200, "HZN_NO_TOKEN", ""}, \
    { -100, "HZN_BAD_LANDING", ""}, \
    { -25 , "HZN_INVALID_SIGNATURE", ""}, \
    { -1  , "HZN_FAILURE", ""}, \
    { 0   , "HZN_SUCCESS", ""}, \
    { 304 , "HZN_NOT_MODIFIED", ""}, \
    { 400 , "HZN_BAD_REQUEST", ""}, \
    { 401 , "HZN_UNAUTHORIZED", ""}, \
    { 403 , "HZN_FORBIDDEN", ""}, \
    { 404 , "HZN_NOT_FOUND", ""}, \
    { 406 , "HZN_NOT_ACCEPTABLE", ""}, \
    { 409 , "HZN_CONFLICT", ""}, \
    { 500 , "HZN_INTERNAL_SERVER_ERROR", ""}, \
};
#define EVENTLOG_CONFIG_PARAMETER_KEY_PATH "Services\\thinapprepo\\Parameters"
#define EVENTLOG_MAX_CONFIG_VALUE_LENGTH   2048

#define BUFFER_SIZE_64	64

#define EVENTLOG_CFG_BOOLEAN_TRUE  "1"
#define EVENTLOG_CFG_BOOLEAN_FALSE "0"

#define EVENTLOG_STATE_ENABLED  "ENABLED"
#define EVENTLOG_STATE_DISABLED "DISABLED"

#ifdef _WIN32

#define VMEVENT_OPTION_LOGGING_LEVEL       "-l"
#define VMEVENT_OPTION_ENABLE_SYSLOG       "-s"
#define VMEVENT_OPTION_ENABLE_CONSOLE      "-c"
#define VMEVENT_OPTION_ENABLE_CONSOLE_LONG "--console-debug"
#define VMEVENT_OPTION_LOG_FILE_NAME       "-L"
#define VMEVENT_CERT_DB "c:\\vmware\\vmevent.db"

#define VMEVENT_IF_HANDLE_T RPC_IF_HANDLE
#define VMEVENT_RPC_BINDING_VECTOR_P_T RPC_BINDING_VECTOR*
#define VMEVENT_RPC_AUTHZ_HANDLE RPC_AUTHZ_HANDLE
#define VMEVENT_RPC_BINDING_HANDLE RPC_BINDING_HANDLE
#define VMEVENT_RPC_C_AUTHN_LEVEL_PKT RPC_C_AUTHN_LEVEL_PKT

#define VMEVENT_NT_SERVICE_NAME _T("VMWareEventService")

#define VMEVENT_CLOSE_HANDLE(handle) \
    {                              \
        if ((handle) != NULL)      \
        {                          \
            CloseHandle((handle)); \
            (handle) = NULL;       \
        }                          \
    }

#define VMEVENT_CLOSE_SERVICE_HANDLE(hServiceHandle) \
    {                                              \
         if ( (hServiceHandle) != NULL )           \
         {                                         \
             CloseServiceHandle((hServiceHandle)); \
             (hServiceHandle) = NULL;              \
         }                                         \
    }

#define VMEVENT_MAX_CONFIG_VALUE_LENGTH   2048
#define VMEVENT_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMwareEventService\\Parameters"
#define VMEVENT_CONFIG_CREDS_KEY_PATH     "SYSTEM\\CurrentControlSet\\services\\VMwareEventService\\Parameters\\Credentials"


#define VMEVENT_ADDR_INFO_NEXT( ai ) ai->ai_next
#define VMEVENT_ADDR_INFO_FLAGS( ai ) ai->ai_flags
#define VMEVENT_ADDR_INFO_ADDR( ai ) ai->ai_addr

#define tcp_close( s )	(shutdown( s, SD_BOTH ), closesocket( s ))

#ifndef VMEVENT_DEBUG_ANY
#define VMEVENT_DEBUG_ANY (-1)
#endif

#ifndef VMEVENT_DEBUG_TRACE
#define VMEVENT_DEBUG_TRACE (1)
#endif

typedef enum
{
	VMEVENT_STATUS_UNKNOWN       = 0,
	VMEVENT_STATUS_INITIALIZING,
	VMEVENT_STATUS_PAUSED,
	VMEVENT_STATUS_RUNNING,
	VMEVENT_STATUS_STOPPING,
	VMEVENT_STATUS_STOPPED,
} VMEVENT_STATUS, *PVMEVENT_STATUS;

#define BAIL_ON_VMEVENT_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMEVENT_ERROR(errCode);          \
        }

#endif
