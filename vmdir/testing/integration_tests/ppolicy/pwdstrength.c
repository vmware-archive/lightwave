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

DWORD
_VmDirTestModifyPassword(
    LDAP *pLd,
    PVMDIR_PP_CTRL_MODIFY   pPPCtrlModify
    )
{
    DWORD dwError = 0;

    LDAPMod addReplace = {0};
    LDAPMod *mods[2] = {0};
    PSTR    ppszValues[] = { NULL, NULL };
    LDAPControl*    psctrls = NULL;
    LDAPControl*    srvCtrls[2] = {NULL, NULL};
    LDAPMessage*    pResult = NULL;
    int iMsgId = 0;
    int iRtn = 0;

    ppszValues[0] = (PSTR)pPPCtrlModify->pszPassword;

    dwError = ldap_control_create(
        LDAP_CONTROL_PASSWORDPOLICYREQUEST, 0, NULL, 0, &psctrls);
    BAIL_ON_VMDIR_ERROR(dwError);

    srvCtrls[0] = psctrls;

    /* Initialize the attribute, specifying 'ADD' as the operation */
    addReplace.mod_op     = LDAP_MOD_REPLACE;
    addReplace.mod_type   = ATTR_USER_PASSWORD;
    addReplace.mod_values = (PSTR*) ppszValues;

    /* Fill the attributes array (remember it must be NULL-terminated) */
    mods[0] = &addReplace;
    mods[1] = NULL;

    dwError = ldap_modify_ext(
        pLd,
        pPPCtrlModify->pszTargetDN,
        mods,
        srvCtrls,
        NULL,
        &iMsgId);
    BAIL_ON_VMDIR_ERROR(dwError);

    iRtn = ldap_result(pLd, iMsgId, LDAP_MSG_ALL, NULL, &pResult);
    if (iRtn != LDAP_RES_MODIFY || !pResult)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = TestPPCtrlParseResult(
        pLd,
        pResult,
        &pPPCtrlModify->ctrlResult
        );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (psctrls)
    {
        ldap_control_free(psctrls);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
_TestPwdStrengthWithCtrl(
    PVMDIR_PPOLICY_TEST_CONTEXT     pPolicyContext,
    PVMDIR_STRENGTH_TEST_REC        pRec
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVMDIR_TEST_STATE pState = pPolicyContext->pTestState;
    VMDIR_PP_CTRL_MODIFY   ctrlModify = {0};

    struct _TestPwdStrengthParam
    {
        LDAP* pLd;
        PCSTR pszTargetDN;
        struct _TestCtrlResult
        {
            DWORD dwResult;
            BOOLEAN bHasPPCtrlResp;
        } ctrlResult[2];
    }
    TestPwdStrengthParam[] =
    {   // normal modify self password
        {   pPolicyContext->pTestState->pLd,
            pPolicyContext->pszTestUserDN,
            {
                { 0, 0},
                {19, 0} // TODO { 19, 1}
            }
        },
        // admin modify user password
        {   pPolicyContext->pTestState->pLd,
            pPolicyContext->pszTestUserDN,
            {
                { 0, 0 },
                {19, 0} // TODO { 19, 1}
            }
        },
        // admin modify admin password
        {   pPolicyContext->pTestState->pLd,
            pPolicyContext->pTestState->pszUserDN,
            {
                { 0, 0},
                {19, 0} //TODO { 19, 1}
            }
        },
    };

    for (dwCnt=0; dwCnt < sizeof(TestPwdStrengthParam)/sizeof(TestPwdStrengthParam[0]); dwCnt++)
    {
        memset(&ctrlModify, 0, sizeof(ctrlModify));
        ctrlModify.pszTargetDN = TestPwdStrengthParam[dwCnt].pszTargetDN;
        ctrlModify.pszPassword = pRec->pszGoodPwd;

        dwError = _VmDirTestModifyPassword(
            TestPwdStrengthParam[dwCnt].pLd,
            &ctrlModify);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssertEquals(ctrlModify.ctrlResult.bHasPPCtrlResponse, TestPwdStrengthParam[dwCnt].ctrlResult[0].bHasPPCtrlResp);
        TestAssertEquals(ctrlModify.ctrlResult.dwOpResult, TestPwdStrengthParam[dwCnt].ctrlResult[0].dwResult);

        memset(&ctrlModify, 0, sizeof(ctrlModify));
        ctrlModify.pszTargetDN = TestPwdStrengthParam[dwCnt].pszTargetDN;
        ctrlModify.pszPassword = pRec->pszBadPwd;

        dwError = _VmDirTestModifyPassword(
            TestPwdStrengthParam[dwCnt].pLd,
            &ctrlModify);
        BAIL_ON_VMDIR_ERROR(dwError);

        TestAssertEquals(ctrlModify.ctrlResult.bHasPPCtrlResponse, TestPwdStrengthParam[dwCnt].ctrlResult[1].bHasPPCtrlResp);
        TestAssertEquals(ctrlModify.ctrlResult.dwOpResult, TestPwdStrengthParam[dwCnt].ctrlResult[1].dwResult);
        // TODO TestAssertEquals(ctrlModify.ctrlResult.PPolicyState.PPolicyError, pRec->PPolicyError);

    }
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
_TestPwdStrength(
    PVMDIR_PPOLICY_TEST_CONTEXT     pPolicyContext,
    PVMDIR_STRENGTH_TEST_REC        pRec
    )
{
    DWORD   dwError = 0;
    PSTR ppszAttrValue[] = { NULL, NULL };

    // prepare strength value
    ppszAttrValue[0] = pRec->pszTestValue;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        pRec->pszAttr,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    // test strength
    dwError = _TestPwdStrengthWithCtrl(pPolicyContext, pRec);
    BAIL_ON_VMDIR_ERROR(dwError);

    // restore strength value
    ppszAttrValue[0] = pRec->pszRestoreValue;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        pRec->pszAttr,
        (PCSTR*)ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestPwdStrength(
    PVMDIR_PPOLICY_TEST_CONTEXT   pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PCSTR   ppszAttrValue[] = { NULL, NULL };
    VMDIR_STRENGTH_TEST_REC initTbl[] =
    {                                                       \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_SIZE,    \
            /*.pszTestValue     = */ "2",                   \
            /*.pszRestoreValue  = */ "2",                   \
            /*.pszGoodPwd       = */ "ab",                  \
            /*.pszBadPwd        = */ "a",                   \
            /*.PPolicyError     = */ PP_passwordTooShort    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_MUN_CHAR,    \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "b1",                  \
            /*.pszBadPwd        = */ "bc",                  \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_SP_CHAR,    \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "c!",                  \
            /*.pszBadPwd        = */ "c1",                  \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_ALPHA_CHAR,  \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "1",                   \
            /*.pszGoodPwd       = */ "d0",                  \
            /*.pszBadPwd        = */ "0!",                  \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_LOWER_CHAR,  \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "eF",                  \
            /*.pszBadPwd        = */ "EF",                  \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MIN_UPPER_CHAR,  \
            /*.pszTestValue     = */ "1",                   \
            /*.pszRestoreValue  = */ "0",                   \
            /*.pszGoodPwd       = */ "Fg",                  \
            /*.pszBadPwd        = */ "fg",                  \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MAX_SIZE,    \
            /*.pszTestValue     = */ "3",                   \
            /*.pszRestoreValue  = */ "20",                  \
            /*.pszGoodPwd       = */ "gHI",                 \
            /*.pszBadPwd        = */ "gHIJ",                \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
        {                                                   \
            /*.pszAttr          = */ ATTR_PASS_MAX_SAME_ADJ_CHAR,    \
            /*.pszTestValue     = */ "2",                   \
            /*.pszRestoreValue  = */ "1",                   \
            /*.pszGoodPwd       = */ "hIIj",                \
            /*.pszBadPwd        = */ "hIIIj",               \
            /*.PPolicyError     = */ PP_insufficientPasswordQuality    \
        },                                                  \
    };

    for (dwCnt=0; dwCnt < sizeof(initTbl)/sizeof(initTbl[0]); dwCnt++)
    {
        PVMDIR_STRENGTH_TEST_REC pRec = &initTbl[dwCnt];
        dwError = _TestPwdStrength(pPolicyContext, pRec);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // restore user password
    ppszAttrValue[0] = pPolicyContext->pszTestUserPassword;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pLdUser,
        pPolicyContext->pszTestUserDN,
        ATTR_USER_PASSWORD,
        ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    // restore admin password
    ppszAttrValue[0] = pPolicyContext->pTestState->pszPassword;
    dwError = VmDirTestReplaceAttributeValues(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pTestState->pszUserDN,
        ATTR_USER_PASSWORD,
        ppszAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    printf("%s %s (%d)\n", __FUNCTION__, dwError ? "failed" : "succeeded", dwError);
    return dwError;

error:
    goto cleanup;
}
