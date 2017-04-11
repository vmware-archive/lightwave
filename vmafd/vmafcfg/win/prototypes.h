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

// config.c

DWORD
VmAfWinCfgOpenConnection(
    PVMAF_CFG_CONNECTION* ppConnection
    );

DWORD
VmAfWinCfgOpenRootKey(
	PVMAF_CFG_CONNECTION pConnection,
	PCSTR                pszKeyName,
    DWORD                dwOptions,
    DWORD                dwAccess,
	PVMAF_CFG_KEY*       ppKey
	);

DWORD
VmAfWinCfgOpenKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    );

DWORD
VmAfWinCfgCreateKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    );

DWORD
VmAfWinCfgDeleteKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey
    );

DWORD
VmAfWinCfgEnumKeys(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PSTR                 **pppszKeyNames,
    PDWORD               pdwKeyNameCount
    );

DWORD
VmAfWinCfgReadStringValue(
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubkey,
    PCSTR                pszName,
    PSTR*                ppszValue
    );

DWORD
VmAfWinCfgReadDWORDValue(
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubkey,
    PCSTR                pszName,
    PDWORD               pdwValue
    );

DWORD
VmAfWinCfgSetValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszValue,
	DWORD               dwType,
	PBYTE               pValue,
	DWORD               dwSize
	);

DWORD
VmAfWinCfgDeleteValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszValue
	);

DWORD
VmAfWinCfgGetSecurity(
	PVMAF_CFG_KEY       pKey,
	PSTR               *ppszSecurityDescriptor
	);

VOID
VmAfWinCfgCloseKey(
    PVMAF_CFG_KEY pKey
    );

VOID
VmAfWinCfgCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

