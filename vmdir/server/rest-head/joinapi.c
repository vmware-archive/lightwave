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
REST_MODULE _api_join_rest_module[] =
{
    {
        "/v1/vmdir/api/join/joinatomic",
        {NULL, NULL, VmDirRESTJoinAtomic, NULL, NULL}
    },
    {0}
};

DWORD
VmDirRESTApiGetJoinModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _api_join_rest_module;
    return 0;
}

static
DWORD
_VmDirGetJoinResultKrbInfo(
    PVMDIR_KRB_INFO pKrbInfo,
    json_t **ppjKrbInfo
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    json_t *pjKrbInfo = NULL;
    json_t *pjBlobArray = NULL;
    uint32_t nOutLength = 0;
    PVMDIR_KRB_BLOB pKrbBlob = NULL;
    PSTR pszBlobBase64 = NULL;

    pjBlobArray = json_array();

    if (!pjBlobArray)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    for (dwIndex = 0; dwIndex < pKrbInfo->dwCount; ++dwIndex)
    {
        pKrbBlob = &pKrbInfo->pKrbBlobs[dwIndex];

        dwError = sasl_encode64((PCSTR) pKrbBlob->krbBlob,
                                pKrbBlob->dwCount,
                                NULL,
                                0,
                                &nOutLength);
        if (dwError == SASL_BUFOVER)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory(nOutLength + 1, (PVOID *)&pszBlobBase64);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = sasl_encode64((PCSTR) pKrbBlob->krbBlob,
                                pKrbBlob->dwCount,
                                pszBlobBase64,
                                nOutLength + 1,
                                NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = json_array_append_new(pjBlobArray, json_string(pszBlobBase64));
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszBlobBase64);
        pszBlobBase64 = NULL;
    }

    pjKrbInfo = json_object();

    if (!pjKrbInfo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    dwError = json_object_set_new(
                  pjKrbInfo,
                  "blobs",
                  pjBlobArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppjKrbInfo = pjKrbInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszBlobBase64);
    return dwError;

error:
    if (pjBlobArray)
    {
        json_decref(pjBlobArray);
    }
    if (pjKrbInfo)
    {
        json_decref(pjKrbInfo);
    }
    goto cleanup;
}

static
DWORD
_VmDirGetJoinResultMachineAccount(
    PVMDIR_MACHINE_INFO_A pMachineInfo,
    json_t **ppjMachineAccount
    )
{
    DWORD dwError = 0;
    json_t *pjMachineAccount = NULL;

    pjMachineAccount = json_object();

    if (!ppjMachineAccount)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    dwError = json_object_set_new(
                  pjMachineAccount,
                  "computer_dn",
                  json_string(pMachineInfo->pszComputerDN));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set_new(
                  pjMachineAccount,
                  "sitename",
                  json_string(pMachineInfo->pszSiteName));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set_new(
                  pjMachineAccount,
                  "machine_guid",
                  json_string(pMachineInfo->pszMachineGUID));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppjMachineAccount = pjMachineAccount;

cleanup:
    return dwError;

error:
    if (pjMachineAccount)
    {
        json_decref(pjMachineAccount);
    }
    goto cleanup;
}

/*
 * make json result out of join response
*/
static
DWORD
_VmDirGetJoinResultJson(
    PVMDIR_MACHINE_INFO_A pMachineInfo,
    PVMDIR_KRB_INFO pKrbInfo,
    json_t **ppjJoin
    )
{
    DWORD dwError = 0;
    json_t *pjJoin = NULL;
    json_t *pjMachineAccount = NULL;
    json_t *pjKrbInfo = NULL;

    if (!pMachineInfo || !ppjJoin)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pjJoin = json_object();
    if (!pjJoin)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    dwError = _VmDirGetJoinResultMachineAccount(pMachineInfo, &pjMachineAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set_new(pjJoin, "machine_account", pjMachineAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pKrbInfo)
    {
        dwError = _VmDirGetJoinResultKrbInfo(pKrbInfo, &pjKrbInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = json_object_set_new(pjJoin, "krb_info", pjKrbInfo);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppjJoin = pjJoin;

cleanup:
    return dwError;

error:
    if (pjJoin)
    {
        json_decref(pjJoin);
    }
    if (pjMachineAccount)
    {
        json_decref(pjMachineAccount);
    }
    if (pjKrbInfo)
    {
        json_decref(pjKrbInfo);
    }
    goto cleanup;
}

/*
 * call atomic join
 * return pass or fail
 */
DWORD
VmDirRESTJoinAtomic(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD dwError = 0;
    PVDIR_REST_OPERATION pRestOp = NULL;
    PSTR pszMachineAccountName = NULL;
    PSTR pszOrgUnit = NULL;
    PCSTR pszBaseDN = NULL;
    BOOLEAN bPrejoined = FALSE;
    PVMDIR_MACHINE_INFO_A  pMachineInfo = NULL;
    PVMDIR_KRB_INFO pKrbInfo = NULL;
    json_t *pjJoinResponse = NULL;

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
                  "org_unit",
                  &pszOrgUnit,
                  FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetBoolParam(
                  pRestOp,
                  "prejoined",
                  &bPrejoined,
                  FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszBaseDN = VmDirSearchDomainDN(pRestOp->pConn->AccessInfo.pszNormBindedDn);
    if (IsNullOrEmptyString(pszBaseDN))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirJoinAtomic(
                  pRestOp->pConn,
                  pszMachineAccountName,
                  pszOrgUnit,
                  bPrejoined,
                  &pMachineInfo,
                  &pKrbInfo
                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetJoinResultJson(pMachineInfo, pKrbInfo, &pjJoinResponse);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetObjData(
                  pRestOp->pResult,
                  "join",
                  pjJoinResponse);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszMachineAccountName);
    VMDIR_SAFE_FREE_MEMORY(pszOrgUnit);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    if (pjJoinResponse)
    {
        json_decref(pjJoinResponse);
    }
    goto cleanup;
}
