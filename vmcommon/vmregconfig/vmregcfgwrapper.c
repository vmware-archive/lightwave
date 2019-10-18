/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"
#include "vmregconfigincludes.h"

/*
 * wrapper get string key
 */
DWORD
VmRegCfgGetKeyStringA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PSTR                pszValue,
    size_t              iValueLen
    )
{
    DWORD   dwError = 0;
    CHAR    szKey[VM_SIZE_512] = {0};

    if (!pszSubKey || !pszKeyName || !pszValue)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigGetKeyA(szKey, pszValue, &iValueLen);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * wrapper get multisz key
 */
DWORD
VmRegCfgGetKeyMultiSZA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PSTR                pszValue,     /* out */
    size_t*             piValueSize   /* in/out */
    )
{
    DWORD   dwError = 0;
    CHAR    szKey[VM_SIZE_512] = {0};

    if (!pszSubKey || !pszKeyName || !pszValue || !piValueSize)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigGetMultiSZKeyA(szKey, pszValue, piValueSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * wrapper get dword key
 */
DWORD
VmRegCfgGetKeyDword(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PDWORD              pdwValue,
    DWORD               dwDefault
    )
{
    DWORD   dwError = 0;
    CHAR    szKey[VM_SIZE_512] = {0};
    CHAR    szValue[VM_SIZE_32] = {0};
    size_t  dwszValueSize = sizeof(szValue);

    if (!pszSubKey || !pszKeyName || !pdwValue)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigGetKeyA(szKey, szValue, &dwszValueSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *pdwValue = atol(szValue);

cleanup:
    return dwError;

error:
    if (pdwValue)
    {
        *pdwValue = dwDefault;
    }
    goto cleanup;
}

/*
 * wrapper set string key
 */
DWORD
VmRegCfgSetKeyStringA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PCSTR               pszValue
    )
{
    DWORD   dwError =  0;
    CHAR    szKey[VM_SIZE_512] = {0};

    if (!pszSubKey || !pszKeyName || !pszValue)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigSetKeyA(szKey, pszValue, VmStringLenA(pszValue));
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * wrapper set multisz key
 */
DWORD
VmRegCfgSetKeyMultiSZA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    PCSTR               pszValue,
    size_t              iValueSize
    )
{
    DWORD   dwError = 0;
    CHAR    szKey[VM_SIZE_512] = {0};

    if (!pszSubKey || !pszKeyName || !pszValue)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigSetMultiSZKeyA(szKey, pszValue, iValueSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * wrapper set dword key
 */
DWORD
VmRegCfgSetKeyDword(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName,
    DWORD               dwValue
    )
{
    DWORD   dwError =  0;
    CHAR    szKey[VM_SIZE_512] = {0};
    CHAR    szValue[VM_SIZE_32] = {0};

    if (!pszSubKey || !pszKeyName)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmStringPrintFA(
            &szValue[0], VM_SIZE_32,
            "%lu", dwValue);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigSetKeyA(szKey, szValue, VmStringLenA(szValue));
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * wrapper delete  key
 */
DWORD
VmRegCfgDeleteKeyA(
    PCSTR               pszSubKey,
    PCSTR               pszKeyName
    )
{
    DWORD   dwError =  0;
    CHAR    szKey[VM_SIZE_512] = {0};

    if (!pszSubKey || !pszKeyName)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmStringPrintFA(
            &szKey[0], VM_SIZE_512,
            "%s\\%s", pszSubKey, pszKeyName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigDeleteKeyA(szKey);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

