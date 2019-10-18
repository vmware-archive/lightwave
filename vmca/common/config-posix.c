/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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

DWORD
VmwPosixCfgReadStringValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;
    CHAR  szValue[VMW_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
    size_t dwszValueSize = sizeof(szValue);
    PSTR  pszValue = NULL;

    dwError = VmRegCfgGetKeyStringA(pszSubkey, pszName, szValue, dwszValueSize);
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
VmwPosixCfgReadStringArrayValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwNumValues,
    PSTR                **pppszValues
    )
{
    DWORD               dwError = 0;
    DWORD               dwIndex = 0;
    DWORD               dwCursorLength = 0;
    CHAR                szValue[VMW_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
    size_t              dwValueSize = sizeof(szValue);
    DWORD               dwNumValues = 0;
    PSTR                pszCursor = NULL;
    PSTR                *ppszValues = NULL;

    if (IsNullOrEmptyString(pszSubkey) ||
        IsNullOrEmptyString(pszName) ||
        !pdwNumValues ||
        !pppszValues)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmRegCfgGetKeyMultiSZA(pszSubkey, pszName, szValue, &dwValueSize);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMCA_ERROR(dwError);

    pszCursor = szValue;
    while (pszCursor && VMCAStringLenA(pszCursor))
    {
        dwCursorLength = VMCAStringLenA(pszCursor);
        ++dwNumValues;
        pszCursor += dwCursorLength + 1;
    }
    dwCursorLength = 0;

    if (dwNumValues)
    {
        dwError = VMCAAllocateMemory(
                        sizeof(PSTR) * dwNumValues,
                        (PVOID *)&ppszValues);
        BAIL_ON_VMCA_ERROR(dwError);

        pszCursor = szValue;

        for (; dwIndex < dwNumValues; ++dwIndex)
        {
            dwCursorLength = VMCAStringLenA(pszCursor);

            dwError = VMCAAllocateStringA(
                              pszCursor,
                              &ppszValues[dwIndex]);
            BAIL_ON_VMCA_ERROR(dwError);

            pszCursor += (dwCursorLength + 1);
        }
    }

    *pdwNumValues   = dwNumValues;
    *pppszValues    = ppszValues;

cleanup:

    return dwError;

error:

    VMCAFreeStringArrayA(ppszValues, dwNumValues);
    if (pppszValues)
    {
        *pppszValues = NULL;
    }
    if (pdwNumValues)
    {
        *pdwNumValues = 0;
    }

    goto cleanup;
}

DWORD
VmwPosixCfgReadDWORDValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD   dwError =0;
    DWORD   dwValue = 0;

    dwError = VmRegCfgGetKeyDword(pszSubkey, pszName, &dwValue, 0);
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
VmwPosixCfgSetDWORDValue(
    PCSTR           pszSubkey,
    PCSTR           pszName,
    DWORD           dwValue
    )
{
    DWORD   dwError = 0;

    if (!pszSubkey || !pszName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmRegCfgSetKeyDword(pszSubkey, pszName, dwValue);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}
