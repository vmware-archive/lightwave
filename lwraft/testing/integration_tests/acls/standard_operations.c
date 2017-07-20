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
TryToDeleteObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer // TODO
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestDeleteUser(pState, pszContainer, pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    return dwError;
error:
    goto cleanup;
}

DWORD
TryToReadProperties(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR pszAttribute = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcSearchForEntryAndAttribute(
                pState->pLd,
                pszUserDn,
                ATTR_SAM_ACCOUNT_NAME,
                &pszAttribute);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (strcmp(pszAttribute, pszUserName) != 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAttribute);
    return dwError;
error:
    goto cleanup;
}

DWORD
TryToReadSD(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR pszAttribute = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcSearchForEntryAndAttribute(
                pState->pLd,
                pszUserDn,
                ATTR_ACL_STRING,
                &pszAttribute);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Make sure there's data. We don't bother validating the contents of the
    // SD as that's installation- and entry-specific.
    //
    if (strlen(pszAttribute) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszAttribute);
    return dwError;
error:
    goto cleanup;
}

DWORD
TryToWriteProperties(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributeValues[] = { "206-555-1212", NULL };
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestAddAttributeValues(
                pState->pLd,
                pszUserDn,
                "telephoneNumber",
                (PCSTR*)ppszAttributeValues);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
TryToWriteSD(
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
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=%s,%s",
                pszUserName,
                pszContainer,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Random SD. Actual values don't (entirely) matter.
    dwError = VmDirAllocateStringPrintf(
                &ppszAttributeValues[0],
                "O:BAG:BAD:(A;;RCRPWPWDSD;;;%s-500)(A;;RCRPWPWDSD;;;%s-544)",
                pszDomainSid,
                pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestReplaceAttributeValues(
                pState->pLd,
                pszUserDn,
                ATTR_ACL_STRING,
                (PCSTR*)ppszAttributeValues);
    BAIL_ON_VMDIR_ERROR(dwError);

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
TryToListObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    )
{
    DWORD dwError = 0;
    PSTR pszDn = NULL;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainer, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

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
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszDn);
    return dwError;
error:
    goto cleanup;
}

DWORD
TryToListChildObjects(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszContainerDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, pszContainerName, pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszContainerDn,
                "cn=%s,%s",
                pszContainerName,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetObjectList(pState->pLd, pszContainerDn, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

#if 0 // TODO
DWORD
TestStandardRightsForUser(
    PVMDIR_TEST_STATE pState,
    LDAP *pLd,
    PCSTR pszContainerName
    )
{
    DWORD dwError = 0;

    dwError = TryToListChildObjects(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TryToDeleteObject(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TryToReadProperties(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TryToReadSD(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TryToWriteProperties(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TryToWriteSD(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TryToListObject(pState, pszContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    TestAssertEquals(dwError, 0);
    goto cleanup;
}
#endif
