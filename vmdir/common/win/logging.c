/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

#define BAIL_ON_VMDIR_LOG_ERROR(dwError)    \
    if (dwError) { goto error; }

PVMDIR_LOG_CTX _gpVmDirLogCtx = NULL;


#ifndef EXTRA_LOG_MESSAGE_LEN
#define EXTRA_LOG_MESSAGE_LEN 256
#endif

#ifndef MAX_LOG_MESSAGE_LEN
#define MAX_LOG_MESSAGE_LEN 256
#endif

#define HEALTHY_LOG_CHECK_INTERVAL_MSEC  (30 * 1000)
#define UNHEALTHY_LOG_CHECK_INTERVAL_MSEC (2 * 1000)

static
PCSTR
_logLevelToTag(
    VMDIR_LOG_LEVEL iLevel
    );

static
VOID
_VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    PCSTR           fmt,
    va_list         args
    );

static
DWORD
_VmDirLogThread(
    PVOID pvArg
    );

static
DWORD
_VmDirOpenLog(
    );

static
VOID
_VmDirCloseLog(
    );

static
DWORD
_VmDirRotateLogs(
    );

static
VOID
_VmDirTruncateLog(
    );

static
DWORD
_VmDirGetFileSize(
    FILE *pFile,
    PINT64 pI64Size
    );

static
DWORD
_VmDirGetRotateLogFilePath(
    PSTR *ppszServerRotateLogFile,
    PCSTR pszServerLogFile,
    DWORD dwLogNumber
    );

static
BOOLEAN
_VmDirTruncateFile(
    FILE* pFile
    );

__declspec(dllexport)
VOID
VmDirLogSetLevel(
    VMDIR_LOG_LEVEL iNewLogLevel
    )
{
   if ( _gpVmDirLogCtx)
   {
       _gpVmDirLogCtx->iLogLevel = iNewLogLevel;
   }
}

__declspec(dllexport)
VMDIR_LOG_LEVEL
VmDirLogGetLevel(
    VOID
    )
{
    return _gpVmDirLogCtx ? _gpVmDirLogCtx->iLogLevel : VMDIR_LOG_INFO;
}

__declspec(dllexport)
VOID
VmDirLogSetMask(
    ULONG             iNewLogMask
    )
{
    if (_gpVmDirLogCtx)
    {
        _gpVmDirLogCtx->iLogMask = iNewLogMask;
    }
}

__declspec(dllexport)
ULONG
VmDirLogGetMask(
    VOID
    )
{
    return _gpVmDirLogCtx ? _gpVmDirLogCtx->iLogMask : 0;
}

__declspec(dllexport)
VOID
VmDirLogTerminate(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    if (_gpVmDirLogCtx)
    {
        HMODULE hLber = NULL;
        int (*pfn_ber_set_option)(void *item, int option, LDAP_CONST void *invalue) = NULL;
        HMODULE hLdap = NULL;
        int (*pfn_ldap_set_option)(LDAP *ld, int option, LDAP_CONST void *invalue) = NULL;

        hLber = GetModuleHandle(L"liblber.dll");
        if (hLber != NULL)
        {
            pfn_ber_set_option = (int (*)(void*, int, LDAP_CONST void *))GetProcAddress(hLber, "ber_set_option");

            if (pfn_ber_set_option)
            {
                pfn_ber_set_option(NULL, LBER_OPT_X_LOG_PROC, NULL);
            }
        }

        hLdap = GetModuleHandle(L"libldap_r.dll");
        if (hLdap != NULL)
        {
            pfn_ldap_set_option = (int (*)(void*, int, LDAP_CONST void *))GetProcAddress(hLdap, "ldap_set_option");

            if (pfn_ldap_set_option)
            {
                pfn_ldap_set_option(NULL, LBER_OPT_X_LOG_PROC, NULL);
            }
        }

        if (_gpVmDirLogCtx->pThread)
        {
            // Signal logging thread to exit
            VMDIR_LOCK_MUTEX(bInLock, _gpVmDirLogCtx->pThreadMutex);
            _gpVmDirLogCtx->bThreadShouldExit = TRUE;
            VmDirConditionSignal(_gpVmDirLogCtx->pThreadCond);
            VMDIR_UNLOCK_MUTEX(bInLock, _gpVmDirLogCtx->pThreadMutex);

            // Wait for thread to stop
            VmDirThreadJoin(_gpVmDirLogCtx->pThread, NULL);

            // Clean up thread resources
            VmDirFreeVmDirThread(_gpVmDirLogCtx->pThread);
            VMDIR_SAFE_FREE_MEMORY(_gpVmDirLogCtx->pThread);
        }

        VMDIR_SAFE_FREE_CONDITION(_gpVmDirLogCtx->pThreadCond);
        VMDIR_SAFE_FREE_MUTEX(_gpVmDirLogCtx->pThreadMutex);

        _VmDirCloseLog();

        VMDIR_SAFE_FREE_MEMORY(_gpVmDirLogCtx->pszSyslogDaemon);
        VMDIR_SAFE_FREE_MEMORY(_gpVmDirLogCtx->pszLogFileName);

        VMDIR_SAFE_FREE_MUTEX(_gpVmDirLogCtx->pLogMutex);

        VMDIR_SAFE_FREE_MEMORY(_gpVmDirLogCtx);
    }

   return;
}

LOGGING_API
DWORD
VmDirLogInternalInitialize(
   PCSTR            pszLogFileName,
   BOOLEAN          bUseSysLog,
   PCSTR            pszSyslogName,
   VMDIR_LOG_LEVEL  iInitLogLevel,
   ULONG            iInitLogMask,
   DWORD            dwMaximumOldFiles,
   INT64            i64MaxLogSizeBytes
   )
{
    DWORD            dwError = 0;
    BOOLEAN          bLogInitFailed = TRUE;
    HMODULE          hLber = NULL;
    HMODULE          hLdap = NULL;
    int (*pfn_ber_set_option)(void *item, int option, LDAP_CONST void *invalue) = NULL;
    int (*pfn_ldap_set_option)(LDAP *ld, int option, LDAP_CONST void *invalue) = NULL;

    if ( _gpVmDirLogCtx )
    {
        bLogInitFailed = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwError = VmDirAllocateMemory(sizeof(*_gpVmDirLogCtx), &_gpVmDirLogCtx);
    BAIL_ON_VMDIR_LOG_ERROR(dwError);

    _gpVmDirLogCtx->iLogLevel = iInitLogLevel;
    _gpVmDirLogCtx->iLogMask  = iInitLogMask;
    _gpVmDirLogCtx->dwMaxOldLogs = dwMaximumOldFiles;
    _gpVmDirLogCtx->i64MaxLogSizeBytes = i64MaxLogSizeBytes;
    _gpVmDirLogCtx->bThreadShouldExit = FALSE;

    dwError = VmDirAllocateMutex(&_gpVmDirLogCtx->pLogMutex);
    BAIL_ON_VMDIR_LOG_ERROR(dwError);

    if (pszLogFileName)
    {
        dwError = VmDirAllocateStringA(pszLogFileName, &_gpVmDirLogCtx->pszLogFileName);
        BAIL_ON_VMDIR_LOG_ERROR(dwError);

        dwError = _VmDirOpenLog();
        BAIL_ON_VMDIR_LOG_ERROR(dwError);

        dwError = VmDirAllocateMutex(&_gpVmDirLogCtx->pThreadMutex);
        BAIL_ON_VMDIR_LOG_ERROR(dwError);

        if (_gpVmDirLogCtx->dwMaxOldLogs != 0 ||
            _gpVmDirLogCtx->i64MaxLogSizeBytes != 0)
        {
            dwError = VmDirAllocateCondition(&_gpVmDirLogCtx->pThreadCond);
            BAIL_ON_VMDIR_LOG_ERROR(dwError);

            dwError = VmDirAllocateMemory(
                            sizeof(VMDIR_THREAD),
                            &_gpVmDirLogCtx->pThread);
            BAIL_ON_VMDIR_LOG_ERROR(dwError);

            dwError = VmDirCreateThread(_gpVmDirLogCtx->pThread,
                            TRUE,
                            _VmDirLogThread,
                            NULL);
            BAIL_ON_VMDIR_LOG_ERROR(dwError);
        }
    }

    hLber = GetModuleHandle(L"liblber.dll");
    if (hLber != NULL)
    {
        pfn_ber_set_option = (int (*)(void*, int, LDAP_CONST void *))GetProcAddress(hLber, "ber_set_option");

        if (pfn_ber_set_option)
        {
            pfn_ber_set_option(NULL, LBER_OPT_X_LOG_PROC, VmDirLog);
        }
    }

    hLdap = GetModuleHandle(L"libldap_r.dll");
    if (hLdap != NULL)
    {
        pfn_ldap_set_option = (int (*)(void*, int, LDAP_CONST void *))GetProcAddress(hLdap, "ldap_set_option");

        if (pfn_ldap_set_option)
        {
            pfn_ldap_set_option(NULL, LBER_OPT_X_LOG_PROC, VmDirLog);
        }
    }

cleanup:
    return dwError;

error:
    if (bLogInitFailed)
    {
        VmDirLogTerminate();
    }

    goto cleanup;
}

__declspec(dllexport)
DWORD
VmDirLogInitialize(
   PCSTR            pszLogFileName,
   BOOLEAN          bUseSysLog,
   PCSTR            pszSyslogName,
   VMDIR_LOG_LEVEL  iInitLogLevel,
   ULONG            iInitLogMask
   )
{
    DWORD dwError = 0;

    dwError = VmDirLogInternalInitialize(
                    pszLogFileName,
                    bUseSysLog,
                    pszSyslogName,
                    iInitLogLevel,
                    iInitLogMask,
                    0,
                    0);

    return dwError;
}

VOID
VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    ULONG           iMask,
    const char*     fmt,
    ...)
{
    DWORD       dwError = 0;
    va_list     va;

    if ( _gpVmDirLogCtx == NULL )
    {
        dwError = 1;
        BAIL_ON_VMDIR_LOG_ERROR(dwError);
    }

    if ( (_gpVmDirLogCtx->iLogLevel >= iLevel )
         &&
         ( (iMask & _gpVmDirLogCtx->iLogMask)
           ||
           (iMask == LDAP_DEBUG_ANY)
           ||
           (iMask == VMDIR_LOG_MASK_ALL) )
       )
    {
        va_start( va, fmt );
        _VmDirLog1(iLevel, fmt, va);
        va_end( va );
    }

error:

    return;
}

__declspec(dllexport)
VOID
VmDirLog(
   ULONG        iMask,
   const char*  fmt,
   ...)
{
    DWORD           dwError = 0;
    va_list         va;
    VMDIR_LOG_LEVEL iLevel = VMDIR_LOG_ERROR; //BUGBUG, should be INFO, but for B/C
                                              // until we convert old VmDirLog

   if ( _gpVmDirLogCtx == NULL )
   {
       dwError = 1;
       BAIL_ON_VMDIR_LOG_ERROR(dwError);
   }

   if ( (_gpVmDirLogCtx->iLogLevel >= iLevel )
        &&
        ( (iMask & _gpVmDirLogCtx->iLogMask)
          ||
          (iMask == LDAP_DEBUG_ANY)
          ||
          (iMask == VMDIR_LOG_MASK_ALL) )
      )
   {
      va_start( va, fmt );
      _VmDirLog1( iLevel, fmt, va );
      va_end( va );
   }

error:

    return;
}

static
DWORD
_VmDirLogThread(
    PVOID pvArg
    )
{
    DWORD dwError = 0;
    DWORD dwTimeout = 0;
    BOOLEAN bInThreadLock = FALSE;
    BOOLEAN bInLogLock = FALSE;

    VMDIR_LOCK_MUTEX(bInThreadLock, _gpVmDirLogCtx->pThreadMutex);
    do
    {
        BOOLEAN bShouldExit = FALSE;

        dwError = VmDirConditionTimedWait(
                            _gpVmDirLogCtx->pThreadCond,
                            _gpVmDirLogCtx->pThreadMutex,
                            dwTimeout
                            );
        if (_gpVmDirLogCtx->bThreadShouldExit == TRUE)
        {
                break;
        }

        if (dwError == ETIMEDOUT || dwError == 0)
        {
            INT64 i64LogSizeBytes = 0;

            VMDIR_LOCK_MUTEX(bInLogLock, _gpVmDirLogCtx->pLogMutex);

            _VmDirGetFileSize(_gpVmDirLogCtx->pFile, &i64LogSizeBytes);

            if (_gpVmDirLogCtx->dwMaxOldLogs > 0 &&
                _gpVmDirLogCtx->i64MaxLogSizeBytes > 0)
            {
                if (i64LogSizeBytes >= _gpVmDirLogCtx->i64MaxLogSizeBytes)
                {
                    _VmDirCloseLog();

                    _VmDirRotateLogs();
                }
            }
            else if (_gpVmDirLogCtx->dwMaxOldLogs == 0 &&
                     _gpVmDirLogCtx->i64MaxLogSizeBytes > 0)
            {
                if (i64LogSizeBytes >= _gpVmDirLogCtx->i64MaxLogSizeBytes)
                {
                    _VmDirTruncateLog();
                }
            }

            dwTimeout = HEALTHY_LOG_CHECK_INTERVAL_MSEC;
            // reopen log -- may need to happen if an error happened last time.
            if (_gpVmDirLogCtx->pFile == NULL)
            {
                dwError = _VmDirOpenLog();
                if (dwError)
                {
                    dwTimeout = UNHEALTHY_LOG_CHECK_INTERVAL_MSEC;
                }
            }
            VMDIR_UNLOCK_MUTEX(bInLogLock, _gpVmDirLogCtx->pLogMutex);
        }
    } while (1);

    VMDIR_UNLOCK_MUTEX(bInThreadLock, _gpVmDirLogCtx->pThreadMutex);
    VMDIR_UNLOCK_MUTEX(bInLogLock, _gpVmDirLogCtx->pLogMutex);

    return dwError;
}

static
PCSTR
_logLevelToTag(
    VMDIR_LOG_LEVEL iLevel
    )
{
   switch( iLevel )
   {
      case VMDIR_LOG_ERROR:
          return "ERROR";
      case VMDIR_LOG_WARNING:
          return "WARNING";
      case VMDIR_LOG_INFO:
          return "INFO";
      case VMDIR_LOG_VERBOSE:
          return "VERBOSE";
      case VMDIR_LOG_DEBUG:
          return "DEBUG";
      default:
         return "INFO";
   }
}

static
VOID
_VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    PCSTR           fmt,
    va_list         args
    )
{
    char        extraLogMessage[EXTRA_LOG_MESSAGE_LEN] = {0};
    char        logMessage[MAX_LOG_MESSAGE_LEN];
    const char* logLevelTag = "";
    SYSTEMTIME systemTime = {0};
    long thread_id = 0;
    BOOLEAN bInLock = FALSE;

    thread_id = (long) PTHREAD_SELF();

    vsnprintf( logMessage, sizeof(logMessage), fmt, args );
    logMessage[sizeof(logMessage)-1] = '\0';

    if ( ! _gpVmDirLogCtx->bSyslog )
    {
        FILE *pLogFile = stderr;

        logLevelTag = _logLevelToTag(iLevel);

        GetSystemTime(&systemTime);
        _snprintf(extraLogMessage, sizeof(extraLogMessage) - 1,
            "%4d-%02d-%02dT%02d:%02d:%02d.%03ldZ:t@%lu:%-3.7s: ",
            systemTime.wYear,
            systemTime.wMonth,
            systemTime.wDay,
            systemTime.wHour,
            systemTime.wMinute,
            systemTime.wSecond,
            systemTime.wMilliseconds,
            (unsigned long) thread_id,
            logLevelTag? logLevelTag : "UNKNOWN");

        VMDIR_LOCK_MUTEX(bInLock, _gpVmDirLogCtx->pLogMutex);

        if (_gpVmDirLogCtx->pFile != NULL)
        {
            pLogFile = _gpVmDirLogCtx->pFile;
        }

        fprintf(pLogFile, "%s%s\n", extraLogMessage, logMessage);

        VMDIR_UNLOCK_MUTEX(bInLock, _gpVmDirLogCtx->pLogMutex);

    }

    return;
}

/*
 * Open the log file, either appending or creating anew.
 */
static
DWORD
_VmDirOpenLog(
    VOID
    )
{
    DWORD dwError = 0;

    _gpVmDirLogCtx->pFile = fopen(_gpVmDirLogCtx->pszLogFileName, "a");
    if ( _gpVmDirLogCtx->pFile == NULL)
    {
       dwError = errno;
       BAIL_ON_VMDIR_LOG_ERROR(dwError);
    }

    setvbuf(_gpVmDirLogCtx->pFile, NULL, _IONBF, 0);

cleanup:
    return dwError;

error:
    _VmDirCloseLog();
    goto cleanup;
}

/*
 * Close the log file.
 */
static
VOID
_VmDirCloseLog(
    VOID
    )
{
    if (_gpVmDirLogCtx && _gpVmDirLogCtx->pFile != NULL)
    {
        fclose(_gpVmDirLogCtx->pFile);
        _gpVmDirLogCtx->pFile = NULL;
    }
}

/*
 * Rotate logs, deleting highest allowed log file if present.
 */
static
DWORD
_VmDirRotateLogs(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszLog1FileName = NULL;
    PSTR pszLog2FileName = NULL;
    DWORD dwLogNumber = 0;

    if (_gpVmDirLogCtx->dwMaxOldLogs == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_LOG_ERROR(dwError);
    }

    for (dwLogNumber = _gpVmDirLogCtx->dwMaxOldLogs;
         dwLogNumber <= _gpVmDirLogCtx->dwMaxOldLogs;
         dwLogNumber--)
    {
        dwError = _VmDirGetRotateLogFilePath(&pszLog1FileName,
                                             _gpVmDirLogCtx->pszLogFileName,
                                             dwLogNumber);
        BAIL_ON_VMDIR_LOG_ERROR(dwError);

        if (dwLogNumber == _gpVmDirLogCtx->dwMaxOldLogs)
        {
            // Oldest allowed log may be present, so delete it to make room for next oldest
            DeleteFileA(pszLog1FileName);
        }
        else
        {
            // Move 'vmdir.log.3' to 'vmdir.log.4'
            MoveFileA(pszLog1FileName, pszLog2FileName);
        }

        VMDIR_SAFE_FREE_MEMORY(pszLog2FileName);
        pszLog2FileName = pszLog1FileName;
        pszLog1FileName = NULL;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLog1FileName);
    VMDIR_SAFE_FREE_MEMORY(pszLog2FileName);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirGetFileSize(
    FILE* pFile,
    PINT64 pI64Size
    )
{
    DWORD dwError = 0;
    int fd = -1;

    fd = _fileno(pFile);
    if (fd == -1)
    {
        dwError = errno;
        BAIL_ON_VMDIR_LOG_ERROR(dwError);
    }

    *pI64Size = _filelengthi64(fd);

error:
    return dwError;
}

/*
 * Given a number (say 8) and "c:\foo\vmdir.log", return "c:\foo\vmdir.log.8"
 * Using zero (0) will just make a copy of the name.
*/
static
DWORD
_VmDirGetRotateLogFilePath(
    PSTR *ppszServerRotateLogFile,
    PCSTR pszServerLogFile,
    DWORD dwLogNumber
    )
{
    DWORD dwError = 0;
    PSTR pszServerRotateLogFile = NULL;
    size_t dwLen = 0;

    dwLen += strlen(pszServerLogFile); // vmdir.log
    // DWORD is represented at most as 10 characters when string (4294967295)
    dwLen += 1 + 10 + 1;               // period, max(len(string(DWORD))), \0
    dwError = VmDirAllocateMemory(dwLen * sizeof(char), &pszServerRotateLogFile);
    BAIL_ON_VMDIR_LOG_ERROR(dwError);

    if (dwLogNumber == 0)
    {
        sprintf_s(pszServerRotateLogFile, dwLen, "%s", pszServerLogFile);
    }
    else
    {
        sprintf_s(pszServerRotateLogFile, dwLen, "%s.%u", pszServerLogFile, dwLogNumber);
    }

    *ppszServerRotateLogFile = pszServerRotateLogFile;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszServerRotateLogFile);

    goto cleanup;
}

static
VOID
_VmDirTruncateLog(
    VOID
    )
{
    if (_gpVmDirLogCtx && _gpVmDirLogCtx->pFile != NULL)
    {
        _VmDirTruncateFile(_gpVmDirLogCtx->pFile);
    }
}

/*
 * Reduce file size to zero.
 */
static
BOOLEAN
_VmDirTruncateFile(
    FILE* pFile
    )
{
    BOOLEAN bSuccess = FALSE;
    int fd = -1;

    fd = _fileno(pFile);
    if (fd != -1)
    {
        if (_chsize_s(fd, 0) == 0)
        {
            bSuccess = TRUE;
        }
    }
    return bSuccess;
}
