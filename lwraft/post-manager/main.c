/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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
DWORD
_VmDirNotifyLikewiseServiceManager(
    VOID
    );

DWORD
VmDirParseArgs(
    int         argc,
    char*       argv[],
    int*        pLoggingLevel,
    PCSTR*      ppszLogFileName,
    PBOOLEAN    pbEnableSysLog,
    PBOOLEAN    pbConsoleMode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int opt = 0;
    setlocale(LC_ALL, "");

    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_LOGGING_LEVEL:
                if( pLoggingLevel != NULL )
                {
                    *pLoggingLevel = atoi( optarg );
                }
                break;

            case VMDIR_OPTION_LOG_FILE_NAME:
                if( ppszLogFileName != NULL )
                {
                    *ppszLogFileName = optarg;
                }
                break;

            case VMDIR_OPTION_ENABLE_SYSLOG:
                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
                }
                break;

            case VMDIR_OPTION_CONSOLE_MODE:
                if ( pbConsoleMode != NULL )
                {
                    *pbConsoleMode = TRUE;
                }
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
error:
    return dwError;
}

DWORD
VmDirPostMgrInit(
    VOID
    )
{
    DWORD               dwError = 0;
    PSTR                pszPostdArgs = NULL;
    PVMDIR_STRING_LIST  pList = NULL;

    dwError = VmDirInitThreadContext();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirProcessTableInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(SIZE_512, (PVOID*) &gPostMgrGlobals.pszPostdPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(SIZE_512, (PVOID*) &pszPostdArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetRegKeyValue(VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_POSTD_PATH, gPostMgrGlobals.pszPostdPath, SIZE_512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetRegKeyValue(VMDIR_CONFIG_PARAMETER_KEY_PATH, VMDIR_POSTD_ARGS, pszPostdArgs, SIZE_512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToTokenList(pszPostdArgs, " ", &pList);
    BAIL_ON_VMDIR_ERROR(dwError);

    gPostMgrGlobals.ppszPostdArgs = (PSTR*) pList->pStringList;

    dwError = VmDirIpcServerInit();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%d", dwError);
    goto cleanup;
}

int
main(
    int     argc,
    char*   argv[]
    )
{
    DWORD        dwError = 0;
    const char   *logFileName = NULL;
    BOOLEAN      bEnableSysLog = FALSE;
    BOOLEAN      bConsoleMode = FALSE;
    int          iLocalLogMask = 0;
    PSTR         pszDCAccount = NULL;

    dwError = VmDirParseArgs(
            argc,
            argv,
            &iLocalLogMask,
            &logFileName,
            &bEnableSysLog,
            &bConsoleMode);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize(logFileName, bEnableSysLog, "post-manager", VMDIR_LOG_INFO, iLocalLogMask);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "POST manager: starting...");

    VmDirBlockSelectedSignals();

    dwError = VmDirPostMgrInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "DC Account: %s", pszDCAccount);

    dwError = _VmDirNotifyLikewiseServiceManager();
    BAIL_ON_VMDIR_ERROR(dwError);

    /*Start group 0 process. This is temporary. Will be moved once promote flow is implemented*/
    dwError = VmDirStartProcess(0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirHandleSignals();
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "POST manager: exiting..." );

    /*Stop all post processes before exiting*/
    dwError = VmDirStopAllProcesses();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirLogTerminate();
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%d", dwError);
    goto cleanup;
}

static
DWORD
_VmDirNotifyLikewiseServiceManager(
    VOID
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PCSTR pszSmNotify = NULL;
    int   ret = 0;
    int   notifyFd = -1;
    char  notifyCode = 0;

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
#define BUFFER_SIZE 1024
            char buffer[BUFFER_SIZE]= {0};
            int errorNumber = errno;

            VmDirStringErrorA(buffer, BUFFER_SIZE, errorNumber);
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                      "Could not notify service manager: %s (%i)",
                      buffer,
                      errorNumber);

            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMDIR_ERROR(dwError);
#undef BUFFER_SIZE
        }
    }

error:
    if(notifyFd != -1)
    {
        close(notifyFd);
    }

    return dwError;
}
