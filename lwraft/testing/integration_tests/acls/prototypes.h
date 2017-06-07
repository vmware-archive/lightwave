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

// administratorrights.c
DWORD
TestStandardRightsForAdminUser(
    PVMDIR_TEST_STATE pState
    );

// administratorsrights.c
DWORD
TestStandardRightsForAdminGroup(
    PVMDIR_TEST_STATE pState
    );

// bad_parameters.c
DWORD
TestBadParameters(
    PVMDIR_TEST_STATE pState
    );

// domainadminsrights.c
DWORD
TestStandardRightsForDomainAdmin(
    PVMDIR_TEST_STATE pState
    );

// domainclientsrights.c
DWORD
TestStandardRightsForDomainClients(
    PVMDIR_TEST_STATE pState
    );

DWORD
TestSecurityDescriptors(
    PVMDIR_TEST_STATE pState
    );

DWORD
TestSecurityDescriptorsSddl(
    PVMDIR_TEST_STATE pState
    );

DWORD
TestProtectedEntries(
    PVMDIR_TEST_STATE pState
    );

// inheritance.c
DWORD
TestSecurityDescriptorInheritance(
    PVMDIR_TEST_STATE
    );

//legacy_access_checks.
DWORD
TestLegacyAccessChecks(
    PVMDIR_TEST_STATE
    );

// standard_rights.c

DWORD
TryToListChildObjects(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );

DWORD
TryToDeleteObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );
DWORD
TryToReadProperties(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );
DWORD
TryToReadSD(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );

DWORD
TryToWriteProperties(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );

DWORD
TryToWriteSD(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );

DWORD
TryToListObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    );

// util.c -- TODO: Move some to core library
DWORD
_GetObjectAcl(
    PVMDIR_TEST_STATE pState,
    PCSTR pszObjectDn,
    PSTR *ppszAcl
    );

DWORD
_VdcSearchForEntryAndAttribute(
    LDAP *pLd,
    PCSTR pszBaseDN,
    PCSTR pszAttribute, // OPTIONAL
    PSTR *ppszValue // OUT OPTIONAL
    );

DWORD
VmDirTestAddAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    );

DWORD
_VdcGetObjectSecurityDescriptor(
    PVMDIR_TEST_STATE pState,
    PCSTR pszObjectDN,
    BYTE **ppbSecurityDescriptor,
    PDWORD pdwSDLength
    );

DWORD
_GetBuiltinGroupSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszGroupCn,
    PSTR *ppszGroupSid
    );

DWORD
_VdcConnectionFromUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserCn,
    LDAP **ppLd
    );

// wellknownsids.c
DWORD
TestWellknownSids(
    PVMDIR_TEST_STATE pState
    );
