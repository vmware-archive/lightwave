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
                VmDirTestQueueTestEnqueue,
                VmDirTestQueueSetupEnqueue_EmptyQueue,
                VmDirTestQueueTearDownEnqueue),
        cmocka_unit_test_setup_teardown(
                VmDirTestQueueTestDequeue,
                VmDirTestQueueSetupDequeue_Nonblocking,
                VmDirTestQueueTearDownDequeue),
        cmocka_unit_test_setup_teardown(
                VmDirTestQueueTestDequeue,
                VmDirTestQueueSetupDequeue_Waiting,
                VmDirTestQueueTearDownDequeue),
        cmocka_unit_test_setup_teardown(
                VmDirTestQueueTestCompare,
                VmDirTestQueueSetupCompare_Equal,
                VmDirTestQueueTearDownCompare),
        cmocka_unit_test_setup_teardown(
                VmDirTestQueueTestCompare,
                VmDirTestQueueSetupCompare_NotEqual,
                VmDirTestQueueTearDownCompare),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
