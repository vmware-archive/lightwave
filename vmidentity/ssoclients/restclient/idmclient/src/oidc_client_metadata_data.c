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

SSOERROR
IdmOidcClientMetadataDataNew(
    IDM_OIDC_CLIENT_METADATA_DATA** ppOidcClientMetadata,
    const IDM_STRING_ARRAY_DATA* redirectUris,
    PCSTRING tokenEndpointAuthMethod,
    const IDM_STRING_ARRAY_DATA* postLogoutRedirectUris,
    PCSTRING logoutUri,
    PCSTRING certSubjectDN,
    const SSO_LONG* authnRequestClientAssertionLifetimeMS)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata = NULL;

    if (ppOidcClientMetadata == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_OIDC_CLIENT_METADATA_DATA), (void**) &pOidcClientMetadata);
    BAIL_ON_ERROR(e);

    if (redirectUris != NULL)
    {
        e = IdmStringArrayDataNew(&(pOidcClientMetadata->redirectUris), redirectUris->ppEntry, redirectUris->length);
        BAIL_ON_ERROR(e);
    }

    if (tokenEndpointAuthMethod != NULL)
    {
        e = RestStringDataNew(&(pOidcClientMetadata->tokenEndpointAuthMethod), tokenEndpointAuthMethod);
        BAIL_ON_ERROR(e);
    }

    if (postLogoutRedirectUris != NULL)
    {
        e = IdmStringArrayDataNew(
            &(pOidcClientMetadata->postLogoutRedirectUris),
            postLogoutRedirectUris->ppEntry,
            postLogoutRedirectUris->length);
        BAIL_ON_ERROR(e);
    }

    if (logoutUri != NULL)
    {
        e = RestStringDataNew(&(pOidcClientMetadata->logoutUri), logoutUri);
        BAIL_ON_ERROR(e);
    }

    if (certSubjectDN != NULL)
    {
        e = RestStringDataNew(&(pOidcClientMetadata->certSubjectDN), certSubjectDN);
        BAIL_ON_ERROR(e);
    }

    if (authnRequestClientAssertionLifetimeMS != NULL)
    {
        e = RestLongDataNew(
            &(pOidcClientMetadata->authnRequestClientAssertionLifetimeMS),
            *authnRequestClientAssertionLifetimeMS);
        BAIL_ON_ERROR(e);
    }

    *ppOidcClientMetadata = pOidcClientMetadata;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientMetadataDataDelete(pOidcClientMetadata);
    }

    return e;
}

void
IdmOidcClientMetadataDataDelete(
    IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata)
{
    if (pOidcClientMetadata != NULL)
    {
        IdmStringArrayDataDelete(pOidcClientMetadata->redirectUris);
        RestStringDataDelete(pOidcClientMetadata->tokenEndpointAuthMethod);
        IdmStringArrayDataDelete(pOidcClientMetadata->postLogoutRedirectUris);
        RestStringDataDelete(pOidcClientMetadata->logoutUri);
        RestStringDataDelete(pOidcClientMetadata->certSubjectDN);
        RestLongDataDelete(pOidcClientMetadata->authnRequestClientAssertionLifetimeMS);
        SSOMemoryFree(pOidcClientMetadata, sizeof(IDM_OIDC_CLIENT_METADATA_DATA));
    }
}

SSOERROR
IdmOidcClientMetadataDataToJson(
    const IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pOidcClientMetadata != NULL)
    {
        e = RestDataToJson(
            pOidcClientMetadata->redirectUris,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "redirectUris",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pOidcClientMetadata->tokenEndpointAuthMethod,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "tokenEndpointAuthMethod",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pOidcClientMetadata->postLogoutRedirectUris,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "postLogoutRedirectUris",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pOidcClientMetadata->logoutUri,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "logoutUri",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pOidcClientMetadata->certSubjectDN,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "certSubjectDN",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pOidcClientMetadata->authnRequestClientAssertionLifetimeMS,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "authnRequestClientAssertionLifetimeMS",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToOidcClientMetadataData(
    PCSSO_JSON pJson,
    IDM_OIDC_CLIENT_METADATA_DATA** ppOidcClientMetadata)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata = NULL;

    if (pJson == NULL || ppOidcClientMetadata == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_OIDC_CLIENT_METADATA_DATA), (void**) &pOidcClientMetadata);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "redirectUris",
        (void**) &(pOidcClientMetadata->redirectUris));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "tokenEndpointAuthMethod",
        (void**) &(pOidcClientMetadata->tokenEndpointAuthMethod));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "postLogoutRedirectUris",
        (void**) &(pOidcClientMetadata->postLogoutRedirectUris));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "logoutUri",
        (void**) &(pOidcClientMetadata->logoutUri));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "certSubjectDN",
        (void**) &(pOidcClientMetadata->certSubjectDN));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "authnRequestClientAssertionLifetimeMS",
        (void**) &(pOidcClientMetadata->authnRequestClientAssertionLifetimeMS));
    BAIL_ON_ERROR(e);

    *ppOidcClientMetadata = pOidcClientMetadata;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientMetadataDataDelete(pOidcClientMetadata);
    }

    return e;
}
