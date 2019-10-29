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
VmAfdParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbEnableConsole,
    PBOOLEAN pbEnableDaemon
)
{
    DWORD dwError = ERROR_SUCCESS;
    int opt = 0;
    setlocale(LC_ALL, "");

    //TODO, change to use long opt
    while ( (opt = getopt( argc, argv, VMAFD_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
        case VMAFD_OPTION_LOGGING_LEVEL:
            if( pLoggingLevel != NULL )
            {
                *pLoggingLevel = atoi( optarg );
            }
            break;

        case VMAFD_OPTION_ENABLE_SYSLOG:
            if ( pbEnableSysLog != NULL )
            {
                *pbEnableSysLog = TRUE;
            }
            break;
        case VMAFD_OPTION_ENABLE_CONSOLE:
            if (pbEnableConsole != NULL )
            {
                *pbEnableConsole = TRUE;
            }
            break;
        case VMAFD_OPTION_ENABLE_DAEMON:
            if (pbEnableDaemon != NULL )
            {
                *pbEnableDaemon = TRUE;
            }
            break;
        default:
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
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
       "          [-s] log to syslog\n"
       "          [-c] log to console\n"
       "          [-d] enable daemon\n",
       pName
   );
}
