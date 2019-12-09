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
IdmOidcClientDataNew(
    IDM_OIDC_CLIENT_DATA** ppOidcClient,
    PCSTRING clientId,
    const IDM_OIDC_CLIENT_METADATA_DATA* oidcclientMetadataDTO)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_OIDC_CLIENT_DATA* pOidcClient = NULL;

    if (ppOidcClient == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_OIDC_CLIENT_DATA), (void**) &pOidcClient);
    BAIL_ON_ERROR(e);

    if (clientId != NULL)
    {
        e = RestStringDataNew(&(pOidcClient->clientId), clientId);
        BAIL_ON_ERROR(e);
    }

    if (oidcclientMetadataDTO != NULL)
    {
        e = IdmOidcClientMetadataDataNew(
            &(pOidcClient->oidcclientMetadataDTO),
            oidcclientMetadataDTO->redirectUris,
            oidcclientMetadataDTO->tokenEndpointAuthMethod,
            oidcclientMetadataDTO->postLogoutRedirectUris,
            oidcclientMetadataDTO->logoutUri,
            oidcclientMetadataDTO->certSubjectDN,
            oidcclientMetadataDTO->authnRequestClientAssertionLifetimeMS);
        BAIL_ON_ERROR(e);
    }

    *ppOidcClient = pOidcClient;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientDataDelete(pOidcClient);
    }

    return e;
}

void
IdmOidcClientDataDelete(
    IDM_OIDC_CLIENT_DATA* pOidcClient)
{
    if (pOidcClient != NULL)
    {
        RestStringDataDelete(pOidcClient->clientId);
        IdmOidcClientMetadataDataDelete(pOidcClient->oidcclientMetadataDTO);
        SSOMemoryFree(pOidcClient, sizeof(IDM_OIDC_CLIENT_DATA));
    }
}

SSOERROR
IdmOidcClientDataToJson(
    const IDM_OIDC_CLIENT_DATA* pOidcClient,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pOidcClient != NULL)
    {
        e = RestDataToJson(pOidcClient->clientId, REST_JSON_OBJECT_TYPE_STRING, NULL, "clientId", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pOidcClient->oidcclientMetadataDTO,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmOidcClientMetadataDataToJson,
            "oidcclientMetadataDTO",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToOidcClientData(
    PCSSO_JSON pJson,
    IDM_OIDC_CLIENT_DATA** ppOidcClient)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_OIDC_CLIENT_DATA* pOidcClient = NULL;

    if (pJson == NULL || ppOidcClient == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_OIDC_CLIENT_DATA), (void**) &pOidcClient);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "clientId", (void**) &(pOidcClient->clientId));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToOidcClientMetadataData,
        "oidcclientMetadataDTO",
        (void**) &(pOidcClient->oidcclientMetadataDTO));
    BAIL_ON_ERROR(e);

    *ppOidcClient = pOidcClient;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientDataDelete(pOidcClient);
    }

    return e;
}
