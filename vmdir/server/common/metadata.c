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

//pszMetadata: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
DWORD
VmDirMetaDataDeserialize(
    PCSTR                        pszMetaData,
    PVMDIR_ATTRIBUTE_METADATA*   ppMetaData
    )
{
    DWORD                 dwError = 0;
    PCSTR                 pDelimiter = ":";
    PVMDIR_STRING_LIST    pStrList = NULL;
    PVMDIR_ATTRIBUTE_METADATA    pMetaData = NULL;

    if (!pszMetaData || !ppMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringToTokenList(pszMetaData, pDelimiter, &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pStrList->dwCount != 5)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_ATTRIBUTE_METADATA), (PVOID*) &pMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToINT64(pStrList->pStringList[0], NULL, &pMetaData->localUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToUINT64(pStrList->pStringList[1], NULL, &pMetaData->version);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pStrList->pStringList[2], &pMetaData->pszOrigInvoId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pStrList->pStringList[3], &pMetaData->pszOrigTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToINT64(pStrList->pStringList[4], NULL, &pMetaData->origUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMetaData = pMetaData;

cleanup:
    VmDirStringListFree(pStrList);
    return dwError;

error:
    VmDirFreeMetaData(pMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirMetaDataSerialize(
    PVMDIR_ATTRIBUTE_METADATA    pMetadata,
    PSTR                         pszMetadata
    )
{
    DWORD    dwError = 0;

    if (VmDirMetaDataIsEmpty(pMetadata) || !pszMetadata)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringPrintFA(
            pszMetadata,
            VMDIR_MAX_ATTR_META_DATA_LEN,
            "%"PRId64":%"PRId64":%s:%s:%"PRId64,
            pMetadata->localUsn,
            pMetadata->version,
            pMetadata->pszOrigInvoId,
            pMetadata->pszOrigTime,
            pMetadata->origUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirMetaDataCopyContent(
    PVMDIR_ATTRIBUTE_METADATA    pSrcMetaData,
    PVMDIR_ATTRIBUTE_METADATA    pDestMetaData
    )
{
    DWORD    dwError = 0;

    if (VmDirMetaDataIsEmpty(pSrcMetaData) || !pDestMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pDestMetaData->localUsn = pSrcMetaData->localUsn;
    pDestMetaData->version = pSrcMetaData->version;

    VMDIR_SAFE_FREE_MEMORY(pDestMetaData->pszOrigInvoId);
    dwError = VmDirAllocateStringA(pSrcMetaData->pszOrigInvoId, &pDestMetaData->pszOrigInvoId);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_SAFE_FREE_MEMORY(pDestMetaData->pszOrigTime);
    dwError = VmDirAllocateStringA(pSrcMetaData->pszOrigTime, &pDestMetaData->pszOrigTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDestMetaData->origUsn = pSrcMetaData->origUsn;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirMetaDataCreate(
    USN                           localUsn,
    UINT64                        version,
    PCSTR                         pszOrigInvoId,
    PCSTR                         pszOrigTimeStamp,
    USN                           origUsn,
    PVMDIR_ATTRIBUTE_METADATA*    ppMetaData
    )
{
    DWORD    dwError = 0;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;

    if (!ppMetaData        ||
        IsNullOrEmptyString(pszOrigInvoId) ||
        IsNullOrEmptyString(pszOrigTimeStamp))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_ATTRIBUTE_METADATA), (PVOID*) &pMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMetaData->localUsn = localUsn;
    pMetaData->version = version;

    dwError = VmDirAllocateStringA(pszOrigInvoId, &pMetaData->pszOrigInvoId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszOrigTimeStamp, &pMetaData->pszOrigTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMetaData->origUsn = origUsn;

    *ppMetaData = pMetaData;

cleanup:
    return dwError;

error:
    VmDirFreeMetaData(pMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirMetaDataSetLocalUsn(
    PVMDIR_ATTRIBUTE_METADATA    pMetaData,
    USN                          localUsn
    )
{
    DWORD            dwError = 0;

    if (!pMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pMetaData->localUsn = localUsn;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirAttributeMetaDataToHashMap(
    PVDIR_ATTRIBUTE   pAttrAttrMetaData,
    PLW_HASHMAP*      ppMetaDataMap
    )
{
    DWORD            dwError = 0;
    DWORD            dwCnt = 0;
    PLW_HASHMAP      pMetaDataMap = NULL;
    PSTR             pszKey = NULL;
    PVMDIR_ATTRIBUTE_METADATA   pMetaData = NULL;

    if (!pAttrAttrMetaData || !ppMetaDataMap)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlCreateHashMap(
            &pMetaDataMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; pAttrAttrMetaData->vals[dwCnt].lberbv.bv_val != NULL; dwCnt++)
    {
        // Format is: <attr name>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
        char* pszMetaData = VmDirStringChrA(pAttrAttrMetaData->vals[dwCnt].lberbv.bv_val, ':');
        if (pszMetaData == NULL)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
        }

        // pszMetaData now points to <local USN>...
        pszMetaData++;

        //pAttrAttrMetaData->vals[i] has <attr type>\0<pszMetaData>
        *(pszMetaData - 1) = '\0';

        dwError = VmDirAllocateStringA(pAttrAttrMetaData->vals[dwCnt].lberbv.bv_val, &pszKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMetaDataDeserialize(pszMetaData, &pMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pMetaDataMap, pszKey, pMetaData, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszKey = NULL; pMetaData = NULL;
    }

    *ppMetaDataMap = pMetaDataMap;

cleanup:
    return dwError;

error:
    if (pMetaDataMap)
    {
        LwRtlHashMapClear(
                pMetaDataMap,
                VmDirFreeMetaDataMapPair,
                NULL);
        LwRtlFreeHashMap(&pMetaDataMap);
    }
    VmDirFreeMetaData(pMetaData);
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

BOOLEAN
VmDirMetaDataIsEmpty(
    PVMDIR_ATTRIBUTE_METADATA    pMetaData
    )
{
    BOOLEAN    bEmpty = TRUE;

    if (pMetaData && pMetaData->pszOrigInvoId)
    {
        bEmpty = FALSE;
    }

    return bEmpty;
}

VOID
VmDirFreeMetaData(
    PVMDIR_ATTRIBUTE_METADATA    pMetaData
    )
{
    VmDirFreeMetaDataContent(pMetaData);
    VMDIR_SAFE_FREE_MEMORY(pMetaData);
}

VOID
VmDirFreeMetaDataContent(
    PVMDIR_ATTRIBUTE_METADATA    pMetaData
    )
{
    if (pMetaData)
    {
        VMDIR_SAFE_FREE_MEMORY(pMetaData->pszOrigInvoId);
        VMDIR_SAFE_FREE_MEMORY(pMetaData->pszOrigTime);
        memset(pMetaData, 0, sizeof(VMDIR_ATTRIBUTE_METADATA));
    }
}

VOID
VmDirFreeAttrMetaDataNode(
    PATTRIBUTE_META_DATA_NODE   pAttrMetaData,
    DWORD                       dwNumAttrMetaData
    )
{
    DWORD    dwCnt = 0;

    for (dwCnt=0; dwCnt < dwNumAttrMetaData; dwCnt++)
    {
        VmDirFreeMetaData(pAttrMetaData[dwCnt].pMetaData);
    }

    VMDIR_SAFE_FREE_MEMORY(pAttrMetaData);
}

VOID
VmDirFreeMetaDataMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    PVMDIR_ATTRIBUTE_METADATA pMetaData = NULL;

    if (pPair->pValue)
    {
        pMetaData = (PVMDIR_ATTRIBUTE_METADATA) pPair->pValue;
        VmDirFreeMetaDataContent(pMetaData);
        VMDIR_SAFE_FREE_MEMORY(pMetaData);
        pPair->pValue = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
}
