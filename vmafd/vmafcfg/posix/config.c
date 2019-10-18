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

DWORD
VmAfPosixCfgReadStringValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD   dwError = 0;
    CHAR    szValue[VMAF_MAX_CONFIG_VALUE_BYTE_LENGTH] = {0};
    size_t  dwszValueSize = sizeof(szValue);
    PSTR    pszValue = NULL;

    dwError = VmRegCfgGetKeyStringA(pszSubkey, pszName, szValue, dwszValueSize);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    dwError = VmAfdAllocateStringA(szValue, &pszValue);
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

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
VmAfPosixCfgReadDWORDValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD   dwError =0;
    DWORD   dwValue =0;

    if (!pszSubkey || !pszName || !pdwValue)
    {
        BAIL_WITH_VMAFD_ERROR(dwError, ERROR_FILE_NOT_FOUND);
    }

    dwError = VmRegCfgGetKeyDword(pszSubkey, pszName, &dwValue, 0);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

    *pdwValue = dwValue;

cleanup:

    return dwError;

error:

    *pdwValue = 0;

    goto cleanup;
}

DWORD
VmAfPosixCfgSetValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PCSTR               pszValue
	)
{
    DWORD dwError = 0;

    if (!pszSubkey || !pszName || IsNullOrEmptyString(pszValue))
    {
	dwError = ERROR_INVALID_PARAMETER;
	BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmRegCfgSetKeyStringA(pszSubkey, pszName, pszValue);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

error:

	return dwError;
}

DWORD
VmAfPosixCfgDeleteValue(
    PCSTR               pszSubkey,
    PCSTR               pszName
    )
{
    DWORD dwError = 0;

    if (!pszSubkey || !pszName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmRegCfgDeleteKeyA(pszSubkey, pszName);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = ERROR_FILE_NOT_FOUND;
    }
    BAIL_ON_VMAF_POSIX_ERROR(dwError);

error:

	return dwError;
}
