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
 * Module Name: ThinAppEventLogService
 *
 * Filename: EventLogcommon.h
 *
 * Abstract:
 *
 * Common utilities between EventLog Service Components.
 *
 * Public header for libEventLogcommon.so
 *
 */

#ifndef __EVENTLOG_COMMON_H__
#define __EVENTLOG_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

// Treat wchar16_t as "unsigned short" (2 bytes char) as in Windows Unicode
typedef char             RP_STR,   *RP_PSTR;
typedef char const       RP_CSTR,  *RP_PCSTR;
typedef WCHAR            RP_WSTR,  *RP_PWSTR;
typedef WCHAR const      RP_CWSTR, *RP_PCWSTR;

typedef void*            LPVOID;
typedef char             *NPSTR, *LPSTR;

#define VMEVENT_SAFE_FREE_STRINGA(PTR)    \
    do {                          \
        if ((PTR)) {              \
            EventLogFreeStringA(PTR); \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#define VMEVENT_SAFE_FREE_STRINGW(PTR)    \
    do {                          \
        if ((PTR)) {              \
            EventLogFreeStringW(PTR); \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#define VMEVENT_SAFE_FREE_MEMORY(PTR)\
    do {                          \
        if ((PTR)) {              \
            EventLogFreeMemory(PTR);  \
            (PTR) = NULL;         \
        }                         \
    } while(0)

#define VMEVENT_LOCK_MUTEX(bInLock, mutex) \
    do {                                \
        if (!(bInLock))                 \
        {                               \
            pthread_mutex_lock(mutex);  \
            (bInLock) = TRUE;           \
        }                               \
    } while (0)

#define VMEVENT_TRYLOCK_MUTEX(bInLock, mutex, dwError) \
    do {                                \
        if (!(bInLock))                 \
        {                               \
            int iResult = pthread_mutex_trylock(mutex);       \
            if (iResult == 0)                                 \
            {                                                 \
                (bInLock) = TRUE;                             \
            }                                                 \
            else                                              \
            {                                                 \
            	if (iResult == EBUSY)                         \
            	{                                             \
            		(dwError) = ERROR_BUSY;                   \
            	}                                             \
            	else                                          \
            	{                                             \
            		(dwError) = LwErrnoToWin32Error(iResult); \
            	}                                             \
            }                                                 \
        }                                                     \
    } while (0)

#define VMEVENT_UNLOCK_MUTEX(bInLock, mutex) \
    do {                                  \
        if ((bInLock))                    \
        {                                 \
            pthread_mutex_unlock(mutex);  \
            (bInLock) = FALSE;            \
        }                                 \
    } while (0)

typedef enum
{
    VMEVENT_LOG_TYPE_CONSOLE = 0,
    VMEVENT_LOG_TYPE_FILE,
    VMEVENT_LOG_TYPE_SYSLOG
} VMEVENT_LOG_TYPE;

#ifndef _VMEVENT_LOG_LEVEL_DEFINED_
#define _VMEVENT_LOG_LEVEL_DEFINED_
typedef enum
{
    VMEVENT_LOG_LEVEL_ERROR = 0,
    VMEVENT_LOG_LEVEL_WARNING,
    VMEVENT_LOG_LEVEL_INFO,
    VMEVENT_LOG_LEVEL_VERBOSE,
    VMEVENT_LOG_LEVEL_DEBUG,
    VMEVENT_LOG_LEVEL_UNKNOWN
} VMEVENT_LOG_LEVEL;
#endif

typedef struct _VMEVENT_LOG_HANDLE* PVMEVENT_LOG_HANDLE;

extern PVMEVENT_LOG_HANDLE gpEventLogLogHandle;
extern VMEVENT_LOG_LEVEL   gEventLogLogLevel;
extern HANDLE              gpEventLog;

extern VMEVENT_LOG_LEVEL EventLogLogGetLevel();

#define VMEVENT_LOG_( LogHandle, Level, Format, ... ) \
    do                                             \
    {                                              \
        EventLogLogMessage(LogHandle,               \
               Level,                              \
               Format,                             \
               ##__VA_ARGS__);                     \
    } while (0)

#define VMEVENT_LOG_GENERAL_( Level, Format, ... )

/*#define VMEVENT_LOG_GENERAL_( Level, Format, ... ) \
    VMEVENT_LOG_( gpEventLogLogHandle, Level, Format, ##__VA_ARGS__ )*/

#define VMEVENT_LOG_ERROR( Format, ... )   \
    VMEVENT_LOG_GENERAL_( VMEVENT_LOG_LEVEL_ERROR, Format, ##__VA_ARGS__ )
#define VMEVENT_LOG_WARNING( Format, ... ) \
    VMEVENT_LOG_GENERAL_( VMEVENT_LOG_LEVEL_WARNING, Format, ##__VA_ARGS__ )
#define VMEVENT_LOG_INFO( Format, ... )    \
    VMEVENT_LOG_GENERAL_( VMEVENT_LOG_LEVEL_INFO, Format, ##__VA_ARGS__ )
#define VMEVENT_LOG_VERBOSE( Format, ... ) \
    VMEVENT_LOG_GENERAL_( VMEVENT_LOG_LEVEL_VERBOSE, Format, ##__VA_ARGS__ )
#define VMEVENT_LOG_DEBUG( Format, ... )       \
    VMEVENT_LOG_GENERAL_(                      \
        VMEVENT_LOG_LEVEL_DEBUG,               \
    Format " [file: %s][line: %d]",     \
    ##__VA_ARGS__, __FILE__, __LINE__ )

#define BAIL_ON_VMEVENT_ERROR(dwError)                                         \
    if (dwError)                                                            \
    {                                                                       \
        VMEVENT_LOG_DEBUG("error code: %#010x", dwError);                      \
        goto error;                                                         \
    }

#define BAIL_ON_VMEVENT_ERROR_NO_LOG(dwError) \
    if (dwError) { goto error; }

#define BAIL_ON_VMEVENT_INVALID_POINTER(p, errCode)     \
        if (p == NULL) {                          \
            errCode = ERROR_INVALID_PARAMETER;    \
            BAIL_ON_VMEVENT_ERROR(errCode);       \
        }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#ifndef VMEVENT_SAFE_LOG_STRING
#define VMEVENT_SAFE_LOG_STRING(str) ((str) ? (str) : "")
#endif

typedef struct _VMEVENT_PACKAGE_CONTAINER*     PVMEVENT_PACKAGE_CONTAINER;

typedef DWORD VMEVENT_SERVER_TASK_STATUS_FLAG;

#define VMEVENT_SERVER_TASK_STATUS_FLAG_IMPORTING 0x00000001
#define VMEVENT_SERVER_TASK_STATUS_FLAG_UPLOADING 0x00000002

#define VMEVENT_REG_KEY_HORIZONURL               "HorizonUrl"
#define VMEVENT_REG_KEY_HORIZONOAUTHCLIENTID     "HorizonOAuthClientId"
#define VMEVENT_REG_KEY_HORIZONOAUTHCLIENTSECRET "HorizonOAuthClientSecret"
#define VMEVENT_REG_KEY_EVENTLOGDBPATH           "DbPath"
#define VMEVENT_REG_KEY_LOGFILEPATH              "LogFilePath"
#define VMEVENT_REG_KEY_CERTFILEPATH             "CertFilePath"
#define VMEVENT_REG_KEY_REMOTEEVENTLOGSHAREPATH "RemoteEventLogSharePath"
#define VMEVENT_REG_KEY_RPCTCPENDPOINT           "RpcTcpEndPoint"
#define VMEVENT_REG_KEY_UPLOADINTERVAL           "UploadIntervalInSECS"
#define VMEVENT_REG_KEY_IMPORTINTERVAL           "ImportIntervalInSECS"
#define VMEVENT_REG_KEY_ENABLEAPPMODTIMECHK      "EnableAppModTimeChk"
#define VMEVENT_REG_KEY_LOGLEVEL                 "LogLevel"
#define VMEVENT_REG_KEY_ENABLEEVENTLOGS          "EnableEventLogs"
#define VMEVENT_REG_KEY_ENABLESERVICE            "EnableService"

// Event logs related constants.

#define VMEVENT_EVENT_SOURCE                    "ThinApp EventLog-Service"

// Event Categories
#define VMEVENT_EVENT_CATEGORY_IMPORT           "Import"
#define VMEVENT_EVENT_CATEGORY_UPLOAD           "Upload"

// Event Types
#define VMEVENT_EVENT_TYPE_INFO                 "Info"
#define VMEVENT_EVENT_TYPE_WARNING              "Warning"
#define VMEVENT_EVENT_TYPE_ERROR                "Error"

// Event IDs
#define VMEVENT_EVENT_ID_IMPORT_CRED_ERROR       0
#define VMEVENT_EVENT_ID_IMPORT_APP_ERROR        1
#define VMEVENT_EVENT_ID_IMPORT_CYCLE_START      2
#define VMEVENT_EVENT_ID_IMPORT_CYCLE_END        3

#define VMEVENT_EVENT_ID_UPLOAD_LOGIN_ERROR      4
#define VMEVENT_EVENT_ID_UPLOAD_UPDATE_ERROR     5
#define VMEVENT_EVENT_ID_UPLOAD_DB_ERROR         6
#define VMEVENT_EVENT_ID_UPLOAD_CYCLE_START      7
#define VMEVENT_EVENT_ID_UPLOAD_CYCLE_END        8

#define VMEVENT_EVENT_ID_SHARE_ACCESS_ERROR      9


#define VMEVENT_EVENT_TABLE_CATEGORY            "Application"

#define VMEVENT_SHARE_STATUS_HOST_NAME_VERIFIED   0x00000001
#define VMEVENT_SHARE_STATUS_SHARE_NAME_VERIFIED  0x00000002
#define VMEVENT_SHARE_STATUS_DOMAIN_JOIN_VERIFIED 0x00000004
#define VMEVENT_SHARE_STATUS_PERMISSION_VERIFIED  0x00000008

#ifndef _WIN32
#define VMEVENT_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VMEVENT_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMEVENT_ASCII_LOWER(c)    ( (c) >= 'a' && (c) <= 'z' )
#define VMEVENT_ASCII_UPPER(c)    ( (c) >= 'A' && (c) <= 'Z' )
#define VMEVENT_ASCII_DIGIT(c)    ( (c) >= '0' && (c) <= '9' )

#define VMEVENT_ASCII_LOWER_TO_UPPER(c) \
if ( VMEVENT_ASCII_LOWER(c) )           \
{                                       \
    (c) = ((c) - 32);                   \
}

#define VMEVENT_ASCII_UPPER_TO_LOWER(c) \
if ( VMEVENT_ASCII_UPPER(c) )           \
{                                       \
    (c) = ((c) + 32);                   \
}

DWORD
EventLogAllocateMemory(
    DWORD dwSize,
    PVOID * ppMemory
    );

VOID
EventLogFreeMemory(
    PVOID pMemory
    );

DWORD
EventLogAllocateStringA(
    PCSTR pszString,
    PSTR * ppszString
    );

DWORD
EventLogAllocateStringPrintfA(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    );

VOID
EventLogFreeStringA(
    PSTR pszString
    );

DWORD
EventLogAllocateStringW(
    PCWSTR pszString,
    PWSTR * ppszString
    );

VOID
EventLogFreeStringW(
    PWSTR pszString
    );

DWORD
EventLogLogSetTypeFile(
    PVMEVENT_LOG_HANDLE plogHandle,
    const char*      inName
    );

DWORD
EventLogSetTypeSyslog(
    PVMEVENT_LOG_HANDLE plogHandle
    );

DWORD
EventLogLogSetTypeConsole(
    PVMEVENT_LOG_HANDLE plogHandle
    );

DWORD
EventLogLogInitialize(
    PVMEVENT_LOG_HANDLE plogHandle
    );

VOID
EventLogLogShutdown(
    PVMEVENT_LOG_HANDLE plogHandle
    );

VOID
EventLogLogMessage(
    PVMEVENT_LOG_HANDLE plogHandle,
    VMEVENT_LOG_LEVEL   level,
    const char*      fmt,
    ...
    );

VMEVENT_LOG_LEVEL
EventLogLogLevelFromInt(
    int intLoglevel
    );

VMEVENT_LOG_LEVEL
EventLogLogLevelFromTag(
    const char* level
    );

const char*
EventLogLogLevelToTag(
    VMEVENT_LOG_LEVEL level
    );

VMEVENT_LOG_LEVEL
EventLogLogGetLevel(
    );

VOID
EventLogLogSetLevel(
    VMEVENT_LOG_LEVEL level
    );

VMEVENT_LOG_TYPE
EventLogLogGetType(
    PVMEVENT_LOG_HANDLE plogHandle
    );

const char*
EventLogLogGetFilename(
    PVMEVENT_LOG_HANDLE plogHandle
    );

DWORD
EventLogAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

DWORD
EventLogAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

int
EventLogStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

ULONG
EventLogGetStringLengthW(
    PCWSTR  pwszStr,
    PSIZE_T pLength
    );

DWORD
EventLogStringLenA(
    PCSTR pszStr
    );

int
VmAfdStringCompareA(
    PCSTR pszStr1,
    PCSTR pszStr2,
    BOOLEAN bIsCaseSensitive
    );

DWORD
EventLogAllocateStringPrintf(
    OUT PSTR* ppszString,
    IN PCSTR pszFormat,
    IN ...
    );

DWORD
EventLogAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

DWORD
EventLogFileExists(
    PCSTR pszFileName,
    PBOOLEAN pbFound
    );

DWORD
EventLogGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostName
    );

DWORD
EventLogGetHostName(
    PSTR* ppszHostName
    );

#ifdef __cplusplus
}
#endif

#endif /* __EVENTLOG_COMMON_H__ */
