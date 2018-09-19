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

DWORD
VmDirReplMetaDataDeserialize(
    PCSTR                              pszReplMetaData,
    PVMDIR_REPL_ATTRIBUTE_METADATA*    ppReplMetaData
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    if (!pszReplMetaData || !ppReplMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // Format is: <attr name>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
    char* pszMetaData = VmDirStringChrA(pszReplMetaData, ':');
    if (pszMetaData == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_INVALID_METADATA);
    }

    // pszMetaData now points to <local USN>...
    pszMetaData++;

    //pszReplMetaData has <attr type>\0<pszMetaData>
    *(pszMetaData - 1) = '\0';

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPL_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszReplMetaData, &pReplMetaData->pszAttrType);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMetaDataDeserialize(pszMetaData, &pReplMetaData->pMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppReplMetaData = pReplMetaData;

cleanup:
    return dwError;

error:
    VmDirFreeReplMetaData(pReplMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirReplMetaDataCreate(
    PCSTR                              pszAttrType,
    USN                                localUsn,
    UINT64                             version,
    PCSTR                              pszOrigInvoId,
    PCSTR                              pszOrigTime,
    USN                                origUsn,
    PVMDIR_REPL_ATTRIBUTE_METADATA*    ppReplMetaData
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    if (IsNullOrEmptyString(pszAttrType) ||
        IsNullOrEmptyString(pszOrigInvoId) ||
        IsNullOrEmptyString(pszOrigTime) ||
        !ppReplMetaData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPL_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszAttrType, &pReplMetaData->pszAttrType);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMetaDataCreate(
            localUsn,
            version,
            pszOrigInvoId,
            pszOrigTime,
            origUsn,
            &pReplMetaData->pMetaData);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppReplMetaData = pReplMetaData;

cleanup:
    return dwError;

error:
    VmDirFreeReplMetaData(pReplMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirFreeReplMetaDataList(
    PVDIR_LINKED_LIST    pMetaDataList
    )
{
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    if (pMetaDataList)
    {
        VmDirLinkedListGetHead(pMetaDataList, &pNode);
        while (pNode)
        {
            if (pNode->pElement)
            {
                pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pNode->pElement;
                VmDirFreeReplMetaData(pReplMetaData);
            }
            pNode = pNode->pNext;
        }
        VmDirFreeLinkedList(pMetaDataList);
    }
}

VOID
VmDirFreeReplMetaData(
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData
    )
{
    if (pReplMetaData)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplMetaData->pszAttrType);
        VmDirFreeMetaData(pReplMetaData->pMetaData);
        VMDIR_SAFE_FREE_MEMORY(pReplMetaData);
    }
}
