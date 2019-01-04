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

#include "includes.h"

/*
 * REST_MODULE (from copenapitypes.h)
 * callback indices must correspond to:
 *      GET, PUT, POST, DELETE, PATCH
 */
REST_MODULE _api_account_rest_module[] =
{
    {
        "/v1/vmdir/api/account/createcomputeraccount",
        {NULL, NULL, VmDirRESTCreateComputerAccount, NULL, NULL}
    },
    {0}
};

DWORD
VmDirRESTGetAccountModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _api_account_rest_module;
    return 0;
}

/*
 * call create computer account
 * return pass or fail
 */
DWORD
VmDirRESTCreateComputerAccount(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD dwError = 0;
    PVDIR_REST_OPERATION pRestOp = NULL;
    PSTR pszMachineAccountName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszOrgUnit = NULL;
    PCSTR pszBaseDN = NULL;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirRESTGetStrParam(
                  pRestOp,
                  "machine_account_name",
                  &pszMachineAccountName,
                  TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetStrParam(
                  pRestOp,
                  "domain_name",
                  &pszDomainName,
                  FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetStrParam(
                  pRestOp,
                  "org_unit",
                  &pszOrgUnit,
                  FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszBaseDN = VmDirSearchDomainDN(pRestOp->pConn->AccessInfo.pszNormBindedDn);
    if (IsNullOrEmptyString(pszBaseDN))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirCreateComputerAccountInternal(
                  pRestOp->pConn,
                  pszMachineAccountName,
                  pszDomainName,
                  pszOrgUnit,
                  &pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetStrData(
                  pRestOp->pResult,
                  "computer_dn",
                  pMachineInfo->pszComputerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetStrData(
                  pRestOp->pResult,
                  "password",
                  pMachineInfo->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetStrData(
                  pRestOp->pResult,
                  "sitename",
                  pMachineInfo->pszSiteName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetStrData(
                  pRestOp->pResult,
                  "machine_guid",
                  pMachineInfo->pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszMachineAccountName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszOrgUnit);
    if (pMachineInfo)
    {
        VmDirFreeMachineInfoA(pMachineInfo);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}
