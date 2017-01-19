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



// config.c

DWORD
VmAfPosixCfgOpenConnection(
    PVMAF_CFG_CONNECTION* ppConnection
    );

DWORD
VmAfPosixCfgOpenRootKey(
	PVMAF_CFG_CONNECTION pConnection,
	PCSTR                pszKeyName,
    DWORD                dwOptions,
    DWORD                dwAccess,
	PVMAF_CFG_KEY*       ppKey
	);

DWORD
VmAfPosixCfgOpenKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    );

DWORD
VmAfPosixCfgCreateKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    );

DWORD
VmAfPosixCfgDeleteKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey
    );

DWORD
VmAfPosixCfgEnumKeys(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PSTR                 **pppszKeyNames,
    PDWORD               pdwKeyNameCount
    );

DWORD
VmAfPosixCfgReadStringValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    );

DWORD
VmAfPosixCfgReadDWORDValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    );

DWORD
VmAfPosixCfgSetValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszValue,
	DWORD               dwType,
	PBYTE               pValue,
	DWORD               dwSize
	);

DWORD
VmAfPosixCfgDeleteValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszValue
	);

DWORD
VmAfPosixCfgGetSecurity(
    PVMAF_CFG_KEY           pKey,
    PSTR                   *ppszSecurityDescriptor
    );

VOID
VmAfPosixCfgCloseKey(
    PVMAF_CFG_KEY pKey
    );

VOID
VmAfPosixCfgCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

