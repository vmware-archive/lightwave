/*
* Copyright � 2012-2018 VMware, Inc.  All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the �License�); you may not
* use this file except in compliance with the License.  You may obtain a copy
* of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an �AS IS� BASIS, without
* warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
* License for the specific language governing permissions and limitations
* under the License.
*/


#include "includes.h"

DWORD
VmDnsPropertyListCreate(
    PVMDNS_PROPERTY_LIST          *ppList
    )
{
    DWORD dwError = 0;
    PVMDNS_PROPERTY_LIST pList = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppList, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_PROPERTY_LIST), (PVOID*)&pList);
    BAIL_ON_VMDNS_ERROR(dwError);

    pList->dwCurrentSize = 0;
    pList->dwMaxSize = 10;

    dwError = VmDnsAllocateMemory(
                        sizeof(PVMDNS_PROPERTY_OBJECT)* pList->dwMaxSize,
                        (PVOID*)&pList->ppProperties);
    BAIL_ON_VMDNS_ERROR(dwError);

    pList->nRefCount = 1;
    *ppList = pList;

cleanup:
    return dwError;

error:

    if (ppList)
    {
        *ppList = NULL;
    }

    VmDnsPropertyListRelease(pList);
    goto cleanup;
}

DWORD
VmDnsPropertyListAdd(
    PVMDNS_PROPERTY_LIST          pList,
    PVMDNS_PROPERTY_OBJECT        pProperty
    )
{
    PVMDNS_PROPERTY_OBJECT *pNewList = NULL;
    DWORD dwError = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pList, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pProperty, dwError);

    if (pList->dwCurrentSize >= pList->dwMaxSize)
    {
        pList->dwMaxSize = 2 * pList->dwMaxSize;

        dwError = VmDnsAllocateMemory(
                        sizeof(PVMDNS_PROPERTY_OBJECT) * pList->dwMaxSize,
                        (PVOID*)&pNewList);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsCopyMemory(
                        pNewList,
                        sizeof(PVMDNS_PROPERTY_OBJECT) * pList->dwMaxSize,
                        pList->ppProperties,
                        sizeof(PVMDNS_PROPERTY_OBJECT) * pList->dwCurrentSize);
        BAIL_ON_VMDNS_ERROR(dwError);

        VMDNS_SAFE_FREE_MEMORY(pList->ppProperties);
        pList->ppProperties = pNewList;
        pNewList = NULL;
    }

    VmDnsPropertyObjectAddRef(pProperty);
    pList->ppProperties[pList->dwCurrentSize] = pProperty;
    ++pList->dwCurrentSize;

cleanup:
    return dwError;

error:

    VMDNS_SAFE_FREE_MEMORY(pNewList);
    goto cleanup;
}

DWORD
VmDnsPropertyListAddList(
    PVMDNS_PROPERTY_LIST          pDestList,
    PVMDNS_PROPERTY_LIST          pSrcList
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD dwPropertyListSize = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pDestList, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pSrcList, dwError);

    if (pSrcList->dwCurrentSize == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwPropertyListSize = VmDnsPropertyListGetSize(pSrcList);

    for (i = 0; i < dwPropertyListSize; i++)
    {
        dwError = VmDnsPropertyListAdd(pDestList, pSrcList->ppProperties[i]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

PVMDNS_PROPERTY_OBJECT
VmDnsPropertyListGetProperty(
    PVMDNS_PROPERTY_LIST pList,
    UINT                 nIndex
    )
{
    PVMDNS_PROPERTY_OBJECT pPropertyObject = NULL;

    if (pList && pList->dwCurrentSize > nIndex)
    {
        pPropertyObject = pList->ppProperties[nIndex];
        VmDnsPropertyObjectAddRef(pPropertyObject);
    }

    return pPropertyObject;
}

UINT
VmDnsPropertyListGetSize(
    PVMDNS_PROPERTY_LIST pList
    )
{
    assert(pList);
    return pList->dwCurrentSize;
}

VOID
VmDnsPropertyListFree(
    PVMDNS_PROPERTY_LIST pList
    )
{
    DWORD i = 0;

    if (pList)
    {
        for (i = 0; i < pList->dwCurrentSize; ++i)
        {
            VmDnsPropertyObjectRelease(pList->ppProperties[i]);
        }
        VMDNS_SAFE_FREE_MEMORY(pList->ppProperties);
        VMDNS_SAFE_FREE_MEMORY(pList);
    }
}

DWORD
VmDnsPropertyListAddRef(
    PVMDNS_PROPERTY_LIST pList
    )
{
    DWORD dwError = 0;
    if (pList)
    {
        dwError = InterlockedIncrement(&pList->nRefCount);
    }
    return dwError;
}

VOID
VmDnsPropertyListRelease(
    PVMDNS_PROPERTY_LIST pList
    )
{
    if (pList)
    {
        if (0 == InterlockedDecrement(&pList->nRefCount))
        {
            VmDnsPropertyListFree(pList);
        }
    }
}
