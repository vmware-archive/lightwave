/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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


#define MAX_LOG_SIZE 10 * 1024 * 1024

// means we will have maximum of 5 logs files for LwCA
#define LOG_GENERATION_MAX 5

#define LWCA_MAX_OLD_LOGS_VALUE_NAME "MaxOldLogs"
#define LWCA_MAX_LOG_SIZE_VALUE_NAME "MaxLogSizeBytes"
#define LWCA_MAX_OLD_LOGS_CAP 64
#define LWCA_MAX_OLD_LOGS_MIN 1
#define LWCA_MAX_LOG_SIZE_MIN 131072

#define LWCA_LOG_STRING "mutentca.log."

#define LWCA_EFAIL -1
#define LWCA_OK 0
#define UNUSED(x) (void)(x)

#define STAT stat
#define UNLINK unlink
#define RENAME rename
#define FORMATSTR "%s/%s%d"
#define LOCAL_TIME localtime_r

#define LWCA_MAX_MSG_SIZE 512

#define STRING_ATOM_EMERGENCY   "\tEmergency\t"
#define STRING_ATOM_ALERT       "\tAlert\t"
#define STRING_ATOM_CRITICAL    "\tCritical\t"
#define STRING_ATOM_ERROR       "\tError\t"
#define STRING_ATOM_WARNING     "\tWarning\t"
#define STRING_ATOM_NOTICE      "\tNotice\t"
#define STRING_ATOM_INFO        "\tInformational\t"
#define STRING_ATOM_DEBUG       "\tDebug\t"



static pthread_rwlock_t  rwloglock = PTHREAD_RWLOCK_INITIALIZER;
static FILE *fpCurrentLog = NULL;
static LWCA_LOG_LEVEL currentLevel = LWCA_LOG_LEVEL_INFO; //default level for now, will change before release
LWCA_LOG_TYPE gLwCALogType = LWCA_LOG_TYPE_FILE;
DWORD gLwCAMaxOldLogs = LOG_GENERATION_MAX;
DWORD gLwCAMaxLogSizeBytes = MAX_LOG_SIZE;
ULONG64 gLwCACurrentLogSizeBytes = 0;

DWORD
_LwCAMkDir( PSTR pszDirName, int mode)
{
    DWORD dwError = 0;
    dwError = mkdir(pszDirName, mode);
    BAIL_ON_LWCA_ERROR(dwError);
error:
    return dwError;
}


DWORD
LwCAGetLogFileDirectory(PSTR *ppszLogFileDir)
{
    DWORD dwError = 0;

    if (ppszLogFileDir == NULL) {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetLogDirectory(ppszLogFileDir);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error :
    goto cleanup;
}


// This function renames logs to rotate.
// for example if max old log is 5, then
// mutentca.log.4 -> mutentca.log.5
// mutentca.log.3 -> mutentca.log.4
// mutentca.log.2 -> mutentca.log.3
// mutentca.log.1 -> mutentca.log.2
// mutentca.log.0 -> mutentca.log.1
// where mutentca.log.1 is the most current log.
DWORD
LwCARenameLogs()
{
    int nCounter = 0;
    PSTR pszSourceFile = NULL;
    PSTR pszDestFile = NULL;
    PSTR pszLogDir = NULL;
    struct STAT st = { 0 };
    DWORD dwError = 0;

    dwError = LwCAGetLogFileDirectory(&pszLogDir);
    BAIL_ON_LWCA_ERROR(dwError);

    if (fpCurrentLog)
    {
        fclose(fpCurrentLog);
        fpCurrentLog = NULL;
    }

    for ( nCounter = (int)gLwCAMaxOldLogs; nCounter > 0 ;
            nCounter --) {

        dwError = LwCAAllocateStringPrintfA(
                      &pszDestFile,
                      FORMATSTR,
                      pszLogDir,
                      LWCA_LOG_STRING,
                      nCounter);

        BAIL_ON_LWCA_ERROR(dwError);

        if(STAT(pszDestFile, &st) == LWCA_OK ) {
            UNLINK(pszDestFile);
        }

        dwError = LwCAAllocateStringPrintfA(
                      &pszSourceFile,
                      FORMATSTR,
                      pszLogDir,
                      LWCA_LOG_STRING,
                      nCounter - 1);

        BAIL_ON_LWCA_ERROR(dwError);

        if (RENAME(pszSourceFile, pszDestFile) != LWCA_OK ) {
            // Emit an Error Event once we have support for that
        }

        LWCA_SAFE_FREE_STRINGA(pszDestFile);
        pszDestFile = NULL;
        LWCA_SAFE_FREE_STRINGA(pszSourceFile);
        pszSourceFile = NULL;
    }

cleanup :
    LWCA_SAFE_FREE_STRINGA(pszLogDir);
    return dwError;
error :
    LWCA_SAFE_FREE_STRINGA(pszDestFile);
    LWCA_SAFE_FREE_STRINGA(pszSourceFile);
    goto cleanup;
}


DWORD
LwCACreatePrimaryLog(PSTR pszLogDir)
{
    DWORD dwError = 0;
    PSTR pszDestFile = NULL;
    if(pszLogDir == NULL) {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringPrintfA(
                  &pszDestFile,
                  FORMATSTR,
                  pszLogDir,
                  LWCA_LOG_STRING,
                  0);// This is always the Current Log File
    BAIL_ON_LWCA_ERROR(dwError);

    if (fpCurrentLog != NULL) {
        fclose(fpCurrentLog);
        fpCurrentLog = NULL;
    }

    fpCurrentLog = fopen(pszDestFile, "a");

    gLwCACurrentLogSizeBytes = 0;

error :
    LWCA_SAFE_FREE_STRINGA(pszDestFile);
    return dwError;
}

DWORD
LwCAInitLog()
{
    DWORD dwError = 0;
    PSTR pszLogDir = NULL;

    pthread_rwlock_wrlock(&rwloglock);

    if (gLwCALogType == LWCA_LOG_TYPE_FILE)
    {
        dwError = LwCAGetLogFileDirectory(&pszLogDir);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCARenameLogs();
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCACreatePrimaryLog(pszLogDir);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    if (gLwCALogType == LWCA_LOG_TYPE_SYSLOG)
    {
        openlog("mutentcad", 0, LOG_DAEMON);
    }

error :
    pthread_rwlock_unlock(&rwloglock);
    LWCA_SAFE_FREE_STRINGA(pszLogDir);

    return dwError;

}

// It is ok to crash since the level is hard
// coded in the messages. Not checking the
// Range since we will send in an ENUM.
PSTR LwCALevelToText(LWCA_LOG_LEVEL level)
{
    PSTR Messages[]= {
        STRING_ATOM_EMERGENCY,
        STRING_ATOM_ALERT,
        STRING_ATOM_CRITICAL,
        STRING_ATOM_ERROR,
        STRING_ATOM_WARNING,
        STRING_ATOM_NOTICE,
        STRING_ATOM_INFO,
        STRING_ATOM_DEBUG
    };
    return Messages[(int)level];
}

int LwCALevelToLen(LWCA_LOG_LEVEL level)
{
    int MessageSize[] ={
        sizeof(STRING_ATOM_EMERGENCY),
        sizeof(STRING_ATOM_ALERT),
        sizeof(STRING_ATOM_CRITICAL),
        sizeof(STRING_ATOM_ERROR),
        sizeof(STRING_ATOM_WARNING),
        sizeof(STRING_ATOM_NOTICE),
        sizeof(STRING_ATOM_INFO),
        sizeof(STRING_ATOM_DEBUG)
    };
    return MessageSize[(int)level];
}

static
int LwCALevelToSysLogLevel(LWCA_LOG_LEVEL level)
{
    if (LWCA_LOG_LEVEL_ERROR >= level)
        return LOG_ERR;
    else
    {
      switch (level)
      {
          case LWCA_LOG_LEVEL_WARNING:
            return LOG_WARNING;
          case LWCA_LOG_LEVEL_NOTICE:
            return LOG_NOTICE;
          case LWCA_LOG_LEVEL_INFO:
            return LOG_INFO;
          case LWCA_LOG_LEVEL_DEBUG:
          default:
            return LOG_DEBUG;
      }
    }

}

DWORD
LwCARotateLogs()
{
    DWORD dwError = 0;
    PSTR pszLogDir = NULL;

    dwError = LwCAGetLogFileDirectory(&pszLogDir);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARenameLogs();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreatePrimaryLog(pszLogDir);
    BAIL_ON_LWCA_ERROR(dwError);

error :
    LWCA_SAFE_FREE_STRINGA(pszLogDir);
    return dwError;

}

LWCA_LOG_LEVEL
LwCALogGetLevel(
)
{
    LWCA_LOG_LEVEL leveltmp;
    pthread_rwlock_rdlock(&rwloglock);
    leveltmp =  currentLevel;
    pthread_rwlock_unlock(&rwloglock);
    return leveltmp;
}

void
LwCALogSetLevel(
    LWCA_LOG_LEVEL level
)
{
    pthread_rwlock_wrlock(&rwloglock);
    currentLevel = level;
    pthread_rwlock_unlock(&rwloglock);
}

void
LwCALog(
    LWCA_LOG_LEVEL level,
    const char*      fmt,
    ...)
{

#define LEVEL_LEN 13
    char logMessage[LWCA_MAX_MSG_SIZE];
    char extraLogMessage[LWCA_MAX_MSG_SIZE] = {0};
    int msgsize;
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    PSTR szTimeString = NULL;
    va_list va;

    // Record time as soon as possible
    if ( (int)currentLevel < (int)level) {
        //Nothing to do
        goto cleanup ;
    }

    // Look like syslogd time stamp
    va_start( va, fmt );
    msgsize = vsnprintf(logMessage, LWCA_MAX_MSG_SIZE, fmt, va );
    if ((msgsize > 0) && (msgsize < LWCA_MAX_MSG_SIZE-2)) {
        logMessage[msgsize++] = '\n';
        logMessage[msgsize] = '\0';
    } else {
        logMessage[LWCA_MAX_MSG_SIZE-1] = '\0';
    }

    va_end( va );

    if (gLwCALogType == LWCA_LOG_TYPE_SYSLOG)
    {
        int sysLogLevel = LwCALevelToSysLogLevel(level);

        snprintf(
            extraLogMessage,
            sizeof(extraLogMessage) - 1,
            "t@%lu: ",
            (unsigned long) pthread_self());

        syslog(sysLogLevel, "%s%s", extraLogMessage, logMessage);
    }
    if (gLwCALogType == LWCA_LOG_TYPE_FILE && fpCurrentLog)
    {

        dwError = LwCAGetUTCTimeString(&szTimeString);
        BAIL_ON_LWCA_ERROR_NO_LOG(dwError);

        pthread_rwlock_wrlock(&rwloglock);
        bLocked = TRUE;

        fwrite((PVOID)szTimeString, strlen(szTimeString), 1, fpCurrentLog);
        fwrite((PVOID)LwCALevelToText(level), LwCALevelToLen(level) - 1 , 1, fpCurrentLog);
        fwrite((PVOID)logMessage, msgsize, 1, fpCurrentLog);
        fflush(fpCurrentLog);

        gLwCACurrentLogSizeBytes += strlen(szTimeString) + LwCALevelToLen(level) - 1 + msgsize;

        if (gLwCACurrentLogSizeBytes >= gLwCAMaxLogSizeBytes)
        {
            dwError = LwCARotateLogs();
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }
    if (gLwCALogType == LWCA_LOG_TYPE_CONSOLE)
    {
        unsigned long ulThreadId = -1;
        ulThreadId = (unsigned long) pthread_self();
        fprintf(stderr, "LwCA:t@%lu:%-3.7s: %s\n",
                ulThreadId,
                LwCALevelToText(level),
                logMessage);
        fflush( stderr );
    }

cleanup:
    if (bLocked)
    {
        pthread_rwlock_unlock(&rwloglock);
    }

    LWCA_SAFE_FREE_STRINGA(szTimeString);
    return ;
error :
    goto cleanup;
}

VOID
LwCATerminateLogging()
{
    if (fpCurrentLog) {
        fclose(fpCurrentLog);
        fpCurrentLog = NULL;
    }

    if (gLwCALogType == LWCA_LOG_TYPE_SYSLOG)
    {
        closelog();
    }

    gLwCALogType = LWCA_LOG_TYPE_CONSOLE;
}
