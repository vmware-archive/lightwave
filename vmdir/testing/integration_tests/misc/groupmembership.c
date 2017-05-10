/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

PCSTR ppszGroups[] =
{
    "GroupTestGroup1",
    "GroupTestGroup2",
};


BOOLEAN
VmDirStringListContainsEx(
    PVMDIR_STRING_LIST pvsList,
    PCSTR pszString,
    BOOLEAN bCaseSensitive
    )
{
    DWORD dwIndex = 0;
    BOOLEAN bFound = FALSE;

    for (dwIndex = 0; dwIndex < pvsList->dwCount; ++dwIndex)
    {
        if (VmDirStringCompareA(pszString, pvsList->pStringList[dwIndex], bCaseSensitive) == 0)
        {
            bFound = TRUE;
            break;
        }
    }

    return bFound;
}

BOOLEAN
VmDirStringListEqualsNoOrder(
    PVMDIR_STRING_LIST pStringListLHS,
    PVMDIR_STRING_LIST pStringListRHS,
    BOOLEAN bCaseSensitive
    )
{
    DWORD dwMatching = 0;
    DWORD dwIndex = 0;

    if (pStringListLHS->dwCount != pStringListRHS->dwCount)
    {
        return FALSE;
    }

    for (dwIndex = 0; dwIndex < pStringListLHS->dwCount; ++dwIndex)
    {
        if (VmDirStringListContainsEx(pStringListLHS, pStringListRHS->pStringList[dwIndex], bCaseSensitive))
        {
            dwMatching++;
        }
    }

    return (dwMatching == pStringListLHS->dwCount);
}

//
// Give the limited user delete access to the user.
//
DWORD
TestModifyUserAcl(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserDN,
    PCSTR pszPermission
    )
{
    DWORD dwError = 0;
    PSTR pszSddlString = NULL;
    PSTR pszNewSddlString = NULL;
    PSTR pszLimitedUserSid = NULL;
    PSTR ppszAttributeValues[] = { NULL, NULL };

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszUserDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                ATTR_ACL_STRING,
                &pszSddlString);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetUserSid(pState, VmDirTestGetInternalUserCn(pState), NULL, &pszLimitedUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszNewSddlString,
                "%s(A;;%s;;;%s)",
                pszSddlString,
                pszPermission,
                pszLimitedUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszAttributeValues[0] = pszNewSddlString;
    dwError = VmDirTestReplaceAttributeValues(
                pState->pLd,
                pszUserDN,
                ATTR_ACL_STRING,
                (PCSTR*)ppszAttributeValues);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSddlString);
    VMDIR_SAFE_FREE_STRINGA(pszNewSddlString);
    VMDIR_SAFE_FREE_STRINGA(pszLimitedUserSid);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestCreateGroups(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST *ppvslGroupDNs
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDIR_STRING_LIST pvslGroups = NULL;
    PSTR pszGroupDN = NULL;

    dwError = VmDirStringListInitialize(&pvslGroups, VMDIR_ARRAY_SIZE(ppszGroups));
    BAIL_ON_VMDIR_ERROR(dwError)

    for (dwIndex = 0; dwIndex < VMDIR_ARRAY_SIZE(ppszGroups); ++dwIndex)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszGroupDN,
                    "cn=%s,cn=%s,%s",
                    ppszGroups[dwIndex],
                    pState->pszTestContainerName,
                    pState->pszBaseDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pvslGroups, pszGroupDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszGroupDN = NULL;

        dwError = VmDirTestCreateGroup(pState, pState->pszTestContainerName, ppszGroups[dwIndex], NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppvslGroupDNs = pvslGroups;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszGroupDN);
    return dwError;
error:
    VmDirStringListFree(pvslGroups);
    goto cleanup;
}

DWORD
TestCleanupGroups(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pvslGroupDNs
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;

    for (dwIndex = 0; dwIndex < pvslGroupDNs->dwCount; ++dwIndex)
    {
        dwError = ldap_delete_ext_s(pState->pLd, pvslGroupDNs->pStringList[dwIndex], NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
TestAddingUserToGroups(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserDN,
    PVMDIR_STRING_LIST pvslGroupDNs
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDIR_STRING_LIST pvsUsersGroups = NULL;
    BOOLEAN bIsListEqual = FALSE;

    for (dwIndex = 0; dwIndex < pvslGroupDNs->dwCount; ++dwIndex)
    {
        dwError = VmDirTestAddUserToGroup(
                    pState->pLd,
                    pszUserDN,
                    pvslGroupDNs->pStringList[dwIndex]);
    }

    dwError = VmDirTestListUsersGroups(pState->pLd, pszUserDN, &pvsUsersGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    bIsListEqual = VmDirStringListEqualsNoOrder(pvsUsersGroups, pvslGroupDNs, FALSE);
    TestAssert(bIsListEqual);

cleanup:
    VmDirStringListFree(pvsUsersGroups);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestExplicitlyRemovingUserFromGroups(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserDN,
    PVMDIR_STRING_LIST pvslGroupDNs
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDIR_STRING_LIST pvsUsersGroups = NULL;

    for (dwIndex = 0; dwIndex < pvslGroupDNs->dwCount; ++dwIndex)
    {
        //
        // We neeed to have the write-property privilege to remove the user from
        // a group.
        //
        dwError = TestModifyUserAcl(pState, pvslGroupDNs->pStringList[dwIndex], "WP");
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError  = VmDirTestRemoveUserFromGroup(
                    pState->pLdLimited,
                    pszUserDN,
                    pvslGroupDNs->pStringList[dwIndex]);
        TestAssertEquals(dwError, 0);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirTestListUsersGroups(pState->pLd, pszUserDN, &pvsUsersGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssert(pvsUsersGroups->dwCount == 0);

cleanup:
    VmDirStringListFree(pvsUsersGroups);
    return dwError;
error:
    goto cleanup;
}

DWORD TestImplicitlyRemovingUserFromGroups(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserDN,
    PVMDIR_STRING_LIST pvslGroupDNs
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsMember = FALSE;
    DWORD dwIndex = 0;
    PVMDIR_STRING_LIST pvsGroupMembers = NULL;

    dwError = ldap_delete_ext_s(pState->pLdLimited, pszUserDN, NULL, NULL);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pvslGroupDNs->dwCount; ++dwIndex)
    {
        dwError = VmDirTestListGroupMembers(
                    pState->pLd,
                    pvslGroupDNs->pStringList[dwIndex],
                    &pvsGroupMembers);
        BAIL_ON_VMDIR_ERROR(dwError);

        bIsMember = VmDirStringListContainsEx(pvsGroupMembers, pszUserDN, FALSE);
        TestAssert(!bIsMember);
        VmDirStringListFree(pvsGroupMembers);
        pvsGroupMembers = NULL;
    }

cleanup:
    VmDirStringListFree(pvsGroupMembers);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestDeletionFailureDoesntTouchGroups(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserDN,
    PVMDIR_STRING_LIST pvslGroupDNs
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pvsUsersGroups = NULL;
    BOOLEAN bEquals = FALSE;

    dwError = ldap_delete_ext_s(pState->pLdLimited, pszUserDN, NULL, NULL);
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    dwError = VmDirTestListUsersGroups(pState->pLd, pszUserDN, &pvsUsersGroups);
    TestAssertEquals(dwError, VMDIR_SUCCESS);
    BAIL_ON_VMDIR_ERROR(dwError);

    bEquals = VmDirStringListEqualsNoOrder(pvsUsersGroups, pvslGroupDNs, FALSE);
    TestAssert(bEquals);

cleanup:
    VmDirStringListFree(pvsUsersGroups);
    return dwError;
error:
    goto cleanup;
}

//
// Verify that a user can be:
// (1) Added to groups
// (2) If we try to delete a user that we don't have permission to delete
//     the user's group membership is untouched.
// (3) Explicitly removed from groups
// (4) When a user is deleted they're properly removed from their groups.
//
DWORD
TestGroupMembership(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pvslGroupDNs = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserDN = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pState->pszTestContainerName, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDN,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pState->pszTestContainerName,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCreateGroups(pState, &pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestAddingUserToGroups(pState, pszUserDN, pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestDeletionFailureDoesntTouchGroups(pState, pszUserDN, pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestModifyUserAcl(pState, pszUserDN, "SD");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestImplicitlyRemovingUserFromGroups(pState, pszUserDN, pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // TestImplicitlyRemovingUserFromGroups deletes the user so we have to
    // re-create it and re-add it to the groups.
    //
    dwError = VmDirTestCreateUser(pState, pState->pszTestContainerName, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = TestAddingUserToGroups(pState, pszUserDN, pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestExplicitlyRemovingUserFromGroups(pState, pszUserDN, pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCleanupGroups(pState, pvslGroupDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszUserDN);
    VmDirStringListFree(pvslGroupDNs);
    return dwError;
error:
    goto cleanup;
}
