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
static REST_MODULE _rest_module[] =
{
    {
        "/v1/mutentca/version",
        {LwCARestGetVersion, NULL, NULL, NULL, NULL}
    },
    {
        "/v1/mutentca/root",
        {LwCARestGetRootCACert, NULL, NULL, NULL, NULL}
    },
    {0}
};

DWORD
LwCARestRootCAModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _rest_module;
    return 0;
}

/*
 * Returns CA Version
 */
DWORD
LwCARestGetVersion(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError     = 0;
    PLWCA_REST_OPERATION    pRestOp     = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCARestResultSetStrData(
                    pRestOp->pResult,
                    LWCA_JSON_KEY_VERSION,
                    LWCA_REST_VERSION);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, dwError);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Returns Root CA Certificate and CRL
 */
DWORD
LwCARestGetRootCACert(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError         = 0;
    PLWCA_REST_OPERATION    pRestOp         = NULL;
    PSTR                    pszRootCAId     = NULL;
    BOOLEAN                 bDetail         = FALSE;
    PLWCA_CERTIFICATE_ARRAY pCACerts        = NULL;
    PLWCA_CRL               pCrl            = NULL;
    PLWCA_STRING_ARRAY      pCrls           = NULL;
    PLWCA_JSON_OBJECT       pJsonRespArray  = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCAGetRootCAId(&pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetBoolParam(pRestOp, LWCA_REST_PARAM_WITH_CRL, &bDetail, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCACertificates(
                        pRestOp->pReqCtx,
                        pszRootCAId,
                        LWCA_DB_GET_ACTIVE_CA_CERT,
                        NULL,
                        &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bDetail)
    {
        dwError = LwCAGetCACrl(pRestOp->pReqCtx, pszRootCAId, &pCrl);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCACreateStringArray(&pCrl, 1, &pCrls);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestMakeGetCAJsonResponse(pCACerts, pCrls, bDetail, &pJsonRespArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetObjData(pRestOp->pResult, LWCA_JSON_KEY_CA_DETAILS, pJsonRespArray);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, dwError);
    LwCAFreeCertificates(pCACerts);
    LwCAFreeCrl(pCrl);
    LwCAFreeStringArray(pCrls);
    LWCA_SAFE_FREE_STRINGA(pszRootCAId);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
