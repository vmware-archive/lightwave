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
VmKdcParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbEnableConsole
)
{
    DWORD dwError = ERROR_SUCCESS;
    int opt = 0;
    setlocale(LC_ALL, "");

    //TODO, change to use long opt
    while ( (opt = getopt( argc, argv, VMKDC_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMKDC_OPTION_LOGGING_LEVEL:
                if( pLoggingLevel != NULL )
                {
                    *pLoggingLevel = atoi( optarg );
                }
                break;

            case VMKDC_OPTION_ENABLE_SYSLOG:
                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
                }
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMKDC_ERROR(dwError);
        }
    }
error:
    return dwError;
}

#else

DWORD
VmKdcParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbEnableConsole
)
{
    DWORD dwError = ERROR_SUCCESS;
    int i = 1; // first arg is the <name of exe>.exe

    while( i < argc )
    {
        if( VmKdcIsCmdLineOption( argv[i] ) != FALSE )
        {
            if ( VmKdcStringCompareA(
                          VMKDC_OPTION_LOGGING_LEVEL, argv[i], TRUE ) == 0 )
            {
                dwError = VmKdcGetCmdLineIntOption(
                    argc, argv, &i, pLoggingLevel
                );
                BAIL_ON_VMKDC_ERROR(dwError);
            }
            else if ( VmKdcStringCompareA(
                          VMKDC_OPTION_ENABLE_SYSLOG, argv[i], TRUE ) == 0 )
            {
                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
                }
            }
#if defined(WIN32) && defined(_DEBUG)
            else if ( VmKdcStringCompareA(
                          VMKDC_OPTION_ENABLE_CONSOLE, argv[i], TRUE ) == 0 ||
                      VmKdcStringCompareA(
                          VMKDC_OPTION_ENABLE_CONSOLE_LONG, argv[i], TRUE ) == 0 )
            {
                if ( pbEnableConsole != NULL )
                {
                    *pbEnableConsole = TRUE;
                }
            }
#endif
            else
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMKDC_ERROR(dwError);
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
       "Usage: %s [-l <logging level>] [-s]",
       pName
   );
}
