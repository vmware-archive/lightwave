/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
REST_MODULE _metrics_rest_module[] =
{
    {
        "/v1/vmdir/metrics/ldap",
        {VmDirRESTMetricsGet, NULL, NULL, NULL, NULL}
    }
};

DWORD
VmDirRESTGetMetricsModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _metrics_rest_module;
    return 0;
}

/*
 * Calculates the count and total time for LDAP metrics
 */
DWORD
VmDirGetOPStatistics(
    ber_tag_t opTag,
    json_t** ppjBodyOut
)
{
    DWORD   dwError = 0;
    uint64_t iCount = 0;
    uint64_t iTotal = 0;
    json_t* pjMetric = NULL;
    PCSTR pszKey = NULL;
    PSTR pszKeyLower = NULL;

    pjMetric = json_object();

    pszKey = VmDirGetOperationStringFromTag(opTag);
    dwError = VmDirAllocASCIIUpperToLower(pszKey, &pszKeyLower);
    BAIL_ON_VMDIR_ERROR(dwError);

    iCount = VmDirOPStatisticGetCount(opTag);
    dwError = json_object_set(pjMetric, "count", json_integer(iCount));
    BAIL_ON_VMDIR_ERROR(dwError);

    iTotal = VmDirOPStatisticGetTotalTime(opTag);
    dwError = json_object_set(pjMetric, "time_total", json_integer(iTotal));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set(*ppjBodyOut, pszKeyLower, pjMetric);
    BAIL_ON_VMDIR_ERROR(dwError);

    pjMetric = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszKeyLower);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s failed, error (%d)", __FUNCTION__, dwError);

    if (pjMetric)
    {
        json_decref(pjMetric);
    }

    goto cleanup;
}

/*
 * Fetches the json response for the LDAP metrics
 */
DWORD
VmDirRESTMetricsLdapGet(
    json_t** ppjBodyOut
)
{
    DWORD   dwError = 0;
    json_t* pjLdapBody = NULL;

    pjLdapBody = json_object();

    /* BIND */
    dwError = VmDirGetOPStatistics(LDAP_REQ_BIND, &pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* ADD */
    dwError = VmDirGetOPStatistics(LDAP_REQ_ADD, &pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* SEARCH */
    dwError = VmDirGetOPStatistics(LDAP_REQ_SEARCH, &pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* UNBIND */
    dwError = VmDirGetOPStatistics(LDAP_REQ_UNBIND, &pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* MODIFY */
    dwError = VmDirGetOPStatistics(LDAP_REQ_MODIFY, &pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* DELETE */
    dwError = VmDirGetOPStatistics(LDAP_REQ_DELETE, &pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set(*ppjBodyOut, "ldap", pjLdapBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    pjLdapBody = NULL;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "%s failed, error (%d)", __FUNCTION__, dwError);

    if (pjLdapBody)
    {
        json_decref(pjLdapBody);
    }

    goto cleanup;
}


/*
 * Performs GET operation for all the VmDir metrics
 */
DWORD
VmDirRESTMetricsGet(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_OPERATION pRestOp = NULL;
    json_t* pjBody = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;
    pjBody = json_object();

    dwError = VmDirRESTMetricsLdapGet(&pjBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetObjData(
                  pRestOp->pResult,
                  "vmdir",
                  pjBody);
    BAIL_ON_VMDIR_ERROR(dwError);

    pjBody = NULL;

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);

    if (pjBody)
    {
        json_decref(pjBody);
    }

    goto cleanup;
}
