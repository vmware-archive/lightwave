/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

static PCSTRING const VECS_SSL_URI = "/afd/vecs/ssl";

SSOERROR
AfdVecsGetSSLCertificates(
    PCREST_CLIENT pClient,
    REST_CERTIFICATE_ARRAY_DATA** ppCertificateArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_CERTIFICATE_ARRAY_DATA* pCertificateArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || ppCertificateArrayReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        VECS_SSL_URI,
        NULL,
        NULL,
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_GET,
        (JsonToDataObjectFunc) RestJsonToCertificateArrayData,
        (void**) &pCertificateArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppCertificateArrayReturn = pCertificateArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppCertificateArrayReturn, (DataObjectToJsonFunc) RestCertificateArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        RestCertificateArrayDataDelete(pCertificateArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}
