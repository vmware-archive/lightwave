/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : defines.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            dns main
 *
 *            argument parsing functions
 *
 */

#include "includes.h"

#ifndef _WIN32

DWORD
VmDnsParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PCSTR* ppszLogFileName,
    int* pLdapPort,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbConsoleMode
)
{
    DWORD dwError = ERROR_SUCCESS;
    int opt = 0;
    setlocale(LC_ALL, "");

    //TODO, change to use long opt
    while ( (opt = getopt( argc, argv, VMDNS_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDNS_OPTION_LOGGING_LEVEL:
                if( pLoggingLevel != NULL )
                {
                    *pLoggingLevel = atoi( optarg );
                }
                break;

            case VMDNS_OPTION_LOG_FILE_NAME:
                if( ppszLogFileName != NULL )
                {
                    *ppszLogFileName = optarg;
                }
                break;

            case VMDNS_OPTION_PORT:
                if( pLdapPort != NULL )
                {
                    *pLdapPort = atoi( optarg );
                }
                break;

            case VMDNS_OPTION_ENABLE_SYSLOG:
                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
                }
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
error:
    return dwError;
}

#else

DWORD
VmDnsParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PCSTR* ppszLogFileName,
    int* pLdapPort,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbConsoleMode
)
{
    DWORD dwError = ERROR_SUCCESS;
    int i = 1; // first arg is the <name of exe>.exe

    while( i < argc )
    {
        if( VmDnsIsCmdLineOption( argv[i] ) != FALSE )
        {
            if ( VmDnsStringCompareA(
                          VMDNS_OPTION_LOGGING_LEVEL, argv[i], TRUE ) == 0 )
            {
                dwError = VmDnsGetCmdLineIntOption(
                    argc, argv, &i, pLoggingLevel
                );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else if ( VmDnsStringCompareA(
                          VMDNS_OPTION_LOG_FILE_NAME, argv[i], TRUE ) == 0 )
            {
                VmDnsGetCmdLineOption( argc, argv, &i, ppszLogFileName );
            }
            else if ( VmDnsStringCompareA(
                          VMDNS_OPTION_PORT, argv[i], TRUE ) == 0 )
            {
                dwError = VmDnsGetCmdLineIntOption(
                    argc, argv, &i, pLdapPort
                );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else if ( VmDnsStringCompareA(
                          VMDNS_OPTION_ENABLE_SYSLOG, argv[i], TRUE ) == 0 )
            {
                if ( pbEnableSysLog != NULL )
                {
                    *pbEnableSysLog = TRUE;
                }
            }
            else if (VmDnsStringCompareA(
                VMDNS_OPTION_CONSOLE_MODE, argv[i], TRUE) == 0)
            {
                if (pbConsoleMode != NULL)
                {
                    *pbConsoleMode = TRUE;
                }
            }
            else
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
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
