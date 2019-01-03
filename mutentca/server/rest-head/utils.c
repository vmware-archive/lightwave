/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
LwCARestMakeGetCAJsonResponse(
    PLWCA_CERTIFICATE_ARRAY     pCACerts,
    PLWCA_STRING_ARRAY          pCRLs,
    BOOLEAN                     bDetail,
    PLWCA_JSON_OBJECT           *ppJsonRespArray
    )
{
    DWORD                       dwError             = 0;
    DWORD                       dwIdx               = 0;
    PLWCA_JSON_OBJECT           pJsonEntry          = NULL;
    PLWCA_JSON_OBJECT           pJsonRespArray      = NULL;

    if (!pCACerts || !ppJsonRespArray)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (bDetail && (!pCRLs || pCRLs->dwCount != pCACerts->dwCount))
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAJsonArrayCreate(&pJsonRespArray);
    BAIL_ON_LWCA_ERROR(dwError);

    for (; dwIdx < pCACerts->dwCount; ++dwIdx)
    {
        dwError = LwCAJsonObjectCreate(&pJsonEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonSetStringToObject(
                            pJsonEntry,
                            LWCA_JSON_KEY_CERT,
                            (PSTR)pCACerts->ppCertificates[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);

        if (bDetail)
        {
            dwError = LwCAJsonSetStringToObject(pJsonEntry, LWCA_JSON_KEY_CRL, pCRLs->ppData[dwIdx]);
            BAIL_ON_LWCA_ERROR(dwError);
        }

        dwError = LwCAJsonAppendJsonToArray(pJsonRespArray, pJsonEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_SAFE_JSON_DECREF(pJsonEntry);
    }

    *ppJsonRespArray = pJsonRespArray;


cleanup:

    LWCA_SAFE_JSON_DECREF(pJsonEntry);

    return dwError;

error:

    LWCA_SAFE_JSON_DECREF(pJsonRespArray);
    if (ppJsonRespArray)
    {
        *ppJsonRespArray = NULL;
    }

    goto cleanup;
}

LWCA_METRICS_REQ_URLS
LwCARestMetricsGetReqUrl(
    PCSTR   pcszPath
    )
{
    int iCount = 0;
    LWCA_METRICS_REQ_URLS reqUrl = LWCA_METRICS_REQ_URL_UNKNOWN;

    if (LwCAStringCompareA(pcszPath, "/v1/mutentca/root", FALSE) == 0)
    {
        reqUrl = LWCA_METRICS_REQ_URL_ROOT;
    }
    else if (LwCAStringCompareA(pcszPath, "/v1/mutentca/certificate", FALSE) == 0)
    {
        reqUrl = LWCA_METRICS_REQ_URL_ROOT_CERT;
    }
    else if (LwCAStringCompareA(pcszPath, "/v1/mutentca/crl", FALSE) == 0)
    {
        reqUrl = LWCA_METRICS_REQ_URL_ROOT_CRL;
    }
    else if (LwCAStringStartsWith(pcszPath, "/v1/mutentca/intermediate", FALSE) == TRUE)
    {
        reqUrl = LWCA_METRICS_REQ_URL_INTERMEDIATE;

        LwCAStringCountSubstring(pcszPath, "certificate", &iCount);
        if (iCount == 1)
        {
            reqUrl = LWCA_METRICS_REQ_URL_INTERMEDIATE_CERT;
            goto ret;
        }

        iCount = 0;
        LwCAStringCountSubstring(pcszPath, "crl", &iCount);
        if (iCount == 1)
        {
            reqUrl = LWCA_METRICS_REQ_URL_INTERMEDIATE_CRL;
            goto ret;
        }
    }

ret:
    return reqUrl;
}

LWCA_METRICS_HTTP_METHODS
LwCARestMetricsGetHttpMethod(
    PCSTR   pcszMethod
    )
{
    LWCA_METRICS_HTTP_METHODS method = LWCA_METRICS_HTTP_METHOD_UNKNOWN;

    if (LwCAStringCompareA(pcszMethod, "GET", FALSE) == 0)
    {
        method = LWCA_METRICS_HTTP_METHOD_GET;
    }
    else if (LwCAStringCompareA(pcszMethod, "POST", FALSE) == 0)
    {
        method = LWCA_METRICS_HTTP_METHOD_POST;
    }
    else if (LwCAStringCompareA(pcszMethod, "DELETE", FALSE) == 0)
    {
        method = LWCA_METRICS_HTTP_METHOD_DELETE;
    }

    return method;
}

LWCA_METRICS_HTTP_CODES
LwCARestMetricsGetHttpCode(
    int     httpStatus
    )
{
    LWCA_METRICS_HTTP_CODES httpCode = LWCA_METRICS_HTTP_OK;

    switch (httpStatus)
    {
    case 200:
        httpCode = LWCA_METRICS_HTTP_OK;
        break;

    case 204:
        httpCode = LWCA_METRICS_HTTP_NO_CONTENT;
        break;

    case 400:
        httpCode = LWCA_METRICS_HTTP_BAD_REQUEST;
        break;

    case 401:
        httpCode = LWCA_METRICS_HTTP_UNAUTHORIZED;
        break;

    case 403:
        httpCode = LWCA_METRICS_HTTP_FORBIDDEN;
        break;

    case 404:
        httpCode = LWCA_METRICS_HTTP_NOT_FOUND;
        break;

    case 409:
        httpCode = LWCA_METRICS_HTTP_CONFLICT;
        break;

    case 500:
    default:
        httpCode = LWCA_METRICS_HTTP_SERVER_ERROR;
        break;
    }

    return httpCode;
}
