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


#if defined(_WIN32) && !defined(WIN2008)
#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define WINVER _WIN32_WINNT
#endif

#include "includes.h"

DWORD
VmDnsWinCfgOpenConnection(
    PVMDNS_CFG_CONNECTION*  ppConnection
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_CONNECTION pConnection = NULL;

    dwError = VmDnsAllocateMemory(sizeof(*pConnection), (PVOID*)&pConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    pConnection->refCount = 1;

    *ppConnection = pConnection;

cleanup:

    return dwError;

error:

    *ppConnection = NULL;

    if (pConnection)
    {
        VmDnsWinCfgCloseConnection(pConnection);
    }

    goto cleanup;
}

DWORD
VmDnsWinCfgOpenRootKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PCSTR                   pszKeyName,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_KEY pKey = NULL;
    VMDNS_CFG_KEY rootKey = {0};

    if (!pConnection || IsNullOrEmptyString(pszKeyName) || !ppKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!strcmp(pszKeyName, "HKEY_LOCAL_MACHINE"))
    {
        rootKey.hKey = HKEY_LOCAL_MACHINE;
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWinCfgOpenKey(
                    pConnection,
                    &rootKey,
                    NULL,
                    dwOptions,
                    dwAccess,
                    &pKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppKey = pKey;

cleanup:

    return dwError;

error:

    if (ppKey)
    {
        *ppKey = NULL;
    }

//    if (pKey)
//    {
//        VmDnsWinCfgCloseKey(pKey);
//    }

    goto cleanup;
}

DWORD
VmDnsWinCfgOpenKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_KEY pKeyLocal = NULL;

    dwError = VmDnsAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    &pKeyLocal->hKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    pKeyLocal->pConnection = VmDnsWinCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmDnsWinCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmDnsWinCfgCreateKey(
    PVMDNS_CFG_CONNECTION   pConnection,
    PVMDNS_CFG_KEY          pKey,
    PCSTR                   pszSubKey,
    DWORD                   dwOptions,
    DWORD                   dwAccess,
    PVMDNS_CFG_KEY*         ppKey
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_KEY pKeyLocal = NULL;

    dwError = VmDnsAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = RegCreateKeyExA(
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    0,
                    NULL,
                    dwOptions,
                    dwAccess,
                    NULL,
                    &pKeyLocal->hKey,
                    NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

    pKeyLocal->pConnection = VmDnsWinCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmDnsWinCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmDnsWinCfgReadStringValue(
    PVMDNS_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;
    CHAR  szValue[VMDNS_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
    DWORD dwszValueSize = sizeof(szValue);
    PSTR  pszValue = NULL;

    dwError = RegGetValueA(
                    pKey->hKey,
                    pszSubkey,
                    pszName,
                    RRF_RT_REG_SZ,
                    NULL,
                    szValue,
                    &dwszValueSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    if (pszValue)
    {
        VmDnsFreeStringA(pszValue);
    }

    goto cleanup;
}

DWORD
VmDnsWinCfgReadDWORDValue(
    PVMDNS_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;
    DWORD dwValueSize = sizeof(dwValue);

    dwError = RegGetValueA(
                    pKey->hKey,
                    pszSubkey,
                    pszName,
                    RRF_RT_REG_DWORD,
                    NULL,
                    (PVOID)&dwValue,
                    &dwValueSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

DWORD
VmDnsWinCfgSetValue(
    PVMDNS_CFG_KEY       pKey,
    PCSTR               pszValue,
    DWORD               dwType,
    PBYTE               pValue,
    DWORD               dwSize
    )
{
    DWORD dwError = 0;

    if (!pKey || IsNullOrEmptyString(pszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = RegSetValueExA(
                    pKey->hKey,
                    pszValue,
                    0,
                    dwType,
                    pValue,
                    dwSize);
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

VOID
VmDnsWinCfgCloseKey(
    PVMDNS_CFG_KEY pKey
    )
{
    if (pKey->pConnection)
    {
        if (pKey->hKey)
        {
            RegCloseKey(pKey->hKey);
        }

        VmDnsWinCfgCloseConnection(pKey->pConnection);
    }
    VmDnsFreeMemory(pKey);
}

VOID
VmDnsWinCfgCloseConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    if (InterlockedDecrement(&pConnection->refCount) == 0)
    {
        VmDnsWinCfgFreeConnection(pConnection);
    }
}

static
PVMDNS_CFG_CONNECTION
VmDnsWinCfgAcquireConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    InterlockedIncrement(&pConnection->refCount);

    return pConnection;
}

static
VOID
VmDnsWinCfgFreeConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    VmDnsFreeMemory(pConnection);
}
