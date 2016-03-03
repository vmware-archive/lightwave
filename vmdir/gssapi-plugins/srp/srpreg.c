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
srp_reg_get_handle(
    void **pphRegistry
    )
{
    PVMDIR_CONFIG_CONNECTION_HANDLE hRegistry = NULL;
    DWORD dwError = 0;

    dwError = VmDirRegConfigHandleOpen(&hRegistry);
    if (dwError == 0)
    {
        *pphRegistry = hRegistry;
    }
    return dwError;
}

VOID
srp_reg_close_handle(
    void *phRegistry
    )
{
    PVMDIR_CONFIG_CONNECTION_HANDLE hRegistry = NULL;

    if (phRegistry)
    {
        hRegistry = (PVMDIR_CONFIG_CONNECTION_HANDLE) phRegistry;
        VmDirRegConfigHandleClose(hRegistry);
    }
}

DWORD
static
_srp_reg_get_value(
    void *hRegistry,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    DWORD  valueType,
    PBYTE  *pRetValue,
    PDWORD pRetValueLen)
{
    DWORD dwError = 0;
    PBYTE pRetAccountDN = NULL;
    DWORD accountRetDNLen = 0;

    if (!*pRetValue)
    {
        dwError = VmDirRegConfigGetValue(hRegistry,
                                         pszSubKey,
                                         pszKeyName,
                                         valueType,
                                         NULL,
                                         &accountRetDNLen);
        if (accountRetDNLen > 0)
        {
            accountRetDNLen += 1; /* Guarantee '\0' terminated for strings*/
            pRetAccountDN = calloc(accountRetDNLen, sizeof(CHAR));
            if (!pRetAccountDN)
            {
                dwError = ERROR_NO_MEMORY;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }
    else
    {
        pRetAccountDN = *pRetValue;
        accountRetDNLen = *pRetValueLen;
    }

    dwError = VmDirRegConfigGetValue(hRegistry,
                                     pszSubKey,
                                     pszKeyName,
                                     valueType,
                                     pRetAccountDN,
                                     &accountRetDNLen);
    BAIL_ON_VMDIR_ERROR(dwError);
    *pRetValue = pRetAccountDN;
    *pRetValueLen = accountRetDNLen;

error:
    if (dwError)
    {
        if (pRetAccountDN && pRetAccountDN != *pRetValue)
        {
            free(pRetAccountDN);
        }
    }
    return dwError;
}

DWORD
srp_reg_get_domain_state(
    void *hRegistry,
    PDWORD pdomainState)
{
    DWORD dwError = 0;
    DWORD domainState = 0;
    DWORD domainStateLen = sizeof(domainState);

    dwError = VmDirRegConfigGetValue(hRegistry,
                                     VMAFD_CONFIG_PARAMETER_KEY_PATH,
                                     VMAFD_REG_KEY_DOMAIN_STATE,
                                     RRF_RT_REG_DWORD,
                                     (PBYTE) &domainState,
                                     &domainStateLen);

    if (dwError == 0)
    {
        *pdomainState = domainState;
    }

    return dwError;
}

DWORD
srp_reg_get_machine_acct_dn(
    void *hRegistry,
    PSTR *ppAccountDN)
{
    DWORD dwError = 0;
    DWORD accountDNLen = 0;
    PBYTE pAccountDN = NULL;

    dwError = _srp_reg_get_value(
                  hRegistry,
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_MACHINE_ACCT,
                  RRF_RT_REG_SZ,
                  &pAccountDN,
                  &accountDNLen);
    if (dwError)
    {
        goto error;
    }
    *ppAccountDN = (PSTR) pAccountDN;
    pAccountDN = NULL;

error:
    if (pAccountDN)
    {
        free(pAccountDN);
    }
    return dwError;
}

DWORD
srp_reg_get_machine_acct_upn(
    void *hRegistry,
    PSTR *ppAccountUpn)
{
    DWORD dwError = 0;
    PBYTE pAccountUpn = NULL;
    PBYTE pAccountDN = NULL;
    DWORD accountDNLen = 0;

    dwError = _srp_reg_get_value(
                  hRegistry,
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_MACHINE_ACCT,
                  RRF_RT_REG_SZ,
                  &pAccountDN,
                  &accountDNLen);
    if (dwError)
    {
        goto error;
    }

    dwError = VMCISLIBAccountDnToUpn(pAccountDN, (PSTR *) &pAccountUpn);
    if (dwError)
    {
        goto error;
    }
    *ppAccountUpn = pAccountUpn;
    pAccountUpn = NULL;

error:
    if (pAccountDN)
    {
        free(pAccountDN);
    }
    if (pAccountUpn)
    {
        free(pAccountUpn);
    }
    return dwError;
}

DWORD
srp_reg_get_machine_acct_password(
    void *hRegistry,
    PSTR *ppMachPwd)
{
    DWORD dwError = 0;
    DWORD machPwdLen = 0;

    dwError = _srp_reg_get_value(
                  hRegistry,
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_MACHINE_PWD,
                  RRF_RT_REG_SZ,
                  (PBYTE *) ppMachPwd,
                  &machPwdLen);
    return dwError;
}

DWORD
srp_reg_get_dc_name(
    void *hRegistry,
    PSTR *ppDcName)
{
    DWORD dwError = 0;
    DWORD dcNameLen = 0;
    PSTR pSzDCName = NULL;

    dwError = _srp_reg_get_value(
                  hRegistry,
                  VMAFD_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_DC_NAME_HA,
                  RRF_RT_REG_SZ,
                  (PBYTE *) &pSzDCName,
                  &dcNameLen);

    if (dwError)
    {
        free(pSzDCName);
        pSzDCName = NULL;

        dwError = _srp_reg_get_value(
                    hRegistry,
                    VMAFD_CONFIG_PARAMETER_KEY_PATH,
                    VMDIR_REG_KEY_DC_NAME,
                    RRF_RT_REG_SZ,
                    (PBYTE *) &pSzDCName,
                    &dcNameLen);
        if (dwError)
        {
            goto error;
        }
    }

    *ppDcName = pSzDCName;
    pSzDCName = NULL;

error:
    free(pSzDCName);
    return dwError;
}
