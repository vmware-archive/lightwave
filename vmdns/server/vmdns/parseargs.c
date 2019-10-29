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


/*
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

DWORD
VmDnsParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PCSTR* ppszLogFileName,
    int* pLdapPort,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbEnableDaemon,
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

	    case VMDNS_OPTION_ENABLE_DAEMON:
                if ( pbEnableDaemon != NULL )
                {
                    *pbEnableDaemon = TRUE;
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
   fprintf(
        stderr,
        "Usage: %s\n"
        "          [-l <logging level>]\n"
        "          [-L <log file name>]\n"
        "          [-s] log to syslog\n"
        "          [-p] ldap port\n"
        "          [-d] enable daemon\n",
        pName
    );
}
