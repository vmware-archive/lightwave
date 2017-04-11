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



#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VMAF_CFG_CONNECTION* PVMAF_CFG_CONNECTION;
typedef struct _VMAF_CFG_KEY*        PVMAF_CFG_KEY;

DWORD
VmAfCfgInit(
    VOID
    );

DWORD
VmAfConfigOpenConnection(
    PVMAF_CFG_CONNECTION* ppConnection
    );

DWORD
VmAfConfigOpenRootKey(
	PVMAF_CFG_CONNECTION pConnection,
	PCSTR                pszKeyName,
    DWORD                dwOptions,
    DWORD                dwAccess,
	PVMAF_CFG_KEY*       ppKey
	);

DWORD
VmAfConfigOpenKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    );

DWORD
VmAfConfigCreateKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey,
    DWORD                dwOptions,
    DWORD                dwAccess,
    PVMAF_CFG_KEY*       ppKey
    );

DWORD
VmAfConfigDeleteKey(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubKey
    );

DWORD
VmAfConfigEnumKeys(
    PVMAF_CFG_CONNECTION pConnection,
    PVMAF_CFG_KEY        pKey,
    PSTR                 **ppszKeyNames,
    PDWORD               pdwKeyNameCount
    );

DWORD
VmAfConfigReadStringValue(
    PVMAF_CFG_KEY        pKey,
    PCSTR                pszSubkey,
    PCSTR                pszName,
    PSTR*                ppszValue
    );

DWORD
VmAfConfigReadDWORDValue(
    PVMAF_CFG_KEY       pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    );

DWORD
VmAfConfigSetValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszName,
	DWORD               dwType,
	PBYTE               pValue,
	DWORD               dwSize
	);

DWORD
VmAfConfigDeleteValue(
	PVMAF_CFG_KEY       pKey,
	PCSTR               pszName
	);

DWORD
VmAfConfigGetSecurity(
    PVMAF_CFG_KEY         pKey,
    PSTR                 *ppszSecurityDescriptor
    );

VOID
VmAfConfigCloseKey(
    PVMAF_CFG_KEY pKey
    );

VOID
VmAfConfigCloseConnection(
    PVMAF_CFG_CONNECTION pConnection
    );

VOID
VmAfCfgShutdown(
    VOID
    );

#ifdef __cplusplus
}
#endif

