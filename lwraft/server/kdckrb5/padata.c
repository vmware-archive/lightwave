/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

VOID
VmKdcFreePaData(
    PVMKDC_PADATA pPaData)
{
    if (pPaData)
    {
        VMKDC_SAFE_FREE_DATA(pPaData->data);
        VMKDC_SAFE_FREE_MEMORY(pPaData);
    }
}

VOID
VmKdcFreeMethodData(
    PVMKDC_METHOD_DATA pMethodData)
{
    DWORD i = 0;

    if (pMethodData)
    {
        for (i=0; i<pMethodData->length; i++)
        {
            VMKDC_SAFE_FREE_PADATA(pMethodData->padata[i]);
        }
        VMKDC_SAFE_FREE_MEMORY(pMethodData->padata);
        VMKDC_SAFE_FREE_MEMORY(pMethodData);
    }
}

DWORD
VmKdcAllocateMethodData(
    DWORD length,
    PVMKDC_METHOD_DATA *ppRetMethodData)
{
    DWORD dwError = 0;
    PVMKDC_METHOD_DATA pMethodData = NULL;


    dwError = VmKdcAllocateMemory(sizeof(VMKDC_METHOD_DATA),
                                  (PVOID*)&pMethodData);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(PVMKDC_DATA) * length,
                                  (PVOID*)&pMethodData->padata);
    BAIL_ON_VMKDC_ERROR(dwError);

    pMethodData->length = length;

    *ppRetMethodData = pMethodData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_METHOD_DATA(pMethodData);
    }
    return dwError;
}

DWORD
VmKdcMakePaData(
    VMKDC_PADATA_TYPE type,
    DWORD length,
    PUCHAR contents,
    PVMKDC_PADATA *ppRetPaData)
{
    DWORD dwError = 0;
    PVMKDC_PADATA pPaData = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_PADATA), (PVOID*)&pPaData);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (contents)
    {
        dwError = VmKdcAllocateData(contents, length, &pPaData->data);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pPaData->type = type;
    *ppRetPaData = pPaData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_PADATA(pPaData);
    }
    return dwError;
}

DWORD
VmKdcFindPaData(
    VMKDC_PADATA_TYPE type,
    PVMKDC_METHOD_DATA pMethodData,
    PVMKDC_PADATA *ppRetPaData)
{
    DWORD dwError = ERROR_NOT_FOUND;
    PVMKDC_PADATA pPaData = NULL;
    DWORD i = 0;

    for (i=0; i<pMethodData->length; i++)
    {
        BAIL_ON_VMKDC_INVALID_POINTER(pMethodData->padata[i], dwError);
        pPaData = pMethodData->padata[i];
        if (pPaData->type == type)
        {
            dwError = 0;
            *ppRetPaData = pPaData;
            break;
        }
    }

error:
    return dwError;
}

DWORD
VmKdcEncodeMethodData(
    PVMKDC_METHOD_DATA pMethodData,
    PVMKDC_DATA *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;
    METHOD_DATA heimMethod;
    unsigned char *methodBufPtr = NULL;
    size_t heimMethodLen = 0;
    size_t methodBufLen = 0;
    int err = 0;
    DWORD i = 0;

    memset(&heimMethod, 0, sizeof(heimMethod));

    dwError = VmKdcAllocateMemory(pMethodData->length * sizeof(PA_DATA),
                                  (PVOID*)&heimMethod.val);
    BAIL_ON_VMKDC_ERROR(dwError);
    heimMethod.len = pMethodData->length;

    for (i=0; i<pMethodData->length; i++)
    {
        heimMethod.val[i].padata_type = pMethodData->padata[i]->type;
        if (pMethodData->padata[i]->data)
        {
            heimMethod.val[i].padata_value.length = VMKDC_GET_LEN_DATA(pMethodData->padata[i]->data);
            heimMethod.val[i].padata_value.data = VMKDC_GET_PTR_DATA(pMethodData->padata[i]->data);
        }
    }

    ASN1_MALLOC_ENCODE(METHOD_DATA,
                       methodBufPtr, methodBufLen,
                       &heimMethod, &heimMethodLen,
                       err);
    if (err != 0 || methodBufLen != heimMethodLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(methodBufPtr, (int) methodBufLen, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }
    VMKDC_SAFE_FREE_MEMORY(heimMethod.val);
    if (methodBufPtr)
    {
        free(methodBufPtr);
        methodBufPtr = NULL;
    }

    return dwError;
}

VOID
VmKdcPrintPaData(
    PVMKDC_PADATA pPaData)
{
    /* TBD */
}

VOID
VmKdcPrintMethodData(
    PVMKDC_METHOD_DATA pMethodData)
{
    /* TBD */
}
