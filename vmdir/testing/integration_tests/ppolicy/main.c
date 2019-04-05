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

#include "includes.h"

VMDIR_PPOLICY_TEST_CONTEXT _gPolicyContext = {0};

static
DWORD
_TestGetPolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    );

static
DWORD
_TestInitPolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    );

static
DWORD
_TestRestorePolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    );

static
DWORD
_TestUpdatePolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext,
    PCSTR*  ppszAry,
    DWORD   dwArySize
    );

static
VOID
_TestFreePolicyContextContent(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    );

DWORD
TestSetup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD   dwError = 0;
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext = NULL;

    pPolicyContext = pState->pContext = &_gPolicyContext;
    _gPolicyContext.pTestState = pState;

    dwError = VmDirAllocateStringPrintf(
        &pPolicyContext->pszPolicyDN,
        "cn=%s,%s",
        PASSWD_LOCKOUT_POLICY_DEFAULT_CN,
        pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
        &pPolicyContext->pszTestUserCN,
        "%s",
        VMDIR_TEST_POLICY_USER_CN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // ignore error
    VmDirTestDeleteUser(pState, NULL, pPolicyContext->pszTestUserCN);

    dwError = VmDirAllocateStringA(
        pState->pszPassword, &pPolicyContext->pszTestUserPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUserEx(
        pState,
        NULL,
        pPolicyContext->pszTestUserCN,
        pPolicyContext->pszTestUserPassword,
        NULL,
        &pPolicyContext->pszTestUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestConnectionUser(
        pState->pszServerName,
        pState->pszDomain,
        pPolicyContext->pszTestUserCN,
        pPolicyContext->pszTestUserPassword,
        &pPolicyContext->pLdUser);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestGetPolicy(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestInitPolicy(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
TestCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD   dwError = 0;
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext = NULL;

    pPolicyContext = (PVMDIR_PPOLICY_TEST_CONTEXT)pState->pContext;

    dwError = VmDirTestDeleteUser(pState, NULL, pPolicyContext->pszTestUserCN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestRestorePolicy(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    _TestFreePolicyContextContent(pPolicyContext);

error:
    return dwError;
}

DWORD
TestRunner(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext = NULL;

    pPolicyContext = (PVMDIR_PPOLICY_TEST_CONTEXT)pState->pContext;

    printf("Testing password policy code ...\n");

    dwError = TestLockout(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestRecycle(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestPwdStrength(pPolicyContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Password policy tests finished successfully.\n");

cleanup:
    return dwError;
error:
    goto cleanup;
}

/*
 * query current policy content, so we can restore after done testing
 */
static
DWORD
_TestGetPolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    PCSTR ppszAttrs[] =
    {
        ATTR_PASS_AUTO_UNLOCK_SEC,
        ATTR_PASS_FAIL_ATTEMPT_SEC,
        ATTR_PASS_MAX_FAIL_ATTEMPT,
        ATTR_PASS_MAX_SAME_ADJ_CHAR,
        ATTR_PASS_MIN_SP_CHAR,
        ATTR_PASS_MIN_MUN_CHAR,
        ATTR_PASS_MIN_LOWER_CHAR,
        ATTR_PASS_MIN_UPPER_CHAR,
        ATTR_PASS_MIN_ALPHA_CHAR,
        ATTR_PASS_MIN_SIZE,
        ATTR_PASS_MAX_SIZE,
        ATTR_PASS_EXP_IN_DAY,
        ATTR_PASS_RECYCLE_CNT,
        ATTR_ENABLED,
        NULL
    };
    LDAPMessage *pResult = NULL;
    BerValue** ppBerValues = NULL;

    dwError = VmDirAllocateMemory(
        sizeof(PSTR)* (sizeof(ppszAttrs)/sizeof(ppszAttrs[0]) * 2 + 1),
        (PVOID)&pPolicyContext->orgPolicy.ppszAttrAndValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        LDAP_SCOPE_BASE,
        NULL,
        (PSTR*)ppszAttrs,
        0,
        NULL,
        NULL,
        NULL,
        -1,
        &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pPolicyContext->pTestState->pLd, pResult) == 1)
    {
        LDAPMessage* pEntry = ldap_first_entry(pPolicyContext->pTestState->pLd, pResult);
        DWORD   dwSize = 0;

        for (dwCnt=0; dwCnt < sizeof(ppszAttrs)/sizeof(ppszAttrs[0]) && ppszAttrs[dwCnt]; dwCnt++)
        {
            ppBerValues = ldap_get_values_len(pPolicyContext->pTestState->pLd, pEntry, ppszAttrs[dwCnt]);
            if (ppBerValues != NULL && ldap_count_values_len(ppBerValues) > 0)
            {
                dwError = VmDirAllocateStringA(ppszAttrs[dwCnt], &pPolicyContext->orgPolicy.ppszAttrAndValue[dwSize++]);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirAllocateStringA(ppBerValues[0]->bv_val, &pPolicyContext->orgPolicy.ppszAttrAndValue[dwSize++]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (ppBerValues)
            {
                ldap_value_free_len(ppBerValues);
                ppBerValues = NULL;
            }
        }
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

cleanup:

    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
        ppBerValues = NULL;
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
        pResult = NULL;
    }

    return dwError;

error:
    goto cleanup;
}

/*
 * update policy definition
 */
static
DWORD
_TestUpdatePolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext,
    PCSTR*  ppszAry,
    DWORD   dwArySize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    DWORD   dwSize = dwArySize/2;
    PCSTR*      ppszAttrAndVal= NULL;
    LDAPMod*    pModifyReplace = NULL;
    LDAPMod**   ppmods = NULL;

    dwError = VmDirAllocateMemory(
        sizeof(PCSTR) * dwSize * 2,
        (PVOID)&ppszAttrAndVal);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
        sizeof(LDAPMod) * dwSize,
        (PVOID)&pModifyReplace);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
        sizeof(LDAPMod*) * dwSize + 1,
        (PVOID)&ppmods);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        pModifyReplace[dwCnt].mod_op = LDAP_MOD_REPLACE;
        pModifyReplace[dwCnt].mod_type = (PSTR)ppszAry[dwCnt*2];

        ppszAttrAndVal[dwCnt*2] = ppszAry[dwCnt*2+1];
        pModifyReplace[dwCnt].mod_values = (PSTR*)&ppszAttrAndVal[dwCnt*2];

        ppmods[dwCnt] = &pModifyReplace[dwCnt];
    }

    ppmods[dwCnt] = NULL;

    dwError = ldap_modify_ext_s(
        pPolicyContext->pTestState->pLd,
        pPolicyContext->pszPolicyDN,
        ppmods,
        NULL,
        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ppszAttrAndVal);
    VMDIR_SAFE_FREE_MEMORY(pModifyReplace);
    VMDIR_SAFE_FREE_MEMORY(ppmods);

    return dwError;

error:
    goto cleanup;
}

/*
 * Initialize to relaxed policy
 */
static
DWORD
_TestInitPolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwSize = 0;
    PCSTR    ppszInitPolicy[] =
    {
        ATTR_PASS_AUTO_UNLOCK_SEC,    "0" ,
        ATTR_PASS_FAIL_ATTEMPT_SEC,   "0" ,
        ATTR_PASS_MAX_FAIL_ATTEMPT,   "1" ,
        ATTR_PASS_MAX_SAME_ADJ_CHAR,  "1" ,
        ATTR_PASS_MIN_SP_CHAR,        "0" ,
        ATTR_PASS_MIN_MUN_CHAR,       "0" ,
        ATTR_PASS_MIN_LOWER_CHAR,     "0" ,
        ATTR_PASS_MIN_UPPER_CHAR,     "0" ,
        ATTR_PASS_MIN_ALPHA_CHAR,     "0" ,
        ATTR_PASS_MIN_SIZE,           "0" ,
        ATTR_PASS_MAX_SIZE,           "20" ,
        ATTR_PASS_EXP_IN_DAY,         "90" ,
        ATTR_PASS_RECYCLE_CNT,        "0" ,
        ATTR_ENABLED,                 "TRUE" ,
        NULL,
    };

    dwSize = sizeof(ppszInitPolicy)/sizeof(ppszInitPolicy[0]);

    dwError = _TestUpdatePolicy(pPolicyContext, ppszInitPolicy, dwSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

/*
 * restore policy prior to test
 */
static
DWORD
_TestRestorePolicy(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwSize = 0;

    for (dwSize=0; pPolicyContext->orgPolicy.ppszAttrAndValue[dwSize]; dwSize++) {}

    dwError = _TestUpdatePolicy(
        pPolicyContext,
        (PCSTR*)pPolicyContext->orgPolicy.ppszAttrAndValue,
        dwSize);

    return dwError;
}

static
VOID
_TestFreePolicyContextContent(
    PVMDIR_PPOLICY_TEST_CONTEXT pPolicyContext
    )
{
    DWORD   dwCnt = 0;

    for (dwCnt=0; pPolicyContext->orgPolicy.ppszAttrAndValue[dwCnt]; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pPolicyContext->orgPolicy.ppszAttrAndValue[dwCnt]);
    }
    VMDIR_SAFE_FREE_MEMORY(pPolicyContext->orgPolicy.ppszAttrAndValue);

    VMDIR_SAFE_FREE_MEMORY(pPolicyContext->pszPolicyDN);
    VMDIR_SAFE_FREE_MEMORY(pPolicyContext->pszTestUserCN);
    VMDIR_SAFE_FREE_MEMORY(pPolicyContext->pszTestUserDN);
    VMDIR_SAFE_FREE_MEMORY(pPolicyContext->pszTestUserPassword);

    if (pPolicyContext->pLdUser)
    {
        ldap_unbind_ext_s(pPolicyContext->pLdUser, NULL, NULL);
    }
}
