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
REST_MODULE gMetricsRestModule[] =
{
    {
        "/v1/dns/metrics",
        {VmDnsRESTMetricsGet, NULL, NULL, NULL, NULL}
    }
};

DWORD
VmDnsRESTGetMetricsModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = gMetricsRestModule;
    return 0;
}

/*
 * Performs GET operation for the metrics group.
 */
DWORD
VmDnsRESTMetricsGet(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD   dwError = 0;
    PVDNS_REST_OPERATION pRestOp = NULL;

    if (!pIn)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pRestOp = (PVDNS_REST_OPERATION)pIn;

    VmMetricsGetPrometheusData(
            gVmDnsMetricsContext,
            &pRestOp->pResult->pszData,
            &pRestOp->pResult->dwDataLen
            );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SET_REST_RESULT(pRestOp, dwError, NULL);

    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
