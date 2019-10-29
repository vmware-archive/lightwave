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

DWORD
VmDirParseArgs(
    int         argc,
    char*       argv[],
    PBOOLEAN    pbDaemonMode,
    int*        pLoggingLevel,
    PCSTR*      ppszLogFileName,
    PBOOLEAN    pbEnableSysLog,
    PBOOLEAN    pbConsoleMode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int opt = 0;
    setlocale(LC_ALL, "");

    //TODO, change to use long opt
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_DAEMON:
                if ( pbDaemonMode != NULL )
                {
                    *pbDaemonMode = TRUE;
                }
                break;

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

            case VMDIR_OPTION_RUN_MODE:
                if ( VmDirStringCompareA(VMDIR_RUN_MODE_RESTORE, optarg, TRUE ) == 0 )
                {
                    VmDirdSetTargetState( VMDIRD_STATE_RESTORE );
                }
                else if ( VmDirStringCompareA(VMDIR_RUN_MODE_STANDALONE, optarg, TRUE ) == 0 )
                {
                    VmDirdSetTargetState( VMDIRD_STATE_STANDALONE );
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
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

VOID
ShowUsage(
    PSTR pName
)
{
    fprintf(
        stderr,
        "Usage: %s\n"
        "          [-l <logging level>]\n"
        "          [-L <log file name>]\n"
        "          [-s] log to syslog\n"
        "          [-c] log to console\n"
        "          [-d] enable daemon\n"
        "          [-m] server run mode (standalone/restore)\n",
        pName
    );
}
