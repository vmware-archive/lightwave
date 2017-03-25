/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

static
PVMW_CFG_CONNECTION
VmwWinCfgAcquireConnection(
    PVMW_CFG_CONNECTION pConnection
    );

static
VOID
VmwWinCfgFreeConnection(
    PVMW_CFG_CONNECTION pConnection
    );

DWORD
VmwWinCfgOpenConnection(
    PVMW_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;

    dwError = VMCAAllocateMemory(sizeof(*pConnection), (PVOID*)&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    pConnection->refCount = 1;

    *ppConnection = pConnection;

cleanup:

    return dwError;

error:

    *ppConnection = NULL;

    if (pConnection)
    {
        VmwWinCfgCloseConnection(pConnection);
    }

    goto cleanup;
}

DWORD
VmwWinCfgOpenRootKey(
    PVMW_CFG_CONNECTION pConnection,
    PCSTR               pszKeyName,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMW_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMW_CFG_KEY pKey = NULL;
    VMW_CFG_KEY rootKey = {0};

    if (!pConnection || IsNullOrEmptyString(pszKeyName) || !ppKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!strcmp(pszKeyName, "HKEY_LOCAL_MACHINE"))
    {
        rootKey.hKey = HKEY_LOCAL_MACHINE;
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmwWinCfgOpenKey(
                    pConnection,
                    &rootKey,
                    NULL,
                    dwOptions,
                    dwAccess,
                    &pKey);
    BAIL_ON_VMCA_ERROR(dwError);

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
//        VmwWinCfgCloseKey(pKey);
//    }

    goto cleanup;
}

DWORD
VmwWinCfgOpenKey(
    PVMW_CFG_CONNECTION pConnection,
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubKey,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMW_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMW_CFG_KEY pKeyLocal = NULL;

    dwError = VMCAAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    &pKeyLocal->hKey);
    BAIL_ON_VMCA_ERROR(dwError);

    pKeyLocal->pConnection = VmwWinCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmwWinCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmwWinCfgCreateKey(
    PVMW_CFG_CONNECTION pConnection,
    PVMW_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMW_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMW_CFG_KEY pKeyLocal = NULL;

    dwError = VMCAAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMCA_ERROR(dwError);

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
    BAIL_ON_VMCA_ERROR(dwError);

    pKeyLocal->pConnection = VmwWinCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmwWinCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmwWinCfgReadStringValue(
    PVMW_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;
    CHAR  szValue[VMW_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
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
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    if (pszValue)
    {
        VMCAFreeStringA(pszValue);
    }

    goto cleanup;
}

DWORD
VmwWinCfgReadDWORDValue(
    PVMW_CFG_KEY       pKey,
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
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

DWORD
VmwWinCfgSetValue(
    PVMW_CFG_KEY        pKey,
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
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = RegSetValueExA(
                    pKey->hKey,
                    pszValue,
                    0,
                    dwType,
                    pValue,
                    dwSize);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}

VOID
VmwWinCfgCloseKey(
    PVMW_CFG_KEY pKey
    )
{
    if (pKey->pConnection)
    {
        if (pKey->hKey)
        {
            RegCloseKey(pKey->hKey);
        }

        VmwWinCfgCloseConnection(pKey->pConnection);
    }
    VMCAFreeMemory(pKey);
}

VOID
VmwWinCfgCloseConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    if (InterlockedDecrement(&pConnection->refCount) == 0)
    {
        VmwWinCfgFreeConnection(pConnection);
    }
}

static
PVMW_CFG_CONNECTION
VmwWinCfgAcquireConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    InterlockedIncrement(&pConnection->refCount);

    return pConnection;
}

static
VOID
VmwWinCfgFreeConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    VMCAFreeMemory(pConnection);
}
