/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#define LOGGING_API __declspec(dllexport)

#define BAIL_ON_VMAFD_LOG_ERROR(dwError)    \
    if (dwError) { goto error; }

PVMAFD_LOG_CTX   _gpVmAfdLogCtx = NULL;

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
    int iLevel
    );

static
VOID
_VmAfdLog1(
    int      iLevel,
    PCSTR    fmt,
    va_list  args
    );

static
void *
_VmAfdLogThread(
    void *pvArg
    );

static
DWORD
_VmAfdOpenLog(
    VOID
    );

static
VOID
_VmAfdCloseLog(
    VOID
    );

static
DWORD
_VmAfdRotateLogs(
    VOID
    );

static
VOID
_VmAfdTruncateLog(
    VOID
    );

static
DWORD
_VmAfdGetFileSize(
    FILE *pFile,
    PINT64 pI64Size
    );

static
DWORD
_VmAfdGetRotateLogFilePath(
    PSTR *ppszServerRotateLogFile,
    PCSTR pszServerLogFile,
    DWORD dwLogNumber
    );

static
BOOLEAN
_VmAfdTruncateFile(
    FILE* pFile
    );

LOGGING_API
VOID
VmAfdLogTerminate(
    VOID
    )
{
    if (_gpVmAfdLogCtx)
    {
        if (_gpVmAfdLogCtx->pThread)
        {
            // Signal logging thread to exit
            pthread_mutex_lock(&_gpVmAfdLogCtx->pThreadMutex);
            _gpVmAfdLogCtx->bThreadShouldExit = TRUE;
            pthread_cond_signal(&_gpVmAfdLogCtx->pThreadCond);
            pthread_mutex_unlock(&_gpVmAfdLogCtx->pThreadMutex);

            // Wait for thread to stop
            pthread_join(*_gpVmAfdLogCtx->pThread, NULL);

            VMAFD_SAFE_FREE_MEMORY(_gpVmAfdLogCtx->pThread);
        }

        if (_gpVmAfdLogCtx->pThreadCond)
        {
            pthread_cond_destroy(&_gpVmAfdLogCtx->pThreadCond);
        }

        if (_gpVmAfdLogCtx->pThreadMutex)
        {
            pthread_mutex_destroy(&_gpVmAfdLogCtx->pThreadMutex);
        }

        _VmAfdCloseLog();

        pthread_mutex_destroy(&_gpVmAfdLogCtx->pLogMutex);

        VMAFD_SAFE_FREE_MEMORY(_gpVmAfdLogCtx->pszSyslogDaemon);
        VMAFD_SAFE_FREE_MEMORY(_gpVmAfdLogCtx->pszLogFileName);
        VMAFD_SAFE_FREE_MEMORY(_gpVmAfdLogCtx);
    }
}

LOGGING_API
DWORD
VmAfdLogInitialize(
   PCSTR            pszLogFileName,
   DWORD            dwMaximumOldFiles,
   INT64            i64MaxLogSizeBytes
   )
{
    DWORD            dwError = 0;
    BOOLEAN          bLogInitFailed = TRUE;

    if ( _gpVmAfdLogCtx )
    {
        bLogInitFailed = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwError = VmAfdAllocateMemory(sizeof(*_gpVmAfdLogCtx), &_gpVmAfdLogCtx);
    BAIL_ON_VMAFD_LOG_ERROR(dwError);

    _gpVmAfdLogCtx->dwMaxOldLogs = dwMaximumOldFiles;
    _gpVmAfdLogCtx->i64MaxLogSizeBytes = i64MaxLogSizeBytes;
    _gpVmAfdLogCtx->bThreadShouldExit = FALSE;

    pthread_mutex_init(&_gpVmAfdLogCtx->pLogMutex, NULL);

    if (pszLogFileName)
    {
        dwError = VmAfdAllocateStringA(pszLogFileName, &_gpVmAfdLogCtx->pszLogFileName);
        BAIL_ON_VMAFD_LOG_ERROR(dwError);

        dwError = _VmAfdOpenLog();
        BAIL_ON_VMAFD_LOG_ERROR(dwError);

        dwError = pthread_mutex_init(&_gpVmAfdLogCtx->pThreadMutex, NULL);
        BAIL_ON_VMAFD_LOG_ERROR(dwError);

        if (_gpVmAfdLogCtx->dwMaxOldLogs != 0 ||
            _gpVmAfdLogCtx->i64MaxLogSizeBytes != 0)
        {
            dwError = pthread_cond_init(&_gpVmAfdLogCtx->pThreadCond, NULL);
            BAIL_ON_VMAFD_LOG_ERROR(dwError);

            dwError = VmAfdAllocateMemory(
                            sizeof(pthread_t),
                            &_gpVmAfdLogCtx->pThread);
            BAIL_ON_VMAFD_LOG_ERROR(dwError);

            dwError = pthread_create(
                            _gpVmAfdLogCtx->pThread,
                            NULL,
                            _VmAfdLogThread,
                            NULL);
            BAIL_ON_VMAFD_LOG_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    if (bLogInitFailed)
    {
        VmAfdLogTerminate();
    }

    goto cleanup;
}

LOGGING_API
VOID
VmAfdLog(
   int iLevel,
   const char*  fmt,
   ...)
{
    DWORD           dwError = 0;
    va_list         va;

    if ( _gpVmAfdLogCtx == NULL )
    {
        dwError = 1;
        BAIL_ON_VMAFD_LOG_ERROR(dwError);
    }

    if (iLevel == VMAFD_DEBUG_ANY || iLevel == VMAFD_DEBUG_ERROR)
    {
        va_start( va, fmt );
        _VmAfdLog1( iLevel, fmt, va );
        va_end( va );
    }

error:

    return;
}

static
void *
_VmAfdLogThread(
    void *pvArg
    )
{
    DWORD dwError = 0;
    DWORD dwTimeout = 0;
    struct timespec timeout = {0};

    pthread_mutex_lock(&_gpVmAfdLogCtx->pThreadMutex);

    do
    {
        BOOLEAN bShouldExit = FALSE;

        timeout.tv_sec = time(NULL) + dwTimeout/1000;
        timeout.tv_nsec = 0;

        dwError = pthread_cond_timedwait(
                            &_gpVmAfdLogCtx->pThreadCond,
                            &_gpVmAfdLogCtx->pThreadMutex,
                            &timeout
                            );
        if (_gpVmAfdLogCtx->bThreadShouldExit == TRUE)
        {
            break;
        }

        if (dwError == ETIMEDOUT || dwError == 0)
        {
            INT64 i64LogSizeBytes = 0;

            pthread_mutex_lock(&_gpVmAfdLogCtx->pLogMutex);

            _VmAfdGetFileSize(_gpVmAfdLogCtx->pFile, &i64LogSizeBytes);

            if (_gpVmAfdLogCtx->dwMaxOldLogs > 0 &&
                _gpVmAfdLogCtx->i64MaxLogSizeBytes > 0)
            {
                if (i64LogSizeBytes >= _gpVmAfdLogCtx->i64MaxLogSizeBytes)
                {
                    _VmAfdCloseLog();

                    _VmAfdRotateLogs();
                }
            }
            else if (_gpVmAfdLogCtx->dwMaxOldLogs == 0 &&
                     _gpVmAfdLogCtx->i64MaxLogSizeBytes > 0)
            {
                if (i64LogSizeBytes >= _gpVmAfdLogCtx->i64MaxLogSizeBytes)
                {
                    _VmAfdTruncateLog();
                }
            }

            dwTimeout = HEALTHY_LOG_CHECK_INTERVAL_MSEC;
            // reopen log -- may need to happen if an error happened last time.
            if (_gpVmAfdLogCtx->pFile == NULL)
            {
                dwError = _VmAfdOpenLog();
                if (dwError)
                {
                    dwTimeout = UNHEALTHY_LOG_CHECK_INTERVAL_MSEC;
                }
            }

            pthread_mutex_unlock(&_gpVmAfdLogCtx->pLogMutex);
        }
    } while (1);

    pthread_mutex_unlock(&_gpVmAfdLogCtx->pThreadMutex);

    return NULL;
}

static
PCSTR
_logLevelToTag(
   int level)
{
   switch( level )
   {
      case VMAFD_DEBUG_ANY:
         return "INFO";
      case VMAFD_DEBUG_TRACE:
         return "TRACE";
      case VMAFD_DEBUG_ERROR:
         return "ERROR";
      case VMAFD_DEBUG_DEBUG:
      default:
         return "DEBUG";
   }
}

static
VOID
_VmAfdLog1(
    int      iLevel,
    PCSTR    fmt,
    va_list  args
    )
{
    char        extraLogMessage[EXTRA_LOG_MESSAGE_LEN] = {0};
    char        logMessage[MAX_LOG_MESSAGE_LEN];
    const char* logLevelTag = "";
    SYSTEMTIME systemTime = {0};
    long thread_id = 0;
    BOOLEAN bInLock = FALSE;

    thread_id = (long) GetCurrentThreadId();

    vsnprintf( logMessage, sizeof(logMessage), fmt, args );
    logMessage[sizeof(logMessage)-1] = '\0';

    if ( ! _gpVmAfdLogCtx->bSyslog )
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

        pthread_mutex_lock(&_gpVmAfdLogCtx->pLogMutex);

        if (_gpVmAfdLogCtx->pFile != NULL)
        {
            pLogFile = _gpVmAfdLogCtx->pFile;
        }

        fprintf(pLogFile, "%s%s\n", extraLogMessage, logMessage);

        pthread_mutex_unlock(&_gpVmAfdLogCtx->pLogMutex);
    }

    return;
}

/*
 * Open the log file, either appending or creating anew.
 */
static
DWORD
_VmAfdOpenLog(
    VOID
    )
{
    DWORD dwError = 0;

    _gpVmAfdLogCtx->pFile = fopen(_gpVmAfdLogCtx->pszLogFileName, "a");
    if ( _gpVmAfdLogCtx->pFile == NULL)
    {
       dwError = errno;
       BAIL_ON_VMAFD_LOG_ERROR(dwError);
    }

    setvbuf(_gpVmAfdLogCtx->pFile, NULL, _IONBF, 0);

cleanup:
    return dwError;

error:
    _VmAfdCloseLog();
    goto cleanup;
}

/*
 * Close the log file.
 */
static
VOID
_VmAfdCloseLog(
    VOID
    )
{
    if (_gpVmAfdLogCtx && _gpVmAfdLogCtx->pFile != NULL)
    {
        fclose(_gpVmAfdLogCtx->pFile);
        _gpVmAfdLogCtx->pFile = NULL;
    }
}

/*
 * Rotate logs, deleting highest allowed log file if present.
 */
static
DWORD
_VmAfdRotateLogs(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszLog1FileName = NULL;
    PSTR pszLog2FileName = NULL;
    DWORD dwLogNumber = 0;

    if (_gpVmAfdLogCtx->dwMaxOldLogs == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_LOG_ERROR(dwError);
    }

    for (dwLogNumber = _gpVmAfdLogCtx->dwMaxOldLogs;
         dwLogNumber <= _gpVmAfdLogCtx->dwMaxOldLogs;
         dwLogNumber--)
    {
        dwError = _VmAfdGetRotateLogFilePath(&pszLog1FileName,
                                             _gpVmAfdLogCtx->pszLogFileName,
                                             dwLogNumber);
        BAIL_ON_VMAFD_LOG_ERROR(dwError);

        if (dwLogNumber == _gpVmAfdLogCtx->dwMaxOldLogs)
        {
            // Oldest allowed log may be present, so delete it to make room for next oldest
            DeleteFileA(pszLog1FileName);
        }
        else
        {
            // Move 'vmdir.log.3' to 'vmdir.log.4'
            MoveFileA(pszLog1FileName, pszLog2FileName);
        }

        VMAFD_SAFE_FREE_MEMORY(pszLog2FileName);
        pszLog2FileName = pszLog1FileName;
        pszLog1FileName = NULL;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszLog1FileName);
    VMAFD_SAFE_FREE_MEMORY(pszLog2FileName);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmAfdGetFileSize(
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
        BAIL_ON_VMAFD_LOG_ERROR(dwError);
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
_VmAfdGetRotateLogFilePath(
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
    dwError = VmAfdAllocateMemory(dwLen * sizeof(char), &pszServerRotateLogFile);
    BAIL_ON_VMAFD_LOG_ERROR(dwError);

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
    VMAFD_SAFE_FREE_MEMORY(pszServerRotateLogFile);

    goto cleanup;
}

static
VOID
_VmAfdTruncateLog(
    VOID
    )
{
    if (_gpVmAfdLogCtx && _gpVmAfdLogCtx->pFile != NULL)
    {
        _VmAfdTruncateFile(_gpVmAfdLogCtx->pFile);
    }
}

/*
 * Reduce file size to zero.
 */
static
BOOLEAN
_VmAfdTruncateFile(
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
