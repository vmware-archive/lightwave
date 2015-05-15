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

static FILE * logFile = NULL;

static const char *
logLevelToTag(
   int level);

static int
logLevelToSysLogLevel(
   int level);

// Returns 0 on success, non-zero on error.
DWORD
VmAuthsvcLogInitialize(
   const char * logFileName)
{
   int   retVal = 0;

   if (logFileName)
   {
      if ((logFile = fopen(logFileName, "a")) == NULL)
      {
         fprintf( stderr, "logFileName: \"%s\" open failed", logFileName);
         retVal = -1;
         goto done;
      }
   }
   if (vmauthsvc_syslog)
   {
      openlog("vmauthsvcd", 0, LOG_DAEMON);
   }

done:
   return retVal;
}

VOID
VmAuthsvcLogTerminate()
{
   if (logFile)
   {
      fclose( logFile );
   }
   if (vmauthsvc_syslog)
   {
      closelog();
   }
}

VOID
VmAuthsvcLog(
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

   if ((level & vmauthsvc_debug) || (level == VMAUTHSVC_DEBUG_ANY))
   {
      va_start( va, fmt );
      vsnprintf( logMessage, sizeof(logMessage), fmt, va );
      logMessage[sizeof(logMessage)-1] = '\0';
      va_end( va );

      if (vmauthsvc_syslog)
      {
         int sysLogLevel = logLevelToSysLogLevel(level);
         snprintf(extraLogMessage, sizeof(extraLogMessage) - 1, "t@%lu: ", (unsigned long) pthread_self());
         syslog(sysLogLevel, "%s%s", extraLogMessage, logMessage);
      }
      else
      {
         clock_gettime(CLOCK_REALTIME, &tspec);
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

         if( logFile != NULL )
         {
            fprintf(logFile, "%s%s\n", extraLogMessage, logMessage);
            fflush( logFile );
         }
         else
         {
            fprintf(stderr, "%s%s\n", extraLogMessage, logMessage);
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
      case VMAUTHSVC_DEBUG_ANY:
         return "ERROR";
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
      case VMAUTHSVC_DEBUG_ANY:
         return LOG_ERR;
      default:
         return LOG_DEBUG;
   }
}

#endif // #ifndef _WIN32
