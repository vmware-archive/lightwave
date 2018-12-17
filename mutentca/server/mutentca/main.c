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

static
VOID
PrintCurrentState(
    VOID
    );

static DWORD
LwCAParseArgs(
    int argc,
    char* argv[],
    PBOOL pbEnableSysLog,
    PBOOL pbConsoleLogging,
    LWCA_LOG_LEVEL  *pSyslogLevel
)
{
    DWORD dwError = LWCA_SUCCESS;
    int opt = 0;
    LWCA_LOG_LEVEL syslogLevel = LWCA_LOG_LEVEL_INFO;
    BOOL bEnableSysLog = FALSE;
    BOOL bEnableConsoleLogging = FALSE;

    while ( (opt = getopt( argc, argv, LWCA_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
        case LWCA_OPTION_ENABLE_SYSLOG:
            bEnableSysLog = TRUE;
            break;
        case LWCA_OPTION_CONSOLE_LOGGING:
            bEnableConsoleLogging = TRUE;
            break;
        case LWCA_OPTION_LOGGING_LEVEL:
            syslogLevel = atoi( optarg );
            break;
        default:
            dwError = LWCA_ERROR_INVALID_PARAMETER;
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

    if (pbEnableSysLog != NULL)
    {
        *pbEnableSysLog = bEnableSysLog;
    }

    if (pbConsoleLogging)
    {
        *pbConsoleLogging = bEnableConsoleLogging;
    }

    if (pSyslogLevel)
    {
        *pSyslogLevel = syslogLevel;
    }

error:
    return dwError;
}

int
main(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    const char* pszSmNotify = NULL;
    int notifyFd = -1;
    int notifyCode = 0;
    int ret = -1;
    LWCA_LOG_LEVEL syslogLevel = LWCA_LOG_LEVEL_INFO;
    BOOL bEnableSysLog = FALSE;
    BOOL bConsoleLogging = FALSE;

    setlocale(LC_ALL, "");

    LwCABlockSelectedSignals();

    dwError = LwCAParseArgs(argc, argv, &bEnableSysLog, &bConsoleLogging, &syslogLevel);
    BAIL_ON_LWCA_ERROR(dwError);

    LwCALogSetLevel(syslogLevel);
    if (bEnableSysLog)
    {
        gLwCALogType = LWCA_LOG_TYPE_SYSLOG;
    }
    else if (bConsoleLogging)
    {
        gLwCALogType = LWCA_LOG_TYPE_CONSOLE;
    }
    else
    {
        gLwCALogType = LWCA_LOG_TYPE_FILE;
    }

    dwError  = LwCAInitialize(0, 0);
    BAIL_ON_LWCA_ERROR(dwError);

    // interact with likewise service manager (start/stop control)
    if ((pszSmNotify = getenv("LIKEWISE_SM_NOTIFY")) != NULL)
    {
        notifyFd = atoi(pszSmNotify);

        do
        {
            ret = write(notifyFd, &notifyCode, sizeof(notifyCode));

        } while (ret != sizeof(notifyCode) && errno == EINTR);

        if (ret < 0)
        {
            LWCA_LOG_ERROR("Could not notify service manager: %s (%i)",
                            strerror(errno),
                            errno);
            dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
            BAIL_ON_LWCA_ERROR(dwError);
        }

        close(notifyFd);
    }

    LWCA_LOG_INFO("MutentCA Service started.");
    PrintCurrentState();

    // main thread waits on signals
    dwError = LwCAHandleSignals();
    BAIL_ON_LWCA_ERROR(dwError);

    LWCA_LOG_INFO("MutentCA Service exiting...");

cleanup:

    LwCAShutdown();

    return (dwError);

error:

    LWCA_LOG_ERROR("MutentCA exiting due to error [code:%d]", dwError);

    goto cleanup;
}

static
VOID
PrintCurrentState(
    VOID
    )
{
    printf("MutantCA Server Functional level is LWCA_FUNC_LEVEL_INITIAL\n");
}
