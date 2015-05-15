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
    return gpVmAfCfgApiGlobals->pCfgPackage->pfnOpenConnection(ppConnection);
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
	return gpVmAfCfgApiGlobals->pCfgPackage->pfnOpenRootKey(
												pConnection,
												pszKeyName,
												dwOptions,
												dwAccess,
												ppKey);
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
    return gpVmAfCfgApiGlobals->pCfgPackage->pfnOpenKey(
                                                pConnection,
                                                pKey,
                                                pszSubKey,
                                                dwOptions,
                                                dwAccess,
                                                ppKey);
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
    return gpVmAfCfgApiGlobals->pCfgPackage->pfnCreateKey(
                                                pConnection,
                                                pKey,
                                                pszSubKey,
                                                dwOptions,
                                                dwAccess,
                                                ppKey);
}

DWORD
VmAfConfigReadStringValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    return gpVmAfCfgApiGlobals->pCfgPackage->pfnReadStringValue(
                                                pKey,
                                                pszSubkey,
                                                pszName,
                                                ppszValue);
}

DWORD
VmAfConfigReadDWORDValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    return gpVmAfCfgApiGlobals->pCfgPackage->pfnReadDWORDValue(
                                                pKey,
                                                pszSubkey,
                                                pszName,
                                                pdwValue);
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
	return gpVmAfCfgApiGlobals->pCfgPackage->pfnSetValue(
												pKey,
												pszName,
												dwType,
												pValue,
												dwSize);
}

VOID
VmAfConfigCloseKey(
    PVMAF_CFG_KEY pKey
    )
{
    if (pKey)
    {
        gpVmAfCfgApiGlobals->pCfgPackage->pfnCloseKey(pKey);
    }
}

VOID
VmAfConfigCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    )
{
    if (pConnection)
    {
        gpVmAfCfgApiGlobals->pCfgPackage->pfnCloseConnection(pConnection);
    }
}
