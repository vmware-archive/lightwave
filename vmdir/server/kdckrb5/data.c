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
VmKdcFreeData(
    PVMKDC_DATA pData)
{
    if (pData && pData->bAllocated)
    {
        VMKDC_SAFE_FREE_MEMORY(pData->ptr);
    }
    VMKDC_SAFE_FREE_MEMORY(pData);
}

DWORD
VmKdcAllocateData(
    PVOID        ptr,
    DWORD        len,
    PVMKDC_DATA *ppRetData)
{
    PVMKDC_DATA pData = NULL;
    DWORD dwError = 0;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_DATA), (PVOID*)&pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    pData->len = len;
    pData->bAllocated = (len > VMKDC_DATA_INTERNAL_BUFSIZE);
    if (pData->bAllocated)
    {
        dwError = VmKdcAllocateMemory(len, &pData->ptr);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    else
    {
        pData->ptr = &pData->internalBuf;
    }

    if (ptr)
    {
        memcpy(pData->ptr, ptr, len);
    }

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pData);
    }
    *ppRetData = pData;
    return dwError;
}

DWORD
VmKdcAllocateDataString(
    PCSTR         ptr,
    PVMKDC_DATA *ppRetData)
{

    PVMKDC_DATA pData = NULL;
    DWORD dwError = 0;
    DWORD len = 0;
    DWORD allocLen = 0;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_DATA), (PVOID*)&pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    len = (DWORD) VmKdcStringLenA(ptr);
    allocLen = len + 1;

    pData->bAllocated = (allocLen > VMKDC_DATA_INTERNAL_BUFSIZE);
    if (pData->bAllocated)
    {
        dwError = VmKdcAllocateMemory(allocLen, &pData->ptr);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    else
    {
        pData->ptr = &pData->internalBuf;
    }

    memcpy(pData->ptr, ptr, len);
    *(((PSTR)pData->ptr)+len) = '\0';
    pData->len = len;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pData);
    }
    *ppRetData = pData;
    return dwError;
}

VOID
VmKdcPrintData(
    PVMKDC_DATA pData)
{
    DWORD dwError = 0;
    char *buf = NULL;
    size_t buflen = 0;
    int datalen = 0;
    PUCHAR dataptr = NULL;
    int i = 0;

    BAIL_ON_VMKDC_INVALID_POINTER(pData, dwError);

    datalen = VMKDC_GET_LEN_DATA(pData);
    dataptr = VMKDC_GET_PTR_DATA(pData);

    buflen = datalen * 2 + 1;

    dwError = VmKdcAllocateMemory(buflen, (PVOID*)&buf);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "length: %d", datalen);
    for (i=0; i<datalen; i++)
    {
        sprintf(&buf[i*2], "%02x", dataptr[i]);
    }
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "data: %s", buf);

error:

    VMKDC_SAFE_FREE_MEMORY(buf);
}
