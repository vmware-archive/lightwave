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


#define MAX_LOG_SIZE 10 * 1024 * 1024

// means we will have maximum of 5 logs files for VMCA
#define LOG_GENERATION_MAX 5

#define VMCA_MAX_OLD_LOGS_VALUE_NAME "MaxOldLogs"
#define VMCA_MAX_LOG_SIZE_VALUE_NAME "MaxLogSizeBytes"
#define VMCA_MAX_OLD_LOGS_CAP 64
#define VMCA_MAX_OLD_LOGS_MIN 1
#define VMCA_MAX_LOG_SIZE_MIN 131072

#define VMCA_LOG_STRING "vmca.log."

#define VMCA_EFAIL -1
#define VMCA_OK 0
#define UNUSED(x) (void)(x)

#ifdef _WIN32
#define STAT _stat
#define UNLINK _unlink
#define FORMATSTR "%s\\%s%d"
#define RENAME rename
#define LOCAL_TIME localtime
#else
#define STAT stat
#define UNLINK unlink
#define RENAME rename
#define FORMATSTR "%s/%s%d"
#define LOCAL_TIME localtime_r
#endif

#define VMCA_MAX_MSG_SIZE 512

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
static VMCA_LOG_LEVEL currentLevel = VMCA_LOG_LEVEL_INFO; //default level for now, will change before release
VMCA_LOG_TYPE gVMCALogType = VMCA_LOG_TYPE_FILE;
DWORD gVMCAMaxOldLogs = LOG_GENERATION_MAX;
DWORD gVMCAMaxLogSizeBytes = MAX_LOG_SIZE;
ULONG64 gVMCACurrentLogSizeBytes = 0;

static
VOID
VMCAReadLogConfig();

DWORD
_VMCAMkDir( PSTR pszDirName, int mode)
{
    DWORD dwError = 0;
#ifdef _WIN32
    UNUSED(mode);
    dwError = _mkdir(pszDirName);
    BAIL_ON_VMCA_ERROR(dwError);
#else
    dwError = mkdir(pszDirName, mode);
    BAIL_ON_VMCA_ERROR(dwError);
#endif
error:
    return dwError;
}


DWORD
VMCAGetLogFileDirectory(PSTR *ppszLogFileDir)
{
    DWORD dwError = 0;

    if (ppszLogFileDir == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetLogDirectory(ppszLogFileDir);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error :
    goto cleanup;
}


// This function renames logs to rotate.
// for example if max old log is 5, then
// vmca.log.4 -> vmca.log.5
// vmca.log.3 -> vmca.log.4
// vmca.log.2 -> vmca.log.3
// vmca.log.1 -> vmca.log.2
// vmca.log.0 -> vmca.log.1
// where vmca.log.1 is the most current log.
DWORD
VMCARenameLogs()
{
    int nCounter = 0;
    PSTR pszSourceFile = NULL;
    PSTR pszDestFile = NULL;
    PSTR pszLogDir = NULL;
    struct STAT st = { 0 };
    DWORD dwError = 0;

    dwError = VMCAGetLogFileDirectory(&pszLogDir);
    BAIL_ON_VMCA_ERROR(dwError);

    if (fpCurrentLog)
    {
        fclose(fpCurrentLog);
        fpCurrentLog = NULL;
    }

    for ( nCounter = (int)gVMCAMaxOldLogs; nCounter > 0 ;
            nCounter --) {

        dwError = VMCAAllocateStringPrintfA(
                      &pszDestFile,
                      FORMATSTR,
                      pszLogDir,
                      VMCA_LOG_STRING,
                      nCounter);

        BAIL_ON_VMCA_ERROR(dwError);

        if(STAT(pszDestFile, &st) == VMCA_OK ) {
            UNLINK(pszDestFile);
        }

        dwError = VMCAAllocateStringPrintfA(
                      &pszSourceFile,
                      FORMATSTR,
                      pszLogDir,
                      VMCA_LOG_STRING,
                      nCounter - 1);

        BAIL_ON_VMCA_ERROR(dwError);

        if (RENAME(pszSourceFile, pszDestFile) != VMCA_OK ) {
            // Emit an Error Event once we have support for that
        }

        VMCA_SAFE_FREE_STRINGA(pszDestFile);
        pszDestFile = NULL;
        VMCA_SAFE_FREE_STRINGA(pszSourceFile);
        pszSourceFile = NULL;
    }

cleanup :
    VMCA_SAFE_FREE_STRINGA(pszLogDir);
    return dwError;
error :
    VMCA_SAFE_FREE_STRINGA(pszDestFile);
    VMCA_SAFE_FREE_STRINGA(pszSourceFile);
    goto cleanup;
}


DWORD
VMCACreatePrimaryLog(PSTR pszLogDir)
{
    DWORD dwError = 0;
    PSTR pszDestFile = NULL;
    if(pszLogDir == NULL) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateStringPrintfA(
                  &pszDestFile,
                  FORMATSTR,
                  pszLogDir,
                  VMCA_LOG_STRING,
                  0);// This is always the Current Log File
    BAIL_ON_VMCA_ERROR(dwError);

    if (fpCurrentLog != NULL) {
        fclose(fpCurrentLog);
        fpCurrentLog = NULL;
    }

    fpCurrentLog = fopen(pszDestFile, "a");

    gVMCACurrentLogSizeBytes = 0;

error :
    VMCA_SAFE_FREE_STRINGA(pszDestFile);
    return dwError;
}

DWORD
VMCAInitLog()
{
    DWORD dwError = 0;
    PSTR pszLogDir = NULL;

    pthread_rwlock_wrlock(&rwloglock);

    VMCAReadLogConfig();

    if (gVMCALogType == VMCA_LOG_TYPE_FILE)
    {
        dwError = VMCAGetLogFileDirectory(&pszLogDir);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCARenameLogs();
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCACreatePrimaryLog(pszLogDir);
        BAIL_ON_VMCA_ERROR(dwError);
    }
#ifndef WIN32
    if (gVMCALogType == VMCA_LOG_TYPE_SYSLOG)
    {
        openlog("vmcad", 0, LOG_DAEMON);
    }
#endif

error :
    pthread_rwlock_unlock(&rwloglock);
    VMCA_SAFE_FREE_STRINGA(pszLogDir);

    return dwError;

}

// It is ok to crash since the level is hard
// coded in the messages. Not checking the
// Range since we will send in an ENUM.
PSTR VMCALevelToText(VMCA_LOG_LEVEL level)
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

int VMCALevelToLen(VMCA_LOG_LEVEL level)
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

#ifndef WIN32
static
int VMCALevelToSysLogLevel(VMCA_LOG_LEVEL level)
{
    if (VMCA_LOG_LEVEL_ERROR >= level)
        return LOG_ERR;
    else
    {
      switch (level)
      {
          case VMCA_LOG_LEVEL_WARNING:
            return LOG_WARNING;
          case VMCA_LOG_LEVEL_NOTICE:
            return LOG_NOTICE;
          case VMCA_LOG_LEVEL_INFO:
            return LOG_INFO;
          case VMCA_LOG_LEVEL_DEBUG:
          default:
            return LOG_DEBUG;
      }
    }

}
#endif

DWORD
VMCARotateLogs()
{
    DWORD dwError = 0;
    PSTR pszLogDir = NULL;

    dwError = VMCAGetLogFileDirectory(&pszLogDir);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARenameLogs();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCACreatePrimaryLog(pszLogDir);
    BAIL_ON_VMCA_ERROR(dwError);

error :
    VMCA_SAFE_FREE_STRINGA(pszLogDir);
    return dwError;

}

VMCA_LOG_LEVEL
VMCALogGetLevel(
)
{
    VMCA_LOG_LEVEL leveltmp;
    pthread_rwlock_rdlock(&rwloglock);
    leveltmp =  currentLevel;
    pthread_rwlock_unlock(&rwloglock);
    return leveltmp;
}

void
VMCALogSetLevel(
    VMCA_LOG_LEVEL level
)
{
    pthread_rwlock_wrlock(&rwloglock);
    currentLevel = level;
    pthread_rwlock_unlock(&rwloglock);
}

void
VMCALog(
    VMCA_LOG_LEVEL level,
    const char*      fmt,
    ...)
{

#define LEVEL_LEN 13
    char logMessage[VMCA_MAX_MSG_SIZE];
    char extraLogMessage[VMCA_MAX_MSG_SIZE] = {0};
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
    msgsize = vsnprintf(logMessage, VMCA_MAX_MSG_SIZE, fmt, va );
    if ((msgsize > 0) && (msgsize < VMCA_MAX_MSG_SIZE-2)) {
        logMessage[msgsize++] = '\n';
        logMessage[msgsize] = '\0';
    } else {
        logMessage[VMCA_MAX_MSG_SIZE-1] = '\0';
    }

    va_end( va );

#ifndef WIN32
    if (gVMCALogType == VMCA_LOG_TYPE_SYSLOG)
    {
        int sysLogLevel = VMCALevelToSysLogLevel(level);

        snprintf(
            extraLogMessage,
            sizeof(extraLogMessage) - 1,
            "t@%lu: ",
            (unsigned long) pthread_self());

        syslog(sysLogLevel, "%s%s", extraLogMessage, logMessage);
    }
#endif
    if (gVMCALogType == VMCA_LOG_TYPE_FILE && fpCurrentLog)
    {

        dwError = VMCAGetUTCTimeString(&szTimeString);
        BAIL_ON_VMCA_ERROR_NO_LOG(dwError);

        pthread_rwlock_wrlock(&rwloglock);
        bLocked = TRUE;

        fwrite((PVOID)szTimeString, strlen(szTimeString), 1, fpCurrentLog);
        fwrite((PVOID)VMCALevelToText(level), VMCALevelToLen(level) - 1 , 1, fpCurrentLog);
        fwrite((PVOID)logMessage, msgsize, 1, fpCurrentLog);
        fflush(fpCurrentLog);

        gVMCACurrentLogSizeBytes += strlen(szTimeString) + VMCALevelToLen(level) - 1 + msgsize;

        if (gVMCACurrentLogSizeBytes >= gVMCAMaxLogSizeBytes)
        {
            dwError = VMCARotateLogs();
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }
    if (gVMCALogType == VMCA_LOG_TYPE_CONSOLE)
    {
        unsigned long ulThreadId = -1;
#ifdef _WIN32
        ulThreadId = (unsigned long) pthread_self().p;
#else
        ulThreadId = (unsigned long) pthread_self();
#endif
        fprintf(stderr, "VMCA:t@%lu:%-3.7s: %s\n",
                ulThreadId,
                VMCALevelToText(level),
                logMessage);
        fflush( stderr );
    }

cleanup:
    if (bLocked)
    {
        pthread_rwlock_unlock(&rwloglock);
    }

    VMCA_SAFE_FREE_STRINGA(szTimeString);
    return ;
error :
    goto cleanup;
}

VOID
VMCATerminateLogging()
{
    if (fpCurrentLog) {
        fclose(fpCurrentLog);
        fpCurrentLog = NULL;
    }

#ifndef WIN32
    if (gVMCALogType == VMCA_LOG_TYPE_SYSLOG)
    {
        closelog();
    }
#endif

    gVMCALogType = VMCA_LOG_TYPE_CONSOLE;
}

static
VOID
VMCAReadLogConfig()
{
    DWORD dwError = 0;
    dwError = VMCAConfigGetDword(VMCA_MAX_OLD_LOGS_VALUE_NAME, &gVMCAMaxOldLogs);
    if (dwError != 0)
    {
        gVMCAMaxOldLogs = LOG_GENERATION_MAX;
        dwError = 0;
    }
    else if (gVMCAMaxOldLogs > VMCA_MAX_OLD_LOGS_CAP)
    {
        gVMCAMaxOldLogs = VMCA_MAX_OLD_LOGS_CAP;
    }
    else if (gVMCAMaxOldLogs < VMCA_MAX_OLD_LOGS_MIN)
    {
        gVMCAMaxOldLogs = VMCA_MAX_OLD_LOGS_MIN;
    }

    dwError = VMCAConfigGetDword(VMCA_MAX_LOG_SIZE_VALUE_NAME, &gVMCAMaxLogSizeBytes);
    if (dwError != 0)
    {
        gVMCAMaxLogSizeBytes = MAX_LOG_SIZE;
        dwError = 0;
    }
    else if (gVMCAMaxLogSizeBytes < VMCA_MAX_LOG_SIZE_MIN)
    {
        gVMCAMaxLogSizeBytes = VMCA_MAX_LOG_SIZE_MIN;
    }
}
