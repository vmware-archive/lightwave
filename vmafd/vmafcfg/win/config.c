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
PVMAF_CFG_CONNECTION
VmAfWinCfgAcquireConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

static
VOID
VmAfWinCfgFreeConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

DWORD
VmAfWinCfgOpenConnection(
    PVMAF_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;

    dwError = VmAfdAllocateMemory(sizeof(*pConnection), (PVOID*)&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    pConnection->refCount = 1;

    *ppConnection = pConnection;

cleanup:

    return dwError;

error:

    *ppConnection = NULL;

    if (pConnection)
    {
        VmAfWinCfgCloseConnection(pConnection);
    }

    goto cleanup;
}

DWORD
VmAfWinCfgOpenRootKey(
    PVMAF_CFG_CONNECTION pConnection,
    PCSTR                pszKeyName,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_KEY pKey = NULL;
    VMAF_CFG_KEY rootKey = {0};

    if (!pConnection || IsNullOrEmptyString(pszKeyName) || !ppKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!strcmp(pszKeyName, "HKEY_LOCAL_MACHINE"))
    {
        rootKey.hKey = HKEY_LOCAL_MACHINE;
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfWinCfgOpenKey(
                                  pConnection,
                                  &rootKey,
                                  NULL,
                                  dwOptions,
                                  dwAccess,
                                  &pKey);
    BAIL_ON_VMAFD_ERROR(dwError);

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
//        VmAfWinCfgCloseKey(pKey);
//    }

    goto cleanup;
}

DWORD
VmAfWinCfgOpenKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_KEY pKeyLocal = NULL;

    dwError = VmAfdAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    &pKeyLocal->hKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    pKeyLocal->pConnection = VmAfWinCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmAfWinCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmAfWinCfgCreateKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_KEY pKeyLocal = NULL;

    dwError = VmAfdAllocateMemory(sizeof(*pKeyLocal), (PVOID*)&pKeyLocal);
    BAIL_ON_VMAFD_ERROR(dwError);

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
    BAIL_ON_VMAFD_ERROR(dwError);

    pKeyLocal->pConnection = VmAfWinCfgAcquireConnection(pConnection);

    *ppKey = pKeyLocal;

cleanup:

    return dwError;

error:

    *ppKey = NULL;

    if (pKeyLocal)
    {
        VmAfWinCfgCloseKey(pKeyLocal);
    }

    goto cleanup;
}

DWORD
VmAfWinCfgDeleteKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey
    )
{
    DWORD dwError = 0;

    dwError = RegDeleteKeyExA(
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    KEY_WOW64_64KEY,
                    0
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = RegDeleteKeyExA(
                    (pKey ? pKey->hKey : NULL),
                    pszSubKey,
                    KEY_WOW64_32KEY,
                    0
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}


DWORD
VmAfWinCfgEnumKeys(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PSTR                 **pppszKeyNames,
    PDWORD                pdwKeyNameCount
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwCount = 0;
    PSTR  pszKeyName = NULL;
    PSTR  *ppszKeyNames = NULL;
    DWORD dwKeyNameSize = 0;


    while(1)
    {
        dwKeyNameSize = VMAF_REG_KEY_NAME_MAX_LENGTH;
        dwError = VmAfdAllocateMemory(
                                 dwKeyNameSize,
                                 (PVOID *)&pszKeyName
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = RegEnumKeyExA(
                      (pKey ? pKey->hKey : NULL),
                      dwCount,
                      pszKeyName,
                      &dwKeyNameSize,
                      NULL,
                      NULL,
                      NULL,
                      NULL
                      );
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
        dwCount++;
        VMAFD_SAFE_FREE_MEMORY(pszKeyName);
    }

    if (dwCount)
    {

        dwError = VmAfdAllocateMemory(
                            sizeof(PSTR)*dwCount,
                            (PVOID *)&ppszKeyNames
                            );

        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; dwIndex < dwCount; ++dwIndex)
    {
        dwKeyNameSize = VMAF_REG_KEY_NAME_MAX_LENGTH;
        dwError = VmAfdAllocateMemory(
                                dwKeyNameSize,
                                (PVOID *)&pszKeyName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError  = RegEnumKeyExA(
                      (pKey ? pKey->hKey : NULL),
                      dwIndex,
                      pszKeyName,
                      &dwKeyNameSize,
                      NULL,
                      NULL,
                      NULL,
                      NULL
                      );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringA(
                                pszKeyName,
                                &ppszKeyNames[dwIndex]
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
        VMAFD_SAFE_FREE_MEMORY(pszKeyName);
    }

    *pppszKeyNames = ppszKeyNames;
    *pdwKeyNameCount = dwCount;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszKeyName);
    return dwError;

error:

    if (pppszKeyNames)
    {
        *pppszKeyNames = NULL;
    }
    if (pdwKeyNameCount)
    {
        *pdwKeyNameCount = 0;
    }
    if (ppszKeyNames)
    {
        VmAfdFreeStringArrayA(ppszKeyNames);
    }
    goto cleanup;
}

DWORD
VmAfWinCfgReadStringValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;
    CHAR  szValue[VMAF_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
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
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    dwError = VmAfdAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    return dwError;

error:

    *ppszValue = NULL;

    if (pszValue)
    {
        VmAfdFreeStringA(pszValue);
    }

    goto cleanup;
}

DWORD
VmAfWinCfgReadDWORDValue(
    PVMAF_CFG_KEY       pKey,
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
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

DWORD
VmAfWinCfgSetValue(
    PVMAF_CFG_KEY       pKey,
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = RegSetValueExA(
                    pKey->hKey,
                    pszValue,
                    0,
                    dwType,
                    pValue,
                    dwSize);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfWinCfgDeleteValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszValue
    )
{
    DWORD dwError = 0;

    if (!pKey || IsNullOrEmptyString(pszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = RegDeleteValueA(
                    pKey->hKey,
                    pszValue
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfWinCfgGetSecurity(
    PVMAF_CFG_KEY         pKey,
    PSTR                 *ppszSecurityDescriptor
    )
{
    DWORD dwError = 0;

    dwError = ERROR_CALL_NOT_IMPLEMENTED;

    return dwError;
}

VOID
VmAfWinCfgCloseKey(
    PVMAF_CFG_KEY pKey
    )
{
    if (pKey->pConnection)
    {
        if (pKey->hKey)
        {
            RegCloseKey(pKey->hKey);
        }

        VmAfWinCfgCloseConnection(pKey->pConnection);
    }
    VmAfdFreeMemory(pKey);
}

VOID
VmAfWinCfgCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    if (InterlockedDecrement(&pConnection->refCount) == 0)
    {
        VmAfWinCfgFreeConnection(pConnection);
    }
}

static
PVMAF_CFG_CONNECTION
VmAfWinCfgAcquireConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    InterlockedIncrement(&pConnection->refCount);

    return pConnection;
}

static
VOID
VmAfWinCfgFreeConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    VmAfdFreeMemory(pConnection);
}
