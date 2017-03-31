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

PCSTR
VmDirTestGetInternalUserCn(
    PVMDIR_TEST_STATE pState
    )
{
    return pState->pszInternalUserName;
}

DWORD
VmDirTestGetInternalUserDn(
    PVMDIR_TEST_STATE pState,
    PSTR *ppszDn
    )
{
    PSTR pszDn = NULL;
    DWORD dwError = 0;

    dwError = VmDirAllocateStringPrintf(
                &pszDn,
                "cn=%s,cn=Users,%s",
                VmDirTestGetInternalUserCn(pState),
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDn = pszDn;

cleanup:
    return dwError;
error:
    goto cleanup;
}

PCSTR
VmDirTestGetTestContainerCn(
    PVMDIR_TEST_STATE pState
    )
{
    return pState->pszTestContainerName;
}

DWORD
VmDirTestGetTestContainerDn(
    PVMDIR_TEST_STATE pState,
    PSTR *ppszDN
    )
{
    DWORD dwError = 0;
    PSTR pszDN = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,%s",
                VmDirTestGetTestContainerCn(pState),
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppszDN = pszDN;

cleanup:
    return dwError;
error:
    goto cleanup;
}
