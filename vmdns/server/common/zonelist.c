/*
* Copyright � 2012-2015 VMware, Inc.  All Rights Reserved.
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


/*
* Module Name:  zonelist.c
*
* Abstract: List of zones mananged by the cache.
*
*/

#include "includes.h"

DWORD
VmDnsZoneListInit(
    PVMDNS_ZONE_LIST* ppDnsList
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_LIST pZoneList = NULL;

    if (!ppDnsList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE_LIST), (PVOID*)&pZoneList);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsList = pZoneList;
    pZoneList = NULL;

cleanup:
    return dwError;

error:

    if (ppDnsList)
    {
        *ppDnsList = NULL;
    }

    if (pZoneList)
    {
        VmDnsZoneListCleanup(pZoneList);
    }

    goto cleanup;
}

VOID
VmDnsZoneListCleanup(
    PVMDNS_ZONE_LIST pDnsZoneList
    )
{
    if (pDnsZoneList)
    {
        int i = 0;
        for (; i < VMDNS_MAX_ZONES; ++i)
        {
            VmDnsZoneObjectRelease(pDnsZoneList->Zones[i]);
        }
        VmDnsFreeMemory(pDnsZoneList);
    }
}

DWORD
VmDnsZoneListAddZone(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PVMDNS_ZONE_OBJECT  pZoneObject
    )
{
    DWORD dwError = 0;
    int i = 0;

    for (; i < VMDNS_MAX_ZONES; ++i)
    {
        if (!pDnsZoneList->Zones[i])
        {
            VmDnsZoneObjectAddRef(pZoneObject);
            pDnsZoneList->Zones[i] = pZoneObject;
            break;
        }
        else
        {
            PVMDNS_ZONE_OBJECT pObject = pDnsZoneList->Zones[i];
            if (0 == VmDnsStringCompareA(pObject->pszName,pZoneObject->pszName , FALSE))
            {
                break;
            }
        }
    }

    if (i == VMDNS_MAX_ZONES)
    {
        dwError = ERROR_OUT_OF_STRUCTURES;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsZoneListRemoveZone(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PVMDNS_ZONE_OBJECT  pZoneObject
    )
{
    DWORD dwError = 0;
    int i = 0;

    for (; i < VMDNS_MAX_ZONES; ++i)
    {
        if (pDnsZoneList->Zones[i] == pZoneObject)
        {
            VmDnsZoneObjectRelease(pZoneObject);
            pDnsZoneList->Zones[i] = NULL;
            break;
        }
    }

    if (i == VMDNS_MAX_ZONES)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsZoneListFindZone(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PCSTR               szZoneName,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    )
{
    DWORD dwError = 0;
    int i = 0;
    PSTR szName = NULL;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    for (; i < VMDNS_MAX_ZONES; ++i)
    {
        if (pDnsZoneList->Zones[i])
        {
            dwError = VmDnsZoneGetName(pDnsZoneList->Zones[i], &szName);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (0 == VmDnsStringCompareA(szZoneName, szName, FALSE))
            {
                pZoneObject = pDnsZoneList->Zones[i];
                VmDnsZoneObjectAddRef(pZoneObject);
                break;
            }

            VMDNS_SAFE_FREE_STRINGA(szName);
        }
    }

    if (!pZoneObject)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *ppZoneObject = pZoneObject;

cleanup:
    VMDNS_SAFE_FREE_STRINGA(szName);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsZoneListFindZoneByQName(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PCSTR               sQName,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    )
{
    DWORD dwError = 0;
    int i = 0, nMaxMatchedIndex = -1;
    PSTR szZoneName = NULL;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    size_t nZoneNameLength = 0, nStartCursor = 0, nQueryNameLength = 0;
    size_t nMaxNameMatched = 0;

    nQueryNameLength = strlen(sQName);

    for (; i < VMDNS_MAX_ZONES; ++i)
    {
        if (pDnsZoneList->Zones[i])
        {
            dwError = VmDnsZoneGetName(pDnsZoneList->Zones[i], &szZoneName);
            BAIL_ON_VMDNS_ERROR(dwError);

            nZoneNameLength = strlen(szZoneName);
            nStartCursor = nQueryNameLength - nZoneNameLength;

            if (nStartCursor >= 0 &&
                !VmDnsStringNCompareA(
                        &sQName[nStartCursor],
                        szZoneName,
                        nZoneNameLength,
                        FALSE)
                        )
            {
                if (nZoneNameLength > nMaxNameMatched)
                {
                    nMaxNameMatched = nZoneNameLength;
                    nMaxMatchedIndex = i;
                }
            }

            VMDNS_SAFE_FREE_STRINGA(szZoneName);
        }
    }

    if (nMaxMatchedIndex != -1 && nMaxMatchedIndex < VMDNS_MAX_ZONES)
    {
        pZoneObject = pDnsZoneList->Zones[nMaxMatchedIndex];
        VmDnsZoneObjectAddRef(pZoneObject);
    }
    else
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *ppZoneObject = pZoneObject;

cleanup:
    VMDNS_SAFE_FREE_STRINGA(szZoneName);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsZoneListGetZones(
    PVMDNS_ZONE_LIST    pDnsZoneList,
    PVMDNS_ZONE_INFO_ARRAY  *ppZoneArray
    )
{
    DWORD dwError = 0, dwCount = 0;
    int i = 0;
    PVMDNS_ZONE_INFO_ARRAY pZoneArray = NULL;

    for (; i < VMDNS_MAX_ZONES; ++i)
    {
        if (pDnsZoneList->Zones[i])
        {
            ++dwCount;
        }
    }

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_ZONE_INFO_ARRAY),
                        (void**)&pZoneArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_ZONE_INFO)*dwCount,
                        (void**)&pZoneArray->ZoneInfos);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (i = 0; i < VMDNS_MAX_ZONES; ++i)
    {
        if (pDnsZoneList->Zones[i])
        {
            dwError = VmDnsZoneCopyZoneInfo(
                                pDnsZoneList->Zones[i],
                                &pZoneArray->ZoneInfos[i]);
            BAIL_ON_VMDNS_ERROR(dwError);

            ++pZoneArray->dwCount;
        }
    }

    *ppZoneArray = pZoneArray;

cleanup:
    return dwError;

error:
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);
    goto cleanup;
}
