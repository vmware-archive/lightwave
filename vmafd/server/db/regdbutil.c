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
    PSTR  pszValue = NULL;
    PWSTR pwszValue = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszValue, dwError);

    dwError = VmAfConfigReadStringValue(
                    pszSubKey,
                    pszValueName,
                    &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszValue, &pwszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszValue = pwszValue;

cleanup:

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
    PCSTR pszSubKey = pszSubKeyParam ? pszSubKeyParam : VMAFD_CONFIG_PARAMETER_KEY_PATH;
    PSTR  pszValue = NULL;

    if (IsNullOrEmptyString(pszValueName) ||
        IsNullOrEmptyString(pwszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pwszValue, &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pszSubKey,
                    pszValueName,
                    pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
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
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    DWORD dwValue = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwValue, dwError);

    dwError = VmAfConfigReadDWORDValue(
                    pszSubKey,
                    pszValueName,
                    &dwValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

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
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    CHAR  szValue[VM_SIZE_128] = {0};

    if (IsNullOrEmptyString(pszValueName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdStringPrintFA(
            &szValue[0], VM_SIZE_128, "%d", dwValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pszSubKey,
                    pszValueName,
                    szValue);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

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

    dwError = VmAfConfigDeleteValue(
                    pszSubKey,
                    pszValueName
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}
