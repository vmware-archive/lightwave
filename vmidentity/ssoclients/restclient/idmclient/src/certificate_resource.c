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

static PCSTRING IDM_CERTIFICATE_SCOPE_TYPE_ENUMS[] =
{
    "TENANT",
    "EXTERNAL_IDP"
};

static PCSTRING const TENANT_URI = "/idm/tenant";
static PCSTRING const TENANT_POST_URI = "/idm/post/tenant";

SSOERROR
IdmCertificateGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_CERTIFICATE_SCOPE_TYPE certificateScopeType,
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA** ppCertificateChainArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppCertificateChainArrayReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "certificates",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri(
        "scope",
        IDM_CERTIFICATE_SCOPE_TYPE_ENUMS[certificateScopeType],
        true,
        resourceUri,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_GET,
        (JsonToDataObjectFunc) IdmJsonToCertificateChainArrayData,
        (void**) &pCertificateChainArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppCertificateChainArrayReturn = pCertificateChainArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppCertificateChainArrayReturn, (DataObjectToJsonFunc) IdmCertificateChainArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmCertificateChainArrayDataDelete(pCertificateChainArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmCertificateDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING fingerprint,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(fingerprint)
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "certificates",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("fingerprint", fingerprint, true, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_DELETE,
        NULL,
        NULL,
        &pError);
    BAIL_ON_ERROR(e);

    error:

    if (e != SSOERROR_NONE)
    {
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmCertificateGetPrivateKey(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_PRIVATE_KEY_DATA** ppPrivateKeyReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_PRIVATE_KEY_DATA* pPrivateKeyReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppPrivateKeyReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "certificates",
        "privatekey",
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToPrivateKeyData,
        (void**) &pPrivateKeyReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppPrivateKeyReturn = pPrivateKeyReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppPrivateKeyReturn, (DataObjectToJsonFunc) IdmPrivateKeyDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPrivateKeyDataDelete(pPrivateKeyReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmCertificateSetCredentials(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pTenantCredentials == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "certificates",
        "privatekey",
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pTenantCredentials,
        (DataObjectToJsonFunc) IdmTenantCredentialsDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        NULL,
        NULL,
        &pError);
    BAIL_ON_ERROR(e);

    error:

    if (e != SSOERROR_NONE)
    {
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);
    return e;
}
