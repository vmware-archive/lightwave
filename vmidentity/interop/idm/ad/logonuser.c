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
 * Module Name:
 *
 *        logonuser.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        Authentication
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#include "stdafx.h"

DWORD
IDMAuthenticateUser(
    LPWSTR pszUserName,
    LPWSTR pszDomainName,
    LPWSTR pszPassword,
    PIDM_USER_INFO * ppIdmUserInformation
    )
{
    DWORD dwError = 0;
    BOOL bRet = FALSE;
    HANDLE hClientToken = NULL;
    PIDM_USER_INFO pIdmUserInformation = NULL;

    if (!pszDomainName || !pszUserName || !pszPassword || !ppIdmUserInformation)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMInitializeSidCache();
    BAIL_ON_ERROR(dwError);

    bRet = LogonUser(
                pszUserName,
                pszDomainName,
                pszPassword,
                LOGON32_LOGON_NETWORK,
                LOGON32_PROVIDER_DEFAULT,
                &hClientToken);
    if (!bRet)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMGetUserInfo(hClientToken, &pIdmUserInformation);
    BAIL_ON_ERROR(dwError);

    *ppIdmUserInformation = pIdmUserInformation;

cleanup:

    if (hClientToken)
    {
        CloseHandle(hClientToken);
    }

    return dwError;

error:
    if (pIdmUserInformation)
    {
        IDMFreeUserInfo(pIdmUserInformation);
    }

    goto cleanup;
}
