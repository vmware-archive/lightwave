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

DWORD
VmDirParseArgs(
    int         argc,
    char*       argv[],
    PCSTR*      ppszBootstrapSchemaFile,
    int*        pLoggingLevel,
    PCSTR*      ppszLogFileName,
    PBOOLEAN    pbEnableSysLog,
    PBOOLEAN    pbConsoleMode,
    PBOOLEAN    pbPatchSchema
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
            case VMDIR_OPTION_BOOTSTRAP_SCHEMA_FILE:
                if( ppszBootstrapSchemaFile != NULL )
                {
                    *ppszBootstrapSchemaFile = optarg;
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

            case VMDIR_OPTION_PATCH_SCHEMA:
                if ( pbPatchSchema != NULL )
                {
                    *pbPatchSchema = TRUE;
                }
                break;

            case VMDIR_OPTION_RUN_MODE:
                if ( VmDirStringCompareA(VMDIR_RUN_MODE_RESTORE, optarg, TRUE ) == 0 )
                {
                    VmDirdSetRunMode( VMDIR_RUNMODE_RESTORE );
                }
                else if ( VmDirStringCompareA(VMDIR_RUN_MODE_STANDALONE, optarg, TRUE ) == 0 )
                {
                    VmDirdSetRunMode( VMDIR_RUNMODE_STANDALONE );
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

#else

DWORD
VmDirParseArgs(
    int         argc,
    char*       argv[],
    PCSTR*      ppszBootstrapSchemaFile,
    int*        pLoggingLevel,
    PCSTR*      ppszLogFileName,
    PBOOLEAN    pbEnableSysLog,
    PBOOLEAN    pbConsoleMode,
    PBOOLEAN    pbPatchSchema
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int i = 1; // first arg is the <name of exe>.exe
    PCSTR   pszRunMode = NULL;

    while( i < argc )
    {
        if( VmDirIsCmdLineOption( argv[i] ) != FALSE )
        {
            if ( VmDirStringCompareA(
                    VMDIR_OPTION_BOOTSTRAP_SCHEMA_FILE, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption(
                    argc, argv, &i, ppszBootstrapSchemaFile
                );
            }
            else if ( VmDirStringCompareA(
                          VMDIR_OPTION_LOGGING_LEVEL, argv[i], TRUE ) == 0 )
            {
                dwError = VmDirGetCmdLineIntOption(
                    argc, argv, &i, pLoggingLevel
                );
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else if ( VmDirStringCompareA(
                          VMDIR_OPTION_LOG_FILE_NAME, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, ppszLogFileName );
            }
            else if ( VmDirStringCompareA(
                          VMDIR_OPTION_ENABLE_SYSLOG, argv[i], TRUE ) == 0 )
            {
                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
                }
            }
            else if ( VmDirStringCompareA(
                            VMDIR_OPTION_CONSOLE_MODE, argv[i], TRUE ) == 0 )
            {
                if ( pbConsoleMode != NULL )
                {
                    *pbConsoleMode = TRUE;
                }
            }
            else if ( VmDirStringCompareA(
                            VMDIR_OPTION_PATCH_SCHEMA, argv[i], TRUE ) == 0 )
            {
                if ( pbPatchSchema != NULL )
                {
                    *pbPatchSchema = TRUE;
                }
            }
            else if ( VmDirStringCompareA(
                            VMDIR_OPTION_RUN_MODE, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszRunMode);

                if ( VmDirStringCompareA(VMDIR_RUN_MODE_RESTORE, pszRunMode, TRUE ) == 0 )
                {
                    VmDirdSetRunMode( VMDIR_RUNMODE_RESTORE );
                }
                else if ( VmDirStringCompareA(VMDIR_RUN_MODE_STANDALONE, pszRunMode, TRUE ) == 0 )
                {
                    VmDirdSetRunMode( VMDIR_RUNMODE_STANDALONE );
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
            }
            else
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        i++;
    } // while

error:

    return dwError;
}

#endif

VOID
ShowUsage(
    PSTR pName
)
{
    //TODO, cleanup after use long opt
   fprintf(
       stderr,
       "Usage: %s [-d <logging level (an integer)>] [-p <ldap port>]",
       pName
   );
}
