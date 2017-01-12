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

extern int  vmdns_syslog_level;

#ifndef _WIN32

#define NSECS_PER_MSEC        1000000
#define EXTRA_LOG_MESSAGE_LEN 128

static FILE * logFile = NULL;

static const char *
logLevelToTag(
   int level);

static int
logLevelToSysLogLevel(
   int level);

// Returns 0 on success, non-zero on error.
DWORD
VmDnsLogInitialize(
    PCSTR pszLogFileName,
    DWORD dwMaximumOldFiles,
	DWORD   dwMaxLogSizeBytes)
{
   DWORD   retVal = 0;

   if (pszLogFileName)
   {
      if ((logFile = fopen(pszLogFileName, "a")) == NULL)
      {
         fprintf( stderr, "logFileName: \"%s\" open failed", pszLogFileName);
         retVal = LwErrnoToWin32Error(errno);
         goto done;
      }
   }
   if (vmdns_syslog)
   {
      openlog("vmdnsd", 0, LOG_DAEMON);
   }

done:
   return retVal;
}

void
VmDnsLogTerminate()
{
   if (logFile)
   {
      fclose( logFile );
   }
   if (vmdns_syslog)
   {
      closelog();
   }
}

void
VmDnsLog(
   VMDNS_LOG_LEVEL  level,
   const char*      fmt,
   ...)
{
   char        extraLogMessage[EXTRA_LOG_MESSAGE_LEN] = {0};
   struct      timespec tspec = {0};
   time_t      ltime;
   struct      tm mytm = {0};
   char        logMessage[MAX_LOG_MESSAGE_LEN];
   va_list     va;
   const char* logLevelTag = "";

   if (level <= vmdns_syslog_level)
   {
      va_start( va, fmt );
      vsnprintf( logMessage, sizeof(logMessage), fmt, va );
      logMessage[sizeof(logMessage)-1] = '\0';
      va_end( va );

      if (vmdns_syslog)
      {
         int sysLogLevel = logLevelToSysLogLevel(level);
         snprintf(extraLogMessage, sizeof(extraLogMessage) - 1, "t@%lu: ", (unsigned long) pthread_self());
         syslog(sysLogLevel, "%s%s", extraLogMessage, logMessage);
      }
      else
      {
         ltime = time(&ltime);
         localtime_r(&ltime, &mytm);
         logLevelTag = logLevelToTag(level);
         snprintf(extraLogMessage, sizeof(extraLogMessage) - 1,
                  "%4d%2d%2d%2d%2d%2d.%03ld:t@%lu:%-3.7s: ",
                  mytm.tm_year+1900,
                  mytm.tm_mon+1,
                  mytm.tm_mday,
                  mytm.tm_hour,
                  mytm.tm_min,
                  mytm.tm_sec,
                  tspec.tv_nsec/NSECS_PER_MSEC,
                  (unsigned long) pthread_self(),
                  logLevelTag? logLevelTag : "UNKNOWN");

         if( logFile != NULL )
         {
            fprintf(logFile, "%s%s\n", extraLogMessage, logMessage);
            fflush( logFile );
         }
         else
         {
             logLevelTag = logLevelToTag(level);
             fprintf(stderr, "VMDNS:t@%lu:%-3.7s: %s\n",
                     (unsigned long) pthread_self(),
                     logLevelTag? logLevelTag : "UNKNOWN",
                     logMessage);
            fflush( stderr );
         }
      }
   }
}

static const char *
logLevelToTag(
   int level)
{
   switch( level )
   {
       case VMDNS_LOG_LEVEL_ERROR:
            return "ERROR";
       case VMDNS_LOG_LEVEL_WARNING:
            return "WARNING";
       case VMDNS_LOG_LEVEL_INFO:
            return "INFO";
       case VMDNS_LOG_LEVEL_DEBUG:
            return "DEBUG";
      default:
            return "DEBUG";
   }
}

static int
logLevelToSysLogLevel(
   int level)
{
   switch( level )
   {
      case VMDNS_LOG_LEVEL_ERROR:
         return LOG_ERR;
      default:
         return LOG_DEBUG;
   }
}

#endif // #ifndef _WIN32
