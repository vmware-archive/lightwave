/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

#define TEST_MODRDN_GROUP           "testmodrdngroup"
#define TEST_MODRDN_CONTAINER_1     "testmodrdnC1"
#define TEST_MODRDN_CONTAINER_2     "testmodrdnC2"

#define TEST_MODRDN_USER_1          "modrdnUser1"
#define TEST_MODRDN_USER_2          "modrdnUser2"
#define TEST_MODRDN_USER_2_NEW      "modrdnUser2New"

VMDIR_MODRDN_TEST_CONTEXT   _gModrdnContext = {0};

static
DWORD
_TestMoveEntry(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    );

static
DWORD
_TestRenameRDN(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    );

static
DWORD
_TestMoveEntryInGroupShouldFail(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    );

static
DWORD
_TestRenameRDNEntryInGroupShouldFail(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    );

static
VOID
_TestFreeModrdnContextContent(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    );

VOID
TestModrdnCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_MODRDN_TEST_CONTEXT pContext = NULL;

    pContext = pState->pContext = &_gModrdnContext;
    _gModrdnContext.pTestState = pState;

    if (!pState->bSkipCleanup)
    {
        VmDirSleep(2); // pause 2 seconds to allow replication converge

        VmDirTestDeleteContainerByDn(pState->pLd, pContext->pszC1DN);
        VmDirTestDeleteContainerByDn(pState->pLd, pContext->pszC2DN);
    }

    _TestFreeModrdnContextContent(pContext);

    return;
}

DWORD
TestModrdnSetup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD   dwError = 0;
    PVMDIR_MODRDN_TEST_CONTEXT pContext = NULL;

    pContext = pState->pContext = &_gModrdnContext;
    _gModrdnContext.pTestState = pState;

    dwError = VmDirAllocateStringPrintf(
            &pContext->pszC1DN,
            "cn=%s,%s",
            TEST_MODRDN_CONTAINER_1,
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pContext->pszC2DN,
            "cn=%s,%s",
            TEST_MODRDN_CONTAINER_2,
            pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pContext->pszUser1DN,
            "cn=%s,%s",
            TEST_MODRDN_USER_1,
            pContext->pszC1DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // move user1 from C1 to C2
    dwError = VmDirAllocateStringPrintf(
            &pContext->pszUser1NewDN,
            "cn=%s,%s",
            TEST_MODRDN_USER_1,
            pContext->pszC2DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pContext->pszUser2DN,
            "cn=%s,%s",
            TEST_MODRDN_USER_2,
            pContext->pszC2DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // rename rdn of user2
    dwError = VmDirAllocateStringPrintf(
            &pContext->pszUser2NewDN,
            "cn=%s,%s",
            TEST_MODRDN_USER_2_NEW,
            pContext->pszC2DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pState, TEST_MODRDN_CONTAINER_1, pContext->pszC1DN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateContainer(pState, TEST_MODRDN_CONTAINER_2, pContext->pszC2DN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, TEST_MODRDN_CONTAINER_1, TEST_MODRDN_USER_1, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, TEST_MODRDN_CONTAINER_2, TEST_MODRDN_USER_2, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pContext->pszGroupDN,
            "cn=%s,%s",
            TEST_MODRDN_GROUP,
            pContext->pszC1DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateGroup(pState, TEST_MODRDN_CONTAINER_1, TEST_MODRDN_GROUP, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestModrdn(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_MODRDN_TEST_CONTEXT pContext = NULL;

    pContext = pState->pContext = &_gModrdnContext;
    _gModrdnContext.pTestState = pState;

    printf("Testing modrdn code ...\n");

    dwError = _TestRenameRDN(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestMoveEntry(pContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        dwError = VmDirTestAddUserToGroupByDn(
                    pContext->pTestState->pLd,
                    pContext->pszUser2NewDN,
                    pContext->pszGroupDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _TestMoveEntryInGroupShouldFail(pContext);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _TestRenameRDNEntryInGroupShouldFail(pContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    printf("Modrdn tests succeded!\n");
    fflush(stdout);

cleanup:
    return dwError;

error:
    printf("Modrdn tests failed with error 0n%d\n", dwError);
    goto cleanup;
}

static
VOID
_TestFreeModrdnContextContent(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    )
{
    VMDIR_SAFE_FREE_MEMORY(pContext->pszC1DN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszC2DN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszUser1DN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszUser1NewDN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszUser2DN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszUser2NewDN);
    VMDIR_SAFE_FREE_MEMORY(pContext->pszGroupDN);
}

/*
 * If DN is in a group, move entry to a different container should fail
 */
static
DWORD
_TestMoveEntryInGroupShouldFail(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    )
{
    DWORD   dwError = 0;
    PVMDIR_TEST_STATE pState = pContext->pTestState;

    dwError = ldap_rename_s(
        pContext->pTestState->pLd,
        pContext->pszUser2NewDN,
        "cn="TEST_MODRDN_USER_2_NEW,
        pContext->pszC1DN,
        0,
        NULL,
        NULL);
    TestAssertEquals(dwError, LDAP_UNWILLING_TO_PERFORM);
    dwError = 0;

    return dwError;
}

/*
 * If DN is in a group, rename RDN should fail
 */
static
DWORD
_TestRenameRDNEntryInGroupShouldFail(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    )
{
    DWORD   dwError = 0;
    PVMDIR_TEST_STATE pState = pContext->pTestState;

    dwError = ldap_rename_s(
        pContext->pTestState->pLd,
        pContext->pszUser2NewDN,
        "cn="TEST_MODRDN_USER_2,
        NULL,
        1,
        NULL,
        NULL);
    TestAssertEquals(dwError, LDAP_UNWILLING_TO_PERFORM);
    dwError = 0;

    return dwError;
}

/*
 * Keep RDN but move to a different container
 */
static
DWORD
_TestMoveEntry(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    )
{
    DWORD   dwError=0;
    PVMDIR_TEST_STATE pState = pContext->pTestState;

    dwError = ldap_rename_s(
        pContext->pTestState->pLd,
        pContext->pszUser1DN,
        "cn="TEST_MODRDN_USER_1,
        pContext->pszC2DN,
        0,
        NULL,
        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssert(VmDirTestCanReadSingleEntry(pContext->pTestState->pLd, pContext->pszUser1DN) == FALSE);
    TestAssert(VmDirTestCanReadSingleEntry(pContext->pTestState->pLd, pContext->pszUser1NewDN) == TRUE);

cleanup:
    return dwError;

error:
    printf("%s failed with error 0n%d\n", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * change RDN yet keep same parent
 */
static
DWORD
_TestRenameRDN(
    PVMDIR_MODRDN_TEST_CONTEXT pContext
    )
{
    DWORD   dwError=0;
    PVMDIR_TEST_STATE pState = pContext->pTestState;

    dwError = ldap_rename_s(
        pContext->pTestState->pLd,
        pContext->pszUser2DN,
        "cn="TEST_MODRDN_USER_2_NEW,
        NULL,
        1,
        NULL,
        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssert(VmDirTestCanReadSingleEntry(pContext->pTestState->pLd, pContext->pszUser2DN) == FALSE);
    TestAssert(VmDirTestCanReadSingleEntry(pContext->pTestState->pLd, pContext->pszUser2NewDN) == TRUE);

cleanup:
    return dwError;

error:
    printf("%s failed with error 0n%d\n", __FUNCTION__, dwError);
    goto cleanup;
}
