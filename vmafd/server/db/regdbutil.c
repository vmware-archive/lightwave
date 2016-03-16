/*
 * Copyright (C) 2011-2013 VMware, Inc. All rights reserved.
 */

#include "includes.h"


DWORD
VmAfdRegGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PWSTR*   ppwszValue      /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PSTR  pszValue = NULL;
    PWSTR pwszValue = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszValue, dwError);

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    pszValueName,
                    &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszValue, &pwszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszValue = pwszValue;

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    VMAFD_SAFE_FREE_STRINGA(pszValue);

    return dwError;

error:

    if (ppwszValue)
    {
        *ppwszValue = NULL;
    }

    goto cleanup;
}

DWORD
VmAfdRegSetString(
    PCSTR    pszSubKeyParam, /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PCWSTR   pwszValue       /* IN     */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = pszSubKeyParam ? pszSubKeyParam : VMAFD_CONFIG_PARAMETER_KEY_PATH;
    PSTR  pszValue = NULL;

    if (IsNullOrEmptyString(pszValueName) ||
        IsNullOrEmptyString(pwszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszValue, &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pParamsKey,
                    pszValueName,
                    REG_SZ,
                    pszValue,
                    (DWORD)strlen(pszValue) + 1);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    VMAFD_SAFE_FREE_STRINGA(pszValue);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdRegGetInteger(
    PCSTR    pszValueName,   /* IN     */
    PDWORD   pdwValue        /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    DWORD dwValue = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwValue, dwError);

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadDWORDValue(
                    pParamsKey,
                    NULL,
                    pszValueName,
                    &dwValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdRegSetInteger(
    PCSTR    pszValueName,   /* IN     */
    DWORD    dwValue         /* IN     */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;

    if (IsNullOrEmptyString(pszValueName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pParamsKey,
                    pszValueName,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(DWORD));
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdRegDeleteValue(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName   /* IN     */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_SET_VALUE,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigDeleteValue(
                    pParamsKey,
                    pszValueName
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}
