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
VmwConfigReadStringValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PSTR*               ppszValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgReadStringValue(
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
VmwConfigReadStringArrayValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwNumValues,
    PSTR                **pppszValues
    )
{
    DWORD               dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgReadStringArrayValue(
                        pszSubkey,
                        pszName,
                        pdwNumValues,
                        pppszValues);
#endif

    return dwError;
}

DWORD
VmwConfigReadDWORDValue(
    PCSTR               pszSubkey,
    PCSTR               pszName,
    PDWORD              pdwValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgReadDWORDValue(
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
    PCSTR           pszSubkey,
    PCSTR           pszName,
    DWORD           dwValue
    )
{
    DWORD dwError = 0;

#ifndef _WIN32
    dwError = VmwPosixCfgSetDWORDValue(
                    pszSubkey,
                    pszName,
                    dwValue);
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

DWORD
VMCAGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszPassword
    )
{
    DWORD   dwError = 0;
    PSTR    pszAccount = NULL;
    PSTR    pszPassword = NULL;

    dwError = VmwConfigReadStringValue(
                    VMDIR_CONFIG_PARAMETER_KEY_PATH,
                    VMDIR_REG_KEY_DC_ACCOUNT_DN,
                    &pszAccount);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    VMDIR_CONFIG_PARAMETER_KEY_PATH,
                    VMDIR_REG_KEY_DC_PASSWORD,
                    &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszPassword = pszPassword;

cleanup:

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

    dwError = VmwConfigReadDWORDValue(
                    VMCA_CONFIG_PARAMETER_KEY_PATH,
                    pcszValueName,
                    &dwValue);
    BAIL_ON_VMCA_ERROR(dwError);

    *pdwOutput = dwValue;

cleanup:

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

    dwError = VmwConfigWriteDWORDValue(
                    VMCA_CONFIG_PARAMETER_KEY_PATH,
                    pcszValueName,
                    dwInput);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

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
