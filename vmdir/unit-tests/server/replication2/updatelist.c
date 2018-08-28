#include "includes.h"

//setup and teardown function
int
VmDirSetupReplUpdateListTest(
    VOID    **state
    )
{
    DWORD                           dwError = 0;
    PVMDIR_REPLICATION_UPDATE_LIST  pUpdateList = NULL;

    dwError = VmDirReplUpdateListAlloc(&pUpdateList);
    assert_int_equal(dwError, 0);

    *state = pUpdateList;

    return 0;
}

int
VmDirTeardownReplUpdateListTest(
    VOID    **state
    )
{
    VmDirFreeReplUpdateList((PVMDIR_REPLICATION_UPDATE_LIST)*state);

    return 0;
}

VOID
VmDirReplUpdateListParseSyncDoneCtl_ValidInput(
    VOID    **state
    )
{
    DWORD                           dwError = 0;
    PVMDIR_REPLICATION_UPDATE_LIST  pReplUpdate = NULL;
    LDAPControl*                    pSearchResCtrl = NULL;

    pReplUpdate = *state;
    dwError = VmDirAllocateMemory(sizeof(LDAPControl), (PVOID*)&pSearchResCtrl);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pSearchResCtrl->ldctl_value.bv_val,
                "12345,7ef77c0f-cff1-4239-b293-39a2b302d5bd:12345,8ef77c0f-cff1-4239-b293-39a2b302d5bd:12346,9ef77c0f-cff1-4239-b293-39a2b302d5bd:12347"
                );
    assert_int_equal(dwError, 0);

    pSearchResCtrl->ldctl_value.bv_len = VmDirStringLenA(pSearchResCtrl->ldctl_value.bv_val);

    dwError = VmDirReplUpdateListParseSyncDoneCtl(pReplUpdate, &pSearchResCtrl);
    assert_int_equal(dwError, 0);

    assert_int_equal(12345, pReplUpdate->newHighWaterMark);
    assert_string_equal(
                "7ef77c0f-cff1-4239-b293-39a2b302d5bd:12345,8ef77c0f-cff1-4239-b293-39a2b302d5bd:12346,9ef77c0f-cff1-4239-b293-39a2b302d5bd:12347",
                pReplUpdate->pNewUtdVector->pszUtdVector
                );
}
