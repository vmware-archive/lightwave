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



#include "includes.h"

#ifndef _WIN32
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#define VMCA_CONFIG_PARAMETER_KEY_PATH "Services\\vmca"
#else
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#define VMCA_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareCertificateService\\Parameters"
#endif

#define VMDIR_REG_KEY_DC_ACCOUNT_DN "dcAccountDN"
#define VMDIR_REG_KEY_DC_PASSWORD   "dcAccountPassword"


DWORD
VmwConfigOpenConnection(
    PVMW_CFG_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
#ifndef _WIN32
    dwError = VmwPosixCfgOpenConnection(ppConnection);
#else
    dwError = VmwWinCfgOpenConnection(ppConnection);
#endif
    return dwError;
}

DWORD
VmwConfigOpenRootKey(
	PVMW_CFG_CONNECTION pConnection,
	PCSTR               pszKeyName,
	DWORD               dwOptions,
	DWORD               dwAccess,
	PVMW_CFG_KEY*       ppKey
	)
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgOpenRootKey(
    				pConnection,
    				pszKeyName,
    				dwOptions,
    				dwAccess,
    				ppKey);
#else
    dwError = VmwWinCfgOpenRootKey(
    				pConnection,
    				pszKeyName,
    				dwOptions,
    				dwAccess,
    				ppKey);
#endif

    return dwError;
}

DWORD
VmwConfigOpenKey(
    PVMW_CFG_CONNECTION pConnection,
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubKey,
    DWORD               dwOptions,
    DWORD               dwAccess,
    PVMW_CFG_KEY*       ppKey
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgOpenKey(
                    pConnection,
                    pKey,
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    ppKey);
#else
    dwError = VmwWinCfgOpenKey(
                    pConnection,
                    pKey,
                    pszSubKey,
                    dwOptions,
                    dwAccess,
                    ppKey);
#endif

    return dwError;
}

DWORD
VmwConfigReadStringValue(
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgReadStringValue(
                    pKey,
                    pszSubkey,
                    pszName,
                    ppszValue);
#else
    dwError = VmwWinCfgReadStringValue(
                    pKey,
                    pszSubkey,
                    pszName,
                    ppszValue);
#endif

    return dwError;
}

DWORD
VmwConfigReadDWORDValue(
    PVMW_CFG_KEY        pKey,
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgReadDWORDValue(
                    pKey,
                    pszSubkey,
                    pszName,
                    pdwValue);
#else
    dwError = VmwWinCfgReadDWORDValue(
                    pKey,
                    pszSubkey,
                    pszName,
                    pdwValue);
#endif

    return dwError;
}

DWORD
VmwConfigWriteDWORDValue(
    PVMW_CFG_KEY        pKey,
    PCSTR               pszName,
    DWORD               dwValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgSetValue(
                    pKey,
                    pszName,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(DWORD));
#else
    dwError = VmwWinCfgSetValue(
                    pKey,
                    pszName,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(DWORD));
#endif

    return dwError;
}

VOID
VmwConfigCloseKey(
    PVMW_CFG_KEY pKey
    )
{
    if (pKey)
    {
#ifndef _WIN32
        VmwPosixCfgCloseKey(pKey);
#else
        VmwWinCfgCloseKey(pKey);
#endif
    }
}

VOID
VmwConfigCloseConnection(
    PVMW_CFG_CONNECTION pConnection
    )
{
    if (pConnection)
    {
#ifndef _WIN32
        VmwPosixCfgCloseConnection(pConnection);
#else
        VmwWinCfgCloseConnection(pConnection);
#endif
    }
}


DWORD
VMCAGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;
    PVMW_CFG_KEY pRootKey = NULL;
    CHAR         szParamsKeyPath[] = VMDIR_CONFIG_PARAMETER_KEY_PATH;
    CHAR         szRegValName_Acct[] = VMDIR_REG_KEY_DC_ACCOUNT_DN;
    CHAR         szRegValName_Pwd[] = VMDIR_REG_KEY_DC_PASSWORD;
    PVMW_CFG_KEY pParamsKey = NULL;
    PSTR         pszAccount = NULL;
    PSTR         pszPassword = NULL;

    dwError = VmwConfigOpenConnection(&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szParamsKeyPath[0],
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    &szRegValName_Acct[0],
                    &pszAccount);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    &szRegValName_Pwd[0],
                    &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszPassword = pszPassword;

cleanup:

    if (pParamsKey)
    {
        VmwConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmwConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmwConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    VMCA_SAFE_FREE_STRINGA(pszAccount);
    VMCA_SAFE_FREE_STRINGA(pszPassword);

    goto cleanup;
}

DWORD
VMCAConfigGetDword(
    PCSTR  pcszValueName,
    DWORD* pdwOutput
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;
    PVMW_CFG_KEY pRootKey = NULL;
    PVMW_CFG_KEY pParamsKey = NULL;
    CHAR  szParamsKeyPath[] = VMCA_CONFIG_PARAMETER_KEY_PATH;

    dwError = VmwConfigOpenConnection(&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szParamsKeyPath[0],
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadDWORDValue(
                    pParamsKey,
                    NULL,
                    pcszValueName,
                    &dwValue);
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwOutput = dwValue;

cleanup:

    if (pParamsKey)
    {
        VmwConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmwConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmwConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VMCAConfigSetDword(
    PCSTR  pcszValueName,
    DWORD  dwInput
    )
{
    DWORD dwError = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;
    PVMW_CFG_KEY pRootKey = NULL;
    PVMW_CFG_KEY pParamsKey = NULL;
    CHAR  szParamsKeyPath[] = VMCA_CONFIG_PARAMETER_KEY_PATH;

    dwError = VmwConfigOpenConnection(&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szParamsKeyPath[0],
                    0,
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigWriteDWORDValue(
                    pParamsKey,
                    pcszValueName,
                    dwInput);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

    if (pParamsKey)
    {
        VmwConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmwConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmwConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    goto cleanup;
}

BOOLEAN
VMCAConfigIsServerOptionEnabled(
    VMCA_SERVER_OPTION  option
    )
{
    DWORD   dwError = 0;
    DWORD   dwCurrentOption = 0;
    BOOLEAN bEnabled = FALSE;

    dwError = VMCAConfigGetDword(VMCA_REG_KEY_SERVER_OPTION, &dwCurrentOption);
    BAIL_ON_VMCA_ERROR(dwError);

    bEnabled = option & dwCurrentOption;

error:
    return bEnabled;
}
