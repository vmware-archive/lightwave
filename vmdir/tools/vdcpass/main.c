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
 * Module Name: vdcpass
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcpass main module entry point
 *
 */

#include "includes.h"

int
main(int argc, char* argv[])
{
    DWORD   dwError = 0;

    PSTR    pszHostURI = NULL;
    PSTR    pszLoginUserDN = NULL;
    PSTR    pszLoginPassword = NULL;
    PSTR    pszNewPassword = NULL;
    PSTR    pszUserDN = NULL;


    setlocale(LC_ALL, "");

    dwError = VmDirParseArgs(
                        argc, argv,
                        &pszHostURI,
                        &pszLoginUserDN,
                        &pszLoginPassword,
                        &pszNewPassword,
                        &pszUserDN);

    if (dwError != ERROR_SUCCESS)
    {
        ShowUsage();
        goto cleanup;
    }

    if (pszUserDN) //set password
    {
        dwError = VmDirSetPassword(
                            pszHostURI,
                            pszLoginUserDN,
                            pszLoginPassword,
                            pszUserDN,
                            pszNewPassword);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("password was set successfully.\n");
    }
    else //change password
    {
        dwError = VmDirChangePassword(
                            pszHostURI,
                            pszLoginUserDN,
                            pszLoginPassword,
                            pszNewPassword);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("password was changed successfully.\n");
    }

cleanup:
    return dwError;

error:
    printf("Vdcpass failed.");

    goto cleanup;
}

