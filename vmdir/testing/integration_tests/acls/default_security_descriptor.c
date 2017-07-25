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

    // set default SD for residentialperson
    // - grant READ_PROP permission to authenticated users
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
    LDAP*   pLd,
    PDWORD  pdwCnt
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pClasses = NULL;

    // read default security descriptor
    dwError = VmDirTestGetObjectList(
            pLd,
            "cn=schemacontext",
            "(defaultsecuritydescriptor=*)",
            ATTR_DEFAULT_SECURITY_DESCRIPTOR,
            &pClasses);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCnt = pClasses->dwCount;

cleanup:
    VmDirStringListFree(pClasses);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadGroupEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd,
    PDWORD              pdwCntProp,
    PDWORD              pdwCntCtrl
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pEntriesByProp = NULL;
    PVMDIR_STRING_LIST  pEntriesByCtrl = NULL;

    // read property
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=group)",
            ATTR_OBJECT_CLASS,
            &pEntriesByProp);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntProp = pEntriesByProp->dwCount;

    // read control
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=group)",
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            &pEntriesByCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntCtrl = pEntriesByCtrl->dwCount;

cleanup:
    VmDirStringListFree(pEntriesByProp);
    VmDirStringListFree(pEntriesByCtrl);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadComputerEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd,
    PDWORD              pdwCntProp,
    PDWORD              pdwCntCtrl
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pEntriesByProp = NULL;
    PVMDIR_STRING_LIST  pEntriesByCtrl = NULL;

    // read property
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            // ignore msDS-ManagedServiceAccount objects
            "(&(objectclass=computer)(!(objectclass=msDS-ManagedServiceAccount)))",
            ATTR_OBJECT_CLASS,
            &pEntriesByProp);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntProp = pEntriesByProp->dwCount;

    // read control
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            // ignore msDS-ManagedServiceAccount objects
            "(&(objectclass=computer)(!(objectclass=msDS-ManagedServiceAccount)))",
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            &pEntriesByCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntCtrl = pEntriesByCtrl->dwCount;

cleanup:
    VmDirStringListFree(pEntriesByProp);
    VmDirStringListFree(pEntriesByCtrl);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadCertAuthEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd,
    PDWORD              pdwCntProp,
    PDWORD              pdwCntCtrl
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pEntriesByProp = NULL;
    PVMDIR_STRING_LIST  pEntriesByCtrl = NULL;

    // read property
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=vmwcertificationauthority)",
            ATTR_OBJECT_CLASS,
            &pEntriesByProp);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntProp = pEntriesByProp->dwCount;

    // read control
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=vmwcertificationauthority)",
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            &pEntriesByCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntCtrl = pEntriesByCtrl->dwCount;

cleanup:
    VmDirStringListFree(pEntriesByProp);
    VmDirStringListFree(pEntriesByCtrl);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadUserEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd,
    PDWORD              pdwCntProp,
    PDWORD              pdwCntCtrl
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pEntriesByProp = NULL;
    PVMDIR_STRING_LIST  pEntriesByCtrl = NULL;

    // read property
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            // ignore computer objects
            "(&(&(objectclass=user)(!(objectclass=computer))))",
            ATTR_OBJECT_CLASS,
            &pEntriesByProp);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntProp = pEntriesByProp->dwCount;

    // read control
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            // ignore computer objects
            "(&(&(objectclass=user)(!(objectclass=computer))))",
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            &pEntriesByCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntCtrl = pEntriesByCtrl->dwCount;

cleanup:
    VmDirStringListFree(pEntriesByProp);
    VmDirStringListFree(pEntriesByCtrl);
    return dwError;

error:
    goto cleanup;
}

DWORD
ReadResidentialPersonEntries(
    PVMDIR_TEST_STATE   pState,
    LDAP*               pLd,
    PDWORD              pdwCntProp,
    PDWORD              pdwCntCtrl
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pEntriesByProp = NULL;
    PVMDIR_STRING_LIST  pEntriesByCtrl = NULL;

    // read property
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=residentialperson)",
            ATTR_OBJECT_CLASS,
            &pEntriesByProp);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntProp = pEntriesByProp->dwCount;

    // read control
    dwError = VmDirTestGetObjectList(
            pLd,
            pState->pszBaseDN,
            "(objectclass=residentialperson)",
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            &pEntriesByCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwCntCtrl = pEntriesByCtrl->dwCount;

cleanup:
    VmDirStringListFree(pEntriesByProp);
    VmDirStringListFree(pEntriesByCtrl);
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
    DWORD   dwCnt = 0;

    // default security descriptors should be readable by administrator
    dwError = ReadDefaultSecurityDescriptors(pState->pLd, &dwCnt);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCnt, 5);

    // default security descriptors should be readable by authenticated users
    dwError = ReadDefaultSecurityDescriptors(pState->pLdLimited, &dwCnt);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCnt, 5);

    // default security descriptors should be readable by anonymous users
    dwError = ReadDefaultSecurityDescriptors(pState->pLdAnonymous, &dwCnt);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwCnt, 5);

    // pass all tests, return 0
    return 0;
}

DWORD
TestGroupEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCntProp = 0;
    DWORD   dwCntCtrl = 0;

    dwError = ReadGroupEntries(pState, pState->pLd, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // group property should be readable by administrator
    TestAssertEquals(dwCntProp, 5);
    // group control should be readable by administrator
    TestAssertEquals(dwCntCtrl, 5);

    dwError = ReadGroupEntries(pState, pState->pLdLimited, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // group property should be readable by authenticated users
    TestAssertEquals(dwCntProp, 5);
    // group control should be readable by authenticated users
    TestAssertEquals(dwCntCtrl, 5);

    dwError = ReadGroupEntries(pState, pState->pLdAnonymous, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // group property should be NOT readable by anonymous users
    TestAssertEquals(dwCntProp, 0);
    // group control should be NOT readable by anonymous users
    TestAssertEquals(dwCntCtrl, 0);

    // pass all tests, return 0
    return 0;
}

DWORD
TestComputerEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCntProp = 0;
    DWORD   dwCntCtrl = 0;

    dwError = ReadComputerEntries(pState, pState->pLd, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // computer property should be readable by administrator
    TestAssertEquals(dwCntProp, 3);
    // computer control should be readable by administrator
    TestAssertEquals(dwCntCtrl, 3);

    dwError = ReadComputerEntries(pState, pState->pLdLimited, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // computer property should be readable by authenticated users
    TestAssertEquals(dwCntProp, 3);
    // computer control should be readable by authenticated users
    TestAssertEquals(dwCntCtrl, 3);

    dwError = ReadComputerEntries(pState, pState->pLdAnonymous, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // computer property should be NOT readable by anonymous users
    TestAssertEquals(dwCntProp, 0);
    // computer control should be NOT readable by anonymous users
    TestAssertEquals(dwCntCtrl, 0);

    // pass all tests, return 0
    return 0;
}

DWORD
TestCertAuthEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCntProp = 0;
    DWORD   dwCntCtrl = 0;

    dwError = ReadCertAuthEntries(pState, pState->pLd, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // cert auth property should be readable by administrator
    TestAssertEquals(dwCntProp, 2);
    // cert auth control should be readable by administrator
    TestAssertEquals(dwCntCtrl, 2);

    dwError = ReadCertAuthEntries(pState, pState->pLdLimited, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // cert auth property should be readable by authenticated users
    TestAssertEquals(dwCntProp, 2);
    // cert auth control should be readable by authenticated users
    TestAssertEquals(dwCntCtrl, 2);

    dwError = ReadCertAuthEntries(pState, pState->pLdAnonymous, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // cert auth property should be NOT readable by anonymous users
    TestAssertEquals(dwCntProp, 0);
    // cert auth control should be NOT readable by anonymous users
    TestAssertEquals(dwCntCtrl, 0);

    // pass all tests, return 0
    return 0;
}

DWORD
TestUserEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCntProp = 0;
    DWORD   dwCntCtrl = 0;

    dwError = ReadUserEntries(pState, pState->pLd, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // user property should be readable by administrator
    TestAssertEquals(dwCntProp, 6);
    // user control should be readable by administrator
    TestAssertEquals(dwCntCtrl, 6);

    dwError = ReadUserEntries(pState, pState->pLdLimited, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // user property should be NOT readable by authenticated users (except self)
    TestAssertEquals(dwCntProp, 1);
    // user control should be readable by authenticated users
    TestAssertEquals(dwCntCtrl, 6);

    dwError = ReadUserEntries(pState, pState->pLdAnonymous, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // user property should be NOT readable by anonymous users
    TestAssertEquals(dwCntProp, 0);
    // user control should be NOT readable by anonymous users
    TestAssertEquals(dwCntCtrl, 0);

    // pass all tests, return 0
    return 0;
}

DWORD
TestResidentialPersonEntriesSD(
    PVMDIR_TEST_STATE   pState
    )
{
    DWORD   dwError = 0;
    DWORD   dwCntProp = 0;
    DWORD   dwCntCtrl = 0;

    dwError = ReadResidentialPersonEntries(pState, pState->pLd, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // residential person property should be readable by administrator
    TestAssertEquals(dwCntProp, 2);
    // residential person control should be readable by administrator
    TestAssertEquals(dwCntCtrl, 2);

    dwError = ReadResidentialPersonEntries(pState, pState->pLdLimited, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // residential person property should be readable by authenticated users
    TestAssertEquals(dwCntProp, 2);
    // residential person control should NOT be readable by authenticated users
    TestAssertEquals(dwCntCtrl, 0);

    dwError = ReadResidentialPersonEntries(pState, pState->pLdAnonymous, &dwCntProp, &dwCntCtrl);
    TestAssertEquals(dwError, 0);
    // residential person property should NOT be readable by anonymous users
    TestAssertEquals(dwCntProp, 0);
    // residential person control should NOT be readable by anonymous users
    TestAssertEquals(dwCntCtrl, 0);

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
