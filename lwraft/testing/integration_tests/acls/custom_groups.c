/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
CleanupCustomSetup(
    PVMDIR_TEST_STATE   pState
    )
{
    (VOID)VmDirTestDeleteUser(pState, NULL, "non_member");

    (VOID)VmDirTestDeleteUser(pState, NULL, "c_client");

    (VOID)VmDirTestDeleteUser(pState, NULL, "c_admin");

    (VOID)VmDirTestDeleteContainer(pState, "CustomObjects");

    (VOID)VmDirTestDeleteGroup(pState, NULL, "CustomClients");

    (VOID)VmDirTestDeleteGroup(pState, NULL, "CustomAdmins");

    return 0;
}

DWORD
InitializeCustomSetup(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainSid = NULL;
    PSTR    pszAdminsSid = NULL;
    PSTR    pszClientsSid = NULL;
    PSTR    pszGroupSD = NULL;
    PSTR    pszContainerSD = NULL;

    pState->pfnCleanupCallback = CleanupCustomSetup;

    // Cleanup leftover from previous run
    dwError = CleanupCustomSetup(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Override SD to be deletable by administrator
    dwError = VmDirAllocateStringPrintf(
            &pszGroupSD, "O:BAG:BAD:(A;;RCRPWPWDSD;;;%s-500)", pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateGroup(pState, NULL, "CustomAdmins", pszGroupSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateGroup(pState, NULL, "CustomClients", pszGroupSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetGroupSid(pState, "CustomAdmins", NULL, &pszAdminsSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetGroupSid(pState, "CustomClients", NULL, &pszClientsSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Grant CustomAdmins READ+WRITE permissions and
    // grant CustomClients READ permission
    dwError = VmDirAllocateStringPrintf(
            &pszContainerSD,
            "O:BAG:BAD:(A;CIOIID;GXRCCCDCRPWP;;;%s)(A;CIOIID;RP;;;%s)",
            pszAdminsSid,
            pszClientsSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pState, "CustomObjects", pszContainerSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Create users and assign memberships
    dwError = VmDirTestCreateUser(pState, NULL, "c_admin", NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, NULL, "c_client", NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, NULL, "non_member", NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestAddUserToGroup(
            pState, "c_admin", NULL, "CustomAdmins", NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestAddUserToGroup(
            pState, "c_client", NULL, "CustomClients", NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    VMDIR_SAFE_FREE_STRINGA(pszAdminsSid);
    VMDIR_SAFE_FREE_STRINGA(pszClientsSid);
    VMDIR_SAFE_FREE_STRINGA(pszGroupSD);
    VMDIR_SAFE_FREE_STRINGA(pszContainerSD);
    return dwError;
}

DWORD
CreateCustomObject(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PCSTR   valsCn[] = { pszName, NULL };
    PCSTR   valsClass[] = { "user", NULL };
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], NULL};

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=CustomObjects,cn=%s,%s",
            pszName,
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(pState->pLdCustom, pszDN, attrs, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
DeleteCustomObject(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=CustomObjects,cn=%s,%s",
            pszName,
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_delete_ext_s(pState->pLdCustom, pszDN, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
ListCustomObjects(
    PVMDIR_TEST_STATE   pState,
    PDWORD              pdwCount
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PVMDIR_STRING_LIST  pObjects = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=CustomObjects,cn=%s,%s",
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetObjectList(pState->pLdCustom, pszDN, &pObjects);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCount = pObjects->dwCount;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    VmDirStringListFree(pObjects);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadCustomObjectProperties(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    pszCN = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=CustomObjects,cn=%s,%s",
            pszName,
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
            pState->pLdCustom, pszDN, LDAP_SCOPE_BASE, NULL, ATTR_CN, &pszCN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszCN);
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
WriteCustomObjectProperties(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PCSTR   ppszAttrVals[] = {"hello world", NULL};

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=CustomObjects,cn=%s,%s",
            pszName,
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestReplaceAttributeValues(
            pState->pLdCustom,
            pszDN,
            ATTR_DESCRIPTION,
            ppszAttrVals);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadCustomObjectSD(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    pszSD = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=CustomObjects,cn=%s,%s",
            pszName,
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
            pState->pLdCustom,
            pszDN,
            LDAP_SCOPE_BASE,
            NULL,
            ATTR_ACL_STRING,
            &pszSD);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSD);
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
WriteCustomObjectSD(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszName
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PCSTR   ppszAttrVals[] = {"O:BAG:BAD:(A;;RCRPWPWDSD;;;BA)", NULL};

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=CustomObjects,cn=%s,%s",
            pszName,
            VmDirTestGetTestContainerCn(pState),
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestReplaceAttributeValues(
            pState->pLdCustom,
            pszDN,
            ATTR_ACL_STRING,
            ppszAttrVals);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    return dwError;

error:
    goto cleanup;

}

DWORD
TestCustomAdminRights(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;

    dwError = VmDirTestConnectionFromUser(pState, "c_admin", &pState->pLdCustom);
    BAIL_ON_VMDIR_ERROR(dwError);

    // should be able to create objects
    dwError = CreateCustomObject(pState, "co-1");
    TestAssertEquals(dwError, 0);

    dwError = CreateCustomObject(pState, "co-2");
    TestAssertEquals(dwError, 0);

    dwError = CreateCustomObject(pState, "co-3");
    TestAssertEquals(dwError, 0);

    // should be able to delete objects
    dwError = DeleteCustomObject(pState, "co-3");
    TestAssertEquals(dwError, 0);

    // should be able to list objects
    dwError = ListCustomObjects(pState, &dwCount);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCount, 2);

    // should be able to read properties
    dwError = ReadCustomObjectProperties(pState, "co-1");
    TestAssertEquals(dwError, 0);

    // should be able to write properties
    dwError = WriteCustomObjectProperties(pState, "co-1");
    TestAssertEquals(dwError, 0);

    // should be able to read SD
    dwError = ReadCustomObjectSD(pState, "co-1");
    TestAssertEquals(dwError, 0);

    // should NOT be able to write SD
    dwError = WriteCustomObjectSD(pState, "co-1");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    dwError = 0;

error:
    VmDirTestLdapUnbind(pState->pLdCustom);
    pState->pLdCustom = NULL;
    return dwError;
}

DWORD
TestCustomClientRights(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;

    dwError = VmDirTestConnectionFromUser(pState, "c_client", &pState->pLdCustom);
    BAIL_ON_VMDIR_ERROR(dwError);

    // should NOT be able to create objects
    dwError = CreateCustomObject(pState, "co-4");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // should NOT be able to delete objects
    dwError = DeleteCustomObject(pState, "co-2");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // should be able to list objects
    dwError = ListCustomObjects(pState, &dwCount);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCount, 2);

    // should be able to read properties
    dwError = ReadCustomObjectProperties(pState, "co-2");
    TestAssertEquals(dwError, 0);

    // should NOT be able to write properties
    dwError = WriteCustomObjectProperties(pState, "co-2");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // should NOT be able to read SD
    dwError = ReadCustomObjectSD(pState, "co-2");
    TestAssertEquals(dwError, ERROR_INVALID_STATE);

    // should NOT be able to write SD
    dwError = WriteCustomObjectSD(pState, "co-2");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    dwError = 0;

error:
    VmDirTestLdapUnbind(pState->pLdCustom);
    pState->pLdCustom = NULL;
    return dwError;
}

DWORD
TestNonMemberRights(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;

    dwError = VmDirTestConnectionFromUser(pState, "non_member", &pState->pLdCustom);
    BAIL_ON_VMDIR_ERROR(dwError);

    // should NOT be able to create objects
    dwError = CreateCustomObject(pState, "co-4");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // should NOT be able to delete objects
    dwError = DeleteCustomObject(pState, "co-2");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // should NOT be able to list objects
    dwError = ListCustomObjects(pState, &dwCount);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCount, 0);

    // should NOT be able to read properties
    dwError = ReadCustomObjectProperties(pState, "co-2");
    TestAssertEquals(dwError, ERROR_INVALID_STATE);

    // should NOT be able to write properties
    dwError = WriteCustomObjectProperties(pState, "co-2");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // should NOT be able to read SD
    dwError = ReadCustomObjectSD(pState, "co-2");
    TestAssertEquals(dwError, ERROR_INVALID_STATE);

    // should NOT be able to write SD
    dwError = WriteCustomObjectSD(pState, "co-2");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    dwError = 0;

error:
    VmDirTestLdapUnbind(pState->pLdCustom);
    pState->pLdCustom = NULL;
    return dwError;
}

DWORD
TestCustomGroups(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    dwError = InitializeCustomSetup(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCustomAdminRights(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCustomClientRights(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestNonMemberRights(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = CleanupCustomSetup(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    TestAssertEquals(dwError, 0);
    goto cleanup;
}
