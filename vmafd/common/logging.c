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

#ifndef _WIN32

#define NSECS_PER_MSEC        1000000
#define EXTRA_LOG_MESSAGE_LEN 128

#define BAIL_ON_VMAFD_LOG_ERROR(dwError) \
    if (dwError) { goto error; }

static const char *
logLevelToTag(
   int level);

static int
logLevelToSysLogLevel(
   int level);

static
VOID
_VmAfdSetLogLevel(
    );

static
DWORD
_VmAfdCreateDirs(
    PCSTR path)
{
    DWORD dwError = 0;
    PSTR dir = NULL;
    PSTR p = NULL;
    int err = 0;

    if (!path || !*path || path[0] != '/')
    {
        dwError = ERROR_NO_SUCH_FILE_OR_DIRECTORY;
        BAIL_ON_VMAFD_LOG_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(path, &dir);
    BAIL_ON_VMAFD_LOG_ERROR(dwError);

    p = dir + 1;
    while ( (p = strchr(p, '/')) )
    {
        *p = '\0';
        err = mkdir(dir, 0755);
        if (err == -1 && errno != EEXIST)
        {
            dwError = ERROR_NO_SUCH_FILE_OR_DIRECTORY;
            BAIL_ON_VMAFD_LOG_ERROR(dwError);
        }
        *p++ = '/';
    }

error:

    VMAFD_SAFE_FREE_STRINGA(dir);

    return 0;
}

DWORD
VmAfdLogInitialize(
    PCSTR pszLogFileName,
    DWORD dwMaximumOldFiles,
    INT64 i64MaxLogSizeBytes)
{
    DWORD dwError = 0;

    if (pszLogFileName)
    {
        dwError = _VmAfdCreateDirs(pszLogFileName);
        BAIL_ON_VMAFD_LOG_ERROR(dwError);

        dwError = VmAfdOpenFilePath(pszLogFileName, "a", &gVmafdLogFile, 0644);
        BAIL_ON_VMAFD_LOG_ERROR(dwError);
    }
    if (vmafd_syslog)
    {
        openlog("vmafdd", 0, LOG_DAEMON);
    }
    _VmAfdSetLogLevel();

error:
    return dwError;
}

void
VmAfdLogTerminate()
{
   if (gVmafdLogFile)
   {
      fclose(gVmafdLogFile);
      gVmafdLogFile = NULL;
   }
   if (vmafd_syslog)
   {
      closelog();
      vmafd_syslog = 0;
   }
}

void
VmAfdLog(
   int level,
   const char*      fmt,
   ...)
{
   char        extraLogMessage[EXTRA_LOG_MESSAGE_LEN] = {0};
   struct      timespec tspec = {0};
   struct      tm mytm = {0};
   char        logMessage[MAX_LOG_MESSAGE_LEN];
   va_list     va;
   const char* logLevelTag = "";

   if (level >= vmafd_syslog_level)
   {
      va_start( va, fmt );
      vsnprintf( logMessage, sizeof(logMessage), fmt, va );
      logMessage[sizeof(logMessage)-1] = '\0';
      va_end( va );

      if (vmafd_syslog)
      {
         int sysLogLevel = logLevelToSysLogLevel(level);
         snprintf(extraLogMessage, sizeof(extraLogMessage) - 1, "t@%lu: ", (unsigned long) pthread_self());
         syslog(sysLogLevel, "%s%s", extraLogMessage, logMessage);
      }
      else
      {
#ifndef __MACH__
         clock_gettime(CLOCK_REALTIME, &tspec);
#endif
         gmtime_r(&tspec.tv_sec, &mytm);
         logLevelTag = logLevelToTag(level);
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

         if( gVmafdLogFile != NULL )
         {
            fprintf(gVmafdLogFile, "%s%s\n", extraLogMessage, logMessage);
            fflush( gVmafdLogFile );
         }
      }
      if (vmafd_console_log)
      {
          logLevelTag = logLevelToTag(level);
          fprintf(stderr, "VMAFD:t@%lu:%-3.7s: %s\n",
                  (unsigned long) pthread_self(),
                  logLevelTag? logLevelTag : "UNKNOWN",
                  logMessage);
          fflush( stderr );
      }
   }
}

static const char *
logLevelToTag(
   int level)
{
   switch( level )
   {
      case VMAFD_DEBUG_ANY:
         return "INFO" ;
      case VMAFD_DEBUG_TRACE:
         return "TRACE";
      case VMAFD_DEBUG_ERROR:
         return "ERROR";
      case VMAFD_DEBUG_WARNING:
         return "WARNING";
      case VMAFD_DEBUG_DEBUG:
         return "DEBUG";
      case VMAFD_DEBUG_INFO:
         return "INFO";
      default:
         return "UKNOWN";
   }
}

static int
logLevelToSysLogLevel(
   int level)
{
   switch( level )
   {
      case VMAFD_DEBUG_ANY:
         return LOG_INFO;
      case VMAFD_DEBUG_TRACE:
         return LOG_NOTICE;
      case VMAFD_DEBUG_ERROR:
         return LOG_ERR;
      case VMAFD_DEBUG_WARNING:
         return LOG_WARNING;
      case VMAFD_DEBUG_DEBUG:
         return LOG_DEBUG;
      case VMAFD_DEBUG_INFO:
         return LOG_INFO;
      default:
         return LOG_ERR;
   }
}

static
VOID
_VmAfdSetLogLevel(
    )
{
    if (vmafd_syslog_level)
    {
        setlogmask(LOG_UPTO(logLevelToSysLogLevel(vmafd_syslog_level)));
    }
    else
    {
        setlogmask(LOG_UPTO(LOG_ERR));
        vmafd_syslog_level = VMAFD_DEBUG_ERROR;
    }
}

#endif // #ifndef _WIN32
