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

//setup and teardown function
int
VmDirSetupValueMetaData(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_VALUE_ATTRIBUTE_METADATA), (PVOID*)&pValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("member", &pValueMetaData->pszAttrType);
    assert_int_equal(dwError, 0);

    pValueMetaData->localUsn = 620972;
    pValueMetaData->version = 1;

    dwError = VmDirAllocateStringA(
            "de62357c-e8b1-4532-9c81-91c4f58c3248", &pValueMetaData->pszOrigInvoId);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(
            "de62357c-e8b1-4532-9c81-91c4f58c3248", &pValueMetaData->pszValChgOrigInvoId);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("20180726170334", &pValueMetaData->pszValChgOrigTime);
    assert_int_equal(dwError, 0);

    pValueMetaData->valChgOrigUsn = 620972;
    pValueMetaData->dwOpCode = 1;
    pValueMetaData->dwValSize = 22;

    dwError = VmDirAllocateStringA("cn=Users,dc=test-20002", &pValueMetaData->pszValue);
    assert_int_equal(dwError, 0);

    *state = pValueMetaData;

    return 0;
}

int
VmDirValueMetaDataFree(
    VOID    **state
    )
{
    VmDirFreeValueMetaData((PVMDIR_VALUE_ATTRIBUTE_METADATA)*state);

    return 0;
}

// Format is: <attr-name>:<local-usn>:<version-no>:<originating-server-id>:
// <value-change-originating-server-id>:<value-change-originating time>:
// <value-change-originating-usn>:<opcode>:<value-size>:<value>
VOID
VmDirValueMetaDataDeserialize_ValidInput(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PSTR                               pszValueMetaData = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    dwError = VmDirAllocateStringA(
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:50:cn=administrator@test-20002,cn=Users,dc=test-20002",
            &pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(pValueMetaData->pszAttrType, "member");
    assert_int_equal(pValueMetaData->localUsn, 620972);
    assert_int_equal(pValueMetaData->version, 1);
    assert_string_equal(pValueMetaData->pszOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigTime, "20180726170334.296");
    assert_int_equal(pValueMetaData->valChgOrigUsn, 620972);
    assert_int_equal(pValueMetaData->dwOpCode, 1);
    assert_int_equal(pValueMetaData->dwValSize, 50);
    assert_string_equal(pValueMetaData->pszValue, "cn=administrator@test-20002,cn=Users,dc=test-20002");

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);
    VmDirFreeValueMetaData(pValueMetaData);
    pValueMetaData = NULL;

    // value with null characters
    dwError = VmDirAllocateAndCopyMemory(
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:52:cn=administrator@test-abcde,\0cn=Users,dc=test-\0abcde",
            174,
            (PVOID*)&pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(pValueMetaData->pszAttrType, "member");
    assert_int_equal(pValueMetaData->localUsn, 620972);
    assert_int_equal(pValueMetaData->version, 1);
    assert_string_equal(pValueMetaData->pszOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigTime, "20180726170334.296");
    assert_int_equal(pValueMetaData->valChgOrigUsn, 620972);
    assert_int_equal(pValueMetaData->dwOpCode, 1);
    assert_int_equal(pValueMetaData->dwValSize, 52);
    assert_memory_equal((PVOID)pValueMetaData->pszValue,
                        (PVOID)"cn=administrator@test-abcde,\0cn=Users,dc=test-\0abcde",
                        pValueMetaData->dwValSize);

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);
    VmDirFreeValueMetaData(pValueMetaData);

    // value with integer
    dwError = VmDirAllocateAndCopyMemory(
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:1:1",
            121,
            (PVOID*)&pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(pValueMetaData->pszAttrType, "member");
    assert_int_equal(pValueMetaData->localUsn, 620972);
    assert_int_equal(pValueMetaData->version, 1);
    assert_string_equal(pValueMetaData->pszOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigTime, "20180726170334.296");
    assert_int_equal(pValueMetaData->valChgOrigUsn, 620972);
    assert_int_equal(pValueMetaData->dwOpCode, 1);
    assert_int_equal(pValueMetaData->dwValSize, 1);
    assert_memory_equal((PVOID)pValueMetaData->pszValue,
                        (PVOID)"1",
                        pValueMetaData->dwValSize);

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);
    VmDirFreeValueMetaData(pValueMetaData);

    // value with integer
    dwError = VmDirAllocateAndCopyMemory(
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:5:12345",
            125,
            (PVOID*)&pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(pValueMetaData->pszAttrType, "member");
    assert_int_equal(pValueMetaData->localUsn, 620972);
    assert_int_equal(pValueMetaData->version, 1);
    assert_string_equal(pValueMetaData->pszOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigTime, "20180726170334.296");
    assert_int_equal(pValueMetaData->valChgOrigUsn, 620972);
    assert_int_equal(pValueMetaData->dwOpCode, 1);
    assert_int_equal(pValueMetaData->dwValSize, 5);
    assert_memory_equal((PVOID)pValueMetaData->pszValue,
                        (PVOID)"12345",
                        pValueMetaData->dwValSize);

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);
    VmDirFreeValueMetaData(pValueMetaData);

    // value with ':'
    dwError = VmDirAllocateAndCopyMemory(
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:15:abcd:efgh:12345",
            136,
            (PVOID*)&pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(pValueMetaData->pszAttrType, "member");
    assert_int_equal(pValueMetaData->localUsn, 620972);
    assert_int_equal(pValueMetaData->version, 1);
    assert_string_equal(pValueMetaData->pszOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigInvoId, "de62357c-e8b1-4532-9c81-91c4f58c3248");
    assert_string_equal(pValueMetaData->pszValChgOrigTime, "20180726170334.296");
    assert_int_equal(pValueMetaData->valChgOrigUsn, 620972);
    assert_int_equal(pValueMetaData->dwOpCode, 1);
    assert_int_equal(pValueMetaData->dwValSize, 15);
    assert_memory_equal((PVOID)pValueMetaData->pszValue,
                        (PVOID)"abcd:efgh:12345",
                        pValueMetaData->dwValSize);

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);
    VmDirFreeValueMetaData(pValueMetaData);
}

VOID
VmDirValueMetaDataDeserialize_InvalidInput(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PSTR                               pszValueMetaData = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    //Null input
    dwError = VmDirValueMetaDataDeserialize(NULL, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirValueMetaDataDeserialize(NULL, &pValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirValueMetaDataDeserialize("", &pValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(pValueMetaData);

    //Invalid MetaData: partial elements
    dwError = VmDirValueMetaDataDeserialize(
            "member:620972:1:620972:1:50:cn=administrator@test-20002,cn=Users,dc=test-20002",
            &pValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    assert_null(pValueMetaData);

    dwError = VmDirValueMetaDataDeserialize(":::::::::", &pValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    assert_null(pValueMetaData);
}

VOID
VmDirValueMetaDataSerialize_ValidInput(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      bervValueMetaData = VDIR_BERVALUE_INIT;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    pValueMetaData = *state;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(
            bervValueMetaData.lberbv_val,
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334:620972:1:22:cn=Users,dc=test-20002");
    VMDIR_SAFE_FREE_MEMORY(bervValueMetaData.lberbv_val);
    bervValueMetaData.lberbv_len = 0;

    // value with null characters
    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValue);
    dwError = VmDirAllocateAndCopyMemory(
            "cn=test\0,dc=lw-testdom\0,dc=com", 31, (PVOID*)&pValueMetaData->pszValue);
    assert_int_equal(dwError, 0);
    pValueMetaData->dwValSize = 31;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, 0);

    assert_memory_equal(
            bervValueMetaData.lberbv_val,
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334:620972:1:31:cn=test\0,dc=lw-testdom\0,dc=com",
            bervValueMetaData.lberbv_len);
    VMDIR_SAFE_FREE_MEMORY(bervValueMetaData.lberbv_val);

    // value with integers
    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValue);
    dwError = VmDirAllocateAndCopyMemory(
            "1", 1, (PVOID*)&pValueMetaData->pszValue);
    assert_int_equal(dwError, 0);
    pValueMetaData->dwValSize = 1;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, 0);

    assert_memory_equal(
            bervValueMetaData.lberbv_val,
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334:620972:1:1:1",
            bervValueMetaData.lberbv_len);
    VMDIR_SAFE_FREE_MEMORY(bervValueMetaData.lberbv_val);

    // value with integers
    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValue);
    dwError = VmDirAllocateAndCopyMemory(
            "12345", 5, (PVOID*)&pValueMetaData->pszValue);
    assert_int_equal(dwError, 0);
    pValueMetaData->dwValSize = 5;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, 0);

    assert_memory_equal(
            bervValueMetaData.lberbv_val,
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334:620972:1:5:12345",
            bervValueMetaData.lberbv_len);
    VMDIR_SAFE_FREE_MEMORY(bervValueMetaData.lberbv_val);

    // value with ':'
    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValue);
    dwError = VmDirAllocateAndCopyMemory(
            "abcd:12345:efgh", 15, (PVOID*)&pValueMetaData->pszValue);
    assert_int_equal(dwError, 0);
    pValueMetaData->dwValSize = 15;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, 0);

    assert_memory_equal(
            bervValueMetaData.lberbv_val,
            "member:620972:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334:620972:1:15:abcd:12345:efgh",
            bervValueMetaData.lberbv_len);
    VMDIR_SAFE_FREE_MEMORY(bervValueMetaData.lberbv_val);
}

VOID
VmDirValueMetaDataSerialize_InvalidInput(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      bervValueMetaData = VDIR_BERVALUE_INIT;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    pValueMetaData = *state;

    dwError = VmDirValueMetaDataSerialize(NULL, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirValueMetaDataSerialize(NULL, &bervValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(bervValueMetaData.lberbv_val);

    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszAttrType);
    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(bervValueMetaData.lberbv_val);

    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszOrigInvoId);
    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(bervValueMetaData.lberbv_val);

    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValChgOrigTime);
    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValChgOrigInvoId);
    VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValue);
    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(bervValueMetaData.lberbv_val);
}
