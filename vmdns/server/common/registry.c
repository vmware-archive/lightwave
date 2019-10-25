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


#define VMDNS_REG_VALUE_FORWARDER "Forwarders"

static
DWORD
VmDnsArrayToString(
    PCSTR* ppszStringArray,
    DWORD dwCount,
    PSTR* ppszString,
    PDWORD pdwLengh
    );


static
DWORD
VmDnsStringToArray(
    PCSTR   ppszString,
    DWORD  dwStringLength,
    PSTR** ppszStringArray,
    PDWORD pdwCount
    );

DWORD
VmDnsRegSaveForwarders(
    DWORD           dwCount,
    PCSTR*           ppszForwarders
    )
{
    DWORD dwError = 0;
    PSTR pszStringForwarders = NULL;
    DWORD dwStringForwardersLen = 0;

    if (!ppszForwarders)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsArrayToString(
                          ppszForwarders,
                          dwCount,
                          &pszStringForwarders,
                          &dwStringForwardersLen
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRegCfgSetKeyMultiSZA(
            VMDNS_CONFIG_PARAMETER_KEY_PATH,
            VMDNS_REG_VALUE_FORWARDER,
            pszStringForwarders,
            (size_t)dwStringForwardersLen);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_MEMORY(pszStringForwarders);

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsRegLoadForwarders(
    PDWORD         pdwCount,
    PSTR**         pppszForwarders
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;
    PSTR*   ppszForwarders = NULL;
    char    szValue[VMDNS_MAX_CONFIG_VALUE_LENGTH] = {0};
    size_t  dwszValueSize = sizeof(szValue);


    if (!pdwCount || !pppszForwarders)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmRegCfgGetKeyMultiSZA(
        VMDNS_CONFIG_PARAMETER_KEY_PATH,
        VMDNS_REG_VALUE_FORWARDER,
        szValue,
        &dwszValueSize);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_NO_DATA;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStringToArray(
                            szValue,
                            dwszValueSize,
                            &ppszForwarders,
                            &dwCount
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    *pppszForwarders = ppszForwarders;
    *pdwCount = dwCount;

cleanup:
    return dwError;

error:

    if (pppszForwarders)
    {
        *pppszForwarders = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppszForwarders)
    {
        VmDnsFreeStringArrayA(ppszForwarders);
    }

    goto cleanup;
}

static
DWORD
VmDnsArrayToString(
    PCSTR*  ppszStringArray,
    DWORD  dwCount,
    PSTR*  ppszString,
    PDWORD pdwStringLength
    )
{
    DWORD dwError = 0;
    PSTR  pszString = NULL;
    PSTR  pszCursor = NULL;
    DWORD dwStrLength = 0;
    DWORD dwStrLengthUsed = 0;
    DWORD dwIndex = 0;

    if (!ppszStringArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for(; dwIndex < dwCount; ++dwIndex)
    {
        dwStrLength += (VmDnsStringLenA(ppszStringArray[dwIndex])) + 1;
    }
    dwStrLength++;

    dwError = VmDnsAllocateMemory(dwStrLength, (PVOID*)&pszString);
    BAIL_ON_VMDNS_ERROR(dwError);

    pszCursor = pszString;

    for (dwIndex = 0; dwIndex < dwCount; ++dwIndex)
    {
        DWORD dwCurrStrLen = VmDnsStringLenA(ppszStringArray[dwIndex]);
        dwError = VmDnsStringNCpyA(
                            pszCursor,
                            dwStrLength - dwStrLengthUsed,
                            ppszStringArray[dwIndex],
                            dwCurrStrLen
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwStrLengthUsed+= dwCurrStrLen+1;
        pszCursor+= dwCurrStrLen+1;
    }

    *ppszString = pszString;
    *pdwStringLength = dwStrLength;
cleanup:

    return dwError;
error:

    if (ppszString)
    {
        *ppszString = NULL;
    }
    if (pdwStringLength)
    {
        *pdwStringLength = 0;
    }
    VMDNS_SAFE_FREE_MEMORY(pszString);
    goto cleanup;
}


static
DWORD
VmDnsStringToArray(
    PCSTR   pszString,
    DWORD  dwStringLength,
    PSTR** pppszStringArray,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PCSTR  pszCursor = pszString;

    PSTR* ppszStringArray = NULL;
    DWORD dwCount = 0;

    while (pszCursor && VmDnsStringLenA(pszCursor))
    {
        DWORD dwCursorLength = VmDnsStringLenA(pszCursor);
        ++dwCount;
        pszCursor += dwCursorLength+1;
    }

    if (dwCount)
    {
        dwError = VmDnsAllocateMemory(sizeof(PSTR)*dwCount, (PVOID*)&ppszStringArray);
        BAIL_ON_VMDNS_ERROR(dwError);

        pszCursor = pszString;

        for (;dwIndex<dwCount;++dwIndex)
        {
            DWORD dwCursorLength = VmDnsStringLenA(pszCursor);
            dwError = VmDnsAllocateStringA(
                                      pszCursor,
                                      &ppszStringArray[dwIndex]
                                      );
            BAIL_ON_VMDNS_ERROR(dwError);
            pszCursor+=dwCursorLength+1;
        }
    }

    *pppszStringArray = ppszStringArray;
    *pdwCount = dwCount;

cleanup:

    return dwError;
error:

    if (pppszStringArray)
    {
        *pppszStringArray = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (ppszStringArray)
    {
        VmDnsFreeStringArrayA(ppszStringArray);
    }

    goto cleanup;

}
