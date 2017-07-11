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
CleanupDefaultSecurityDescriptorTestData(
    PVMDIR_TEST_STATE   pState
    )
{
    // delete default SD for residentialperson
    (VOID)VmDirTestReplaceAttributeValues(
            pState->pLd,
            "cn=residentialperson,cn=schemacontext",
            "defaultsecuritydescriptor",
            NULL);

    // delete test computers
    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=testcomputer001,ou=Computers");

    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=testcomputer002,ou=Computers");

    // delete test cert auths
    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=testcert001,cn=Certificate-Authorities,cn=Configuration");

    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=testcert002,cn=Certificate-Authorities,cn=Configuration");

    // delete test users
    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=testuser001,cn=users");

    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=testuser002,cn=users");

    // delete test residential persons
    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=residentialperson001,cn=users");

    (VOID)VmDirTestDeleteObjectByDNPrefix(
            pState, "cn=residentialperson002,cn=users");

    return 0;
}

DWORD
InitializeDefaultSecurityDescriptorTestData(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    PCSTR   pszDefaultSD[] = { "D:(A;;RP;;;S-1-0-0-545)", NULL };

    pState->pfnCleanupCallback = CleanupDefaultSecurityDescriptorTestData;

    // set default SD for residentialperson - read prop only
    dwError = VmDirTestReplaceAttributeValues(
            pState->pLd,
            "cn=residentialperson,cn=schemacontext",
            "defaultsecuritydescriptor",
            pszDefaultSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create test computers
    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=testcomputer001,ou=Computers", "computer");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=testcomputer002,ou=Computers", "computer");
    BAIL_ON_VMDIR_ERROR(dwError);

    // create test cert auths
    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=testcert001,cn=Certificate-Authorities,cn=Configuration", "vmwcertificationauthority");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=testcert002,cn=Certificate-Authorities,cn=Configuration", "vmwcertificationauthority");
    BAIL_ON_VMDIR_ERROR(dwError);

    // create test users
    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=testuser001,cn=users", "user");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=testuser002,cn=users", "user");
    BAIL_ON_VMDIR_ERROR(dwError);

    // create test residential persons
    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=residentialperson001,cn=users", "residentialperson");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateObjectByDNPrefix(
            pState, "cn=residentialperson002,cn=users", "residentialperson");
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
ReadDefaultSecurityDescriptors(
    LDAP*   pLd
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszDN = NULL;
    PVMDIR_STRING_LIST  pClasses = NULL;

    dwError = VmDirTestGetObjectList(
            pLd,
            "cn=schemacontext",
            "(defaultsecuritydescriptor=*)",
            &pClasses);
    BAIL_ON_VMDIR_ERROR(dwError);

    // expecting 4 classes
    dwError = pClasses->dwCount == 0 ? VMDIR_ERROR_INSUFFICIENT_ACCESS : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pClasses->dwCount != 4 ? VMDIR_ERROR_INVALID_STATE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    // cn=group,cn=schemacontext
    // cn=computer,cn=schemacontext
    // cn=vmwcertificationauthority,cn=schemacontext
    // cn=residentialperson,cn=schemacontext (custom)
    for (i = 0; i < pClasses->dwCount; i++)
    {
        pszDN = pClasses->pStringList[i];
        if (!VmDirStringStartsWith(pszDN, "cn=group,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=computer,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=vmwcertificationauthority,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=residentialperson,", FALSE))
        {
            dwError = VMDIR_ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VmDirStringListFree(pClasses);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadGroupEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszDN = NULL;
    PSTR    pszSecDesc = NULL;
    PVMDIR_STRING_LIST  pEntries = NULL;

    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=group)",
            &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    // expecting 5 entries
    dwError = pEntries->dwCount == 0 ? VMDIR_ERROR_INSUFFICIENT_ACCESS : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pEntries->dwCount != 5 ? VMDIR_ERROR_INVALID_STATE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    // cn=Users,cn=Builtin,...
    // cn=Administrators,cn=Builtin,...
    // cn=DCAdmins,cn=Builtin,...
    // cn=DCClients,cn=Builtin,...
    // cn=CAAdmins,cn=Builtin,...
    for (i = 0; i < pEntries->dwCount; i++)
    {
        pszDN = pEntries->pStringList[i];
        if (!VmDirStringStartsWith(pszDN, "cn=Users,cn=Builtin,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=Administrators,cn=Builtin,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=DCAdmins,cn=Builtin,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=DCClients,cn=Builtin,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=CAAdmins,cn=Builtin,", FALSE))
        {
            dwError = VMDIR_ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // read security descriptor
        VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
        dwError = VmDirTestGetAttributeValueString(
                pLd,
                pszDN,
                LDAP_SCOPE_BASE,
                NULL,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                &pszSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
    VmDirStringListFree(pEntries);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadComputerEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszDN = NULL;
    PSTR    pszDCContainer = NULL;
    PSTR    pszSecDesc = NULL;
    PVMDIR_STRING_LIST  pEntries = NULL;

    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            // ignore msDS-ManagedServiceAccount objects
            "(&(objectclass=computer)(!(objectclass=msDS-ManagedServiceAccount)))",
            &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    // expecting 3 entries
    dwError = pEntries->dwCount == 0 ? VMDIR_ERROR_INSUFFICIENT_ACCESS : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pEntries->dwCount != 3 ? VMDIR_ERROR_INVALID_STATE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDCContainer,
            "ou=Domain Controllers,%s",
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // ...,ou=Domain Controllers,...
    // cn=testcomputer001,ou=Computers,...
    // cn=testcomputer002,ou=Computers,...
    for (i = 0; i < pEntries->dwCount; i++)
    {
        pszDN = pEntries->pStringList[i];
        if (!VmDirStringEndsWith(pszDN, pszDCContainer, FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=testcomputer001,ou=Computers,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=testcomputer002,ou=Computers,", FALSE))
        {
            dwError = VMDIR_ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // read security descriptor
        VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
        dwError = VmDirTestGetAttributeValueString(
                pLd,
                pszDN,
                LDAP_SCOPE_BASE,
                NULL,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                &pszSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDCContainer);
    VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
    VmDirStringListFree(pEntries);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadCertAuthEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszDN = NULL;
    PSTR    pszSecDesc = NULL;
    PVMDIR_STRING_LIST  pEntries = NULL;

    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=vmwcertificationauthority)",
            &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    // expecting 2 entry
    dwError = pEntries->dwCount == 0 ? VMDIR_ERROR_INSUFFICIENT_ACCESS : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pEntries->dwCount != 2 ? VMDIR_ERROR_INVALID_STATE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    // cn=testcert001,cn=Certificate-Authorities,cn=Configuration,...
    // cn=testcert002,cn=Certificate-Authorities,cn=Configuration,...
    for (i = 0; i < pEntries->dwCount; i++)
    {
        pszDN = pEntries->pStringList[i];
        if (!VmDirStringStartsWith(pszDN, "cn=testcert001,cn=Certificate-Authorities,cn=Configuration,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=testcert002,cn=Certificate-Authorities,cn=Configuration,", FALSE))
        {
            dwError = VMDIR_ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // read security descriptor
        VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
        dwError = VmDirTestGetAttributeValueString(
                pLd,
                pszDN,
                LDAP_SCOPE_BASE,
                NULL,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                &pszSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
    VmDirStringListFree(pEntries);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadUserEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszDN = NULL;
    PSTR    pszFilter = NULL;
    PSTR    pszSecDesc = NULL;
    PVMDIR_STRING_LIST  pEntries = NULL;

    // exclude test internal user and computers from search
    dwError = VmDirAllocateStringPrintf(
            &pszFilter,
            "(&(&(objectclass=user)(!(objectclass=computer)))(!(cn=%s)))",
            VmDirTestGetInternalUserCn(pState));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetObjectList(
            pLd, pState->pszBaseDN, pszFilter, &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    // expecting 5 entry
    dwError = pEntries->dwCount == 0 ? VMDIR_ERROR_INSUFFICIENT_ACCESS : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pEntries->dwCount != 5 ? VMDIR_ERROR_INVALID_STATE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    // cn=Administrator,cn=users,...
    // cn=krbtgt/VMWARE.COM,cn=users,...
    // cn=K/M,cn=users,...
    // cn=testuser001,cn=users,...
    // cn=testuser002,cn=users,...
    for (i = 0; i < pEntries->dwCount; i++)
    {
        pszDN = pEntries->pStringList[i];
        if (!VmDirStringStartsWith(pszDN, "cn=Administrator,cn=users,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=krbtgt/VMWARE.COM,cn=users,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=K/M,cn=users,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=testuser001,cn=users,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=testuser002,cn=users,", FALSE))
        {
            dwError = VMDIR_ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // read security descriptor
        VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
        dwError = VmDirTestGetAttributeValueString(
                pLd,
                pszDN,
                LDAP_SCOPE_BASE,
                NULL,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                &pszSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
    VMDIR_SAFE_FREE_STRINGA(pszFilter);
    VmDirStringListFree(pEntries);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadResidentialPersonEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszDN = NULL;
    PSTR    pszSecDesc = NULL;
    PVMDIR_STRING_LIST  pEntries = NULL;

    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=residentialperson)",
            &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    // expecting 2 entry
    dwError = pEntries->dwCount == 0 ? VMDIR_ERROR_INSUFFICIENT_ACCESS : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pEntries->dwCount != 2 ? VMDIR_ERROR_INVALID_STATE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    // cn=residentialperson001,cn=users,...
    // cn=residentialperson002,cn=users,...
    for (i = 0; i < pEntries->dwCount; i++)
    {
        pszDN = pEntries->pStringList[i];
        if (!VmDirStringStartsWith(pszDN, "cn=residentialperson001,cn=users,", FALSE) &&
            !VmDirStringStartsWith(pszDN, "cn=residentialperson002,cn=users,", FALSE))
        {
            dwError = VMDIR_ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // read security descriptor - should fail for non-admin
        VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
        dwError = VmDirTestGetAttributeValueString(
                pLd,
                pszDN,
                LDAP_SCOPE_BASE,
                NULL,
                ATTR_OBJECT_SECURITY_DESCRIPTOR,
                &pszSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszSecDesc);
    VmDirStringListFree(pEntries);
    return dwError;

error:
    goto cleanup;
}

DWORD
TestSystemDefaultSecurityDescriptors(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    // default security descriptors should be readable by administrator
    dwError = ReadDefaultSecurityDescriptors(pState->pLd);
    TestAssertEquals(dwError, 0);

    // default security descriptors should be readable by authenticated users
    dwError = ReadDefaultSecurityDescriptors(pState->pLdLimited);
    TestAssertEquals(dwError, 0);

    // default security descriptors should be readable by anonymous users
    dwError = ReadDefaultSecurityDescriptors(pState->pLdAnonymous);
    TestAssertEquals(dwError, 0);

    // pass all tests, return 0
    return 0;
}

DWORD
TestGroupEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    // group entries should be readable by administrator
    dwError = ReadGroupEntries(pState, pState->pLd);
    TestAssertEquals(dwError, 0);

    // group entries should be readable by authenticated users
    dwError = ReadGroupEntries(pState, pState->pLdLimited);
    TestAssertEquals(dwError, 0);

    // group entries should be NOT readable by anonymous users
    dwError = ReadGroupEntries(pState, pState->pLdAnonymous);
    TestAssertEquals(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    return 0;
}

DWORD
TestComputerEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    // computer entries should be readable by administrator
    dwError = ReadComputerEntries(pState, pState->pLd);
    TestAssertEquals(dwError, 0);

    // computer entries should be readable by authenticated users
    dwError = ReadComputerEntries(pState, pState->pLdLimited);
    TestAssertEquals(dwError, 0);

    // computer entries should be NOT readable by anonymous users
    dwError = ReadComputerEntries(pState, pState->pLdAnonymous);
    TestAssertEquals(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    return 0;
}

DWORD
TestCertAuthEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    // cert auth entries should be readable by administrator
    dwError = ReadCertAuthEntries(pState, pState->pLd);
    TestAssertEquals(dwError, 0);

    // cert auth entries should be readable by authenticated users
    dwError = ReadCertAuthEntries(pState, pState->pLdLimited);
    TestAssertEquals(dwError, 0);

    // cert auth entries should be NOT readable by anonymous users
    dwError = ReadCertAuthEntries(pState, pState->pLdAnonymous);
    TestAssertEquals(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    return 0;
}

DWORD
TestUserEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    // user entries should be readable by administrator
    dwError = ReadUserEntries(pState, pState->pLd);
    TestAssertEquals(dwError, 0);

    // user entries should be NOT readable by authenticated users
    dwError = ReadUserEntries(pState, pState->pLdLimited);
    TestAssertEquals(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);

    // user entries should be NOT readable by anonymous users
    dwError = ReadUserEntries(pState, pState->pLdAnonymous);
    TestAssertEquals(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    return 0;
}

DWORD
TestResidentialPersonEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    // residential person entries should be readable by administrator
    dwError = ReadResidentialPersonEntries(pState, pState->pLd);
    TestAssertEquals(dwError, 0);

    // residential person entries should be readable by authenticated users, but NOT SD
    dwError = ReadResidentialPersonEntries(pState, pState->pLdLimited);
    TestAssertEquals(dwError, ERROR_INVALID_STATE);

    // residential person entries should NOT be readable by anonymous users
    dwError = ReadResidentialPersonEntries(pState, pState->pLdAnonymous);
    TestAssertEquals(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);

    // pass all tests, return 0
    return 0;
}

DWORD
TestDefaultSecurityDescriptor(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;

    dwError = InitializeDefaultSecurityDescriptorTestData(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestSystemDefaultSecurityDescriptors(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestGroupEntriesSD(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestComputerEntriesSD(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCertAuthEntriesSD(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestUserEntriesSD(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestResidentialPersonEntriesSD(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = CleanupDefaultSecurityDescriptorTestData(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    TestAssertEquals(dwError, 0);
    goto cleanup;
}
