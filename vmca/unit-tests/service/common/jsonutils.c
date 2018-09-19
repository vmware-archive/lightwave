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

#define VMCA_TEMPFILE_NAME_TEMPLATE "/tmp/tempjsonXXXXXX"
#define VMCA_VALID_JSON "{\"obj1\":{\"key1\":\"val1\",\"key2\":\"val2\"},\"obj2\":{\"key1\":\"val1\",\"key2\":\"val2\"}}"
#define VMCA_INVALID_JSON "i am not a JSON object"
#define VMCA_JSON_OBJ1_KEY "obj1"
#define VMCA_JSON_OBJ1_VAL "{\"key1\":\"val1\",\"key2\":\"val2\"}"
#define VMCA_JSON_KEY1 "key1"
#define VMCA_JSON_KEY1_VAL "val1"


static
DWORD
_VMCAJsonCreateTempFile(
    PCSTR           pcszFileContents,
    PSTR            *ppszFileName
    );


VOID
VMCAJsonLoadObjectFromFile_ValidInput(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszFileName = NULL;
    json_error_t            jsonError = {0};
    PVMCA_JSON_OBJECT       pJsonExpected = NULL;
    PVMCA_JSON_OBJECT       pJsonActual = NULL;

    pJsonExpected = json_loads(VMCA_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonExpected);

    dwError = _VMCAJsonCreateTempFile(VMCA_VALID_JSON, &pszFileName);
    assert_int_equal(dwError, 0);
    assert_non_null(pszFileName);

    dwError = VMCAJsonLoadObjectFromFile(pszFileName, &pJsonActual);
    assert_int_equal(dwError, 0);
    assert_non_null(pJsonActual);

    dwError = json_equal(pJsonExpected, pJsonActual);
    assert_true(dwError);

    VMCAJsonCleanupObject(pJsonExpected);
    VMCAJsonCleanupObject(pJsonActual);
    VMCA_SAFE_FREE_STRINGA(pszFileName);
}

VOID
VMCAJsonLoadObjectFromFile_InvalidInput(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PSTR                    pszFileName = NULL;
    PVMCA_JSON_OBJECT       pJson = NULL;

    dwError = _VMCAJsonCreateTempFile(VMCA_INVALID_JSON, &pszFileName);
    assert_int_equal(dwError, 0);
    assert_non_null(pszFileName);

    dwError = VMCAJsonLoadObjectFromFile(pszFileName, &pJson);
    assert_int_equal(dwError, VMCA_JSON_FILE_LOAD_ERROR);
    assert_null(pJson);

    VMCAJsonCleanupObject(pJson);
    VMCA_SAFE_FREE_STRINGA(pszFileName);
}

VOID
VMCAJsonGetObjectFromKey_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PVMCA_JSON_OBJECT       pJson = NULL;
    PVMCA_JSON_OBJECT       pJsonObjExpected = NULL;
    PVMCA_JSON_OBJECT       pJsonObjActual = NULL;

    pJson = json_loads(VMCA_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    pJsonObjExpected = json_loads(VMCA_JSON_OBJ1_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJsonObjExpected);

    dwError = VMCAJsonGetObjectFromKey(pJson, VMCA_JSON_OBJ1_KEY, &pJsonObjActual);
    assert_int_equal(dwError, 0);
    assert_non_null(pJsonObjActual);

    dwError = json_equal(pJsonObjExpected, pJsonObjActual);
    assert_true(dwError);

    VMCAJsonCleanupObject(pJson);
    VMCAJsonCleanupObject(pJsonObjExpected);
}

VOID
VMCAJsonGetObjectFromKey_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PVMCA_JSON_OBJECT       pJson = NULL;
    PVMCA_JSON_OBJECT       pJsonObj = NULL;

    pJson = json_loads(VMCA_VALID_JSON, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = VMCAJsonGetObjectFromKey(pJson, "NotAKey", &pJsonObj);
    assert_int_equal(dwError, VMCA_JSON_PARSE_ERROR);
    assert_null(pJsonObj);

    VMCAJsonCleanupObject(pJson);
}

VOID
VMCAJsonGetStringFromKey_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PVMCA_JSON_OBJECT       pJson = NULL;
    PSTR                    pszValue = NULL;

    pJson = json_loads(VMCA_JSON_OBJ1_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = VMCAJsonGetStringFromKey(pJson, VMCA_JSON_KEY1, &pszValue);
    assert_int_equal(dwError, 0);
    assert_non_null(pszValue);

    assert_string_equal(VMCA_JSON_KEY1_VAL, pszValue);

    VMCAJsonCleanupObject(pJson);
    VMCA_SAFE_FREE_STRINGA(pszValue);
}

VOID
VMCAJsonGetStringFromKey_Invalid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    json_error_t            jsonError = {0};
    PVMCA_JSON_OBJECT       pJson = NULL;
    PSTR                    pszValue = NULL;

    pJson = json_loads(VMCA_JSON_OBJ1_VAL, JSON_DECODE_ANY, &jsonError);
    assert_non_null(pJson);

    dwError = VMCAJsonGetStringFromKey(pJson, "NotAKey", &pszValue);
    assert_int_equal(dwError, VMCA_JSON_PARSE_ERROR);
    assert_null(pszValue);

    VMCAJsonCleanupObject(pJson);
    VMCA_SAFE_FREE_STRINGA(pszValue);
}


static
DWORD
_VMCAJsonCreateTempFile(
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
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                    (strlen(VMCA_TEMPFILE_NAME_TEMPLATE) * sizeof(char)) + 1,
                    (PVOID *)&pszFileName);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAStringNCpyA(pszFileName,
                    VMCAStringLenA(VMCA_TEMPFILE_NAME_TEMPLATE),
                    VMCA_TEMPFILE_NAME_TEMPLATE,
                    VMCAStringLenA(VMCA_TEMPFILE_NAME_TEMPLATE));
    BAIL_ON_VMCA_ERROR(dwError);

    fd = mkstemp(pszFileName);
    if (fd < 1)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    ret = write(fd, pcszFileContents, strlen(pcszFileContents));
    if (ret != VMCAStringLenA(pcszFileContents) && errno == EINTR)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppszFileName = pszFileName;

cleanup:

    close(fd);

    return dwError;

error:

    VMCA_SAFE_FREE_STRINGA(pszFileName);
    if (ppszFileName)
    {
        *ppszFileName = NULL;
    }

    goto cleanup;
}
