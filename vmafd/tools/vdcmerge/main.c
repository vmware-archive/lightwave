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
 * Module Name: vdcmerge
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcmerge main module entry point
 *
 */

#include "includes.h"

int
main(int argc, char* argv[])
{
    DWORD       dwError = 0;

    PSTR        pszTargetHost = NULL;
    PSTR        pszSourceUserName = NULL;
    PSTR        pszTargetUserName = NULL;
    PSTR        pszSourcePassword = NULL;
    PSTR        pszTargetPassword = NULL;
#if 0
    CHAR        pszPath[MAX_PATH];
#endif
#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

#if  0
    dwError = VmAfdGetVmAfdLogPath(pszPath, "vdcmerge.log");
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLogInitialize(pszPath, FALSE, 1);
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    dwError = VmAfdParseArgs(
        argc, argv, &pszTargetHost, &pszSourceUserName, &pszTargetUserName,
        &pszSourcePassword, &pszTargetPassword);

    if (dwError)
    {
        ShowUsage();
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdMergeVmDir(
                    "localhost",
                    pszSourceUserName,
                    pszSourcePassword,
                    pszTargetHost,
                    pszTargetUserName,
                    pszTargetPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Vdcmerge succeeded\n");

cleanup:
    VmAfdLogTerminate();
    return dwError > 255 ? 255 : dwError;
error:
#if 1
    printf("Vdcmerge failed. Error[%d]\n", dwError);
#else
    printf("Vdcmerge failed. Error[%d] - %s\n", dwError, VmAfdGetErrorMessage(dwError));
#endif

    goto cleanup;
}

