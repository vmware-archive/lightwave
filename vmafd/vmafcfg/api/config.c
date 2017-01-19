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
VmAfConfigOpenConnection(
    PVMAF_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;

    if (!ppConnection || !gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnOpenConnection(ppConnection);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigOpenRootKey(
	PVMAF_CFG_CONNECTION pConnection,
	PCSTR                pszKeyName,
    DWORD                dwOptions,
    DWORD                dwAccess,
	PVMAF_CFG_KEY*       ppKey
	)
{
      DWORD dwError = 0;

      if (!gpVmAfCfgApiGlobals->pCfgPackage)
      {
          dwError =  ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR(dwError);
      }

      dwError = gpVmAfCfgApiGlobals->pCfgPackage->pfnOpenRootKey(
                            pConnection,
                            pszKeyName,
                            dwOptions,
                            dwAccess,
                            ppKey);
      BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

      return dwError;
error:

      goto cleanup;
}

DWORD
VmAfConfigOpenKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnOpenKey(
                                                pConnection,
                                                pKey,
                                                pszSubKey,
                                                dwOptions,
                                                dwAccess,
                                                ppKey);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigCreateKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnCreateKey(
                                                pConnection,
                                                pKey,
                                                pszSubKey,
                                                dwOptions,
                                                dwAccess,
                                                ppKey);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigDeleteKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey
    )
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnDeleteKey(
                                                pConnection,
                                                pKey,
                                                pszSubKey
                                                );
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigEnumKeys(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PSTR                 **ppszKeyNames,
    PDWORD               pdwKeyNameCount
    )
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnEnumKeys(
                                                pConnection,
                                                pKey,
                                                ppszKeyNames,
                                                pdwKeyNameCount
                                                );
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigReadStringValue(
    PVMAF_CFG_KEY       pKey,
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
                                                pKey,
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
    PVMAF_CFG_KEY       pKey,
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
                                                pKey,
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
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszName,
	DWORD               dwType,
	PBYTE               pValue,
	DWORD               dwSize
	)
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

	  dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnSetValue(
												pKey,
												pszName,
												dwType,
												pValue,
												dwSize);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigDeleteValue(
	PVMAF_CFG_KEY       pKey,
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
                                                              pKey,
                                                              pszName
                                                              );
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfConfigGetSecurity(
    PVMAF_CFG_KEY         pKey,
    PSTR                 *ppszSecurityDescriptor
    )
{
    DWORD dwError = 0;

    if (!gpVmAfCfgApiGlobals->pCfgPackage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError =  gpVmAfCfgApiGlobals->pCfgPackage->pfnGetSecurity(
                   pKey,
                   ppszSecurityDescriptor
                   );
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmAfConfigCloseKey(
    PVMAF_CFG_KEY pKey
    )
{
    if (pKey && gpVmAfCfgApiGlobals->pCfgPackage)
    {
        gpVmAfCfgApiGlobals->pCfgPackage->pfnCloseKey(pKey);
    }
}

VOID
VmAfConfigCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    if (pConnection && gpVmAfCfgApiGlobals->pCfgPackage)
    {
        gpVmAfCfgApiGlobals->pCfgPackage->pfnCloseConnection(pConnection);
    }
}
