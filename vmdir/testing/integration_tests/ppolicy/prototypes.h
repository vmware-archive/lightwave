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

DWORD
TestPwdStrength(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    );

DWORD
TestLockout(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    );

DWORD
TestRecycle(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    );

DWORD
TestPolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    );

DWORD
TestCtrlBind(
     PVMDIR_PP_CTRL_BIND pCtrlBind,
     int        iTimeout,
     LDAPControl** psctrls,
     LDAPControl** pcctrls,
     LDAP**     ppOutLd,
     LDAPMessage** ppOutResult
     );

DWORD
TestPPCtrlBind(
    PVMDIR_PP_CTRL_BIND pCtrlBind
    );

DWORD
TestPPCtrlParseResult(
    LDAP*           pLd,
    LDAPMessage*    pResult,
    PVMDIR_PP_CTRL_RESULT pCtrlResult
    );

DWORD
TestModifyPassword(
    LDAP *pLd,
    PVMDIR_PP_CTRL_MODIFY   pPPCtrlModify
    );
