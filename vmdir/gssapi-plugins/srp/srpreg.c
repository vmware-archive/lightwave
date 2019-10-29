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
srp_reg_get_domain_state(
    PDWORD pdomainState)
{
    DWORD dwError = 0;
    DWORD domainState = 0;

    dwError = VmRegCfgGetKeyDword(
            VMAFD_CONFIG_PARAMETER_KEY_PATH,
            VMAFD_REG_KEY_DOMAIN_STATE,
            &domainState,
            0);
    if (dwError == 0)
    {
        *pdomainState = domainState;
    }

    return dwError;
}

DWORD
srp_reg_get_machine_acct_dn(
    PSTR *ppAccountDN
    )
{
    DWORD dwError = 0;
    CHAR    dnBuf[VM_SIZE_512] = {0};
    size_t  accountDNLen = sizeof(dnBuf);
    PSTR    pszAccountDN = NULL;

    dwError = VmRegCfgGetKeyStringA(
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_MACHINE_ACCT,
                  dnBuf,
                  accountDNLen);
    if (dwError)
    {
        goto error;
    }

    dwError = VmAllocateStringA(dnBuf, &pszAccountDN);
    BAIL_ON_VMDIR_ERROR_NO_LINE(dwError);

    *ppAccountDN = (PSTR) pszAccountDN;
    pszAccountDN = NULL;

error:
    if (pszAccountDN)
    {
        free(pszAccountDN);
    }
    return dwError;
}

DWORD
srp_reg_get_machine_acct_upn(
    PSTR *ppAccountUpn
    )
{
    DWORD   dwError = 0;
    PBYTE   pAccountUpn = NULL;
    PSTR    pszAccountDN = NULL;

    dwError = srp_reg_get_machine_acct_dn(&pszAccountDN);
    if (dwError)
    {
        goto error;
    }

    dwError = VMCISLIBAccountDnToUpn(pszAccountDN, (PSTR *) &pAccountUpn);
    if (dwError)
    {
        goto error;
    }
    *ppAccountUpn = pAccountUpn;
    pAccountUpn = NULL;

error:
    if (pszAccountDN)
    {
        free(pszAccountDN);
    }
    if (pAccountUpn)
    {
        free(pAccountUpn);
    }
    return dwError;
}

DWORD
srp_reg_get_machine_acct_password(
    PSTR *ppMachPwd
    )
{
    DWORD   dwError = 0;
    CHAR    pwdBuf[VM_SIZE_32] = {0};
    size_t  pwdLen = sizeof(pwdBuf);
    PSTR    pszPassword = NULL;

    dwError = VmRegCfgGetKeyStringA(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                VMDIR_REG_KEY_MACHINE_PWD,
                pwdBuf,
                pwdLen);
    if (dwError)
    {
        goto error;
    }

    dwError = VmAllocateStringA(pwdBuf, &pszPassword);
    BAIL_ON_VMDIR_ERROR_NO_LINE(dwError);

    *ppMachPwd = pszPassword;
    pszPassword = NULL;

error:
    return dwError;
}

DWORD
srp_reg_get_dc_name(
    PSTR *ppDcName
    )
{
    DWORD   dwError = 0;
    PSTR    pSzDCName = NULL;
    CHAR    dcBuf[VM_SIZE_64] = {0};
    size_t  dcLen = sizeof(dcBuf);

    dwError = VmRegCfgGetKeyStringA(
            VMAFD_CONFIG_PARAMETER_KEY_PATH,
            VMDIR_REG_KEY_DC_NAME_HA,
            dcBuf,
            dcLen);
    if (dwError)
    {
        dwError = VmRegCfgGetKeyStringA(
                VMAFD_CONFIG_PARAMETER_KEY_PATH,
                VMDIR_REG_KEY_DC_NAME,
                dcBuf,
                dcLen);
    }

    BAIL_ON_VMDIR_ERROR_NO_LINE(dwError);

    dwError = VmAllocateStringA(dcBuf, &pSzDCName);
    BAIL_ON_VMDIR_ERROR_NO_LINE(dwError);

    *ppDcName = pSzDCName;
    pSzDCName = NULL;

error:
    free(pSzDCName);
    return dwError;
}
