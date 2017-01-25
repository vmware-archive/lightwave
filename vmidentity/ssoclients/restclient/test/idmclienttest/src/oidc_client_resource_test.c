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

static PCSTRING tenant = "test_tenant_name";
static PSTRING clientId = "21362950-79bd-4ac0-aeab-3fed6facee4f";

PCSTRING
IdmOidcClientRegisterTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_OIDC_CLIENT_DATA* pOidcClientReturn = NULL;

    IDM_STRING_ARRAY_DATA* pRedirectUris = NULL;
    PSTRING pRedirectUriArray[] =
    {
        "https://abc.com/redirect1",
        "https://abc.com/redirect2"
    };

    PSTRING tokenEndpointAuthMethod = "none";
    IDM_STRING_ARRAY_DATA* pPostLogoutRedirectUris = NULL;
    PSTRING logoutUri = NULL;
    PSTRING certSubjectDN = NULL;
    SSO_LONG authnRequestClientAssertionLifetimeMS = 100;

    IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata = NULL;


    e = IdmStringArrayDataNew(
        &pRedirectUris,
        pRedirectUriArray,
        (size_t) (sizeof(pRedirectUriArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmOidcClientMetadataDataNew(
        &pOidcClientMetadata,
        pRedirectUris,
        tokenEndpointAuthMethod,
        pPostLogoutRedirectUris,
        logoutUri,
        certSubjectDN,
        &authnRequestClientAssertionLifetimeMS);
    BAIL_ON_ERROR(e);

    e = IdmOidcClientRegister(pBearerTokenClient, tenant, pOidcClientMetadata, &pOidcClientReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmOidcClientGetAllTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArrayReturn = NULL;

    e = IdmOidcClientGetAll(pBearerTokenClient, tenant, &pOidcClientArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmOidcClientGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_OIDC_CLIENT_DATA* pOidcClientReturn = NULL;

    e = IdmOidcClientGet(pBearerTokenClient, tenant, clientId, &pOidcClientReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmOidcClientUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    IDM_OIDC_CLIENT_DATA* pOidcClientReturn = NULL;

    IDM_STRING_ARRAY_DATA* pRedirectUris = NULL;
    PSTRING pRedirectUriArray[] =
    {
        "https://abc.com/redirect1",
        "https://abc.com/redirect2"
    };

    PSTRING tokenEndpointAuthMethod = "none";
    IDM_STRING_ARRAY_DATA* pPostLogoutRedirectUris = NULL;
    PSTRING logoutUri = NULL;
    PSTRING certSubjectDN = NULL;
    SSO_LONG authnRequestClientAssertionLifetimeMS = 100;

    IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata = NULL;

    e = IdmStringArrayDataNew(
        &pRedirectUris,
        pRedirectUriArray,
        (size_t) (sizeof(pRedirectUriArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmOidcClientMetadataDataNew(
        &pOidcClientMetadata,
        pRedirectUris,
        tokenEndpointAuthMethod,
        pPostLogoutRedirectUris,
        logoutUri,
        certSubjectDN,
        &authnRequestClientAssertionLifetimeMS);
    BAIL_ON_ERROR(e);

    e = IdmOidcClientUpdate(pBearerTokenClient, tenant, clientId, pOidcClientMetadata, &pOidcClientReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmOidcClientDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    e = IdmOidcClientDelete(pBearerTokenClient, tenant, clientId, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
