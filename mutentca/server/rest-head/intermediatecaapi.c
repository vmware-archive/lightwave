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
        "/v1/mutentca/intermediate",
        {NULL, NULL, LwCARestCreateIntermediateCA, NULL, NULL}
    },
    {
        "/v1/mutentca/intermediate/*",
        {LwCARestGetIntermediateCACert, NULL, NULL, LwCARestRevokeIntermediateCA, NULL}
    },
    {0}
};

DWORD
LwCARestIntermediateCAModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _rest_module;
    return 0;
}

/*
 * Creates Intermediate CA and returns its CA cert
 */
DWORD
LwCARestCreateIntermediateCA(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError         = 0;
    PLWCA_REST_OPERATION    pRestOp         = NULL;
    PSTR                    pszRequestId    = NULL;
    PLWCA_REST_INT_CA_SPEC  pIntCASpec      = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts        = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_REQ_ID, &pszRequestId, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetIntCAInputSpec(pRestOp->pjBody, &pIntCASpec);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateIntermediateCA(
                            pRestOp->pReqCtx,
                            pIntCASpec->pszCAId,
                            pIntCASpec->pszParentCAId,
                            pIntCASpec->pIntCAReqData,
                            pIntCASpec->pCertValidity,
                            &pCACerts
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetCertArrayData(
                  pRestOp->pResult,
                  LWCA_JSON_KEY_CERTS,
                  pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError, NULL);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LwCARestFreeIntCAInputSpec(pIntCASpec);
    LwCAFreeCertificates(pCACerts);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Returns Intermediate CA Certificate and CRL
 */
DWORD
LwCARestGetIntermediateCACert(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError         = 0;
    PLWCA_REST_OPERATION    pRestOp         = NULL;
    PSTR                    pszRequestId    = NULL;
    PSTR                    pszCAId         = NULL;
    BOOLEAN                 bDetail         = FALSE;
    PLWCA_CRL               pCrl            = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts        = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_REQ_ID, &pszRequestId, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_CA_ID, &pszCAId, TRUE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetBoolParam(pRestOp, LWCA_REST_PARAM_WITH_CRL, &bDetail, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCACertificates(pszCAId, &pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetCertArrayData(
                        pRestOp->pResult,
                        LWCA_JSON_KEY_CERTS,
                        pCACerts);
    BAIL_ON_LWCA_ERROR(dwError);

    if (bDetail)
    {
        dwError = LwCAGetCACrl(pRestOp->pReqCtx, pszCAId, &pCrl);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCARestResultSetStrData(
                            pRestOp->pResult,
                            LWCA_JSON_KEY_CRL,
                            pCrl);
        BAIL_ON_LWCA_ERROR(dwError);
    }

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError, NULL);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LWCA_SAFE_FREE_STRINGA(pszCAId);
    LwCAFreeCertificates(pCACerts);
    LwCAFreeCrl(pCrl);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Revokes Intermediate CA
 */
DWORD
LwCARestRevokeIntermediateCA(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError         = 0;
    PLWCA_REST_OPERATION    pRestOp         = NULL;
    PSTR                    pszRequestId    = NULL;
    PSTR                    pszCAId         = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_REQ_ID, &pszRequestId, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_CA_ID, &pszCAId, TRUE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARevokeIntermediateCA(pRestOp->pReqCtx, pszCAId);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError, NULL);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LWCA_SAFE_FREE_STRINGA(pszCAId);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
