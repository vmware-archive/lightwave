/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#include <includes.h>

DWORD
VMCASrvGetMachineAccountInfoA(
    PSTR* ppszAccount,
    PSTR* ppszDomainName,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    PVMW_CFG_CONNECTION pConnection = NULL;
    PVMW_CFG_KEY pRootKey = NULL;
    CHAR         szParamsKeyPath[] = VMDIR_CONFIG_PARAMETER_KEY_PATH;
    CHAR         szRegValName_Acct[] = VMDIR_REG_KEY_DC_ACCOUNT;
    CHAR         szRegValName_Pwd[] = VMDIR_REG_KEY_DC_PASSWORD;
    CHAR         szAfdParamsKeyPath[] = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    CHAR         szRegValDomain_Name[] = VMAFD_REG_KEY_DOMAIN_NAME;
    PVMW_CFG_KEY pParamsKey = NULL;
    PVMW_CFG_KEY pAfdParamsKey = NULL;
    PSTR         pszAccount = NULL;
    PSTR         pszPassword = NULL;
    PSTR         pszDomainName = NULL;

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


    dwError = VmwConfigOpenKey(
                    pConnection,
                    pRootKey,
                    &szAfdParamsKeyPath[0],
                    0,
                    KEY_READ,
                    &pAfdParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    pAfdParamsKey,
                    NULL,
                    &szRegValDomain_Name[0],
                    &pszDomainName);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszDomainName = pszDomainName;
    *ppszPassword = pszPassword;
cleanup:

    if (pParamsKey)
    {
        VmwConfigCloseKey(pParamsKey);
    }
    if (pAfdParamsKey)
    {
        VmwConfigCloseKey(pAfdParamsKey);
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
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(pszPassword);

    goto cleanup;
}
