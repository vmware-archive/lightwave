/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

int main(VOID)
{
    int    retVal = 0;

    retVal = VmDirStringToBervalContent(
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd", &gVmdirServerGlobals.invocationId);
    assert_int_equal(retVal, 0);

    const struct CMUnitTest updatelist_tests[] =
    {
        //updatelist.c
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateListParseSyncDoneCtl_ValidInput,
                VmDirSetupReplUpdateListTest,
                VmDirTeardownReplUpdateListTest),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateListExpand_AddTest,
                VmDirSetupReplUpdateListExpand_AddTest,
                VmDirTeardownReplUpdateListTest),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateListExpand_ModifyTest,
                VmDirSetupReplUpdateListExpand_ModifyTest,
                VmDirTeardownReplUpdateListTest),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateListExpand_DeleteTest,
                VmDirSetupReplUpdateListExpand_DeleteTest,
                VmDirTeardownReplUpdateListTest),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateListExpand_AddTombstoneTest,
                VmDirSetupReplUpdateListExpand_AddTombstoneTest,
                VmDirTeardownReplUpdateListTest),
    };

    const struct CMUnitTest update_tests[] =
    {
        //update.c
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateToUSNList_NoDupMetaDataOnly,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateToUSNList_DupMetaDataOnly,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateToUSNList_MetaDataOnly,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateToUSNList_ValueMetaDataOnly,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateToUSNList_MetaDataAndValueMetaData,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateToUSNList_MetaDataAndValueMetaDataSame,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirReplUpdateLocalUsn_ValidInput,
                NULL,
                NULL),
    };

    const struct CMUnitTest extractevent_tests[] =
    {
        //extractevents.c
        cmocka_unit_test_setup_teardown(
                VmDirExtractEventAttributeChanges_ValidInput,
                VmDirSetupExtractEventAttributeChanges,
                VmDirTeardownExtractEvent),
        cmocka_unit_test_setup_teardown(
                VmDirExtractEventAttributeValueChanges_ValidInput,
                VmDirSetupExtractEventAttributeValueChanges,
                VmDirTeardownExtractEvent),
        cmocka_unit_test_setup_teardown(
                VmDirExtractEventPopulateMustAttributes_ValidInput,
                VmDirSetupExtractEventPopulateMustAttributes,
                VmDirTeardownExtractEvent),
        cmocka_unit_test_setup_teardown(
                VmDirExtractEventPopulateOperationAttributes_ValidInput,
                VmDirSetupExtractEventPopulateOperationAttributes,
                VmDirTeardownExtractEvent),
    };

    retVal = cmocka_run_group_tests(updatelist_tests, NULL, NULL);
    if (retVal)
    {
        print_message("UpdateList tests failed: %d", retVal);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = cmocka_run_group_tests(update_tests, NULL, NULL);
    if (retVal)
    {
        print_message("Update tests failed: %d", retVal);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = cmocka_run_group_tests(extractevent_tests, NULL, NULL);
    if (retVal)
    {
        print_message("ExtractEvent tests failed: %d", retVal);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    VmDirFreeBervalContent(&gVmdirServerGlobals.invocationId);
    return retVal;

error:
    goto cleanup;
}
