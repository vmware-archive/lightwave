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
#define JSON_RESULT_TEST_KEY_CERTS "certs"
#define JSON_RESULT_TEST_KEY_CN "cn"
#define JSON_RESULT_TEST_KEY_SUBJECTDN "subjectdn"
#define JSON_RESULT_TEST_VALUE_CN0 "CN=19C,CN=Certificate-Authorities,cn=Configuration,dc=lightwave,dc=local"
#define JSON_RESULT_TEST_VALUE_CN1 "CN=123"
#define JSON_RESULT_TEST_VALUE_SUBJECTDN0 "CN=CA,DC=lightwave,DC=local, C=US, O=server.lightwave.local"
#define JSON_RESULT_TEST_VALUE_SUBJECTDN1 "CN=CA"
#define JSON_RESULT_TEST_ITERATE_ARRAY "\{\"certs\":[{\"cn\":\"CN=19C,CN=Certificate-Authorities,cn=Configuration,dc=lightwave,dc=local\",\"subjectdn\":\"CN=CA,DC=lightwave,DC=local, C=US, O=server.lightwave.local\"},{\"cn\":\"CN=123\",\"subjectdn\":\"CN=CA\"}]}"

typedef struct _VM_JSON_RESULT_TEST_SIMPLE
{
    BOOLEAN bRefreshed;
    PCSTR pszPassword;
}VM_JSON_RESULT_TEST_SIMPLE, *PVM_JSON_RESULT_TEST_SIMPLE;

typedef struct _VM_JSON_RESULT_TEST_ARRAY_ELEMENT
{
    PCSTR pszCN;
    PCSTR pszSubjectDN;
}VM_JSON_RESULT_TEST_ARRAY_ELEMENT, *PVM_JSON_RESULT_TEST_ARRAY_ELEMENT;

typedef struct _VM_JSON_RESULT_TEST_ARRAY
{
    size_t nSize;
    PVM_JSON_RESULT_TEST_ARRAY_ELEMENT pElements;
}VM_JSON_RESULT_TEST_ARRAY, *PVM_JSON_RESULT_TEST_ARRAY;

typedef struct _VM_JSON_RESULT_TEST_ARRAY_ELEMENT_CBDATA
{
    size_t nIndex;
    PVM_JSON_RESULT_TEST_ARRAY_ELEMENT pElement;
}VM_JSON_RESULT_TEST_ARRAY_ELEMENT_CBDATA, *PVM_JSON_RESULT_TEST_ARRAY_ELEMENT_CBDATA;

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

static
VOID
_VmJsonFreeResultTestArray(
    PVM_JSON_RESULT_TEST_ARRAY pArray
    )
{
    if (pArray)
    {
        VM_COMMON_SAFE_FREE_MEMORY(pArray->pElements);
        VmFreeMemory(pArray);
    }
}

static
DWORD
_VmJsonObjectPopulateArrayElementCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT_TEST_ARRAY_ELEMENT_CBDATA pElementData = NULL;
    PCSTR pszCN = NULL;
    PCSTR pszSubjectDN = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pElementData = (PVM_JSON_RESULT_TEST_ARRAY_ELEMENT_CBDATA)pUserData;

    pszCN = pElementData->nIndex == 0 ? JSON_RESULT_TEST_VALUE_CN0 : JSON_RESULT_TEST_VALUE_CN1;
    pszSubjectDN = pElementData->nIndex == 0 ? JSON_RESULT_TEST_VALUE_SUBJECTDN0 : JSON_RESULT_TEST_VALUE_SUBJECTDN1;

    if (!VmStringCompareA(JSON_RESULT_TEST_KEY_CN, pszKey, TRUE))
    {
        if (pValue->nType == JSON_RESULT_STRING &&
            !VmStringCompareA(pValue->value.pszValue, pszCN, TRUE))
        {
            pElementData->pElement->pszCN = pValue->value.pszValue;
        }
        else
        {
            fprintf(stderr, "FAIL: expected type %d with value: %s. got type %d with value: %s\n",
                    JSON_RESULT_STRING, pszCN, pValue->nType, pValue->value.pszValue);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }
    else if (!VmStringCompareA(JSON_RESULT_TEST_KEY_SUBJECTDN, pszKey, TRUE))
    {
        if (pValue->nType == JSON_RESULT_STRING &&
            !VmStringCompareA(pValue->value.pszValue, pszSubjectDN, TRUE))
        {
            pElementData->pElement->pszSubjectDN = pValue->value.pszValue;
        }
        else
        {
            fprintf(stderr, "FAIL: expected type %d with value: %s. got type %d with value: %s\n",
                    JSON_RESULT_STRING, pszCN, pValue->nType, pValue->value.pszValue);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_VmJsonObjectPopulateArrayCB(
    PVOID pUserData,
    size_t nSize,
    size_t nIndex,
    PVM_JSON_POSITION pPosition
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT_TEST_ARRAY pArray = NULL;
    VM_JSON_RESULT_TEST_ARRAY_ELEMENT_CBDATA elementCBData = {0};

    if (!pUserData || !pPosition || !nSize)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pArray = (PVM_JSON_RESULT_TEST_ARRAY)pUserData;

    if (!pArray->pElements)
    {
        dwError = VmAllocateMemory(
                      sizeof(VM_JSON_RESULT_TEST_ARRAY_ELEMENT) * nSize,
                      (PVOID *)&pArray->pElements);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pArray->nSize = nSize;
    }

    elementCBData.nIndex = nIndex;
    elementCBData.pElement = &pArray->pElements[nIndex];

    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  &elementCBData,
                  _VmJsonObjectPopulateArrayElementCB);
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_VmJsonObjectPopulateArrayKeyCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT_TEST_ARRAY *ppArray = NULL;
    PVM_JSON_RESULT_TEST_ARRAY pArray = NULL;

    if (!pUserData || !pszKey || !pValue)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    ppArray = (PVM_JSON_RESULT_TEST_ARRAY *)pUserData;

    if (!VmStringCompareA(JSON_RESULT_TEST_KEY_CERTS, pszKey, TRUE))
    {
        if (pValue->nType == JSON_RESULT_ARRAY)
        {
            dwError = VmAllocateMemory(
                          sizeof(VM_JSON_RESULT_TEST_ARRAY),
                          (PVOID *)&pArray);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = VmJsonResultIterateArrayAt(
                          pValue->value.pArray,
                          pArray,
                          _VmJsonObjectPopulateArrayCB);
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
        else
        {
            fprintf(stderr, "FAIL: expected '%s' array. got %d\n",
                    JSON_RESULT_TEST_KEY_CERTS, pValue->nType);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }

    *ppArray = pArray;

cleanup:
    return dwError;

error:
    _VmJsonFreeResultTestArray(pArray);
    goto cleanup;
}

static
VOID
_VmJsonPrintTestArray(
    PVM_JSON_RESULT_TEST_ARRAY pArray
    )
{
    size_t nIndex = 0;
    while(nIndex < pArray->nSize)
    {
        fprintf(stdout, "Item: [%ld / %ld]\n", nIndex, pArray->nSize);
        fprintf(stdout, "cn: %s\n", pArray->pElements[nIndex].pszCN);
        fprintf(stdout, "subjectdn: %s\n", pArray->pElements[nIndex].pszSubjectDN);
        ++nIndex;
    }
}

static
DWORD
_VmJsonResultArrayIterate()
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pResult = NULL;
    PVM_JSON_POSITION pPositionRoot = NULL;
    PVM_JSON_RESULT_TEST_ARRAY pArray = NULL;

    dwError = VmJsonResultInit(&pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultLoadString(JSON_RESULT_TEST_ITERATE_ARRAY, pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pResult, &pPositionRoot);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmJsonResultIterateObjectAt(
                  pPositionRoot,
                  &pArray,
                  _VmJsonObjectPopulateArrayKeyCB);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    _VmJsonPrintTestArray(pArray);

    fprintf(stdout, "PASS: jsonresult iterate array test\n");

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
    dwError += _VmJsonResultArrayIterate();

    return dwError;
}
