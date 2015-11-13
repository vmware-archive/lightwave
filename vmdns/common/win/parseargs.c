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
 * Module   : logging.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Common Utilities (Client & Server)
 *
 *            cmd line arg parsing
 *
 */

#include "includes.h"

#ifdef _WIN32

BOOLEAN
VmDnsIsCmdLineOption(
    char* arg
)
{
    return ( IsNullOrEmptyString( arg ) == FALSE )
           &&
           ( arg[0] == '-' );
}

VOID
VmDnsGetCmdLineOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    PCSTR* ppszOptionValue
)
{
    if ( (*pCurrentIndex + 1 < argc)
         &&
         ( VmDnsIsCmdLineOption(argv[*pCurrentIndex + 1]) == FALSE) )
    {
        *pCurrentIndex = *pCurrentIndex + 1;
        if ( ppszOptionValue != NULL )
        {
            *ppszOptionValue = argv[*pCurrentIndex];
        }
    }
    else
    {
        if ( ppszOptionValue != NULL )
        {
            *ppszOptionValue = NULL;
        }
    }
}

DWORD
VmDnsGetCmdLineIntOption(
    int argc,
    PSTR argv[],
    int* pCurrentIndex,
    int* pValue
)
{
    DWORD dwError = ERROR_SUCCESS;
    PCSTR pOptValStr = NULL;

    VmDnsGetCmdLineOption( argc, argv, pCurrentIndex, &pOptValStr );
    if ( ( pOptValStr != NULL ) && (pValue != NULL) )
    {
        *pValue = atoi(pOptValStr);
        if ( ( errno == EINVAL )  || (errno == ERANGE) )
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

error:

    return dwError;
}

DWORD
VmDnsAllocateArgsAFromArgsW(
    int argc,
    WCHAR* argv[],
    PSTR** argvA
)
{
    DWORD dwError = ERROR_SUCCESS;
    char** retArgV = NULL;

    if ( argvA == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    *argvA = NULL;

    if ( (argc > 0) && ( argv != NULL ) )
    {
        int i = 0;
        dwError = VmDnsAllocateMemory(
            argc*sizeof(char*), (PVOID*)(&retArgV) );
        BAIL_ON_VMDNS_ERROR(dwError);
        for(i = 0; i < argc; i++)
        {
            if( IsNullOrEmptyString( argv[i] ) == FALSE )
            {
                dwError = VmDnsAllocateStringAFromW( argv[i], (retArgV + i));
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else
            {
                retArgV[i] = NULL;
            }
        }
    }

    *argvA = retArgV;
    retArgV = NULL;

error:

    VmDnsDeallocateArgsA( argc, retArgV );
    retArgV = NULL;

    return dwError;
}

VOID
VmDnsDeallocateArgsA(
    int argc,
    PSTR argv[]
)
{
    if ( (argc > 0) && ( argv != NULL ) )
    {
        int i = 0;
        for(i = 0; i < argc; i++)
        {
            if(argv[i] != NULL)
            {
                VmDnsFreeStringA( argv[i] );
                argv[i] = NULL;
            }
        }
        VmDnsFreeMemory( argv );
    }
}

#endif
