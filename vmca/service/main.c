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

static
VOID
PrintCurrentState(
    VOID
    );

static
DWORD
VMCAParseArgs(
    int argc,
    char* argv[],
    PBOOL pbEnableSysLog,
    PBOOL pbConsoleLogging,
    PBOOL pbEnableDaemon
)
{
    DWORD dwError = ERROR_SUCCESS;
    int opt = 0;
    BOOL bEnableSysLog = FALSE;
    BOOL bEnableDaemon = FALSE;
    BOOL bEnableConsoleLogging = FALSE;

    while ( (opt = getopt( argc, argv, VMCA_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
        case VMCA_OPTION_ENABLE_DAEMON:
            bEnableDaemon = TRUE;
            break;
        case VMCA_OPTION_ENABLE_SYSLOG:
            bEnableSysLog = TRUE;
            break;
        case VMCA_OPTION_CONSOLE_LOGGING:
            bEnableConsoleLogging = TRUE;
            break;
        default:
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    if (pbEnableDaemon != NULL)
    {
        *pbEnableDaemon = bEnableDaemon;
    }

    if (pbEnableSysLog != NULL)
    {
        *pbEnableSysLog = bEnableSysLog;
    }

    if (pbConsoleLogging)
    {
        *pbConsoleLogging = bEnableConsoleLogging;
    }

error:
    return dwError;
}

static
DWORD
VMCAInitVmRegConfig(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmRegConfigInit();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMDIR_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMAFD_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRegConfigAddFile(VMREGCONFIG_VMCA_REG_CONFIG_FILE, FALSE);
    BAIL_ON_VMCA_ERROR(dwError);

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
    BOOL bEnableSysLog = FALSE;
    BOOL bEnableDaemon = FALSE;
    BOOL bConsoleLogging = FALSE;

    setlocale(LC_ALL, "");

    VMCABlockSelectedSignals();

    dwError = VMCAParseArgs(argc, argv, &bEnableSysLog, &bConsoleLogging, &bEnableDaemon);
    BAIL_ON_VMCA_ERROR(dwError);

    if (bEnableDaemon)
    {
        dwError = VmDaemon();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (bEnableSysLog)
    {
        gVMCALogType = VMCA_LOG_TYPE_SYSLOG;
    }
    else if (bConsoleLogging)
    {
        gVMCALogType = VMCA_LOG_TYPE_CONSOLE;
    }
    else
    {
        gVMCALogType = VMCA_LOG_TYPE_FILE;
    }

    dwError = VMCAInitVmRegConfig();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError  = VMCAInitialize(0, 0);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("VM Certificate Service started.");

#ifdef REST_ENABLED
#ifndef _WIN32
    dwError = VMCARestServiceStartup();
    BAIL_ON_VMCA_ERROR(dwError);
    VMCA_LOG_INFO("VM Certificate ReST Protocol started.");
#endif
#endif

    PrintCurrentState();

    if (bEnableDaemon)
    {
        dwError = VmDaemonReady();
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // main thread waits on signals
    dwError = VMCAHandleSignals();
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("VM Certificate Service exiting...");

cleanup:

    VmRegConfigFree();
    VMCAShutdown();
#ifdef REST_ENABLED
#ifndef _WIN32
    VMCARestServiceShutdown();
#endif
#endif
    return (dwError);

error:

    VMCA_LOG_ERROR("VM Certificate exiting due to error [code:%d]", dwError);

    goto cleanup;
}

static
VOID
PrintCurrentState(
	VOID
	)
{
	DWORD dwFuncLevel = VMCASrvGetFuncLevel();

    if (dwFuncLevel == VMCA_FUNC_LEVEL_INITIAL) {
        printf("VMCA Server Functional level is VMCA_FUNC_LEVEL_INITIAL\n");
    }

    if ((dwFuncLevel & VMCA_FUNC_LEVEL_SELF_CA) == VMCA_FUNC_LEVEL_SELF_CA) {
        printf("VMCA Server Functional level is VMCA_FUNC_LEVEL_SELF_CA\n");
    }
}

