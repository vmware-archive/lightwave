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



#include "includes.h"

static
PVMW_CFG_CONNECTION
VmwPosixCfgAcquireConnection(
    PVMW_CFG_CONNECTION pConnection
    );

static
VOID
VmwPosixCfgFreeConnection(
    PVMW_CFG_CONNECTION pConnection
    );

DWORD
VmwPosixCfgOpenRootKey(
    PVMW_CFG_CONNECTION pConnection,
    PCSTR               pszKeyName,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMW_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMW_CFG_KEY pKey = NULL;

    if (!pConnection || IsNullOrEmptyString(pszKeyName) || !ppKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (strcmp(pszKeyName, "HKEY_LOCAL_MACHINE"))
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmwPosixCfgOpenKey(
                    pConnection,
                    NULL,
                    "HKEY_THIS_MACHINE",
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
//        VmwPosixCfgCloseKey(pKey);
//    }

    goto cleanup;
}

DWORD
VmwPosixCfgOpenConnection(
    PVMW_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;

    dwError = VMCAAllocateMemory(sizeof(*pConnection), (PVOID*)&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    pConnection->refCount = 1;

    dwError = RegOpenServer(&pConnection->hConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

    return dwError;

error:

    *ppConnection = NULL;

    if (pConnection)
    {
        VmwPosixCfgCloseConnection(pConnection);
    }

    goto cleanup;
}

DWORD
VmwPosixCfgOpenKey(
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
                    pConnection->hConnection,
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    &pKeyLocal->hKey);
    BAIL_ON_VMCA_ERROR(dwError);

    pKeyLocal->pConnection = VmwPosixCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmwPosixCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmwPosixCfgReadStringValue(
    PVMW_CFG_KEY        pKey,
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
                    pKey->pConnection->hConnection,
                    pKey->hKey,
                    pszSubkey,
                    pszName,
                    RRF_RT_REG_SZ,
                    NULL,
                    szValue,
                    &dwszValueSize);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
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
VmwPosixCfgReadDWORDValue(
    PVMW_CFG_KEY        pKey,
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
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

DWORD
VmwPosixCfgSetValue(
    PVMW_CFG_KEY    pKey,
    PCSTR           pszValue,
    DWORD           dwType,
    PBYTE           pValue,
    DWORD           dwSize
    )
{
    DWORD dwError = 0;

    if (!pKey || IsNullOrEmptyString(pszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = RegSetValueExA(
                    pKey->pConnection->hConnection,
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
VmwPosixCfgCloseKey(
    PVMW_CFG_KEY pKey
    )
{
    if (pKey->pConnection)
    {
        if (pKey->hKey)
        {
            RegCloseKey(pKey->pConnection->hConnection, pKey->hKey);
        }

        VmwPosixCfgCloseConnection(pKey->pConnection);
    }
    VMCAFreeMemory(pKey);
}

VOID
VmwPosixCfgCloseConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    if (InterlockedDecrement(&pConnection->refCount) == 0)
    {
        VmwPosixCfgFreeConnection(pConnection);
    }
}

static
PVMW_CFG_CONNECTION
VmwPosixCfgAcquireConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    InterlockedIncrement(&pConnection->refCount);

    return pConnection;
}

static
VOID
VmwPosixCfgFreeConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    if (pConnection->hConnection)
    {
        RegCloseServer(pConnection->hConnection);
    }
    VMCAFreeMemory(pConnection);
}
