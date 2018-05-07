/*
 * Copyright © 2187 VMware, Inc.  All Rights Reserved.
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
_RaftCliForeResetAdminCreds(
    PCSTR   pszDomain,
    PSTR*   ppszNewPassword
    );

DWORD
RaftCliDRNodeRestoreFromDB(
    PCSTR pszLogin,
    PCSTR pszPassword
    )
{
    DWORD   dwError = 0;
    DWORD   dwState = 0;
    PSTR    pszLocalPassword = NULL;
    PSTR    pszDCAccount = NULL;
    PSTR    pszDomain = NULL;

    if ((pszLogin && !pszPassword) || (!pszLogin && pszPassword))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirGetDomainName("localhost", &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    // By now, we should have copied backup DB into /var/lib/vmware/post/partner/ directory.
    // This will trigger vmdir to swap and pick up backup DB + clean up all DC objects.
    dwError = VmDirServerReset(&dwState);
    if (dwError != 0)
    {
        printf("!!!!! FATAL ERROR !!!!! %s call VmDirServerReset failed (%u)\n", __FUNCTION__, dwError);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = _RaftCliForeResetAdminCreds(pszDomain, &pszLocalPassword);
        if (dwError != 0)
        {
            printf("!!!!! FATAL ERROR !!!!! %s call _RaftCliForeResetAdminCreds failed (%u)\n", __FUNCTION__, dwError);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // recreate DC account for this DR node
    dwError = VmDirSetupDefaultAccount(
        pszDomain,
        pszDCAccount,
        pszDCAccount,
        pszLogin ? pszLogin : RAFT_LOGIN_DEFAULT,
        pszPassword ? pszPassword : pszLocalPassword);
   if (dwError != 0)
   {
       printf("!!!!! FATAL ERROR !!!!! %s call VmDirSetupDefaultAccount failed (%u)\n", __FUNCTION__, dwError);
   }
   BAIL_ON_VMDIR_ERROR(dwError);

   printf("\n --------------------------------------------------------------------------------------\n");
   printf("    Raft Cluster node %s successfully restored from backup database\n", pszDCAccount);
   printf(" --------------------------------------------------------------------------------------\n\n");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszLocalPassword);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_RaftCliForeResetAdminCreds(
    PCSTR   pszDomain,
    PSTR*   ppszNewPassword
    )
{
    DWORD   dwError = 0;
    PSTR    pszUPN = NULL;
    PBYTE   pLocalPasswordByte = NULL;
    DWORD   dwPasswordSize = 0;
    PSTR    pszLocalPass = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszUPN,
                "%s@%s",
                RAFT_LOGIN_DEFAULT,
                pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirForceResetPassword(
        pszUPN,
        &pLocalPasswordByte,
        &dwPasswordSize);
    if (dwError == 0)
    {
        dwError = VmDirAllocateAndCopyMemory(pLocalPasswordByte, dwPasswordSize+1, (PVOID*)&pszLocalPass);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("New password for %s is -\n%s\n", pszUPN, pszLocalPass);
    }
    else
    {
        printf("!!!!! FATAL ERROR !!!!! %s call VmDirForceResetPassword failed (%u)\n", __FUNCTION__, dwError);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszNewPassword = pszLocalPass;
    pszLocalPass = NULL;

cleanup:
    if (pLocalPasswordByte)
    {
        memset(pLocalPasswordByte, 0, dwPasswordSize);
        VMDIR_SAFE_FREE_MEMORY(pLocalPasswordByte);
    }

    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszLocalPass);

    return dwError;

error:
    goto cleanup;
}
