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

int
main(
    int argc,
    char* argv[]
    )
{
    DWORD   dwError = 0;
    CHAR    pszPath[MAX_PATH];
    PSTR   pszServerName = NULL;
    PSTR   pszUserName = NULL;
    PSTR   pszPassword = NULL;
    PSTR   pszPasswordBuf = NULL;
    PSTR   pszErrorMessage = NULL;

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "vdcleavefed.log");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize( pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirLogSetLevel( VMDIR_LOG_VERBOSE );

    dwError = VmDirParseArgs( argc, argv, &pszServerName, &pszUserName, &pszPassword);
    if (dwError != 0)
    {
        ShowUsage();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszUserName == NULL)
    {
        //Must use administrator as userName
        ShowUsage();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(VMDIR_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    if(pszPassword == NULL)
    {
            // read passowrd from stdin
            VmDirReadString("password: ", pszPasswordBuf, VMDIR_MAX_PWD_LEN, TRUE);
    } else
    {
            dwError = VmDirStringCpyA(pszPasswordBuf, VMDIR_MAX_PWD_LEN, pszPassword);
            BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszServerName == NULL)
    {
        //local leavefe instead of offline
        printf("vdcleavefd for local server\n");
        dwError = _VdcSetReadOnlyState();
        BAIL_ON_VMDIR_ERROR(dwError);
    } else
    {
         printf("vdcleavefd offline for server %s\n", pszServerName);
    }

    dwError = VmDirLeaveFederation(pszServerName, pszUserName, pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf(" Leave federation cleanup done\n");

cleanup:
    VMDIR_SECURE_FREE_STRINGA(pszPasswordBuf);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);

    VmDirLogTerminate();
    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Leave federation cleanup failed. Error[%d] - %s\n",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");

    goto cleanup;
}

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
