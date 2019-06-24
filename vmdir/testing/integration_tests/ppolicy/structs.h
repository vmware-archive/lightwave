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
    PSTR    pszTestUserUPN;
    PSTR    pszTestUserPassword;
    LDAP*   pLdUser;

} VMDIR_PPOLICY_TEST_CONTEXT, *PVMDIR_PPOLICY_TEST_CONTEXT;

typedef struct _VMDIR_STRENGTH_TEST_REC
{
    PSTR    pszAttr;
    PSTR    pszTestValue;
    PSTR    pszRestoreValue;
    PSTR    pszGoodPwd;
    PSTR    pszBadPwd;
    LDAPPasswordPolicyError PPolicyError;
} VMDIR_STRENGTH_TEST_REC, *PVMDIR_STRENGTH_TEST_REC;

typedef struct _VMDIR_PP_CTRL_RESULT
{
    DWORD       dwOpResult;
    BOOLEAN     bHasPPCtrlResponse;
    VDIR_PPOLICY_STATE  PPolicyState;
} VMDIR_PP_CTRL_RESULT, *PVMDIR_PP_CTRL_RESULT;

typedef struct _VMDIR_PP_CTRL_BIND
{
    PCSTR pszMech;  // SASL Mech: "simple" or "srp"
    PCSTR pszHost;
    PCSTR pszBindCN;
    PCSTR pszBindDN;
    PCSTR pszBindUPN;
    PCSTR pszPassword;
    PCSTR pszDomain;

    VMDIR_PP_CTRL_RESULT ctrlResult;
} VMDIR_PP_CTRL_BIND, *PVMDIR_PP_CTRL_BIND;

typedef struct _VMDIR_PP_CTRL_MODIFY
{
    PCSTR       pszTargetDN;
    PCSTR       pszPassword;

    VMDIR_PP_CTRL_RESULT    ctrlResult;
} VMDIR_PP_CTRL_MODIFY, *PVMDIR_PP_CTRL_MODIFY;
