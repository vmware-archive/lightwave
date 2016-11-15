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
 * Module Name: vdcleavefed
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcleavefed module entry point
 *
 */

#include "includes.h"

DWORD
_VdcSetReadOnlyState(
    VOID
    );

static
int
VmDirMain(
    int argc,
    char* argv[]
    )
{
    DWORD   dwError = 0;
    CHAR    pszPath[MAX_PATH];
    PSTR   pszRaftLeader = NULL;
    PSTR   pszServerToLeave = NULL;
    PSTR   pszUserName = NULL;
    PSTR   pszPassword = NULL;
    CHAR   pszPasswordBuf[VMDIR_MAX_PWD_LEN + 1] = {0};
    PSTR   pszErrorMessage = NULL;

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "vdcleavefed.log");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize( pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirLogSetLevel( VMDIR_LOG_VERBOSE );

    dwError = VmDirParseArgs( argc, argv, &pszRaftLeader, &pszServerToLeave, &pszUserName, &pszPassword);
    if (dwError != 0)
    {
        ShowUsage();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszUserName == NULL || pszServerToLeave == NULL || pszRaftLeader == NULL)
    {
        ShowUsage();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszPassword == NULL)
    {
        // read password from stdin
        VmDirReadString(
            "password: ",
            pszPasswordBuf,
            sizeof(pszPasswordBuf),
            TRUE);
        pszPassword = pszPasswordBuf;
    }

    dwError = VmDirLeaveFederation(pszRaftLeader, pszServerToLeave, pszUserName, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf(" Leave federation cleanup done\n");

cleanup:

    memset(pszPasswordBuf, 0, sizeof(pszPasswordBuf));
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);

    VmDirLogTerminate();
    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Leave federation cleanup failed. Error[%d] - %s\n",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");

    goto cleanup;
}

#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwError = 0;
    PSTR* ppszArgs = NULL;
    int   iArg = 0;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmDirAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMain(argc, ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }
        VmDirFreeMemory(ppszArgs);
    }

    return dwError;
}
#else

int main(int argc, char* argv[])
{
    return VmDirMain(argc, argv);
}

#endif

DWORD
_VdcSetReadOnlyState(
    VOID
    )
{
    DWORD       dwError = 0;

    dwError = VmDirSetState( NULL, VMDIRD_STATE_READ_ONLY );
    BAIL_ON_VMDIR_ERROR( dwError );

    printf(" set local vmdir state to READ_ONLY\n");

cleanup:
    return dwError;

error:
    printf(" set VMDIR_STATE_READ_ONLY failed, (%u)\n", dwError);
    goto cleanup;
}
