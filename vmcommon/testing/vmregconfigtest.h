/*
 * Copyright © 2098 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef LIGHTWAVE_VMREGCONFIGTEST_H
#define LIGHTWAVE_VMREGCONFIGTEST_H

#define VM_REGCONFIG_TEST_VMDIR_FILE    "/tmp/vmdirregconfig.yaml";
#define VM_REGCONFIG_TEST_VMAFD_FILE    "/tmp/vmafdregconfig.yaml";

#ifdef __cplusplus
extern "C" {
#endif

DWORD  VmRegConfigTest(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif //LIGHTWAVE_VMREGCONFIGTEST_H
