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

#include "includes.h"

#define LWCA_TEMPFILE_NAME_TEMPLATE "/tmp/tempjsonXXXXXX"
#define LWCA_VALID_JSON "{\"obj1\":{\"key1\":\"val1\",\"key2\":\"val2\"},\"obj2\":{\"key1\":[\"val11\",\"val12\"],\"key2\":[\"val21\",\"val22\"]},\"obj3\":{\"key1\":\"1514764800\"}}"
#define LWCA_INVALID_JSON "i am not a JSON object"
#define LWCA_JSON_OBJ1_KEY "obj1"
#define LWCA_JSON_OBJ1_VAL "{\"key1\":\"val1\",\"key2\":\"val2\"}"
#define LWCA_JSON_KEY1 "key1"
#define LWCA_JSON_OBJ1_KEY1_VAL "val1"
#define LWCA_JSON_OBJ2_VAL "{\"key1\":[\"val11\",\"val12\"],\"key2\":[\"val21\",\"val22\"]}"
#define LWCA_JSON_OBJ2_KEY1_VAL_SIZE 2
#define LWCA_JSON_OBJ2_KEY1_VAL1 "val11"
#define LWCA_JSON_OBJ2_KEY1_VAL2 "val12"
#define LWCA_JSON_OBJ3_VAL "{\"key1\":\"1514764800\"}"
#define LWCA_JSON_OBJ3_KEY1_VAL 1514764800

static
DWORD
_LwCAJsonCreateTempFile(
    PCSTR           pcszFileContents,
    PSTR            *ppszFileName
    );


VOID
LwCAJsonLoadObjectFromFile_ValidInput(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszFileName = NULL;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJsonExpected = NULL;
    PLWCA_JSON_OBJECT       pJsonActual = NULL;

    pJsonExpected = json_loads(LWCA_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonExpected);

    dwError = _LwCAJsonCreateTempFile(LWCA_VALID_JSON, &pszFileName);
    assert_int_equal(dwError, 0);
    assert_non_null(pszFileName);

    dwError = LwCAJsonLoadObjectFromFile(pszFileName, &pJsonActual);
    assert_int_equal(dwError, 0);
    assert_non_null(pJsonActual);

    dwError = json_equal(pJsonExpected, pJsonActual);
    assert_true(dwError);

    LwCAJsonCleanupObject(pJsonExpected);
    LwCAJsonCleanupObject(pJsonActual);
    LWCA_SAFE_FREE_STRINGA(pszFileName);
}

VOID
LwCAJsonLoadObjectFromFile_InvalidInput(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszFileName = NULL;
    PLWCA_JSON_OBJECT       pJson = NULL;

    dwError = _LwCAJsonCreateTempFile(LWCA_INVALID_JSON, &pszFileName);
    assert_int_equal(dwError, 0);
    assert_non_null(pszFileName);

    dwError = LwCAJsonLoadObjectFromFile(pszFileName, &pJson);
    assert_int_equal(dwError, LWCA_JSON_FILE_LOAD_ERROR);
    assert_null(pJson);

    LwCAJsonCleanupObject(pJson);
    LWCA_SAFE_FREE_STRINGA(pszFileName);
}

VOID
LwCAJsonGetObjectFromKey_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    PLWCA_JSON_OBJECT       pJsonObjExpected = NULL;
    PLWCA_JSON_OBJECT       pJsonObjActual = NULL;

    pJson = json_loads(LWCA_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    pJsonObjExpected = json_loads(LWCA_JSON_OBJ1_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonObjExpected);

    // Test with bOptional not set
    dwError = LwCAJsonGetObjectFromKey(pJson, FALSE, LWCA_JSON_OBJ1_KEY, &pJsonObjActual);
    assert_int_equal(dwError, 0);
    assert_non_null(pJsonObjActual);

    dwError = json_equal(pJsonObjExpected, pJsonObjActual);
    assert_true(dwError);

    pJsonObjActual = NULL;

    // Test with bOptional set
    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_JSON_OBJ1_KEY, &pJsonObjActual);
    assert_int_equal(dwError, 0);
    assert_non_null(pJsonObjActual);

    dwError = json_equal(pJsonObjExpected, pJsonObjActual);
    assert_true(dwError);

    LwCAJsonCleanupObject(pJson);
    LwCAJsonCleanupObject(pJsonObjExpected);
}

VOID
LwCAJsonGetObjectFromKey_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    PLWCA_JSON_OBJECT       pJsonObj = NULL;

    pJson = json_loads(LWCA_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetObjectFromKey(pJson, FALSE, "NotAKey", &pJsonObj);
    assert_int_equal(dwError, LWCA_JSON_PARSE_ERROR);
    assert_null(pJsonObj);

    pJsonObj = NULL;

    // Test with bOptional Set
    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, "NotAKey", &pJsonObj);
    assert_int_equal(dwError, 0);
    assert_null(pJsonObj);

    LwCAJsonCleanupObject(pJson);
}

VOID
LwCAJsonGetStringFromKey_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    PSTR                    pszValue = NULL;

    pJson = json_loads(LWCA_JSON_OBJ1_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetStringFromKey(pJson, FALSE, LWCA_JSON_KEY1, &pszValue);
    assert_int_equal(dwError, 0);
    assert_non_null(pszValue);
    assert_string_equal(LWCA_JSON_OBJ1_KEY1_VAL, pszValue);

    LWCA_SAFE_FREE_STRINGA(pszValue);

    // Test with bOptional set
    dwError = LwCAJsonGetStringFromKey(pJson, TRUE, LWCA_JSON_KEY1, &pszValue);
    assert_int_equal(dwError, 0);
    assert_non_null(pszValue);
    assert_string_equal(LWCA_JSON_OBJ1_KEY1_VAL, pszValue);

    LwCAJsonCleanupObject(pJson);
    LWCA_SAFE_FREE_STRINGA(pszValue);
}

VOID
LwCAJsonGetStringFromKey_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    PSTR                    pszValue = NULL;

    pJson = json_loads(LWCA_JSON_OBJ1_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetStringFromKey(pJson, FALSE, "NotAKey", &pszValue);
    assert_int_equal(dwError, LWCA_JSON_PARSE_ERROR);
    assert_null(pszValue);

    LWCA_SAFE_FREE_STRINGA(pszValue);

    // Test with bOptional set
    dwError = LwCAJsonGetStringFromKey(pJson, TRUE, "NotAKey", &pszValue);
    assert_int_equal(dwError, 0);
    assert_null(pszValue);

    LwCAJsonCleanupObject(pJson);
    LWCA_SAFE_FREE_STRINGA(pszValue);
}

VOID
LwCAJsonGetStringArrayFromKey_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    PLWCA_STRING_ARRAY      pStrArray = NULL;

    pJson = json_loads(LWCA_JSON_OBJ2_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetStringArrayFromKey(pJson, FALSE, LWCA_JSON_KEY1, &pStrArray);
    assert_int_equal(dwError, 0);
    assert_non_null(pStrArray);
    assert_non_null(pStrArray->ppData);
    assert_int_equal(pStrArray->dwCount, LWCA_JSON_OBJ2_KEY1_VAL_SIZE);
    assert_string_equal(LWCA_JSON_OBJ2_KEY1_VAL1, pStrArray->ppData[0]);
    assert_string_equal(LWCA_JSON_OBJ2_KEY1_VAL2, pStrArray->ppData[1]);

    LwCAFreeStringArray(pStrArray);

    // Test with bOptional set
    dwError = LwCAJsonGetStringArrayFromKey(pJson, TRUE, LWCA_JSON_KEY1, &pStrArray);
    assert_int_equal(dwError, 0);
    assert_non_null(pStrArray);
    assert_non_null(pStrArray->ppData);
    assert_int_equal(pStrArray->dwCount, LWCA_JSON_OBJ2_KEY1_VAL_SIZE);
    assert_string_equal(LWCA_JSON_OBJ2_KEY1_VAL1, pStrArray->ppData[0]);
    assert_string_equal(LWCA_JSON_OBJ2_KEY1_VAL2, pStrArray->ppData[1]);

    LwCAJsonCleanupObject(pJson);
    LwCAFreeStringArray(pStrArray);
}

VOID
LwCAJsonGetStringArrayFromKey_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    PLWCA_STRING_ARRAY      pStrArray = NULL;

    pJson = json_loads(LWCA_JSON_OBJ2_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetStringArrayFromKey(pJson, FALSE, "NotAKey", &pStrArray);
    assert_int_equal(dwError, LWCA_JSON_PARSE_ERROR);
    assert_null(pStrArray);

    LwCAFreeStringArray(pStrArray);

    // Test with bOptional set
    dwError = LwCAJsonGetStringArrayFromKey(pJson, TRUE, "NotAKey", &pStrArray);
    assert_int_equal(dwError, 0);
    assert_null(pStrArray);

    LwCAJsonCleanupObject(pJson);
    LwCAFreeStringArray(pStrArray);
}

VOID
LwCAJsonGetTimeFromKey_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    time_t                  tExpectedValue = LWCA_JSON_OBJ3_KEY1_VAL;
    time_t                  tValue = 0;

    pJson = json_loads(LWCA_JSON_OBJ3_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetTimeFromKey(pJson, FALSE, LWCA_JSON_KEY1, &tValue);
    assert_int_equal(dwError, 0);
    assert_memory_equal((void*)&tValue, (void*)&tExpectedValue, 8);

    // Test with bOptional set
    dwError = LwCAJsonGetTimeFromKey(pJson, TRUE, LWCA_JSON_KEY1, &tValue);
    assert_int_equal(dwError, 0);
    assert_memory_equal((void*)&tValue, (void*)&tExpectedValue, 8);

    LwCAJsonCleanupObject(pJson);
}

VOID
LwCAJsonGetTimeFromKey_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PLWCA_JSON_OBJECT       pJson = NULL;
    time_t                  tExpectedValue = 0;
    time_t                  tValue = 0;

    pJson = json_loads(LWCA_JSON_OBJ3_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    // Test with bOptional not set
    dwError = LwCAJsonGetTimeFromKey(pJson, FALSE, "NotAKey", &tValue);
    assert_int_equal(dwError, LWCA_JSON_PARSE_ERROR);
    assert_memory_equal((void*)&tValue, (void*)&tExpectedValue, 8);

    // Test with bOptional set
    dwError = LwCAJsonGetTimeFromKey(pJson, TRUE, "NotAKey", &tValue);
    assert_int_equal(dwError, 0);
    assert_memory_equal((void*)&tValue, (void*)&tExpectedValue, 8);

    LwCAJsonCleanupObject(pJson);
}
static
DWORD
_LwCAJsonCreateTempFile(
    PCSTR           pcszFileContents,
    PSTR            *ppszFileName
    )
{
    DWORD           dwError = 0;
    int             ret = 0;
    int             fd = 0;
    PSTR            pszFileName = NULL;

    if (IsNullOrEmptyString(pcszFileContents) || !ppszFileName)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(
                    (strlen(LWCA_TEMPFILE_NAME_TEMPLATE) * sizeof(char)) + 1,
                    (PVOID *)&pszFileName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAStringNCpyA(pszFileName,
                    LwCAStringLenA(LWCA_TEMPFILE_NAME_TEMPLATE),
                    LWCA_TEMPFILE_NAME_TEMPLATE,
                    LwCAStringLenA(LWCA_TEMPFILE_NAME_TEMPLATE));
    BAIL_ON_LWCA_ERROR(dwError);

    fd = mkstemp(pszFileName);
    if (fd < 1)
    {
        dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    ret = write(fd, pcszFileContents, strlen(pcszFileContents));
    if (ret != LwCAStringLenA(pcszFileContents) && errno == EINTR)
    {
        dwError = LWCA_ERRNO_TO_LWCAERROR(errno);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszFileName = pszFileName;

cleanup:

    close(fd);

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszFileName);
    if (ppszFileName)
    {
        *ppszFileName = NULL;
    }

    goto cleanup;
}
