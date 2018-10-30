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

#define MAX_PATH 256

#define SECURITY_AWS_KMS_CONFIG_GOOD \
"{\"kms\":{\"cmk_id\":\"test_cmk_id_good\",\"key_spec\":\"AES_256\"}}"

#define SECURITY_AWS_KMS_CONFIG_BAD_NO_ROOT \
"{\"kms_no_root\":{\"cmk_id\":\"test_cmk_id_bad\"}}"

#define SECURITY_AWS_KMS_CONFIG_BAD_NO_CMK_ID \
"{\"kms\":{\"cmk_id_bad\":\"test_cmk_id_bad\"}}"

#define SECURITY_AWS_KMS_CONFIG_BAD_NO_KEYSPEC \
"{\"kms\":{\"cmk_id\":\"test_cmk_id\",\"key_spec_bad\":\"keyspec_bad\"}}"

char _tmpfilename[MAX_PATH];

static
PCSTR
_FileWithContents(
    PCSTR pszText);

VOID
Security_Aws_Kms_Tests_Config_Parse_Good(
    void **state
    )
{
    DWORD dwError = 0;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    dwError = pState->pInterface->pFnInitialize(
                  _FileWithContents(SECURITY_AWS_KMS_CONFIG_GOOD),
                  &pState->pHandle);
    assert_true(dwError == 0);
}

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_No_Root(
    void **state
    )
{
    DWORD dwError = 0;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    dwError = pState->pInterface->pFnInitialize(
                  _FileWithContents(SECURITY_AWS_KMS_CONFIG_BAD_NO_ROOT),
                  &pState->pHandle);
    assert_true(dwError == LWCA_SECURITY_AWS_KMS_CONFIG_ROOT_NOT_FOUND);
}

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_No_CMKId(
    void **state
    )
{
    DWORD dwError = 0;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    dwError = pState->pInterface->pFnInitialize(
                  _FileWithContents(SECURITY_AWS_KMS_CONFIG_BAD_NO_CMK_ID),
                  &pState->pHandle);

    assert_true(dwError == LWCA_SECURITY_AWS_KMS_INVALID_CONFIG);
}

VOID
Security_Aws_Kms_Tests_Config_Parse_Bad_No_KeySpec(
    void **state
    )
{
    DWORD dwError = 0;
    PSECURITY_AWS_KMS_TEST_STATE pState = *state;

    dwError = pState->pInterface->pFnInitialize(
                  _FileWithContents(SECURITY_AWS_KMS_CONFIG_BAD_NO_KEYSPEC),
                  &pState->pHandle);

    assert_true(dwError == LWCA_SECURITY_AWS_KMS_INVALID_CONFIG);
}

static
PCSTR
_FileWithContents(
    PCSTR pszText)
{
    int bytesWritten = 0;
    int bytesToWrite = strlen(pszText);

    strcpy(_tmpfilename, "/tmp/kmstest.XXXXXX");
    int fd = mkstemp(_tmpfilename);
    assert_true(fd != -1);

    bytesWritten = write(fd, pszText, strlen(pszText));
    assert_true(bytesWritten == bytesToWrite);

    close(fd);
    return _tmpfilename;
}
