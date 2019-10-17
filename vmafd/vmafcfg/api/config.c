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
VmAfConfigReadStringValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{

    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnReadStringValue(
                                                pszSubkey,
                                                pszName,
                                                ppszValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigReadDWORDValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnReadDWORDValue(
                                                pszSubkey,
                                                pszName,
                                                pdwValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigSetValue(
    PCSTR               pszSubkey,
	PCSTR               pszName,
	PCSTR               pszValue
	)
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

	  dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnSetValue(
                                                pszSubkey,
                                                pszName,
                                                pszValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigDeleteValue(
    PCSTR               pszSubkey,
	PCSTR               pszName
	)
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

	  dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnDeleteValue(
                                                              pszSubkey,
                                                              pszName);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

