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
        "/v1/mutentca/certificate",
        {NULL, NULL, LwCARestGetRootCASignedCert, LwCARestRevokeRootCASignedCert, NULL}
    },
    {
        "/v1/mutentca/intermediate/*/certificate",
        {NULL, NULL, LwCARestGetIntermediateCASignedCert, LwCARestRevokeIntermediateCASignedCert, NULL}
    },
    {0}
};

DWORD
LwCARestCertificatesModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _rest_module;
    return 0;
}

/*
 * Returns Root CA Signed Certificate
 */
DWORD
LwCARestGetRootCASignedCert(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                       dwError         = 0;
    PLWCA_REST_OPERATION        pRestOp         = NULL;
    PSTR                        pszRequestId    = NULL;
    PLWCA_REST_SIGN_CERT_SPEC   pSignCertSpec   = NULL;
    PLWCA_CERTIFICATE           pCert           = NULL;
    PSTR                        pszChainOfTrust = NULL;
    PSTR                        pszRootCAId     = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_REQ_ID, &pszRequestId, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetSignCertInputSpec(pRestOp->pjBody, &pSignCertSpec);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetRootCAId(&pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetSignedCertificate(
                            pRestOp->pReqCtx,
                            pszRootCAId,
                            pSignCertSpec->pszCSR,
                            pSignCertSpec->pCertValidity,
                            pSignCertSpec->signAlgorithm,
                            &pCert
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetChainOfTrust(pszRootCAId, &pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetStrData(
                  pRestOp->pResult,
                  LWCA_JSON_KEY_CERT,
                  pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetStrData(
                  pRestOp->pResult,
                  LWCA_JSON_KEY_TRUSTCHAIN,
                  pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LwCARestFreeSignCertInputSpec(pSignCertSpec);
    LwCAFreeCertificate(pCert);
    LWCA_SAFE_FREE_STRINGA(pszChainOfTrust);
    LWCA_SAFE_FREE_STRINGA(pszRootCAId);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Revokes Root CA Signed Certificate
 */
DWORD
LwCARestRevokeRootCASignedCert(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError         = 0;
    PLWCA_REST_OPERATION    pRestOp         = NULL;
    PSTR                    pszRequestId    = NULL;
    PLWCA_CERTIFICATE       pCert           = NULL;
    PSTR                    pszRootCAId     = NULL;

    if (!pIn)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pRestOp = (PLWCA_REST_OPERATION)pIn;

    dwError = LwCARestGetStrParam(pRestOp, LWCA_REST_PARAM_REQ_ID, &pszRequestId, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestGetCertificateInput(pRestOp->pjBody, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetRootCAId(&pszRootCAId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARevokeCertificate(pRestOp->pReqCtx, pszRootCAId, pCert);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LwCAFreeCertificate(pCert);
    LWCA_SAFE_FREE_STRINGA(pszRootCAId);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Returns Intermediate CA Signed Certificate
 */
DWORD
LwCARestGetIntermediateCASignedCert(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                       dwError         = 0;
    PLWCA_REST_OPERATION        pRestOp         = NULL;
    PSTR                        pszRequestId    = NULL;
    PSTR                        pszCAId         = NULL;
    PSTR                        pszChainOfTrust = NULL;
    PLWCA_REST_SIGN_CERT_SPEC   pSignCertSpec   = NULL;
    PLWCA_CERTIFICATE           pCert           = NULL;

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

    dwError = LwCARestGetSignCertInputSpec(pRestOp->pjBody, &pSignCertSpec);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetSignedCertificate(
                            pRestOp->pReqCtx,
                            pszCAId,
                            pSignCertSpec->pszCSR,
                            pSignCertSpec->pCertValidity,
                            pSignCertSpec->signAlgorithm,
                            &pCert
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetChainOfTrust(pszCAId, &pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetStrData(
                  pRestOp->pResult,
                  LWCA_JSON_KEY_CERT,
                  pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetStrData(
                  pRestOp->pResult,
                  LWCA_JSON_KEY_TRUSTCHAIN,
                  pszChainOfTrust);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LWCA_SAFE_FREE_STRINGA(pszCAId);
    LwCARestFreeSignCertInputSpec(pSignCertSpec);
    LWCA_SAFE_FREE_STRINGA(pszChainOfTrust);
    LwCAFreeCertificate(pCert);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Revokes Intermediate CA Signed Certificate
 */
DWORD
LwCARestRevokeIntermediateCASignedCert(
    PVOID   pIn,
    PVOID*  ppOut
    )
{
    DWORD                   dwError         = 0;
    PLWCA_REST_OPERATION    pRestOp         = NULL;
    PSTR                    pszRequestId    = NULL;
    PSTR                    pszCAId         = NULL;
    PLWCA_CERTIFICATE       pCert           = NULL;

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

    dwError = LwCARestGetCertificateInput(pRestOp->pjBody, &pCert);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARevokeCertificate(pRestOp->pReqCtx, pszCAId, pCert);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    LwCASetRestResult(pRestOp, pszRequestId, dwError);
    LWCA_SAFE_FREE_STRINGA(pszRequestId);
    LWCA_SAFE_FREE_STRINGA(pszCAId);
    LwCAFreeCertificate(pCert);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
