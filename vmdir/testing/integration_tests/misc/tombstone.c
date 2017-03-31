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

//
// Will return failure if it can't find the specified tombstone / entry.
//
DWORD
_VdcSearchForDeletedEntry(
    PVMDIR_TEST_STATE pState,
    PCSTR pszEntryName,
    PCSTR pszEntryGuid
    )
{
    DWORD dwError = 0;
    LDAPMessage* pResult = NULL;
    PCSTR ppszAttrs[] = { NULL };
    PSTR pszObjectDN = NULL;
    LDAPControl *pCtrl = NULL;
    LDAPControl *pServerCtrl[2] = { NULL, NULL };

    dwError = VmDirAllocateStringPrintf(
                &pszObjectDN,
                "cn=%s#objectGuid:%s,cn=Deleted Objects,%s",
                pszEntryName,
                pszEntryGuid,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_control_create(
                        VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS,
                        0,
                        NULL,
                        0,
                        &pCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerCtrl[0] = pCtrl;

    dwError = ldap_search_ext_s(
                pState->pLd,
                pszObjectDN,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                (PSTR*)ppszAttrs,
                TRUE,
                pServerCtrl,
                NULL,
                NULL,
                0,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    if (pCtrl)
    {
        ldap_control_free(pCtrl);
    }

    VMDIR_SAFE_FREE_STRINGA(pszObjectDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
_VdcGetObjectGuid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainerName,
    PCSTR psUserName,
    PSTR *ppszGuid
    )
{
    DWORD dwError = 0;
    PSTR pszGuid = NULL;
    LDAPMessage *pSearchRes = NULL;
    PCSTR pszSearchFilter = "(objectclass=*)";
    PCSTR ppszAttrs[] = {"objectGUID", NULL};
    BerValue **ppBerValues = NULL;
    PSTR pszBaseDN = NULL;
    LDAPMessage *pEntry = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszBaseDN,
                "cn=%s,cn=%s,%s",
                psUserName,
                pszContainerName,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                pState->pLd,
                pszBaseDN,
                LDAP_SCOPE_SUBTREE,
                pszSearchFilter,
                (PSTR *)ppszAttrs,
                TRUE,
                NULL,
                NULL,
                NULL,
                0,
                &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pState->pLd, pSearchRes);
    if (pEntry == NULL)
    {
        dwError = LDAP_NO_SUCH_ATTRIBUTE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerValues = ldap_get_values_len(pState->pLd, pEntry, "objectGUID");
    if (ppBerValues == NULL)
    {
        dwError = LDAP_NO_SUCH_ATTRIBUTE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ppBerValues[0]->bv_val, &pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGuid = pszGuid;
    pszGuid = NULL;

cleanup:
    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    VMDIR_SAFE_FREE_STRINGA(pszBaseDN);
    VMDIR_SAFE_FREE_STRINGA(pszGuid);
    return dwError;
error:
    goto cleanup;
}

VOID
_TombstoneWait(
    DWORD dwFrequency,
    DWORD dwPeriod
    )
{
    VmDirSleep((dwFrequency + dwPeriod) * 1000);
}

VOID
GetTombstoneTimeouts(
    PDWORD pdwFrequency,
    PDWORD pdwPeriod
    )
{
    DWORD dwFrequency = 0;
    DWORD dwPeriod = 0;

    (VOID)VmDirGetRegKeyValueDword(
            "Services\\vmdir\\Parameters",
            VMDIR_REG_KEY_TOMBSTONE_EXPIRATION_IN_SEC,
            &dwPeriod,
            45 * 24 * 60 * 60);

    (VOID)VmDirGetRegKeyValueDword(
            "Services\\vmdir\\Parameters",
            VMDIR_REG_KEY_TOMBSTONE_REAPING_FREQ_IN_SEC,
            &dwFrequency,
            24 * 60 * 60);

    *pdwFrequency = dwFrequency;
    *pdwPeriod = dwPeriod;
}

//
// Exercises the code that reaps deleted objects after a certain period of time.
//
DWORD
TestTombstone(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR psUserName = NULL;
    PSTR pszGuid = NULL;
    DWORD dwFrequency = 0;
    DWORD dwPeriod = 0;

    printf("Testing tombstone reaping code ...\n");

    GetTombstoneTimeouts(&dwFrequency, &dwPeriod);
    if (dwFrequency > 60 || dwPeriod > 60)
    {
        printf("Tombstone timeouts are too big, making testing impossible.\n");
        printf("Skipping these tests for now. Please modify the values via the registry and restart vmdir\n");
        dwError = 0;
        goto cleanup;
    }

    dwError = VmDirTestGetGuid(&psUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, VmDirTestGetTestContainerCn(pState), psUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VdcGetObjectGuid(pState, VmDirTestGetTestContainerCn(pState), psUserName, &pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestDeleteUser(pState, VmDirTestGetTestContainerCn(pState), psUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Check that user has a tombstone entry.
    //
    dwError = _VdcSearchForDeletedEntry(pState, psUserName, pszGuid);
    TestAssertEquals(dwError, 0);

    _TombstoneWait(dwFrequency, dwPeriod);

    dwError = _VdcSearchForDeletedEntry(pState, psUserName, pszGuid);
    TestAssertEquals(dwError, LDAP_NO_SUCH_OBJECT);

    printf("Tombstone tests succeded!\n");
    dwError = 0;
cleanup:
    VMDIR_SAFE_FREE_STRINGA(psUserName);
    VMDIR_SAFE_FREE_STRINGA(pszGuid);
    return dwError;

error:
    printf("Tombstone tests failed with error 0n%d\n", dwError);
    goto cleanup;
}
