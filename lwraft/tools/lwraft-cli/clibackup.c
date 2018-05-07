/*
 * Copyright © 018 VMware, Inc.  All Rights Reserved.
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

static
DWORD
_RaftCliLDAPDBBackup(
    PCSTR   pszServerName,
    PCSTR   pszLogin,
    PCSTR   pszPassword,
    PCSTR   pszBackupPath
    );

DWORD
RaftCliDBBackup(
    PCSTR pszServerName,
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszBackupPath
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalDCAccount = NULL;
    PSTR    pszLocalPassword = NULL;
    PCSTR   pszLocalServerName = pszServerName ? pszServerName : "localhost";

    if (IsNullOrEmptyString(pszBackupPath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!pszLogin)
    {
        dwError = VmDirRegReadDCAccount(&pszLocalDCAccount);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pszPassword)
    {
        dwError = VmDirReadDCAccountPassword(&pszLocalPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _RaftCliLDAPDBBackup(
        pszLocalServerName,
        pszLogin ? pszLogin : pszLocalDCAccount,
        pszPassword ? pszPassword : pszLocalPassword,
        pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

   printf("\n --------------------------------------------------------------------------------------\n");
   printf("    LDAP database backup on host %s, path %s finished successfully\n", pszLocalServerName, pszBackupPath);
   printf(" --------------------------------------------------------------------------------------\n\n");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszLocalPassword);

    return dwError;

error:
    printf("\n --------------------------------------------------------------------------------------\n");
    printf("    LDAP database backup on host %s failed (%d)\n", pszLocalServerName, dwError);
    printf(" --------------------------------------------------------------------------------------\n\n");
    goto cleanup;
}

static
DWORD
_RaftCliLDAPDBBackup(
    PCSTR   pszServerName,
    PCSTR   pszLogin,
    PCSTR   pszPassword,
    PCSTR   pszBackupPath
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomain = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;

    dwError = VmDirGetDomainName(pszServerName, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirOpenServerA(
        pszServerName,
        pszLogin,
        pszDomain,
        pszPassword,
        0,
        NULL,
        &hServer);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBackupDB(
        hServer,
        pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    if (hServer)
    {
        VmDirCloseServer(hServer);
    }

    return dwError;

error:
    goto cleanup;
}
