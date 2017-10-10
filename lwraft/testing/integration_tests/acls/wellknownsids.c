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

BOOLEAN
CompareWellknownSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszObjectSid,
    PCSTR pszDomainSid,
    DWORD dwWellknownRid
    )
{
    PSTR pszComputedSid = NULL;
    DWORD dwError = 0;
    BOOLEAN bMatch = FALSE;

    dwError = VmDirAllocateStringPrintf(
                &pszComputedSid,
                "%s-%d",
                pszDomainSid,
                dwWellknownRid);
    TestAssert(dwError == 0);

    bMatch = (VmDirStringCompareA(pszObjectSid, pszComputedSid, TRUE) == 0);

    VMDIR_SAFE_FREE_STRINGA(pszComputedSid);

    return bMatch;
}

DWORD
TestWellknownSids(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszObjectSid = NULL;
    PSTR pszDomainSid = NULL;
    PSTR pszKerberosUser = NULL;

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    TestAssert(dwError == 0);

    dwError = VmDirTestGetUserSid(pState, "administrator", NULL, &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, DOMAIN_ALIAS_RID_ADMINS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszKerberosUser, "%s/%d", NULL, &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirTestGetUserSid(pState, pszKerberosUser, NULL, &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, DOMAIN_USER_RID_KRBTGT));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "DCAdmins", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, DOMAIN_GROUP_RID_ADMINS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "DCClients", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, DOMAIN_GROUP_RID_USERS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "Administrators", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, DOMAIN_GROUP_RID_ADMINS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "Users", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, DOMAIN_ALIAS_RID_USERS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    return dwError;
error:
    goto cleanup;
}
