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

#define JSON_RESULT_SIMPLE \
"{\"string\": \"abc\"}"

#define JSON_RESULT_SIMPLE2 \
"{\"string\": \"abc\", \"string2\": \"abc2\"}"

#define JSON_RESULT_FILE_CONTENTS \
"{\
    \"string\": \"abc\",\
    \"string2\": \"abc2\"\
}"

typedef struct _VM_JSON_RESULT_TEST_OBJECT_SIMPLE_
{
    PSTR pszStrVal;
    PSTR pszStrVal2;
}VM_JSON_RESULT_TEST_OBJECT_SIMPLE, *PVM_JSON_RESULT_TEST_OBJECT_SIMPLE;

static
DWORD
_VmJsonResultFromObjectMapSimpleTestPass(
    );

static
DWORD
_VmJsonResultFromObjectMapSimpleTestPass2(
    );

static
DWORD
_VmJsonResultDumpObjectMapToFileTestPass1(
    );

DWORD
VmJsonResultFromObjectMapTest()
{
    DWORD dwError = 0;

    dwError = _VmJsonResultFromObjectMapSimpleTestPass();
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmJsonResultFromObjectMapSimpleTestPass2();
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmJsonResultDumpObjectMapToFileTestPass1();
    BAIL_ON_VM_COMMON_ERROR(dwError);
error:
    return dwError;
}

static
DWORD
_VmJsonResultFromObjectMapSimpleTestPass(
    )
{
    DWORD dwError = 0;
    VM_JSON_RESULT_TEST_OBJECT_SIMPLE stSimple = {0};
    PVM_JSON_RESULT pResult = NULL;

    VM_JSON_OBJECT_MAP simple_object_map[] =
    {
        {"string", JSON_RESULT_STRING, {(PVOID)&stSimple.pszStrVal}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    stSimple.pszStrVal = "abc";

    dwError = VmJsonResultFromObjectMap(
                  simple_object_map,
                  &pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = strcmp(JSON_RESULT_SIMPLE, json_dumps(pResult->pJsonRoot, 0));

cleanup:
    VmJsonResultFreeHandle(pResult);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmJsonResultFromObjectMapSimpleTestPass2(
    )
{
    DWORD dwError = 0;
    VM_JSON_RESULT_TEST_OBJECT_SIMPLE stSimple = {0};
    PVM_JSON_RESULT pResult = NULL;

    VM_JSON_OBJECT_MAP simple_object_map[] =
    {
        {"string", JSON_RESULT_STRING, {(PVOID)&stSimple.pszStrVal}},
        {"string2", JSON_RESULT_STRING, {(PVOID)&stSimple.pszStrVal2}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    stSimple.pszStrVal = "abc";
    stSimple.pszStrVal2 = "abc2";

    dwError = VmJsonResultFromObjectMap(
                  simple_object_map,
                  &pResult);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = strcmp(JSON_RESULT_SIMPLE2, json_dumps(pResult->pJsonRoot, 0));

cleanup:
    VmJsonResultFreeHandle(pResult);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmJsonResultDumpObjectMapToFileTestPass1(
    )
{
    DWORD dwError = 0;
    VM_JSON_RESULT_TEST_OBJECT_SIMPLE stSimple = {0};
    PVM_JSON_RESULT pResult = NULL;
    PSTR pszFile = "/tmp/vmcommonobjectmap.out";
    PSTR pszFileContents = NULL;

    VM_JSON_OBJECT_MAP simple_object_map[] =
    {
        {"string", JSON_RESULT_STRING, {(PVOID)&stSimple.pszStrVal}},
        {"string2", JSON_RESULT_STRING, {(PVOID)&stSimple.pszStrVal2}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    stSimple.pszStrVal = "abc";
    stSimple.pszStrVal2 = "abc2";

    dwError = VmJsonResultDumpObjectMapToFile(
                  simple_object_map,
                  pszFile);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszFileContents);
    VmJsonResultFreeHandle(pResult);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}
