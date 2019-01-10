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

#ifndef _LWCA_SECURITY_AWS_KMS_UNITTEST_STRUCTS_H_
#define _LWCA_SECURITY_AWS_KMS_UNITTEST_STRUCTS_H_

typedef struct _SECURITY_AWS_KMS_TEST_STATE_
{
    VOID *module;
    PLWCA_SECURITY_INTERFACE pInterface;
    PLWCA_SECURITY_HANDLE pHandle;
    PCSTR pszVersion;
    PCSTR pszName;
}SECURITY_AWS_KMS_TEST_STATE, *PSECURITY_AWS_KMS_TEST_STATE;

#endif /* _VMCA_SECURITY_AWS_KMS_UNITTEST_STRUCTS_H_ */
