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

VOID
_VdcSendRandomAclStrings(
    PVMDIR_TEST_STATE pState
    )
{
    PCSTR pszBaseDN = pState->pszBaseDN;
    DWORD dwError = 0;
    int i = 0;
    PCSTR ppszRandomStrings[] = {
        "abc",
        "xyz",
        "hello, world!",
        "()"
    };

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszRandomStrings); ++i)
    {
        dwError = VmDirTestReplaceBinaryAttributeValues(
                    pState->pLd,
                    pszBaseDN,
                    ATTR_OBJECT_SECURITY_DESCRIPTOR,
                    (PBYTE)ppszRandomStrings[i],
                    strlen(ppszRandomStrings[i]));
       TestAssert(dwError == LDAP_INVALID_SYNTAX);
    }
}

DWORD
_VdcPermuteSecurityDescriptor(
    PVMDIR_TEST_STATE pState,
    const BYTE *pbOriginalSecurityDescriptor,
    DWORD dwSDLength
    )
{
    PCSTR pszBaseDN = pState->pszBaseDN;
    BYTE bOldValue = 0;
    int i = 0;
    DWORD dwError = 0;
    BYTE *pbSecDescriptor = NULL;
    BOOLEAN bReturn = FALSE;
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor = NULL;

    dwError = VmDirAllocateAndCopyMemory(
                (PVOID)pbOriginalSecurityDescriptor,
                dwSDLength,
                (PVOID*)&pbSecDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    srand(time(NULL));

    for (i = 0; i < 100; ++i)
    {
        bOldValue = pbSecDescriptor[i];
        pbSecDescriptor[i] = rand() % 256;

        dwError = VmDirTestReplaceBinaryAttributeValues(
                    pState->pLd,
                    pszBaseDN,
                    ATTR_OBJECT_SECURITY_DESCRIPTOR,
                    pbSecDescriptor,
                    dwSDLength);
        TestAssert(dwError == 0 || dwError == LDAP_INVALID_SYNTAX);

        pSecurityDescriptor = (PSECURITY_DESCRIPTOR_RELATIVE)pbSecDescriptor;
        bReturn = RtlValidRelativeSecurityDescriptor(
                    pSecurityDescriptor,
                    dwSDLength,
                    OWNER_SECURITY_INFORMATION |
                        GROUP_SECURITY_INFORMATION |
                        DACL_SECURITY_INFORMATION);
        //
        // RtlValidRelativeSecurityDescriptor should return false unless
        // the VmDirTestReplaceBinaryAttributeValues call succeeded (absent networking
        // or memory issues, which aren't relevant in the controlled
        // environment of this test).
        //
        TestAssert(bReturn == FALSE || dwError == 0);
        pbSecDescriptor[i] = bOldValue;
    }

    //
    // Clear out any error as if it were significant it would have been
    // caught and handled above.
    //
    dwError = 0;

cleanup:
    VmDirFreeMemory(pbSecDescriptor);

    return dwError;

error:
    goto cleanup;
}

VOID
AllObjectsShouldHaveASecurityDescriptor(
    PVMDIR_TEST_STATE pState
    )
{
    PCSTR pszBaseDN = pState->pszBaseDN;
    LDAPMessage *pSearchRes = NULL;
    PCSTR pszSearchFilter = "(!(nTSecurityDescriptor=*))";
    DWORD dwError = 0;
    DWORD dwCount = 0;

    dwError = ldap_search_ext_s(
                pState->pLd,
                pszBaseDN,
                LDAP_SCOPE_SUBTREE,
                pszSearchFilter,
                NULL,
                TRUE,
                NULL,
                NULL,
                NULL,
                0,
                &pSearchRes);
    TestAssert(dwError == 0);

    dwCount = ldap_count_entries(pState->pLd, pSearchRes);
    TestAssertMsg(
        dwCount == 0,
        "Found some entries that don't have security descriptors!");

    ldap_msgfree(pSearchRes);
}

VOID
SecurityDescriptorsShouldntBeDeletable(
    PVMDIR_TEST_STATE pState
    )
{
    PCSTR pszBaseDN = pState->pszBaseDN;
    PSTR pszAdminDN = NULL;
    DWORD dwError = 0;
    LDAPMod mod = {0};
    LDAPMod *mods[2] = {&mod, NULL};

    dwError = VmDirAllocateStringPrintf(
                &pszAdminDN,
                "cn=Administrator,cn=Users,%s",
                pszBaseDN);
    TestAssert(dwError == 0);

    mod.mod_op = LDAP_MOD_DELETE;
    mod.mod_type = ATTR_OBJECT_SECURITY_DESCRIPTOR;

    dwError = ldap_modify_ext_s(pState->pLd, pszAdminDN, mods, NULL, NULL);
    TestAssertMsg(
        dwError == LDAP_CONSTRAINT_VIOLATION,
        "Security descriptors shouldn't be deletable");

    mod.mod_op = LDAP_MOD_DELETE;
    mod.mod_type = ATTR_ACL_STRING;

    //
    // TODO -- right now this returns success (but doesn't delete the
    // ntSecurityDescriptor attribute or anything). Conceptually, deleting
    // a computed attribute should return an error (right?).
    //
#if 0

    dwError = ldap_modify_ext_s(pState->pLd, pszAdminDN, mods, NULL, NULL);
printf("dwError ==> %d\n", dwError);
    TestAssertMsg(
        dwError == LDAP_CONSTRAINT_VIOLATION,
        "Security descriptors shouldn't be deletable via vmwAclString");
#endif

    VMDIR_SAFE_FREE_STRINGA(pszAdminDN);
}

VOID
TestAclReadPermissionRespected(
    PVMDIR_TEST_STATE pState
    )
{
    PSTR pszUserContainerDN = NULL;
    PSTR pszContainerSD = NULL;
    DWORD dwError = 0;

    dwError = VmDirTestGetTestContainerDn(pState, &pszUserContainerDN);
    TestAssert(dwError == 0);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszUserContainerDN,
                LDAP_SCOPE_BASE,
                "(cn=Users)",
                ATTR_ACL_STRING,
                &pszContainerSD);
    TestAssert(dwError == ERROR_INVALID_STATE); // TODO -- Get this to translate to VMDIR_ERROR_ACCESS_DENIED

    VMDIR_SAFE_FREE_STRINGA(pszUserContainerDN);
    VMDIR_SAFE_FREE_STRINGA(pszContainerSD);
}

VOID
TestAclWritePermissionRespected(
    PVMDIR_TEST_STATE pState
    )
{
    PSTR pszContainerSD = NULL;
    PSTR pszUserContainerDN = NULL;
    DWORD dwError = 0;

    dwError = VmDirTestGetTestContainerDn(pState, &pszUserContainerDN);
    TestAssert(dwError == 0);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszUserContainerDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                ATTR_ACL_STRING,
                &pszContainerSD);
    TestAssert(dwError == 0);

    //
    // Try to set the security descriptor as a limited user. This should fail
    // as we don't have write access to the ACL.
    //
    dwError = VmDirTestReplaceBinaryAttributeValues(
                pState->pLdLimited,
                pszUserContainerDN,
                ATTR_ACL_STRING,
                pszContainerSD,
                strlen(pszContainerSD));
    TestAssert(dwError == LDAP_INSUFFICIENT_ACCESS);

    //
    // Now make sure the write works as administrator.
    //
    dwError = VmDirTestReplaceBinaryAttributeValues(
                pState->pLd,
                pszUserContainerDN,
                ATTR_ACL_STRING,
                pszContainerSD,
                strlen(pszContainerSD));
    TestAssert(dwError == 0);

    VMDIR_SAFE_FREE_STRINGA(pszUserContainerDN);
    VMDIR_SAFE_FREE_STRINGA(pszContainerSD);
}

VOID
CreateNewUserWithSecurityDescriptor(
    PVMDIR_TEST_STATE pState,
    PBYTE pbSecurityDescriptor,
    DWORD dwLength
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestCreateUserWithSecurityDescriptor(
                pState,
                "testcontainer", // TODO -- Query test container
                pszUserName,
                pbSecurityDescriptor,
                dwLength);
    TestAssert(dwError == 0);

    dwError = VmDirTestDeleteUser(pState, "testcontainer", pszUserName);
    TestAssert(dwError == 0);

    VMDIR_SAFE_FREE_STRINGA(pszUserName);
}


//
// NB -- We don't try to clean up (delete) the class that we create because
// objects under "cn=schemacontext" can't be deleted.
//
DWORD
TestClassBasedAclCode(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszClassName = NULL;
    PSTR pszObjectName = NULL;
    PSTR pszObjectDn = NULL;

    dwError = VmDirTestGetGuid(&pszObjectName);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestGetGuid(&pszClassName);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestCreateClass(pState, pszClassName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateObject(pState, "testcontainer"/*TODO*/, pszClassName, pszObjectName);
    printf("create object ==> %d\n", dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO - Check ACL
cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszObjectDn);

    // TODO -- Routine that does the printf and delete
    dwError = VmDirAllocateStringPrintf(&pszObjectDn, "cn=%s,cn=testcontainer,%s", pState->pszBaseDN);
    ldap_delete_ext_s(pState->pLd, pszObjectName, NULL, NULL);
    VMDIR_SAFE_FREE_STRINGA(pszObjectDn);
    return dwError;
error:
    goto cleanup;
}

VOID
CreateNewUserWithSddlString(
    PVMDIR_TEST_STATE pState,
    PCSTR pszSddlString
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssert(dwError == 0); // TODO

    // TODO -- container name
    dwError = VmDirTestCreateUser(pState, "testcontainer", pszUserName, pszSddlString);
    TestAssert(dwError == 0);

    dwError = VmDirTestDeleteUser(pState, "testcontainer", pszUserName);
    TestAssert(dwError == 0);
}

//
// All objects should be owned by the administrator account, even if they're
// created with a different account.
//
DWORD
TestOwnerAndGroupInformation(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszAcl = NULL;
    PSTR pszDomainSid = NULL;
    PSTR pszOwnerAndGroup = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssert(dwError == 0); // TODO
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO -- container name
    dwError = VmDirTestCreateUserWithLimitedAccount(pState, "testcontainer", pszUserName, NULL);
    TestAssert(dwError == 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pState->pszBaseDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                ATTR_ACL_STRING,
                &pszAcl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    TestAssert(dwError == 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszOwnerAndGroup,
                "O:%s-500G:BA",
                pszDomainSid);
    TestAssert(dwError == 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringStrA(pszAcl, pszOwnerAndGroup) == NULL)
    {
        TestAssert(false);
    }

cleanup:
    dwError = VmDirTestDeleteUser(pState, "testcontainer", pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_SAFE_FREE_STRINGA(pszOwnerAndGroup);
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    return dwError;
error:
    TestAssert(false);
    goto cleanup;
}

//
// Security principals (users and groups) should be able to read and write
// their own properties.
// TODO: Test groups
// TODO: Reset SN after modifying?
//
DWORD
SecurityPrincipalsShouldBeAbleToReadWriteSelf(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszUserDn = NULL;
    PSTR pszAttribute = NULL;
    PCSTR ppszVals[] = { NULL, NULL };

    dwError = VmDirTestGetInternalUserDn(pState, &pszUserDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLdLimited,
                pszUserDn,
                LDAP_SCOPE_BASE,
                "(objectClass=User)",
                ATTR_SN,
                &pszAttribute);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszVals[0] = "New SN";
    dwError = VmDirTestReplaceAttributeValues(
                pState->pLdLimited,
                pszUserDn,
                ATTR_SN,
                ppszVals);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLdLimited,
                pszUserDn,
                LDAP_SCOPE_BASE,
                "(objectClass=User)",
                ATTR_SN,
                &pszAttribute);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAttribute);
    return dwError;
error:
    TestAssert(dwError == 0);
    goto cleanup;
}

#if 0
DWORD _GetBuiltinGroupSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszGroupCn,
    PSTR *ppszGroupSid
    )
{
    PSTR pszGroupDn = NULL;
    PSTR pszGroupSid = NULL;
    DWORD dwError = 0;

    dwError = VmDirAllocateStringPrintf(
                &pszGroupDn,
                "cn=%s,cn=Builtin,%s",
                pszGroupCn,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszGroupDn,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                "objectSid",
                &pszGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGroupSid = pszGroupSid;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszGroupDn);
    return dwError;
error:
    goto cleanup;
}
#endif

DWORD
TestDeleteObjectPermission(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    //
    // TODO -- Creating a second user is currently easier than modifying the ACL
    // on the existing user.
    //
    PSTR pszUserName2 = NULL;
    PSTR pszLimitedUserSid = NULL;
    PSTR pszAclString = NULL;
#if 0
    PSTR pszContainerName = NULL;

    dwError = VmDirTestGetGuid(&pszContainerName); // TODO -- We need to be freeing these strings.
    TestAssert(dwError == 0); // TODO
#endif
    BAIL_ON_VMDIR_ERROR(dwError); // TODO

    dwError = VmDirTestGetGuid(&pszUserName);
    dwError = VmDirTestGetGuid(&pszUserName2);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestCreateUser(pState, "testcontainer", pszUserName, NULL);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestDeleteUserEx(pState, "testcontainer", pszUserName, TRUE);
    TestAssert(dwError == LDAP_INSUFFICIENT_ACCESS);

    dwError = VmDirTestGetUserSid(pState, VmDirTestGetInternalUserCn(pState), NULL, &pszLimitedUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszAclString,
                "O:S-1-7-21-2091630953-806373015-2991373445-997581456-500G:BAD:(A;;SD;;;%s)",
                pszLimitedUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, "testcontainer", pszUserName2, pszAclString);
    TestAssert(dwError == 0); // TODO

    dwError = VmDirTestDeleteUserEx(pState, "testcontainer", pszUserName2, TRUE);
    TestAssert(dwError == 0);

cleanup:
    VmDirTestDeleteUser(pState, "testcontainer", pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszLimitedUserSid);
    return dwError;
error:
    TestAssert(dwError == 0);
    goto cleanup;
}

#if 0
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

#if 1 // TODO
#define VMDIR_DOMAIN_USER_RID_ADMIN      500 // Administrator user
#define VMDIR_DOMAIN_ADMINS_RID          512 // Domain Admins group
#define VMDIR_DOMAIN_CLIENTS_RID         513 // Domain Users group
#define VMDIR_DOMAIN_ALIAS_RID_ADMINS    544 // BUILTIN\Administrators group
#define VMDIR_DOMAIN_ALIAS_RID_USERS     545 // BUILTIN\Users group
#endif

//
// TODO -- Do per tenant
//
DWORD
TestWellknownSids(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszObjectSid = NULL;
    PSTR pszDomainSid = NULL;

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    TestAssert(dwError == 0);

    dwError = VmDirTestGetUserSid(pState, "administrator", NULL, &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, VMDIR_DOMAIN_USER_RID_ADMIN));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "DCAdmins", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, VMDIR_DOMAIN_ADMINS_RID));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "DCClients", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, VMDIR_DOMAIN_CLIENTS_RID));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "Administrators", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, VMDIR_DOMAIN_ALIAS_RID_ADMINS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetBuiltinGroupSid(pState, "Users", &pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);
    TestAssert(CompareWellknownSid(pState, pszObjectSid, pszDomainSid, VMDIR_DOMAIN_ALIAS_RID_USERS));
    VMDIR_SAFE_FREE_STRINGA(pszObjectSid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    return dwError;
error:
    goto cleanup;
}
#endif

DWORD
TestDomainAdminPrivileges(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;
    PSTR pszGroupDn = NULL;
    LDAP *pLd = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssert(dwError == 0);

    dwError = VmDirTestCreateUser(pState, "testcontainer", pszUserName, NULL);
    TestAssert(dwError == 0);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=testcontainer,%s",
                pszUserName,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszGroupDn,
                "cn=DCAdmins,cn=Builtin,%s",
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcConnectionFromUser(pState, pszUserName, &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

#if 0 // TODO -- This isn't returning an error for this case.
    dwError = VmDirTestGetObjectList(pLd, pState->pszBaseDN, NULL);
    printf("ldapgetobjectlist returned %d\n", dwError);
    TestAssert(dwError != 0);
#endif
    VmDirTestLdapUnbind(pLd); pLd = NULL;

    dwError = VmDirTestAddUserToGroup(pState->pLd, pszUserDn, pszGroupDn);
    TestAssert(dwError == 0);

    dwError = _VdcConnectionFromUser(pState, pszUserName, &pLd); // TODO -- Why are we re-opening this connection?
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetObjectList(pLd, pState->pszBaseDN, NULL);
    TestAssert(dwError == 0);
    VmDirTestLdapUnbind(pLd); pLd = NULL;

    dwError = VmDirTestRemoveUserFromGroup(pState->pLd, pszUserDn, pszGroupDn);
    TestAssert(dwError == 0);

cleanup:
    VmDirTestDeleteUser(pState, "testcontainer", pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszUserDn);
    VMDIR_SAFE_FREE_STRINGA(pszGroupDn);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestRoundTrip(
    PVMDIR_TEST_STATE pState,
    BYTE *pbSecurityDescriptor,
    DWORD dwLength
    )
{
    DWORD dwError = 0;

    //
    // First, make sure we can round-trip the current SD.
    //
    dwError = VmDirTestReplaceBinaryAttributeValues(
                pState->pLd,
                pState->pszBaseDN,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                pbSecurityDescriptor,
                dwLength);
    TestAssert(dwError == 0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

// TODO -- Test that has a class-based ACL and make sure that it's merged
//         properly with inherited ACLs.
// TODO -- Tests that make sure that group permissions work.
// TODO -- Tests that make sure that nested groups work.
// TODO -- Tests that make sure domain admins/clients privileges don't
//         extend to secondary tenants.
// TODO -- Test protected ACL flag (inherited ACEs aren't inherited).
// TODO -- Test behavior where owner doesn't have explicit ACE; do they still
//         get any access?
// TODO -- Make sure specifying an acl string and binary SD fails.
// TODO -- Test that makes sure that an ordinary user has no rights.
// TODO -- Make sure that if I create a user under cn=builtin,<domain> that I
//         can subsequently delete that user (currently will fail).
DWORD
TestSecurityDescriptors(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    BYTE *pbSecurityDescriptor = NULL;
    DWORD dwLength = 0;
    PSTR pszSddlString = NULL;

    printf("Testing security descriptor related code ...\n");

    dwError = _VdcGetObjectSecurityDescriptor(
                pState,
                pState->pszBaseDN,
                &pbSecurityDescriptor,
                &dwLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pState->pszBaseDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                ATTR_ACL_STRING,
                &pszSddlString);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestWellknownSids(pState);

    // TestCreatorGroupSid(pState);
    // TestCreatorOwnerSid(pState);

    // TODO
    // TestClassBasedAclCode(pState);

    TestDeleteObjectPermission(pState);

    TestStandardRightsForAdminUser(pState);
    TestStandardRightsForAdminGroup(pState);

    TestDomainAdminPrivileges(pState);

    SecurityPrincipalsShouldBeAbleToReadWriteSelf(pState);
    TestOwnerAndGroupInformation(pState);

    AllObjectsShouldHaveASecurityDescriptor(pState);

    SecurityDescriptorsShouldntBeDeletable(pState);

    //
    // These two calls verify that we can specify the SD descriptor (either
    // via nTSecurityDescriptor or vmwAclString) when we create an object (as
    // opposed to just being able to change it later).
    //
    // TODO -- Currently hitting some error.
    // CreateNewUserWithSecurityDescriptor(pState, pbSecurityDescriptor, dwLength);
    CreateNewUserWithSddlString(pState, pszSddlString);

    TestRoundTrip(pState, pbSecurityDescriptor, dwLength);

    //
    // Second, we send a bunch of random garbage.
    //
    _VdcSendRandomAclStrings(pState);

    dwError = _VdcPermuteSecurityDescriptor(
                pState,
                (const BYTE *)pbSecurityDescriptor,
                dwLength);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Finally, set the SD back to its initial (good) value.
    //
    dwError = VmDirTestReplaceBinaryAttributeValues(
                pState->pLd,
                pState->pszBaseDN,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                pbSecurityDescriptor,
                dwLength);
    if (dwError != 0)
    {
        printf("Failed to set SD back to original value!\n");
        BAIL_ON_VMDIR_ERROR(dwError);
    }


    TestAclReadPermissionRespected(pState);
    TestAclWritePermissionRespected(pState);

    // TODO -- Standardize output
    printf("Security descriptor tests succceeded!\n");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pbSecurityDescriptor);
    VMDIR_SAFE_FREE_STRINGA(pszSddlString);
    return dwError;

error:
    printf("Security Descriptor tests failed with error 0n%d\n", dwError);
    goto cleanup;
}
