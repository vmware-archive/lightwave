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
VmDirSetupMetaData(
    VOID    **state
    )
{
    DWORD                        dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA    pMetaData = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_ATTRIBUTE_METADATA), (PVOID*)&pMetaData);
    assert_int_equal(dwError, 0);

    pMetaData->localUsn = 123;
    pMetaData->version = 56;

    dwError = VmDirAllocateStringA(
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd", &pMetaData->pszOrigInvoId);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA("20180702222545.584", &pMetaData->pszOrigTime);
    assert_int_equal(dwError, 0);

    pMetaData->origUsn = 100;

    *state = pMetaData;

    return 0;
}


int
VmDirMetaDataFree(
    VOID    **state
    )
{
    VmDirFreeMetaData((PVMDIR_ATTRIBUTE_METADATA)*state);

    return 0;
}


int
VmDirSetupMetaDataAttribute(
    VOID    **state
    )
{
    DWORD             dwError = 0;
    PVDIR_ATTRIBUTE   pAttrAttrMetaData = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ATTRIBUTE),
            (PVOID*)&pAttrAttrMetaData);
    assert_int_equal(dwError, 0);

    // add one more BerValue as Encode/Decode entry in data store layer needs it.
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * 3,
            (PVOID*)&pAttrAttrMetaData->vals);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(
           "cn:100:23:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584:100",
          &pAttrAttrMetaData->vals[0].lberbv.bv_val);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(
           "sn:110:25:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584:105",
          &pAttrAttrMetaData->vals[1].lberbv.bv_val);
    assert_int_equal(dwError, 0);

    pAttrAttrMetaData->vals[2].lberbv.bv_val = NULL;

    *state = pAttrAttrMetaData;

    return 0;
}


int
VmDirFreeMetaDataAttribute(
    VOID    **state
    )
{
    PVDIR_ATTRIBUTE   pAttrAttrMetaData = NULL;

    pAttrAttrMetaData = *state;

    VMDIR_SAFE_FREE_MEMORY(pAttrAttrMetaData->vals[0].lberbv.bv_val);
    VMDIR_SAFE_FREE_MEMORY(pAttrAttrMetaData->vals[1].lberbv.bv_val);

    VMDIR_SAFE_FREE_MEMORY(pAttrAttrMetaData->vals);

    VMDIR_SAFE_FREE_MEMORY(pAttrAttrMetaData);

    return 0;
}

//unit test functions - VmDirMetaDataDeserialize
VOID
VmDirMetaDataDeserialize_ValidInput(
    VOID    **state
    )
{
    DWORD                        dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA    pMetaData = NULL;

    dwError = VmDirMetaDataDeserialize(
            "12345:58:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584:12345",
            &pMetaData);
    assert_int_equal(dwError, 0);

    assert_int_equal(pMetaData->localUsn, 12345);
    assert_int_equal(pMetaData->version, 58);
    assert_string_equal(pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pMetaData->origUsn, 12345);

    VmDirFreeMetaData(pMetaData);
}

VOID
VmDirMetaDataDeserialize_InvalidInput(
    VOID    **state
    )
{
    DWORD                        dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA    pMetaData = NULL;

    //Null input
    dwError = VmDirMetaDataDeserialize(NULL, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirMetaDataDeserialize("", &pMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(pMetaData);

    dwError = VmDirMetaDataDeserialize(
            "12345:58:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584:12345",
            NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(pMetaData);

    //Invalid MetaData: 4 elements
    dwError = VmDirMetaDataDeserialize(
            "12345:58:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584",
            &pMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    assert_null(pMetaData);

    //Invalid MetaData: 6 elements
    dwError = VmDirMetaDataDeserialize(
            "12345:58:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584:123:123",
            &pMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    assert_null(pMetaData);

    //Invalid MetaData: empty elements
    dwError = VmDirMetaDataDeserialize("123:22:::123", &pMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    assert_null(pMetaData);

}

//unit test functions - VmDirMetaDataDeserialize
VOID
VmDirMetaDataSerialize_ValidInput(
    VOID    **state
    )
{
    PSZ_METADATA_BUF            pszMetaData = {'\0'};
    DWORD                       dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;

    pMetaData = *state;

    dwError = VmDirMetaDataSerialize(pMetaData, pszMetaData);
    assert_int_equal(dwError, 0);
    assert_string_equal(
            "123:56:7ef77c0f-cff1-4239-b293-39a2b302d5bd:20180702222545.584:100",
            pszMetaData);
}

VOID
VmDirMetaDataSerialize_InvalidInput(
    VOID    **state
    )
{
    PSZ_METADATA_BUF            pszMetaData = {'\0'};
    DWORD                       dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;

    pMetaData = *state;

    //Null input
    dwError = VmDirMetaDataSerialize(NULL, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirMetaDataSerialize(pMetaData, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirMetaDataSerialize(NULL, pszMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    //Invalid input
    VMDIR_SAFE_FREE_MEMORY(pMetaData->pszOrigInvoId);
    VMDIR_SAFE_FREE_MEMORY(pMetaData->pszOrigTime);
    dwError = VmDirMetaDataSerialize(pMetaData, pszMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
}

//unit test functions - VmDirMetaDataCopyContent
VOID
VmDirMetaDataCopyContent_ValidInput(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;
    VMDIR_ATTRIBUTE_METADATA    destMetaData = {0};

    pMetaData = *state;

    dwError = VmDirMetaDataCopyContent(pMetaData, &destMetaData);
    assert_int_equal(dwError, 0);

    assert_int_equal(pMetaData->localUsn, destMetaData.localUsn);
    assert_int_equal(pMetaData->version, destMetaData.version);
    assert_string_equal(pMetaData->pszOrigInvoId, destMetaData.pszOrigInvoId);
    assert_string_equal(pMetaData->pszOrigTime, destMetaData.pszOrigTime);
    assert_int_equal(pMetaData->origUsn, destMetaData.origUsn);

    VMDIR_SAFE_FREE_MEMORY(destMetaData.pszOrigInvoId);
    VMDIR_SAFE_FREE_MEMORY(destMetaData.pszOrigTime);
}


VOID
VmDirMetaDataCopyContent_InvalidInput(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;
    VMDIR_ATTRIBUTE_METADATA    destMetaData = {0};

    pMetaData = *state;

    //Null input
    dwError = VmDirMetaDataCopyContent(NULL, &destMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirMetaDataCopyContent(pMetaData, NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    //Invalid input
    VMDIR_SAFE_FREE_MEMORY(pMetaData->pszOrigInvoId);
    VMDIR_SAFE_FREE_MEMORY(pMetaData->pszOrigTime);
    dwError = VmDirMetaDataCopyContent(pMetaData, &destMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
}

//unit test functions - VmDirMetaDataCreate
VOID
VmDirMetaDataCreate_ValidInput(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;

    dwError = VmDirMetaDataCreate(
            12345,
            58,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            "20180702222545.584",
            12345,
            &pMetaData);
    assert_int_equal(dwError, 0);

    assert_int_equal(pMetaData->localUsn, 12345);
    assert_int_equal(pMetaData->version, 58);
    assert_string_equal(pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pMetaData->origUsn, 12345);

    VmDirFreeMetaData(pMetaData);
}

VOID
VmDirMetaDataCreate_InvalidInput(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;

    //Null input
    dwError = VmDirMetaDataCreate(
            12345,
            58,
            NULL,
            NULL,
            12345,
            &pMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(pMetaData);

    dwError = VmDirMetaDataCreate(
            12345,
            58,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            "20180702222545.584",
            12345,
            NULL);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    //Invalid input
    dwError = VmDirMetaDataCreate(
            12345,
            58,
            "",
            "20180702222545.584",
            12345,
            &pMetaData);
    assert_int_equal(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    assert_null(pMetaData);
}
