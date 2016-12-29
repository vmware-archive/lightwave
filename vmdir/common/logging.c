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



#include "includes.h"

#define BAIL_ON_VMDIR_LOG_ERROR(dwError)    \
    if (dwError) { goto error; }

#define NSECS_PER_MSEC        1000000
#define EXTRA_LOG_MESSAGE_LEN 128

static PVMDIR_LOG_CTX   _gpVmDirLogCtx = NULL;

static
int
_logLevelToSysLogLevel(
    VMDIR_LOG_LEVEL iLevel
    );

static
PCSTR
_logLevelToTag(
    VMDIR_LOG_LEVEL iLevel
    );

static
void
_VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    PCSTR           fmt,
    va_list         args
    );

void
VmDirLogSetLevel(
    VMDIR_LOG_LEVEL iNewLogLevel
    )
{
   if ( _gpVmDirLogCtx)
   {
       _gpVmDirLogCtx->iLogLevel = iNewLogLevel;

       if (_gpVmDirLogCtx->bSyslog )
       {
           setlogmask( LOG_UPTO(_logLevelToSysLogLevel( iNewLogLevel )) );
       }
   }
}

VMDIR_LOG_LEVEL
VmDirLogGetLevel(
    VOID
    )
{
    return _gpVmDirLogCtx ? _gpVmDirLogCtx->iLogLevel : VMDIR_LOG_INFO;
}

void
VmDirLogSetMask(
    ULONG             iNewLogMask
    )
{
    if (_gpVmDirLogCtx)
    {
        _gpVmDirLogCtx->iLogMask = iNewLogMask;
    }
}

ULONG
VmDirLogGetMask(
    VOID
    )
{
    return _gpVmDirLogCtx ? _gpVmDirLogCtx->iLogMask : 0;
}

VOID
VmDirLogTerminate(
    VOID
    )
{
    if ( _gpVmDirLogCtx )
    {
        if ( _gpVmDirLogCtx->pFile )
        {
            fclose( _gpVmDirLogCtx->pFile );
        }

        if ( _gpVmDirLogCtx->bSyslog )
        {
            if ( _gpVmDirLogCtx->pszSyslogDaemon )
            {
                closelog();
                VMDIR_SAFE_FREE_MEMORY( _gpVmDirLogCtx->pszSyslogDaemon);
            }
        }

        VMDIR_SAFE_FREE_MEMORY( _gpVmDirLogCtx->pszLogFileName );
        VMDIR_SAFE_FREE_MEMORY( _gpVmDirLogCtx );
    }

    return;

}

DWORD
VmDirLogInitialize(
    PCSTR            pszLogFileName,
    BOOLEAN          bUseSysLog,
    PCSTR            pszSyslogName,
    VMDIR_LOG_LEVEL  iInitLogLevel,
    ULONG            iInitLogMask
   )
{
   DWORD            dwError = 0;
   BOOLEAN          bLogInitFailed = TRUE;

   if ( _gpVmDirLogCtx )
   {
       bLogInitFailed = FALSE;
       dwError = VMDIR_ERROR_INVALID_PARAMETER;
       BAIL_ON_VMDIR_LOG_ERROR(dwError);
   }

   dwError = VmDirAllocateMemory( sizeof(*_gpVmDirLogCtx), (PVOID*)&_gpVmDirLogCtx);
   BAIL_ON_VMDIR_LOG_ERROR(dwError);

   if (pszLogFileName)
   {
       dwError = VmDirAllocateStringA(pszLogFileName, &_gpVmDirLogCtx->pszLogFileName);
       BAIL_ON_VMDIR_LOG_ERROR(dwError);

       if ((_gpVmDirLogCtx->pFile = fopen(_gpVmDirLogCtx->pszLogFileName, "a")) == NULL)
       {
           dwError = VMDIR_ERROR_IO;
           BAIL_ON_VMDIR_LOG_ERROR(dwError);
       }
   }

   if (bUseSysLog)
   {
       _gpVmDirLogCtx->bSyslog = bUseSysLog;

       if ( pszSyslogName ) // vmdirclient case, this is NULL.
       {
           dwError = VmDirAllocateStringA( pszSyslogName, &_gpVmDirLogCtx->pszSyslogDaemon);
           BAIL_ON_VMDIR_LOG_ERROR(dwError);
           openlog(_gpVmDirLogCtx->pszSyslogDaemon, 0, LOG_DAEMON);
           setlogmask( LOG_UPTO(_logLevelToSysLogLevel( iInitLogLevel )) );
       }
   }
   else
   {
       setlogmask( LOG_UPTO(LOG_ERR) );
   }

   _gpVmDirLogCtx->iLogLevel = iInitLogLevel;
   _gpVmDirLogCtx->iLogMask  = iInitLogMask;

cleanup:

   return dwError;

error:

    if (bLogInitFailed)
    {
        VmDirLogTerminate();
    }

    goto cleanup;
}



void
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
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
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

void
VmDirLog(
   ULONG        iMask,
   const char*  fmt,
   ...)
{
    DWORD           dwError = 0;
    va_list         va;
    VMDIR_LOG_LEVEL iLevel = VMDIR_LOG_ERROR;  //BUGBUG, should be INFO, but for B/C
                                               // until we convert old VmDirLog

   if ( _gpVmDirLogCtx == NULL )
   {
       dwError = VMDIR_ERROR_INVALID_PARAMETER;
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
int
_logLevelToSysLogLevel(
    VMDIR_LOG_LEVEL iLevel
    )
{
    switch( iLevel )
    {
       case VMDIR_LOG_ERROR:
           return LOG_ERR;
       case VMDIR_LOG_WARNING:
           return LOG_WARNING;
       case VMDIR_LOG_INFO:
           return LOG_INFO;
       case VMDIR_LOG_VERBOSE:
           return LOG_DEBUG;
       case VMDIR_LOG_DEBUG:
           return LOG_DEBUG;
       default:
          return LOG_INFO;
    }
}

static
void
_VmDirLog1(
    VMDIR_LOG_LEVEL iLevel,
    PCSTR           fmt,
    va_list         args
    )
{
    char        extraLogMessage[EXTRA_LOG_MESSAGE_LEN] = {0};
    char        logMessage[MAX_LOG_MESSAGE_LEN];
    const char* logLevelTag = "";
    struct      timespec tspec = {0};
    struct      tm mytm = {0};

    vsnprintf( logMessage, sizeof(logMessage), fmt, args );
    logMessage[sizeof(logMessage)-1] = '\0';

    if ( _gpVmDirLogCtx->bSyslog )
    {
        int sysLogLevel = _logLevelToSysLogLevel(iLevel);

        snprintf( extraLogMessage, sizeof(extraLogMessage) - 1, "t@%lu: ",
                  (unsigned long) pthread_self());
        syslog( sysLogLevel, "%s%s", extraLogMessage, logMessage);
    }
    else
    {
        logLevelTag = _logLevelToTag(iLevel);

#ifndef __MACH__
	clock_gettime(CLOCK_REALTIME, &tspec);
#endif
	gmtime_r(&tspec.tv_sec, &mytm);
        snprintf(extraLogMessage, sizeof(extraLogMessage) - 1,
                "%4d-%02d-%02dT%02d:%02d:%02d.%03ldZ:t@%lu:%-3.7s: ",
                 mytm.tm_year+1900,
                 mytm.tm_mon+1,
                 mytm.tm_mday,
                 mytm.tm_hour,
                 mytm.tm_min,
                 mytm.tm_sec,
                 tspec.tv_nsec/NSECS_PER_MSEC,
                 (unsigned long) pthread_self(),
                 logLevelTag? logLevelTag : "UNKNOWN");

        if ( _gpVmDirLogCtx->pFile != NULL )
        {
            fprintf(_gpVmDirLogCtx->pFile, "%s%s\n", extraLogMessage, logMessage);
            fflush( _gpVmDirLogCtx->pFile );
        }
        else
        {
            logLevelTag = _logLevelToTag(iLevel);
            fprintf(stderr, "VMDIR:t@%lu:%-3.7s: %s\n",
                    (unsigned long) pthread_self(),
                    logLevelTag? logLevelTag : "UNKNOWN",
                    logMessage);
            fflush( stderr );
        }
    }

    return;
}
