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

static
PVMW_DEPLOY_LOG_CONTEXT
VmwDeployAcquireLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    );

static
VOID
VmwDeployLogToFile(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL    logLevel,
    PCSTR                   pszFormat,
    va_list                 args
    );

static
VOID
VmwDeployLogToSyslog(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL    logLevel,
    PCSTR                   pszFormat,
    va_list                 args
    );

static
PCSTR
VmwDeployLogLevelToTag(
    VMW_DEPLOY_LOG_LEVEL logLevel
    );

static
VOID
VmwDeployFreeLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    );

DWORD
VmwDeployCreateLogContext(
    VMW_DEPLOY_LOG_TARGET    logTarget,
    VMW_DEPLOY_LOG_LEVEL     logLevel,
    PCSTR                    pszFilePath,
    PVMW_DEPLOY_LOG_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PVMW_DEPLOY_LOG_CONTEXT pContext = NULL;

    dwError = VmwDeployAllocateMemory(
                sizeof(VMW_DEPLOY_LOG_CONTEXT),
                (PVOID*)&pContext);
    BAIL_ON_DEPLOY_ERROR(dwError);

    pContext->refcount = 1;

    pContext->logLevel  = logLevel;
    pContext->logTarget = logTarget;

    switch (logTarget)
    {
       case VMW_DEPLOY_LOG_TARGET_FILE:

               if (IsNullOrEmptyString(pszFilePath))
               {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
               }
               else if (!strcmp(pszFilePath, "."))
               {
                    pContext->fp_out = stdout;
                    pContext->fp_err = stderr;
               }
               else
               {
                    pContext->fp_out = fopen(pszFilePath, "a");
                    if (pContext->fp_out == NULL)
                    {
                        dwError = LwErrnoToWin32Error(errno);
                        BAIL_ON_DEPLOY_ERROR(dwError);
                    }
               }

               pContext->pfnLogCallback = &VmwDeployLogToFile;

               break;

       case VMW_DEPLOY_LOG_TARGET_SYSLOG:

               openlog("ic-deploy", LOG_PID, LOG_AUTHPRIV);
               setlogmask(LOG_UPTO(LOG_INFO));

               pContext->bSyslogOpened = TRUE;

               pContext->pfnLogCallback = &VmwDeployLogToSyslog;

               break;

       default:

               dwError = ERROR_NOT_SUPPORTED;
               BAIL_ON_DEPLOY_ERROR(dwError);

               break;
    }

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    if (pContext)
    {
        VmwDeployReleaseLogContext(pContext);
    }

    goto cleanup;
}

DWORD
VmwDeploySetLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    PVMW_DEPLOY_LOG_CONTEXT pCurContext = NULL;

    VMW_DEPLOY_LOCK_MUTEX(&gLogMutex, bLocked);

    pCurContext = gpLogContext;

    gpLogContext = VmwDeployAcquireLogContext(pContext);

    VMW_DEPLOY_UNLOCK_MUTEX(&gLogMutex, bLocked);

    if (pCurContext)
    {
        VmwDeployReleaseLogContext(pCurContext);
    }

    return dwError;
}

VOID
VmwDeployLogMessage(
    VMW_DEPLOY_LOG_LEVEL logLevel,
    PCSTR                pszFormat,
    ...
    )
{
    BOOLEAN bLocked = FALSE;
    PVMW_DEPLOY_LOG_CONTEXT pCurContext = NULL;

    VMW_DEPLOY_LOCK_MUTEX(&gLogMutex, bLocked);

    pCurContext = VmwDeployAcquireLogContext(gpLogContext);

    VMW_DEPLOY_UNLOCK_MUTEX(&gLogMutex, bLocked);

    if (pCurContext)
    {
        if (logLevel <= pCurContext->logLevel)
        {
            va_list argList;
            va_start(argList, pszFormat);

            pCurContext->pfnLogCallback(
                            pCurContext,
                            logLevel,
                            pszFormat,
                            argList);

            va_end(argList);
        }
        VmwDeployReleaseLogContext(pCurContext);
    }
}

DWORD
VmwDeployGetLogLevel(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL*   pLogLevel
    )
{
    DWORD dwError = 0;

    if (!pContext || !pLogLevel)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *pLogLevel = pContext->logLevel;

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeploySetLogLevel(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL    logLevel
    )
{
    DWORD dwError = 0;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pContext->logLevel = logLevel;

error:

    return dwError;
}

VOID
VmwDeployReleaseLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    )
{
    if (InterlockedDecrement(&pContext->refcount) == 0)
    {
        VmwDeployFreeLogContext(pContext);
    }
}

static
PVMW_DEPLOY_LOG_CONTEXT
VmwDeployAcquireLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    )
{
    if (pContext)
    {
        InterlockedIncrement(&pContext->refcount);
    }

    return pContext;
}

static
VOID
VmwDeployLogToFile(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL    logLevel,
    PCSTR                   pszFormat,
    va_list                 args
    )
{
    char szTime[128];
    struct tm tmpTime = {0};
    FILE* fp = pContext->fp_out;

    time_t now = time(NULL);
    localtime_r(&now, &tmpTime);
    strftime(szTime, sizeof(szTime), VMW_DEPLOY_LOG_TIME_FORMAT, &tmpTime);

    if (logLevel == VMW_DEPLOY_LOG_LEVEL_ERROR && pContext->fp_err)
    {
        fp = pContext->fp_err;
    }

    fprintf(fp, "%s:%s:", szTime, VmwDeployLogLevelToTag(logLevel));
    vfprintf(fp, pszFormat, args);
    fprintf(fp, "\n");
    fflush(fp);
}

static
VOID
VmwDeployLogToSyslog(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL    logLevel,
    PCSTR                   pszFormat,
    va_list                 args
    )
{
    int priority = LOG_ERR;

    switch (logLevel)
    {
        case VMW_DEPLOY_LOG_LEVEL_ERROR:

                priority = LOG_ERR;

                break;

        case VMW_DEPLOY_LOG_LEVEL_WARNING:

                priority = LOG_WARNING;

                break;

        case VMW_DEPLOY_LOG_LEVEL_INFO:

                priority = LOG_INFO;

                break;

        case VMW_DEPLOY_LOG_LEVEL_VERBOSE:

                priority = LOG_INFO;

                break;

        case VMW_DEPLOY_LOG_LEVEL_DEBUG:

                priority = LOG_INFO;

                break;

        default:

                priority = LOG_INFO;

                break;
    }

    vsyslog(priority, pszFormat, args);
}

static
PCSTR
VmwDeployLogLevelToTag(
    VMW_DEPLOY_LOG_LEVEL logLevel
    )
{
    switch (logLevel)
    {
        case VMW_DEPLOY_LOG_LEVEL_ERROR:

                return "ERROR";

        case VMW_DEPLOY_LOG_LEVEL_WARNING:

            return "WARNING";

        case VMW_DEPLOY_LOG_LEVEL_INFO:

            return "INFO";

        case VMW_DEPLOY_LOG_LEVEL_VERBOSE:

            return "VERBOSE";

        case VMW_DEPLOY_LOG_LEVEL_DEBUG:

            return "DEBUG";

        default:

            return "UNKNOWN";
    }
}

static
VOID
VmwDeployFreeLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    )
{
    if (pContext->fp_out)
    {
        fclose(pContext->fp_out);
        pContext->fp_out = NULL;
    }
    if (pContext->fp_err)
    {
        fclose(pContext->fp_err);
        pContext->fp_err = NULL;
    }
    if (pContext->bSyslogOpened)
    {
        closelog();
        pContext->bSyslogOpened = FALSE;
    }

    VmwDeployFreeMemory(pContext);
}


