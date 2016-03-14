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

static
PVMDNS_CFG_CONNECTION
VmDnsPosixCfgAcquireConnection(
    PVMDNS_CFG_CONNECTION pConnection
    );

static
VOID
VmDnsPosixCfgFreeConnection(
    PVMDNS_CFG_CONNECTION pConnection
    );

DWORD
VmDnsPosixCfgOpenRootKey(
    PVMDNS_CFG_CONNECTION pConnection,
    PCSTR               pszKeyName,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMDNS_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_KEY pKey = NULL;

    if (!pConnection || IsNullOrEmptyString(pszKeyName) || !ppKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (strcmp(pszKeyName, "HKEY_LOCAL_MACHINE"))
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsPosixCfgOpenKey(
                    pConnection,
                    NULL,
                    "HKEY_THIS_MACHINE",
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
//        VmDnsPosixCfgCloseKey(pKey);
//    }

    goto cleanup;
}

DWORD
VmDnsPosixCfgOpenConnection(
    PVMDNS_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_CONNECTION pConnection = NULL;

    dwError = VmDnsAllocateMemory(sizeof(*pConnection), (PVOID*)&pConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    pConnection->refCount = 1;

    dwError = RegOpenServer(&pConnection->hConnection);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

    return dwError;

error:

    *ppConnection = NULL;

    if (pConnection)
    {
        VmDnsPosixCfgCloseConnection(pConnection);
    }

    goto cleanup;
}

DWORD
VmDnsPosixCfgOpenKey(
    PVMDNS_CFG_CONNECTION pConnection,
    PVMDNS_CFG_KEY        pKey,
    PCSTR               pszSubKey,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMDNS_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMDNS_CFG_KEY pKeyLocal = NULL;

    dwError = VmDnsAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    pConnection->hConnection,
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    &pKeyLocal->hKey);
    BAIL_ON_VMDNS_ERROR(dwError);

    pKeyLocal->pConnection = VmDnsPosixCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmDnsPosixCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmDnsPosixCfgReadStringValue(
    PVMDNS_CFG_KEY        pKey,
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
                    pKey->pConnection->hConnection,
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
VmDnsPosixCfgReadDWORDValue(
    PVMDNS_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError =0;
    DWORD dwValue = 0;
    DWORD dwValueSize = sizeof(dwValue);

    dwError = RegGetValueA(
                    pKey->pConnection->hConnection,
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

VOID
VmDnsPosixCfgCloseKey(
    PVMDNS_CFG_KEY pKey
    )
{
    if (pKey->pConnection)
    {
        if (pKey->hKey)
        {
            RegCloseKey(pKey->pConnection->hConnection, pKey->hKey);
        }

        VmDnsPosixCfgCloseConnection(pKey->pConnection);
    }
    VmDnsFreeMemory(pKey);
}

VOID
VmDnsPosixCfgCloseConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    if (InterlockedDecrement(&pConnection->refCount) == 0)
    {
        VmDnsPosixCfgFreeConnection(pConnection);
    }
}

static
PVMDNS_CFG_CONNECTION
VmDnsPosixCfgAcquireConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    InterlockedIncrement(&pConnection->refCount);

    return pConnection;
}

static
VOID
VmDnsPosixCfgFreeConnection(
    PVMDNS_CFG_CONNECTION pConnection
    )
{
    if (pConnection->hConnection)
    {
        RegCloseServer(pConnection->hConnection);
    }
    VmDnsFreeMemory(pConnection);
}
