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
    DWORD   dwError = 0;
    PSTR    pszAccount = NULL;
    PSTR    pszPassword = NULL;
    PSTR    pszDomainName = NULL;

    dwError = VmwConfigReadStringValue(
                    VMDIR_CONFIG_PARAMETER_KEY_PATH,
                    VMDIR_REG_KEY_DC_ACCOUNT,
                    &pszAccount);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    VMDIR_CONFIG_PARAMETER_KEY_PATH,
                    VMDIR_REG_KEY_DC_PASSWORD,
                    &pszPassword);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                    VMAFD_CONFIG_PARAMETER_KEY_PATH,
                    VMAFD_REG_KEY_DOMAIN_NAME,
                    &pszDomainName);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszAccount = pszAccount;
    *ppszDomainName = pszDomainName;
    *ppszPassword = pszPassword;

cleanup:

    return dwError;

error:

    VMCA_SAFE_FREE_STRINGA(pszAccount);
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(pszPassword);

    goto cleanup;
}
