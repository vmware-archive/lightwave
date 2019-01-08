/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

PSTR
LwCAMetricsReqUrlString(
    LWCA_METRICS_REQ_URLS reqUrl
    )
{
    static PSTR pszReqUrls[LWCA_METRICS_REQ_URL_COUNT] =
    {
        "/v1/mutentca/root",
        "/v1/mutentca/certificate",
        "/v1/mutentca/crl",
        "/v1/mutentca/intermediate/{ca-id}",
        "/v1/mutentca/intermediate/{ca-id}/certificate",
        "/v1/mutentca/intermediate/{ca-id}/crl"
    };

    return pszReqUrls[reqUrl];
}

PSTR
LwCAMetricsHttpMethodString(
    LWCA_METRICS_HTTP_METHODS method
    )
{
    static PSTR pszHttpMethod[LWCA_METRICS_HTTP_METHOD_COUNT] =
    {
        "GET",
        "POST",
        "DELETE"
    };

    return pszHttpMethod[method];
}

PSTR
LwCAMetricsHttpStatusCodeString(
    LWCA_METRICS_HTTP_CODES code
    )
{
    static PSTR pszHttpCodes[LWCA_METRICS_HTTP_CODE_COUNT] =
    {
        "200",
        "204",
        "400",
        "401",
        "403",
        "404",
        "409",
        "500"
    };

    return pszHttpCodes[code];
}

PSTR
LwCAMetricsApiNameString(
    LWCA_METRICS_API_NAMES api
    )
{
    static PSTR pszApiNames[LWCA_METRICS_API_COUNT] =
    {
        "CreateRootCA",
        "CreateIntermediateCA",
        "GetCACertificates",
        "GetSignedCertificate",
        "GetChainOfTrust",
        "GetCACrl",
        "RevokeIntermediateCA",
        "RevokeCertificate"
    };

    return pszApiNames[api];
}

PSTR
LwCAMetricsSecurityApiNameString(
    LWCA_METRICS_SECURITY_APIS api
    )
{
    static PSTR pszSecurityApiNames[LWCA_METRICS_SECURITY_COUNT] =
    {
        "AddKeyPair",
        "CreateKeyPair",
        "SignX509Cert",
        "SignX509Request",
        "SignX509CRL"
    };

    return pszSecurityApiNames[api];
}

PSTR
LwCAMetricsResponseString(
    LWCA_METRICS_RESPONSE_CODES code
    )
{
    static PSTR pszResponseCodes[LWCA_METRICS_RESPONSE_COUNT] =
    {
        "Success",
        "Error"
    };

    return pszResponseCodes[code];
}
