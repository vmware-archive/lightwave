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
"{\
     \"boolean\":true,\
     \"string\":\"abc\",\
     \"int\":123,\
     \"real\":123.456,\
     \"object\":\
     {\
         \"object_boolean\":true,\
         \"object_string\":\"object_abc\",\
         \"object_int\":1123,\
         \"object_real\":1123.456\
     }\
 }"

typedef struct _VM_JSON_RESULT_TEST_OBJECT_SIMPLE_
{
    BOOLEAN bBoolVal;
}VM_JSON_RESULT_TEST_OBJECT_SIMPLE, *PVM_JSON_RESULT_TEST_OBJECT_SIMPLE;

typedef struct _VM_JSON_RESULT_TEST_SIMPLE_
{
    BOOLEAN bBoolVal;
    PSTR pszStrVal;
    int nIntVal;
    double dDoubleVal;
    VM_JSON_RESULT_TEST_OBJECT_SIMPLE stObject;
}VM_JSON_RESULT_TEST_SIMPLE, *PVM_JSON_RESULT_TEST_SIMPLE;

static
DWORD
_VmJsonResultObjectMapSimpleTestPass(
    );

static
DWORD
_VmJsonResultObjectMapObjectTestPass(
    );

DWORD
VmJsonResultObjectMapTest()
{
    DWORD dwError = 0;

    dwError = _VmJsonResultObjectMapSimpleTestPass();
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmJsonResultObjectMapObjectTestPass();
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_VmJsonResultObjectMapSimpleTestPass(
    )
{
    DWORD dwError = 0;
    VM_JSON_RESULT_TEST_SIMPLE stSimple = {0};

    VM_JSON_OBJECT_MAP simple_object_map[] =
    {
        {"boolean", JSON_RESULT_BOOLEAN, {&stSimple.bBoolVal}},
        {"string", JSON_RESULT_STRING, {(VOID *)&stSimple.pszStrVal}},
        {"int", JSON_RESULT_INTEGER, {(VOID *)&stSimple.nIntVal}},
        {"real", JSON_RESULT_REAL, {(VOID *)&stSimple.dDoubleVal}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    dwError = VmJsonResultMapObject(
                  JSON_RESULT_SIMPLE,
                  simple_object_map);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (stSimple.bBoolVal &&
        VmStringCompareA(stSimple.pszStrVal, "abc", TRUE) == 0 &&
        stSimple.nIntVal == 123 &&
        stSimple.dDoubleVal == 123.456)
    {
        fprintf(stdout, "PASS: jsonresult simple map test positive\n");
    }
    else
    {
        dwError = 1;
    }

cleanup:
    VmFreeMemory(stSimple.pszStrVal);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmJsonResultObjectMapObjectTestPass(
    )
{
    DWORD dwError = 0;
    VM_JSON_RESULT_TEST_SIMPLE stSimple = {0};

    VM_JSON_OBJECT_MAP object_in_object_map[] =
    {
        {"object_boolean", JSON_RESULT_BOOLEAN, {&stSimple.stObject.bBoolVal}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    VM_JSON_OBJECT_MAP object_map[] =
    {
        {"boolean", JSON_RESULT_BOOLEAN, {&stSimple.bBoolVal}},
        {"string", JSON_RESULT_STRING, {(VOID *)&stSimple.pszStrVal}},
        {"int", JSON_RESULT_INTEGER, {(VOID *)&stSimple.nIntVal}},
        {"real", JSON_RESULT_REAL, {(VOID *)&stSimple.dDoubleVal}},
        {"object", JSON_RESULT_OBJECT, {(VOID *)object_in_object_map}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    dwError = VmJsonResultMapObject(
                  JSON_RESULT_SIMPLE,
                  object_map);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (stSimple.bBoolVal &&
        VmStringCompareA(stSimple.pszStrVal, "abc", TRUE) == 0 &&
        stSimple.nIntVal == 123 &&
        stSimple.dDoubleVal == 123.456 &&
        stSimple.stObject.bBoolVal)
    {
        fprintf(stdout, "PASS: jsonresult object in object map positive\n");
    }
    else
    {
        dwError = 1;
    }

cleanup:
    VmFreeMemory(stSimple.pszStrVal);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}
