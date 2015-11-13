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
 * Module   : logging.c
 *
 * Abstract :
 *
 *            VMware Domain Name Service
 *
 *            Common Utilities (Client & Server)
 *
 *            Logging
 *
 */

#include "includes.h"

extern int  vmdns_syslog_level;

#define BAIL_ON_VMDNS_LOG_ERROR(dwError)    \
    if (dwError) { goto error; }

PVMDNS_LOG_CTX   _gpVmDnsLogCtx = NULL;

#ifndef EXTRA_LOG_MESSAGE_LEN
#define EXTRA_LOG_MESSAGE_LEN 256
#endif

#ifndef MAX_LOG_MESSAGE_LEN
#define MAX_LOG_MESSAGE_LEN 256
#endif

#define HEALTHY_LOG_CHECK_INTERVAL_MSEC  (30 * 1000)
#define UNHEALTHY_LOG_CHECK_INTERVAL_MSEC (2 * 1000)

#define VMDNS_KEY_VALUE_LOG_PATH "LogPath"
#define VMDNS_KEY_VALUE_LOG_SIZE "MaxLogSize"
#define VMDNS_KEY_VALUE_LOG_CAP  "MaxLogFileCount"

static
PCSTR
_logLevelToTag(
    int iLevel
    );

static
VOID
_VmDnsLog1(
    int      iLevel,
    PCSTR    fmt,
    va_list  args
    );

static
void *
_VmDnsLogThread(
    void *pvArg
    );

static
DWORD
_VmDnsOpenLog(
    VOID
    );

static
VOID
_VmDnsCloseLog(
    VOID
    );

static
DWORD
_VmDnsRotateLogs(
    VOID
    );

static
VOID
_VmDnsTruncateLog(
    VOID
    );

static
DWORD
_VmDnsGetFileSize(
    FILE *pFile,
    PINT64 pI64Size
    );

static
DWORD
_VmDnsGetRotateLogFilePath(
    PSTR *ppszServerRotateLogFile,
    PCSTR pszServerLogFile,
    DWORD dwLogNumber
    );

static
BOOLEAN
_VmDnsTruncateFile(
    FILE* pFile
    );

VOID
VmDnsLogTerminate(
    VOID
    )
{
    if (_gpVmDnsLogCtx)
    {
        if (_gpVmDnsLogCtx->pThread)
        {
            // Signal logging thread to exit
            pthread_mutex_lock(&_gpVmDnsLogCtx->pThreadMutex);
            _gpVmDnsLogCtx->bThreadShouldExit = TRUE;
            pthread_cond_signal(&_gpVmDnsLogCtx->pThreadCond);
            pthread_mutex_unlock(&_gpVmDnsLogCtx->pThreadMutex);

            // Wait for thread to stop
            pthread_join(*_gpVmDnsLogCtx->pThread, NULL);

            VMDNS_SAFE_FREE_MEMORY(_gpVmDnsLogCtx->pThread);
        }

        if (_gpVmDnsLogCtx->pThreadCond)
        {
            pthread_cond_destroy(&_gpVmDnsLogCtx->pThreadCond);
        }
        if (_gpVmDnsLogCtx->pThreadMutex)
        {
            pthread_mutex_destroy(&_gpVmDnsLogCtx->pThreadMutex);
        }

        _VmDnsCloseLog();

        if (_gpVmDnsLogCtx->pLogMutex)
        {
            pthread_mutex_destroy(&_gpVmDnsLogCtx->pLogMutex);
        }

        VMDNS_SAFE_FREE_MEMORY(_gpVmDnsLogCtx->pszSyslogDaemon);
        VMDNS_SAFE_FREE_MEMORY(_gpVmDnsLogCtx->pszLogFileName);
        VMDNS_SAFE_FREE_MEMORY(_gpVmDnsLogCtx);
    }

   return;
}

DWORD
VmDnsLogInitialize(
   PCSTR    pszLogFileName,
   DWORD    dwMaximumOldFiles,
   DWORD    dwMaxLogSizeBytes
   )
{
    DWORD            dwError = 0;
    BOOLEAN          bLogInitFailed = TRUE;

    if ( _gpVmDnsLogCtx )
    {
        bLogInitFailed = FALSE;
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwError = VmDnsAllocateMemory(sizeof(*_gpVmDnsLogCtx), &_gpVmDnsLogCtx);
    BAIL_ON_VMDNS_LOG_ERROR(dwError);

    _gpVmDnsLogCtx->dwMaxOldLogs = dwMaximumOldFiles;
    _gpVmDnsLogCtx->dwMaxLogSizeBytes = dwMaxLogSizeBytes;
    _gpVmDnsLogCtx->bThreadShouldExit = FALSE;

    pthread_mutex_init(&_gpVmDnsLogCtx->pLogMutex, NULL);

    if (pszLogFileName)
    {
        dwError = VmDnsAllocateStringA(pszLogFileName, &_gpVmDnsLogCtx->pszLogFileName);
        BAIL_ON_VMDNS_LOG_ERROR(dwError);

        dwError = _VmDnsOpenLog();
        BAIL_ON_VMDNS_LOG_ERROR(dwError);

        dwError = pthread_mutex_init(&_gpVmDnsLogCtx->pThreadMutex, NULL);
        BAIL_ON_VMDNS_LOG_ERROR(dwError);

        if (_gpVmDnsLogCtx->dwMaxOldLogs != 0 ||
            _gpVmDnsLogCtx->dwMaxLogSizeBytes != 0)
        {
            dwError = pthread_cond_init(&_gpVmDnsLogCtx->pThreadCond, NULL);
            BAIL_ON_VMDNS_LOG_ERROR(dwError);

            dwError = VmDnsAllocateMemory(
                            sizeof(pthread_t),
                            &_gpVmDnsLogCtx->pThread);
            BAIL_ON_VMDNS_LOG_ERROR(dwError);

            dwError = pthread_create(
                            _gpVmDnsLogCtx->pThread,
                            NULL,
                            _VmDnsLogThread,
                            NULL);
            BAIL_ON_VMDNS_LOG_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    if (bLogInitFailed)
    {
        VmDnsLogTerminate();
    }

    goto cleanup;
}

VOID
VmDnsLog(
   int iLevel,
   const char*  fmt,
   ...)
{
    DWORD           dwError = 0;
    va_list         va;

    if ( _gpVmDnsLogCtx == NULL )
    {
        dwError = 1;
        BAIL_ON_VMDNS_LOG_ERROR(dwError);
    }

    if (iLevel <= vmdns_syslog_level)
    {
        va_start( va, fmt );
        _VmDnsLog1( iLevel, fmt, va );
        va_end( va );
    }

error:

    return;
}

static
void *
_VmDnsLogThread(
    void *pvArg
    )
{
    DWORD dwError = 0;
    DWORD dwTimeout = 60 * 1000;
    struct timespec timeout = {0};

    pthread_mutex_lock(&_gpVmDnsLogCtx->pThreadMutex);

    do
    {
        BOOLEAN bShouldExit = FALSE;

        timeout.tv_sec = time(NULL) + dwTimeout/1000;
        timeout.tv_nsec = 0;

        dwError = pthread_cond_timedwait(
                            &_gpVmDnsLogCtx->pThreadCond,
                            &_gpVmDnsLogCtx->pThreadMutex,
                            &timeout
                            );
        if (_gpVmDnsLogCtx->bThreadShouldExit == TRUE)
        {
            break;
        }

        if (dwError == ETIMEDOUT || dwError == 0)
        {
            DWORD dwLogSizeBytes = 0;

            pthread_mutex_lock(&_gpVmDnsLogCtx->pLogMutex);

            _VmDnsGetFileSize(_gpVmDnsLogCtx->pFile, &dwLogSizeBytes);

            if (_gpVmDnsLogCtx->dwMaxOldLogs > 0 &&
                _gpVmDnsLogCtx->dwMaxLogSizeBytes > 0)
            {
                if (dwLogSizeBytes >= _gpVmDnsLogCtx->dwMaxLogSizeBytes)
                {
                    _VmDnsCloseLog();

                    _VmDnsRotateLogs();
                }
            }
            else if (_gpVmDnsLogCtx->dwMaxOldLogs == 0 &&
                     _gpVmDnsLogCtx->dwMaxLogSizeBytes > 0)
            {
                if (dwLogSizeBytes >= _gpVmDnsLogCtx->dwMaxLogSizeBytes)
                {
                    _VmDnsTruncateLog();
                }
            }

            dwTimeout = HEALTHY_LOG_CHECK_INTERVAL_MSEC;
            // reopen log -- may need to happen if an error happened last time.
            if (_gpVmDnsLogCtx->pFile == NULL)
            {
                dwError = _VmDnsOpenLog();
                if (dwError)
                {
                    dwTimeout = UNHEALTHY_LOG_CHECK_INTERVAL_MSEC;
                }
            }

            pthread_mutex_unlock(&_gpVmDnsLogCtx->pLogMutex);
        }
    } while (1);

    pthread_mutex_unlock(&_gpVmDnsLogCtx->pThreadMutex);

    return NULL;
}

static
PCSTR
_logLevelToTag(
   int level)
{
   switch( level )
   {
      case VMDNS_LOG_LEVEL_INFO:
         return "INFO";
      case VMDNS_LOG_LEVEL_ERROR:
         return "ERROR";
      case VMDNS_LOG_LEVEL_DEBUG:
      default:
         return "DEBUG";
   }
}

static
VOID
_VmDnsLog1(
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

    if ( ! _gpVmDnsLogCtx->bSyslog )
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

        pthread_mutex_lock(&_gpVmDnsLogCtx->pLogMutex);

        if (_gpVmDnsLogCtx->pFile != NULL)
        {
            pLogFile = _gpVmDnsLogCtx->pFile;
        }

        fprintf(pLogFile, "%s%s\n", extraLogMessage, logMessage);

        pthread_mutex_unlock(&_gpVmDnsLogCtx->pLogMutex);
#ifdef WIN32
        OutputDebugStringA(extraLogMessage);
        OutputDebugStringA(logMessage);
        OutputDebugStringA("\n");
#endif
    }

    return;
}

/*
 * Open the log file, either appending or creating anew.
 */
static
DWORD
_VmDnsOpenLog(
    VOID
    )
{
    DWORD dwError = 0;

    _gpVmDnsLogCtx->pFile = fopen(_gpVmDnsLogCtx->pszLogFileName, "a");
    if ( _gpVmDnsLogCtx->pFile == NULL)
    {
       dwError = errno;
       BAIL_ON_VMDNS_LOG_ERROR(dwError);
    }

    setvbuf(_gpVmDnsLogCtx->pFile, NULL, _IONBF, 0);

cleanup:
    return dwError;

error:
    _VmDnsCloseLog();
    goto cleanup;
}

/*
 * Close the log file.
 */
static
VOID
_VmDnsCloseLog(
    VOID
    )
{
    if (_gpVmDnsLogCtx && _gpVmDnsLogCtx->pFile != NULL)
    {
        fclose(_gpVmDnsLogCtx->pFile);
        _gpVmDnsLogCtx->pFile = NULL;
    }
}

/*
 * Rotate logs, deleting highest allowed log file if present.
 */
static
DWORD
_VmDnsRotateLogs(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszLog1FileName = NULL;
    PSTR pszLog2FileName = NULL;
    DWORD dwLogNumber = 0;

    if (_gpVmDnsLogCtx->dwMaxOldLogs == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_LOG_ERROR(dwError);
    }

    for (dwLogNumber = _gpVmDnsLogCtx->dwMaxOldLogs;
         dwLogNumber <= _gpVmDnsLogCtx->dwMaxOldLogs;
         dwLogNumber--)
    {
        dwError = _VmDnsGetRotateLogFilePath(&pszLog1FileName,
                                             _gpVmDnsLogCtx->pszLogFileName,
                                             dwLogNumber);
        BAIL_ON_VMDNS_LOG_ERROR(dwError);

        if (dwLogNumber == _gpVmDnsLogCtx->dwMaxOldLogs)
        {
            // Oldest allowed log may be present, so delete it to make room for next oldest
            DeleteFileA(pszLog1FileName);
        }
        else
        {
            // Move 'vmdir.log.3' to 'vmdir.log.4'
            MoveFileA(pszLog1FileName, pszLog2FileName);
        }

        VMDNS_SAFE_FREE_MEMORY(pszLog2FileName);
        pszLog2FileName = pszLog1FileName;
        pszLog1FileName = NULL;
    }

cleanup:
    VMDNS_SAFE_FREE_MEMORY(pszLog1FileName);
    VMDNS_SAFE_FREE_MEMORY(pszLog2FileName);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDnsGetFileSize(
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
        BAIL_ON_VMDNS_LOG_ERROR(dwError);
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
_VmDnsGetRotateLogFilePath(
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
    dwError = VmDnsAllocateMemory(dwLen * sizeof(char), &pszServerRotateLogFile);
    BAIL_ON_VMDNS_LOG_ERROR(dwError);

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
    VMDNS_SAFE_FREE_MEMORY(pszServerRotateLogFile);

    goto cleanup;
}

static
VOID
_VmDnsTruncateLog(
    VOID
    )
{
    if (_gpVmDnsLogCtx && _gpVmDnsLogCtx->pFile != NULL)
    {
        _VmDnsTruncateFile(_gpVmDnsLogCtx->pFile);
    }
}

/*
 * Reduce file size to zero.
 */
static
BOOLEAN
_VmDnsTruncateFile(
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
