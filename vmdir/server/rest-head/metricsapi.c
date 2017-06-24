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
        {VmDirRESTMetricsLdapGet, NULL, NULL, NULL, NULL}
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
 * Performs GET operation for the LDAP metrics group.
 */
DWORD
VmDirRESTMetricsLdapGet(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_OPERATION pRestOp = NULL;
    uint16_t iCount = 0;
    uint16_t iAvg = 0;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    /* BIND */

    iCount = VmDirOPStatisticGetCount(LDAP_REQ_BIND);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_bind_count",
                  iCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    iAvg = VmDirOPStatisticGetAvgTime(LDAP_REQ_BIND);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_bind_average",
                  iAvg);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* ADD */

    iCount = VmDirOPStatisticGetCount(LDAP_REQ_ADD);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_add_count",
                  iCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    iAvg = VmDirOPStatisticGetAvgTime(LDAP_REQ_ADD);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_add_average",
                  iAvg);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* SEARCH */

    iCount = VmDirOPStatisticGetCount(LDAP_REQ_SEARCH);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_search_count",
                  iCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    iAvg = VmDirOPStatisticGetAvgTime(LDAP_REQ_SEARCH);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_search_average",
                  iAvg);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* UNBIND */

    iCount = VmDirOPStatisticGetCount(LDAP_REQ_UNBIND);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_unbind_count",
                  iCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    iAvg = VmDirOPStatisticGetAvgTime(LDAP_REQ_UNBIND);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_unbind_average",
                  iAvg);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* MODIFY */

    iCount = VmDirOPStatisticGetCount(LDAP_REQ_MODIFY);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_modify_count",
                  iCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    iAvg = VmDirOPStatisticGetAvgTime(LDAP_REQ_MODIFY);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_modify_average",
                  iAvg);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* DELETE */

    iCount = VmDirOPStatisticGetCount(LDAP_REQ_DELETE);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_delete_count",
                  iCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    iAvg = VmDirOPStatisticGetAvgTime(LDAP_REQ_DELETE);
    dwError = VmDirRESTResultSetIntData(
                  pRestOp->pResult,
                  "ldap_delete_average",
                  iAvg);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);

    goto cleanup;
}
