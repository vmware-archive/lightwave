/*
 * Copyright ©2017 VMware, Inc.  All Rights Reserved.
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
 * Module Name: lwraft_test_access
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * access token testing main module entry point
 *
 */

#include "includes.h"

static
DWORD
test_access_token(
    PCSTR pszUpn
    );

int
main(
    int argc,
    char *argv[])
{
    DWORD dwError = 0;
    int exit_status = EXIT_SUCCESS;
    PSTR pszUpn = "Administrator";

    if (argc > 1)
    {
        pszUpn = argv[1];
    }

    dwError = test_access_token(pszUpn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return exit_status;

error:
    exit_status = EXIT_FAILURE;
    goto cleanup;
}

static
DWORD
test_access_token(
    PCSTR pszUpn)
{
    PTOKEN_USER              pUser  = NULL;
    PTOKEN_GROUPS            pGroups = NULL;
    PACCESS_TOKEN            pAccessToken = NULL;
    PSTR                     pszUserSid = NULL;
    PSTR                     pszGroupSid = NULL;
    PLW_MAP_SECURITY_CONTEXT pMapSecurityContext = NULL;
    DWORD                    ulBufLen = 0;
    DWORD                    dwError = 0;
    DWORD                    dwIndex = 0;

    dwError = LwMapSecurityCreateContext(&pMapSecurityContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Creating access token for UPN: %s\n", pszUpn);

    dwError = LwMapSecurityCreateAccessTokenFromCStringUsername(
                  pMapSecurityContext,
                  &pAccessToken,
                  pszUpn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get user sid
    dwError = RtlQueryAccessTokenInformation(
                  pAccessToken,
                  TokenUser,
                  NULL,
                  0,
                  &ulBufLen);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

    pUser = RtlMemoryAllocate(ulBufLen, TRUE);
    if (!pUser)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = RtlQueryAccessTokenInformation(
                  pAccessToken,
                  TokenUser,
                  pUser,
                  ulBufLen,
                  &ulBufLen);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RtlAllocateCStringFromSid(
                  &pszUserSid,
                  pUser->User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("User SID: %s\n", pszUserSid);

    dwError = RtlQueryAccessTokenInformation(
                  pAccessToken,
                  TokenGroups,
                  NULL,
                  0,
                  &ulBufLen);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

    pGroups = RtlMemoryAllocate(ulBufLen, TRUE);
    if (!pGroups)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = RtlQueryAccessTokenInformation(
                  pAccessToken,
                  TokenGroups,
                  pGroups,
                  ulBufLen,
                  &ulBufLen);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pGroups->GroupCount; dwIndex++)
    {
        dwError = RtlAllocateCStringFromSid(
                      &pszGroupSid,
                      pGroups->Groups[dwIndex].Sid);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("Group SID: %s\n", pszGroupSid);
    }

cleanup:
    if (pMapSecurityContext)
    {
        LwMapSecurityFreeContext(&pMapSecurityContext);
    }
    if (pAccessToken)
    {
        RtlReleaseAccessToken(&pAccessToken);
    }
    if (pszUserSid)
    {
        RtlMemoryFree(pszUserSid);
    }
    if (pGroups)
    {
        RtlMemoryFree(pGroups);
    }
    if (pUser)
    {
        RtlMemoryFree(pUser);
    }

    return dwError;

error:
    goto cleanup;
}
