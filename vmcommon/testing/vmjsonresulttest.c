/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"
#include "vmjsonresult.h"

#define JSON_RESULT_TEST_INIT_PASS "{\"refreshed\":\"false\"}"
#define JSON_RESULT_TEST_INIT_FAIL "{\"refreshed:\"false\"}"
#define JSON_RESULT_TEST_ITERATE_SIMPLE "{\"refreshed\":false,\"password\":\"abc\"}"

typedef struct _VM_JSON_RESULT_TEST_SIMPLE
{
    BOOLEAN bRefreshed;
    PCSTR pszPassword;
}VM_JSON_RESULT_TEST_SIMPLE, *PVM_JSON_RESULT_TEST_SIMPLE;

static
DWORD
_VmJsonResultInitTestPass()
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pResult = NULL;

    dwError = VmJsonResultInit(&pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultLoadString(JSON_RESULT_TEST_INIT_PASS, pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    fprintf(stdout, "PASS: jsonresult init test positive\n");

cleanup:
    VmJsonResultFreeHandle(pResult);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmJsonResultInitTestFail()
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pResult = NULL;
    int nErrorLine = 1; /* expected error line */

    dwError = VmJsonResultInit(&pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultLoadString(JSON_RESULT_TEST_INIT_FAIL, pResult);
    if (dwError == VM_COMMON_ERROR_JSON_LOAD_FAILURE)
    {
        dwError = 0;
    }
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (nErrorLine == pResult->nJsonErrorLine)
    {
        fprintf(stdout, "PASS: jsonresult expected result for test\n");
    }
    else
    {
        fprintf(stderr, "FAIL: jsonresult expected line %d, got line %d\n",
                nErrorLine, pResult->nJsonErrorLine);
    }
    fprintf(stdout, "PASS: jsonresult init negative test\n");

cleanup:
    VmJsonResultFreeHandle(pResult);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmJsonObjectPopulateSimpleCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT_TEST_SIMPLE pSimple = NULL;
    PCSTR pszExpectedPass = "abc";

    if (!pUserData || !pszKey || !pValue)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pSimple = (PVM_JSON_RESULT_TEST_SIMPLE)pUserData;

    if (!VmStringCompareA("refreshed", pszKey, TRUE))
    {
        if (pValue->nType == JSON_RESULT_BOOLEAN)
        {
            pSimple->bRefreshed = pValue->value.bValue;
        }
        else
        {
            fprintf(stderr, "FAIL: expected 'refreshed' boolean. got %d\n", pValue->nType);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }
    else if (!VmStringCompareA("password", pszKey, TRUE))
    {
        /* ensure string type */
        if (pValue->nType == JSON_RESULT_STRING)
        {
            pSimple->pszPassword = pValue->value.pszValue;
        }
        else
        {
            fprintf(stderr, "FAIL: expected 'password' string type. got %d type\n", pValue->nType);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
        /* ensure string value */
        if (VmStringCompareA(pszExpectedPass, pSimple->pszPassword, TRUE))
        {
            fprintf(stderr, "FAIL: password value. expected: %s, found %s\n",
                    pszExpectedPass, pSimple->pszPassword);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_VmJsonResultObjectIterate()
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pResult = NULL;
    PVM_JSON_POSITION pPositionRoot = NULL;
    VM_JSON_RESULT_TEST_SIMPLE resultSimple = {0};

    dwError = VmJsonResultInit(&pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultLoadString(JSON_RESULT_TEST_ITERATE_SIMPLE, pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pResult, &pPositionRoot);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultIterateObjectAt(
                  pPositionRoot,
                  &resultSimple,
                  _VmJsonObjectPopulateSimpleCB);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    fprintf(stdout, "result = {refreshed:%s, password: %s}\n",
            resultSimple.bRefreshed?"true":"false",
            resultSimple.pszPassword);

    fprintf(stdout, "PASS: jsonresult iterate object test\n");

cleanup:
    VmJsonResultFreeHandle(pResult);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

DWORD
VmJsonResultTest()
{
    DWORD dwError = 0;

    dwError = _VmJsonResultInitTestPass();
    dwError += _VmJsonResultInitTestFail();
    dwError += _VmJsonResultObjectIterate();

    return dwError;
}
