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
REST_MODULE _api_certs_rest_module[] =
{
    {
        "/v1/vmdir/api/certs/rootcerts",
        {VmDirRESTGetRootCerts, NULL, NULL, NULL, NULL}
    },
    {0}
};

DWORD
VmDirRESTApiGetCertsModule(
    PREST_MODULE*   ppRestModule
    )
{
    *ppRestModule = _api_certs_rest_module;
    return 0;
}

/* add a cert item to json array */
static
DWORD
_VmDirAddCertItemToJsonArray(
    PVMDIR_CA_CERT pCert,
    BOOLEAN bDetail,
    json_t *pjCertArray
    )
{
    DWORD dwError = 0;
    json_t *pjCert = NULL;

    pjCert = json_object();
    if (!pjCert)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    dwError = json_object_set_new(pjCert, "cn", json_string(pCert->pCN));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = json_object_set_new(
                  pjCert,
                  "subjectdn",
                  json_string(pCert->pSubjectDN));
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bDetail)
    {
        dwError = json_object_set_new(
                      pjCert,
                      "cert",
                      json_string(pCert->pCert));
        BAIL_ON_VMDIR_ERROR(dwError);

        /* crl is optional so set to empty string if null */
        dwError = json_object_set_new(
                      pjCert,
                      "crl",
                      json_string(pCert->pCrl ? pCert->pCrl : ""));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = json_array_append_new(pjCertArray, pjCert);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pjCert)
    {
        json_decref(pjCert);
    }
    goto cleanup;
}

/* convert certs array to json array */
static
DWORD
_VmDirGetCertsJson(
    PVMDIR_CA_CERT_ARRAY pCertArray,
    BOOLEAN bDetail,
    json_t **ppjCerts
    )
{
    DWORD dwError = 0;
    json_t *pjCerts = NULL;
    DWORD dwIndex = 0;

    /* return empty array if there are no certs */
    pjCerts = json_array();
    if (!pjCerts)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_MEMORY);
    }

    for (dwIndex = 0; dwIndex < pCertArray->dwCount; ++dwIndex)
    {
        dwError = _VmDirAddCertItemToJsonArray(
                      &pCertArray->pCACerts[dwIndex],
                      bDetail,
                      pjCerts);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppjCerts = pjCerts;

cleanup:
    return dwError;

error:
    if (pjCerts)
    {
        json_decref(pjCerts);
    }
    goto cleanup;
}

/*
 * get certs
 * return cert array
 */
DWORD
VmDirRESTGetRootCerts(
    void*   pIn,
    void**  ppOut
    )
{
    DWORD dwError = 0;
    PVDIR_REST_OPERATION pRestOp = NULL;
    PSTR pszCACNFilter = NULL;
    BOOLEAN bDetail = FALSE;
    PVMDIR_CA_CERT_ARRAY pCerts = NULL;
    json_t* pjCerts = NULL;
    PCSTR pszBaseDN = NULL;

    if (!pIn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRestOp = (PVDIR_REST_OPERATION)pIn;

    dwError = VmDirRESTGetStrParam(
                  pRestOp,
                  "ca_cn_filter",
                  &pszCACNFilter,
                  FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTGetBoolParam(
                  pRestOp,
                  "detail",
                  &bDetail,
                  FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszBaseDN = VmDirSearchDomainDN(pRestOp->pConn->AccessInfo.pszNormBindedDn);
    if (IsNullOrEmptyString(pszBaseDN))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirGetCACerts(
                  pRestOp->pConn,
                  pszBaseDN,
                  pszCACNFilter,
                  bDetail,
                  &pCerts);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetCertsJson(pCerts, bDetail, &pjCerts);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultSetObjData(
                  pRestOp->pResult,
                  "certs",
                  pjCerts);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeCACertArray(pCerts);
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszCACNFilter);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    if (pjCerts)
    {
        json_decref(pjCerts);
    }
    goto cleanup;
}
