/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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

typedef struct _VMDIR_ORIGINAL_POLICY
{
    PSTR* ppszAttrAndValue;
} VMDIR_ORIGINAL_POLICY, *PVMDIR_ORIGINAL_POLICY;

typedef struct _VMDIR_PPOLICY_TEST_CONTEXT
{
    PVMDIR_TEST_STATE           pTestState;
    VMDIR_ORIGINAL_POLICY        orgPolicy;
    PSTR    pszPolicyDN;
    PSTR    pszTestUserDN;
    PSTR    pszTestUserCN;
    PSTR    pszTestUserPassword;
    LDAP*   pLdUser;

} VMDIR_PPOLICY_TEST_CONTEXT, *PVMDIR_PPOLICY_TEST_CONTEXT;

