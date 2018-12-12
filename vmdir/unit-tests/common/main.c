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
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test_setup_teardown(
                VmDirMergeSort_OneNodeInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMergeSort_TwoNodesInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMergeSort_ThreeNodesInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMergeSort_DescendingOrder,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMergeSort_AscendingOrder,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirLinkedListAppendListToTail_ValidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirLinkedListAppendListToTail_InvalidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirSortedLinkedListInsert_AscendingOneElement,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirSortedLinkedListInsert_AscendingTwoElements,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirSortedLinkedListInsert_AscendingThreeElements,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirSortedLinkedListInsert_Ascending,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirSortedLinkedListInsert_Descending,
                NULL,
                NULL),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
