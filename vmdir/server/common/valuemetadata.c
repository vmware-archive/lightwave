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

// Format is: <attr-name>:<local-usn>:<version-no>:<originating-server-id>:
// <value-change-originating-server-id>:<value-change-originating time>:
// <value-change-originating-usn>:<opcode>:<value-size>:<value>
DWORD
VmDirValueMetaDataDeserialize(
    PCSTR                               pszValueMetaData,
    PVMDIR_VALUE_ATTRIBUTE_METADATA*    ppValueMetaData
    )
{
    DWORD                              dwError = 0;
    PSTR                               pszValue = NULL;
    PCSTR                              pDelimiter = ":";
    PVMDIR_STRING_LIST                 pStrList = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pszValueMetaData || !ppValueMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOG_INFO(
            LDAP_DEBUG_REPL_ATTR, "%s: value: %s ", __FUNCTION__, pszValueMetaData);

    dwError = VmDirStringToTokenList(pszValueMetaData, pDelimiter, &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    //value could contain ':'
    if (pStrList->dwCount < 10)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_VALUE_ATTRIBUTE_METADATA), (PVOID*)&pValueMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    pValueMetaData->pszAttrType = (PSTR)pStrList->pStringList[0];
    pStrList->pStringList[0] = NULL;

    dwError = VmDirStringToINT64(pStrList->pStringList[1], NULL, &pValueMetaData->localUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToUINT64(pStrList->pStringList[2], NULL, &pValueMetaData->version);
    BAIL_ON_VMDIR_ERROR(dwError);

    pValueMetaData->pszOrigInvoId = (PSTR)pStrList->pStringList[3];
    pStrList->pStringList[3] = NULL;

    pValueMetaData->pszValChgOrigInvoId = (PSTR)pStrList->pStringList[4];
    pStrList->pStringList[4] = NULL;

    pValueMetaData->pszValChgOrigTime = (PSTR)pStrList->pStringList[5];
    pStrList->pStringList[5] = NULL;

    dwError = VmDirStringToINT64(pStrList->pStringList[6], NULL, &pValueMetaData->valChgOrigUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToUINT32(pStrList->pStringList[7], NULL, &pValueMetaData->dwOpCode);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToUINT32(pStrList->pStringList[8], NULL, &pValueMetaData->dwValSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * Encoded value could have null characters - treat value as blob
     * value could contain ':' so safer way to find the value is the
     * iterating from the beginnging
     */
    pszValue = (PSTR)pszValueMetaData;
    VALUE_META_TO_NEXT_FIELD(pszValue, 9);

    dwError = VmDirAllocateAndCopyMemory(
            pszValue,
            pValueMetaData->dwValSize,
            (PVOID*)&pValueMetaData->pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirValueMetaDataIsValid(pValueMetaData) == FALSE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    }

    *ppValueMetaData = pValueMetaData;

cleanup:
    VmDirStringListFree(pStrList);
    return dwError;

error:
    VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirValueMetaDataSerialize(
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData,
    PVDIR_BERVALUE                     pBervValueMetaData
    )
{
    DWORD    dwError = 0;
    DWORD    dwLength = 0;
    DWORD    dwPartialValueMetaDataLen = 0;
    PSTR     pszValueMetaData = NULL;

    if (!pBervValueMetaData ||
        VmDirValueMetaDataIsEmpty(pValueMetaData))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    //+1 for null character, so it is safe to print pszValueMetaData for logging
    dwLength = VmDirStringLenA(pValueMetaData->pszAttrType) +
               pValueMetaData->dwValSize +
               VMDIR_PARTIAL_ATTR_VALUE_META_DATA_LEN + 1;

    dwError = VmDirAllocateMemory(dwLength, (PVOID*)&pszValueMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(
            pszValueMetaData,
            dwLength,
            "%s:%"PRId64":%"PRIu64":%s:%s:%s:%"PRId64":%u:%u:",
            pValueMetaData->pszAttrType,
            pValueMetaData->localUsn,
            pValueMetaData->version,
            pValueMetaData->pszOrigInvoId,
            pValueMetaData->pszValChgOrigInvoId,
            pValueMetaData->pszValChgOrigTime,
            pValueMetaData->valChgOrigUsn,
            pValueMetaData->dwOpCode,
            pValueMetaData->dwValSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Encoded value could have null characters - treat value as blob
    dwPartialValueMetaDataLen = VmDirStringLenA(pszValueMetaData);

    dwError = VmDirCopyMemory(
            pszValueMetaData + dwPartialValueMetaDataLen,
            (dwLength - dwPartialValueMetaDataLen),
            pValueMetaData->pszValue,
            pValueMetaData->dwValSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(
            LDAP_DEBUG_REPL_ATTR, "%s: value: %s ", __FUNCTION__, pszValueMetaData);

    pBervValueMetaData->lberbv_val = pszValueMetaData;
    pBervValueMetaData->lberbv_len = dwPartialValueMetaDataLen + pValueMetaData->dwValSize;
    pBervValueMetaData->bOwnBvVal = TRUE;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirValueMetaDataCreate(
    PCSTR                               pszAttrType,
    USN                                 localUsn,
    UINT64                              version,
    PCSTR                               pszOrigInvoId,
    PCSTR                               pszValChgOrigInvoId,
    PCSTR                               pszValChgOrigTime,
    USN                                 valChgOrigUsn,
    DWORD                               dwOpCode,
    PVDIR_BERVALUE                      pBervValue,
    PVMDIR_VALUE_ATTRIBUTE_METADATA*    ppValueMetaData
    )
{
    DWORD                              dwError = 0;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pszAttrType ||
        !pszOrigInvoId ||
        !pszValChgOrigInvoId ||
        !pszValChgOrigTime ||
        !pBervValue ||
        !ppValueMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_VALUE_ATTRIBUTE_METADATA), (PVOID*)&pValueMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszAttrType, &pValueMetaData->pszAttrType);
    BAIL_ON_VMDIR_ERROR(dwError);

    pValueMetaData->localUsn = localUsn;
    pValueMetaData->version = version;

    dwError = VmDirAllocateStringA(pszOrigInvoId, &pValueMetaData->pszOrigInvoId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszValChgOrigInvoId, &pValueMetaData->pszValChgOrigInvoId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszValChgOrigTime, &pValueMetaData->pszValChgOrigTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    pValueMetaData->valChgOrigUsn = valChgOrigUsn;
    pValueMetaData->dwOpCode = dwOpCode;

    dwError = VmDirAllocateAndCopyMemory(
            pBervValue->lberbv_val,
            pBervValue->lberbv_len,
            (PVOID*)&pValueMetaData->pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    pValueMetaData->dwValSize = pBervValue->lberbv_len;

    *ppValueMetaData = pValueMetaData;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    goto cleanup;
}

DWORD
VmDirAttributeValueMetaDataToList(
    PVDIR_ATTRIBUTE       pAttrAttrValueMetaData,
    PVDIR_LINKED_LIST*    ppValueMetaDataList
    )
{
    DWORD                              dwError = 0;
    DWORD                              dwCnt = 0;
    PVDIR_LINKED_LIST                  pValueMetaDataList = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pAttrAttrValueMetaData || !ppValueMetaDataList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListCreate(&pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0;
         pAttrAttrValueMetaData->vals[dwCnt].lberbv.bv_val != NULL;
         dwCnt++)
    {
        dwError = VmDirValueMetaDataDeserialize(
                pAttrAttrValueMetaData->vals[dwCnt].lberbv.bv_val, &pValueMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(pValueMetaDataList, pValueMetaData, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pValueMetaData = NULL;
    }

    *ppValueMetaDataList = pValueMetaDataList;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeValueMetaDataList(pValueMetaDataList);
    VmDirFreeValueMetaData(pValueMetaData);
    goto cleanup;
}

DWORD
VmDirAttributeValueMetaDataListConvertToDequeue(
    PVDIR_LINKED_LIST    pValueMetaDataList,
    PDEQUE               pValueMetaDataQueue
    )
{
    DWORD                              dwError = 0;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;
    PVDIR_LINKED_LIST_NODE             pCurrNode = NULL;
    PVDIR_LINKED_LIST_NODE             pNextNode = NULL;

    if (!pValueMetaDataList || !pValueMetaDataQueue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListGetHead(pValueMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pCurrNode->pElement;

        pNextNode = pCurrNode->pNext;

        dwError = dequePush(pValueMetaDataQueue, pValueMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListRemove(pValueMetaDataList, pCurrNode);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCurrNode = pNextNode;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeAttrValueMetaDataDequeueContent(pValueMetaDataQueue);
    goto cleanup;
}

BOOLEAN
VmDirValueMetaDataIsValid(
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData
    )
{
    BOOLEAN    bIsValid = FALSE;

    if (pValueMetaData &&
        !IsNullOrEmptyString(pValueMetaData->pszAttrType) &&
        !IsNullOrEmptyString(pValueMetaData->pszOrigInvoId) &&
        !IsNullOrEmptyString(pValueMetaData->pszValChgOrigInvoId) &&
        !IsNullOrEmptyString(pValueMetaData->pszValChgOrigTime) &&
        !IsNullOrEmptyString(pValueMetaData->pszValue) &&
        pValueMetaData->localUsn != 0 &&
        pValueMetaData->valChgOrigUsn != 0)
    {
        bIsValid = TRUE;
    }

    return bIsValid;
}

BOOLEAN
VmDirValueMetaDataIsEmpty(
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData
    )
{
    BOOLEAN    bEmpty = FALSE;

    if (!pValueMetaData ||
        IsNullOrEmptyString(pValueMetaData->pszAttrType) ||
        pValueMetaData->localUsn == 0)
    {
        bEmpty = TRUE;
    }

    return bEmpty;
}

VOID
VmDirFreeValueMetaData(
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData
    )
{
    if (pValueMetaData)
    {
        VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszAttrType);
        VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszOrigInvoId);
        VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValChgOrigInvoId);
        VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValChgOrigTime);
        VMDIR_SAFE_FREE_MEMORY(pValueMetaData->pszValue);
        VMDIR_SAFE_FREE_MEMORY(pValueMetaData);
    }
}

VOID
VmDirFreeValueMetaDataList(
    PVDIR_LINKED_LIST    pValueMetaDataList
    )
{
    PVDIR_LINKED_LIST_NODE             pNode = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (pValueMetaDataList)
    {
        VmDirLinkedListGetHead(pValueMetaDataList, &pNode);
        while (pNode)
        {
            if (pNode->pElement)
            {
                pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pNode->pElement;
                VmDirFreeValueMetaData(pValueMetaData);
            }
            pNode = pNode->pNext;
        }
        VmDirFreeLinkedList(pValueMetaDataList);
    }
}

VOID
VmDirFreeAttrValueMetaDataDequeueContent(
    PDEQUE  pValueMetaDataQueue
    )
{
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;
    while(!dequeIsEmpty(pValueMetaDataQueue))
    {
        dequePopLeft(pValueMetaDataQueue, (PVOID*)&pValueMetaData);
        VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    }
}
