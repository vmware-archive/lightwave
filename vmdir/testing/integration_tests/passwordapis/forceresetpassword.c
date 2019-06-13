/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_PasswordBytesToString(
    PBYTE pbPassword,
    DWORD dwLength,
    PSTR *ppszPassword
    )
{
    DWORD dwError = 0;
    PSTR pszPassword = NULL;
    DWORD dwIndex = 0;

    dwError = VmDirAllocateMemory(dwLength + 1, (PVOID*)&pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < dwLength; ++dwIndex)
    {
        pszPassword[dwIndex] = pbPassword[dwIndex];
    }
    pszPassword[dwIndex] = '\0';

    *ppszPassword = pszPassword;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
TestForceResetPassword(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PBYTE pbPassword = NULL;
    DWORD dwLength = 0;
    PSTR pszUserName = NULL;
    PSTR pszUserUPN = NULL;
    PSTR pszPassword = NULL;
    LDAP *pLd = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserUPN,
                "%s@%s",
                pszUserName,
                pState->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, VmDirTestGetTestContainerCn(pState), pszUserName, NULL);
    TestAssert(dwError == 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirForceResetPassword(pszUserUPN, &pbPassword, &dwLength);
    TestAssert(dwError == 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _PasswordBytesToString(pbPassword, dwLength, &pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Now make sure we can connect with the new password.
    //
    dwError = VmDirSafeLDAPBind(
                &pLd,
                pState->pszServerName,
                pszUserUPN,
                pszPassword);
    TestAssert(dwError == 0);

cleanup:
    VmDirTestDeleteUser(pState, VmDirTestGetTestContainerCn(pState), pszUserName);
    VMDIR_SAFE_FREE_MEMORY(pbPassword);
    VMDIR_SAFE_FREE_STRINGA(pszPassword);
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszUserUPN);
    VmDirTestLdapUnbind(pLd);

    return dwError;

error:
    goto cleanup;
}
