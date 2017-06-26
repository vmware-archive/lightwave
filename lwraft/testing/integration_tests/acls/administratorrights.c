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

#if 0 // TODO

DWORD
AdminShouldBeAbleToDeleteObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer // TODO
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirTestDeleteUser(pState, pszContainer, pszUserName);
    TestAssertEquals(dwError, 0);

    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    return dwError;
}

DWORD
AdminShouldBeAbleToReadProperties(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR pszAttribute = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    TestAssertEquals(dwError, 0);

    dwError = _VdcSearchForEntryAndAttribute(
                pState->pLd,
                pszUserDn,
                ATTR_SAM_ACCOUNT_NAME,
                &pszAttribute);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(pszAttribute, pszUserName);

    VMDIR_SAFE_FREE_STRINGA(pszAttribute);
    return dwError;
}

DWORD
AdminShouldBeAbleToReadSD(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR pszAttribute = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    TestAssertEquals(dwError, 0);

    dwError = _VdcSearchForEntryAndAttribute(
                pState->pLd,
                pszUserDn,
                ATTR_ACL_STRING,
                &pszAttribute);
    TestAssertEquals(dwError, 0);
    TestAssert(strlen(pszAttribute) > 0);

    VMDIR_SAFE_FREE_STRINGA(pszAttribute);
    return dwError;
}

DWORD
AdminShouldBeAbleToWriteProperties(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributeValues[] = { NULL, NULL };
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    TestAssertEquals(dwError, 0);

    ppszAttributeValues[0] = "206-555-1212";
    dwError = VmDirTestAddAttributeValues(
                pState->pLd,
                pszUserDn,
                "telephoneNumber",
                (PCSTR*)ppszAttributeValues);
    TestAssertEquals(dwError, 0);

    return dwError;
}

DWORD
AdminShouldBeAbleToWriteSD(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributeValues[] = { NULL, NULL };
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;
    PSTR pszDomainSid = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    TestAssertEquals(dwError, 0);

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Random SD. Actual values don't (entirely) matter.
    dwError = VmDirAllocateStringPrintf(
                &ppszAttributeValues[0],
                "O:BAG:BAD:(A;;RCRPWPWDSD;;;%s-500)",
                pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestReplaceAttributeValues(
                pState->pLd,
                pszUserDn,
                ATTR_ACL_STRING,
                (PCSTR*)ppszAttributeValues);
    TestAssertEquals(dwError, 0);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(ppszAttributeValues[0]);
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszUserDn);
    return dwError;
error:
    goto cleanup;
}

DWORD
AdminShouldBeAbleToListObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR pszDn = NULL;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pszDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcSearchForEntryAndAttribute(
                pState->pLd,
                pszDn,
                NULL,
                NULL);
    TestAssertEquals(dwError, 0);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszDn);
    return dwError;
error:
    goto cleanup;
}

DWORD
AdminShouldBeAbleToListChildObjects(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszContainerDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    TestAssertEquals(dwError, 0); // TODO

    dwError = VmDirTestCreateUser(pState, pszContainerName, pszUserName, NULL);
    TestAssertEquals(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pszContainerDn,
                "cn=%s,%s",
                pszContainerName,
                pState->pszBaseDN);
    TestAssertEquals(dwError, 0);

    dwError = VmDirTestGetObjectList(pState->pLd, pszContainerDn, NULL);
    TestAssertEquals(dwError, 0);
    return dwError;
}

DWORD
TestStandardRightsForAdminUser(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PCSTR pszContainerName = VmDirTestGetTestContainerCn(pState);

    dwError = AdminShouldBeAbleToListChildObjects(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AdminShouldBeAbleToDeleteObject(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AdminShouldBeAbleToReadProperties(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AdminShouldBeAbleToReadSD(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AdminShouldBeAbleToWriteProperties(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AdminShouldBeAbleToWriteSD(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AdminShouldBeAbleToListObject(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    TestAssertEquals(dwError, 0);
    goto cleanup;
}

#endif
DWORD
TestStandardRightsForAdminUser(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PCSTR pszContainerName = VmDirTestGetTestContainerCn(pState);

    dwError = TryToListChildObjects(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    dwError = TryToDeleteObject(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    dwError = TryToReadProperties(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    dwError = TryToReadSD(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    dwError = TryToWriteProperties(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    dwError = TryToWriteSD(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    dwError = TryToListObject(pState, pszContainerName);
    TestAssertEquals(dwError, 0);

    return dwError;
}
