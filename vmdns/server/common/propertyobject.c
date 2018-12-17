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
VmDnsPropertyObjectCreate(
    PVMDNS_PROPERTY           pProperty,
    PVMDNS_PROPERTY_OBJECT    *ppPropertyObj
    )
{
    DWORD dwError = 0;
    PVMDNS_PROPERTY_OBJECT pPropertyObj = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppPropertyObj, dwError);

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_PROPERTY_OBJECT),
                    (PVOID*)&pPropertyObj);
    BAIL_ON_VMDNS_ERROR(dwError);

    pPropertyObj->nRefCount = 1;
    pPropertyObj->pProperty = pProperty;

    *ppPropertyObj = pPropertyObj;

cleanup:
    return dwError;

error:

    if (ppPropertyObj)
    {
        *ppPropertyObj = NULL;
    }

    VmDnsPropertyObjectRelease(pPropertyObj);
    goto cleanup;
}

DWORD
VmDnsPropertyObjectAddRef(
    PVMDNS_PROPERTY_OBJECT    pPropertyObj
    )
{
    DWORD dwError = 0;
    if (pPropertyObj)
    {
        dwError = InterlockedIncrement(&pPropertyObj->nRefCount);
    }
    return dwError;
}

VOID
VmDnsPropertyObjectRelease(
    PVMDNS_PROPERTY_OBJECT    pPropertyObj
    )
{
    if (pPropertyObj)
    {
        if (0 == InterlockedDecrement(&pPropertyObj->nRefCount))
        {
            VMDNS_FREE_PROPERTY(pPropertyObj->pProperty);
            VMDNS_SAFE_FREE_MEMORY(pPropertyObj);
        }
    }
}
