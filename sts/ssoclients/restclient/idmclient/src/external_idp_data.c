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
IdmExternalIdpDataNew(
    IDM_EXTERNAL_IDP_DATA** ppExternalIdp,
    PCSTRING entityID,
    PCSTRING alias,
    const IDM_STRING_ARRAY_DATA* nameIDFormats,
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* ssoServices,
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* sloServices,
    const IDM_CERTIFICATE_CHAIN_DATA* signingCertificates,
    const IDM_STRING_MAP_DATA* subjectFormats,
    const IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* tokenClaimGroups,
    const bool* jitEnabled,
    PCSTRING upnSuffix)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EXTERNAL_IDP_DATA* pExternalIdp = NULL;

    if (ppExternalIdp == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EXTERNAL_IDP_DATA), (void**) &pExternalIdp);
    BAIL_ON_ERROR(e);

    if (entityID != NULL)
    {
        e = RestStringDataNew(&(pExternalIdp->entityID), entityID);
        BAIL_ON_ERROR(e);
    }

    if (alias != NULL)
    {
        e = RestStringDataNew(&(pExternalIdp->alias), alias);
        BAIL_ON_ERROR(e);
    }

    if (nameIDFormats != NULL)
    {
        e = IdmStringArrayDataNew(&(pExternalIdp->nameIDFormats), nameIDFormats->ppEntry, nameIDFormats->length);
        BAIL_ON_ERROR(e);
    }

    if (ssoServices != NULL)
    {
        e = IdmServiceEndpointArrayDataNew(&(pExternalIdp->ssoServices), ssoServices->ppEntry, ssoServices->length);
        BAIL_ON_ERROR(e);
    }

    if (sloServices != NULL)
    {
        e = IdmServiceEndpointArrayDataNew(&(pExternalIdp->sloServices), sloServices->ppEntry, sloServices->length);
        BAIL_ON_ERROR(e);
    }

    if (signingCertificates != NULL)
    {
        e = IdmCertificateChainDataNew(&(pExternalIdp->signingCertificates), signingCertificates->certificates);
        BAIL_ON_ERROR(e);
    }

    if (subjectFormats != NULL)
    {
        e = IdmStringMapDataNew(&(pExternalIdp->subjectFormats), subjectFormats->ppEntry, subjectFormats->length);
        BAIL_ON_ERROR(e);
    }

    if (tokenClaimGroups != NULL)
    {
        e = IdmTokenClaimGroupArrayDataNew(
            &(pExternalIdp->tokenClaimGroups),
            tokenClaimGroups->ppEntry,
            tokenClaimGroups->length);
        BAIL_ON_ERROR(e);
    }

    if (jitEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pExternalIdp->jitEnabled), *jitEnabled);
        BAIL_ON_ERROR(e);
    }

    if (upnSuffix != NULL)
    {
        e = RestStringDataNew(&(pExternalIdp->upnSuffix), upnSuffix);
        BAIL_ON_ERROR(e);
    }

    *ppExternalIdp = pExternalIdp;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpDataDelete(pExternalIdp);
    }

    return e;
}

void
IdmExternalIdpDataDelete(
    IDM_EXTERNAL_IDP_DATA* pExternalIdp)
{
    if (pExternalIdp != NULL)
    {
        RestStringDataDelete(pExternalIdp->entityID);
        RestStringDataDelete(pExternalIdp->alias);
        IdmStringArrayDataDelete(pExternalIdp->nameIDFormats);
        IdmServiceEndpointArrayDataDelete(pExternalIdp->ssoServices);
        IdmServiceEndpointArrayDataDelete(pExternalIdp->sloServices);
        IdmCertificateChainDataDelete(pExternalIdp->signingCertificates);
        IdmStringMapDataDelete(pExternalIdp->subjectFormats);
        IdmTokenClaimGroupArrayDataDelete(pExternalIdp->tokenClaimGroups);
        RestBooleanDataDelete(pExternalIdp->jitEnabled);
        RestStringDataDelete(pExternalIdp->upnSuffix);
        SSOMemoryFree(pExternalIdp, sizeof(IDM_EXTERNAL_IDP_DATA));
    }
}

SSOERROR
IdmExternalIdpDataToJson(
    const IDM_EXTERNAL_IDP_DATA* pExternalIdp,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pExternalIdp != NULL)
    {
        e = RestDataToJson(pExternalIdp->entityID, REST_JSON_OBJECT_TYPE_STRING, NULL, "entityID", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pExternalIdp->alias, REST_JSON_OBJECT_TYPE_STRING, NULL, "alias", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pExternalIdp->nameIDFormats,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "nameIDFormats",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pExternalIdp->ssoServices,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmServiceEndpointArrayDataToJson,
            "ssoServices",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pExternalIdp->sloServices,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmServiceEndpointArrayDataToJson,
            "sloServices",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pExternalIdp->signingCertificates,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmCertificateChainDataToJson,
            "signingCertificates",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pExternalIdp->subjectFormats,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmStringMapDataToJson,
            "subjectFormats",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pExternalIdp->tokenClaimGroups,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmTokenClaimGroupArrayDataToJson,
            "tokenClaimGroups",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pExternalIdp->jitEnabled, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "jitEnabled", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pExternalIdp->upnSuffix, REST_JSON_OBJECT_TYPE_STRING, NULL, "upnSuffix", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToExternalIdpData(
    PCSSO_JSON pJson,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdp)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EXTERNAL_IDP_DATA* pExternalIdp = NULL;

    if (pJson == NULL || ppExternalIdp == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EXTERNAL_IDP_DATA), (void**) &pExternalIdp);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "entityID", (void**) &(pExternalIdp->entityID));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "alias", (void**) &(pExternalIdp->alias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "nameIDFormats",
        (void**) &(pExternalIdp->nameIDFormats));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToServiceEndpointArrayData,
        "ssoServices",
        (void**) &(pExternalIdp->ssoServices));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToServiceEndpointArrayData,
        "sloServices",
        (void**) &(pExternalIdp->sloServices));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToCertificateChainData,
        "signingCertificates",
        (void**) &(pExternalIdp->signingCertificates));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringMapData,
        "subjectFormats",
        (void**) &(pExternalIdp->subjectFormats));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToTokenClaimGroupArrayData,
        "tokenClaimGroups",
        (void**) &(pExternalIdp->tokenClaimGroups));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "jitEnabled",
        (void**) &(pExternalIdp->jitEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "upnSuffix",
        (void**) &(pExternalIdp->upnSuffix));
    BAIL_ON_ERROR(e);

    *ppExternalIdp = pExternalIdp;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpDataDelete(pExternalIdp);
    }

    return e;
}
