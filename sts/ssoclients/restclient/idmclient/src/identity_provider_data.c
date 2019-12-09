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
IdmIdentityProviderDataNew(
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProvider,
    PCSTRING domainType,
    PCSTRING name,
    PCSTRING alias,
    PCSTRING type,
    PCSTRING authenticationType,
    PCSTRING friendlyName,
    const SSO_LONG* searchTimeOutInSeconds,
    PCSTRING username,
    PCSTRING password,
    const bool* machineAccount,
    PCSTRING servicePrincipalName,
    PCSTRING userBaseDN,
    PCSTRING groupBaseDN,
    const IDM_STRING_ARRAY_DATA* connectionStrings,
    const IDM_STRING_MAP_DATA* attributesMap,
    const IDM_STRING_ARRAY_DATA* upnSuffixes,
    const IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* schema,
    const bool* matchingRuleInChainEnabled,
    const bool* baseDnForNestedGroupsEnabled,
    const bool* directGroupsSearchEnabled,
    const bool* siteAffinityEnabled,
    const REST_CERTIFICATE_ARRAY_DATA* certificates)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider = NULL;

    if (ppIdentityProvider == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_IDENTITY_PROVIDER_DATA), (void**) &pIdentityProvider);
    BAIL_ON_ERROR(e);

    if (domainType != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->domainType), domainType);
        BAIL_ON_ERROR(e);
    }

    if (name != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->name), name);
        BAIL_ON_ERROR(e);
    }

    if (alias != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->alias), alias);
        BAIL_ON_ERROR(e);
    }

    if (type != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->type), type);
        BAIL_ON_ERROR(e);
    }

    if (authenticationType != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->authenticationType), authenticationType);
        BAIL_ON_ERROR(e);
    }

    if (friendlyName != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->friendlyName), friendlyName);
        BAIL_ON_ERROR(e);
    }

    if (searchTimeOutInSeconds != NULL)
    {
        e = RestLongDataNew(&(pIdentityProvider->searchTimeOutInSeconds), *searchTimeOutInSeconds);
        BAIL_ON_ERROR(e);
    }

    if (username != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->username), username);
        BAIL_ON_ERROR(e);
    }

    if (password != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->password), password);
        BAIL_ON_ERROR(e);
    }

    if (machineAccount != NULL)
    {
        e = RestBooleanDataNew(&(pIdentityProvider->machineAccount), *machineAccount);
        BAIL_ON_ERROR(e);
    }

    if (servicePrincipalName != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->servicePrincipalName), servicePrincipalName);
        BAIL_ON_ERROR(e);
    }

    if (userBaseDN != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->userBaseDN), userBaseDN);
        BAIL_ON_ERROR(e);
    }

    if (groupBaseDN != NULL)
    {
        e = RestStringDataNew(&(pIdentityProvider->groupBaseDN), groupBaseDN);
        BAIL_ON_ERROR(e);
    }

    if (connectionStrings != NULL)
    {
        e = IdmStringArrayDataNew(
            &(pIdentityProvider->connectionStrings),
            connectionStrings->ppEntry,
            connectionStrings->length);
        BAIL_ON_ERROR(e);
    }

    if (attributesMap != NULL)
    {
        e = IdmStringMapDataNew(&(pIdentityProvider->attributesMap), attributesMap->ppEntry, attributesMap->length);
        BAIL_ON_ERROR(e);
    }

    if (upnSuffixes != NULL)
    {
        e = IdmStringArrayDataNew(&(pIdentityProvider->upnSuffixes), upnSuffixes->ppEntry, upnSuffixes->length);
        BAIL_ON_ERROR(e);
    }

    if (schema != NULL)
    {
        e = IdmSchemaObjectMappingMapDataNew(&(pIdentityProvider->schema), schema->ppEntry, schema->length);
        BAIL_ON_ERROR(e);
    }

    if (matchingRuleInChainEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pIdentityProvider->matchingRuleInChainEnabled), *matchingRuleInChainEnabled);
        BAIL_ON_ERROR(e);
    }

    if (baseDnForNestedGroupsEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pIdentityProvider->baseDnForNestedGroupsEnabled), *baseDnForNestedGroupsEnabled);
        BAIL_ON_ERROR(e);
    }

    if (directGroupsSearchEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pIdentityProvider->directGroupsSearchEnabled), *directGroupsSearchEnabled);
        BAIL_ON_ERROR(e);
    }

    if (siteAffinityEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pIdentityProvider->siteAffinityEnabled), *siteAffinityEnabled);
        BAIL_ON_ERROR(e);
    }

    if (certificates != NULL)
    {
        e = RestCertificateArrayDataNew(&(pIdentityProvider->certificates), certificates->ppEntry, certificates->length);
        BAIL_ON_ERROR(e);
    }

    *ppIdentityProvider = pIdentityProvider;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderDataDelete(pIdentityProvider);
    }

    return e;
}

void
IdmIdentityProviderDataDelete(
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider)
{
    if (pIdentityProvider != NULL)
    {
        RestStringDataDelete(pIdentityProvider->domainType);
        RestStringDataDelete(pIdentityProvider->name);
        RestStringDataDelete(pIdentityProvider->alias);
        RestStringDataDelete(pIdentityProvider->type);
        RestStringDataDelete(pIdentityProvider->authenticationType);
        RestStringDataDelete(pIdentityProvider->friendlyName);
        RestLongDataDelete(pIdentityProvider->searchTimeOutInSeconds);
        RestStringDataDelete(pIdentityProvider->username);
        RestStringDataDelete(pIdentityProvider->password);
        RestBooleanDataDelete(pIdentityProvider->machineAccount);
        RestStringDataDelete(pIdentityProvider->servicePrincipalName);
        RestStringDataDelete(pIdentityProvider->userBaseDN);
        RestStringDataDelete(pIdentityProvider->groupBaseDN);
        IdmStringArrayDataDelete(pIdentityProvider->connectionStrings);
        IdmStringMapDataDelete(pIdentityProvider->attributesMap);
        IdmStringArrayDataDelete(pIdentityProvider->upnSuffixes);
        IdmSchemaObjectMappingMapDataDelete(pIdentityProvider->schema);
        RestBooleanDataDelete(pIdentityProvider->matchingRuleInChainEnabled);
        RestBooleanDataDelete(pIdentityProvider->baseDnForNestedGroupsEnabled);
        RestBooleanDataDelete(pIdentityProvider->directGroupsSearchEnabled);
        RestBooleanDataDelete(pIdentityProvider->siteAffinityEnabled);
        RestCertificateArrayDataDelete(pIdentityProvider->certificates);
        SSOMemoryFree(pIdentityProvider, sizeof(IDM_IDENTITY_PROVIDER_DATA));
    }
}

SSOERROR
IdmIdentityProviderDataToJson(
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pIdentityProvider != NULL)
    {
        e = RestDataToJson(
            pIdentityProvider->domainType,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "domainType",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pIdentityProvider->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pIdentityProvider->alias, REST_JSON_OBJECT_TYPE_STRING, NULL, "alias", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pIdentityProvider->type, REST_JSON_OBJECT_TYPE_STRING, NULL, "type", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->authenticationType,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "authenticationType",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->friendlyName,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "friendlyName",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->searchTimeOutInSeconds,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "searchTimeOutInSeconds",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pIdentityProvider->username, REST_JSON_OBJECT_TYPE_STRING, NULL, "username", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pIdentityProvider->password, REST_JSON_OBJECT_TYPE_STRING, NULL, "password", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->machineAccount,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "machineAccount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->servicePrincipalName,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "servicePrincipalName",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->userBaseDN,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "userBaseDN",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->groupBaseDN,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "groupBaseDN",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->connectionStrings,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "connectionStrings",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->attributesMap,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmStringMapDataToJson,
            "attributesMap",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->upnSuffixes,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "upnSuffixes",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->schema,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmSchemaObjectMappingMapDataToJson,
            "schema",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->matchingRuleInChainEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "matchingRuleInChainEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->baseDnForNestedGroupsEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "baseDnForNestedGroupsEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->directGroupsSearchEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "directGroupsSearchEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->siteAffinityEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "siteAffinityEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pIdentityProvider->certificates,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) RestCertificateArrayDataToJson,
            "certificates",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToIdentityProviderData(
    PCSSO_JSON pJson,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProvider)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider = NULL;

    if (pJson == NULL || ppIdentityProvider == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_IDENTITY_PROVIDER_DATA), (void**) &pIdentityProvider);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "domainType",
        (void**) &(pIdentityProvider->domainType));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pIdentityProvider->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "alias", (void**) &(pIdentityProvider->alias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "type", (void**) &(pIdentityProvider->type));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "authenticationType",
        (void**) &(pIdentityProvider->authenticationType));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "friendlyName",
        (void**) &(pIdentityProvider->friendlyName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "searchTimeOutInSeconds",
        (void**) &(pIdentityProvider->searchTimeOutInSeconds));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "username",
        (void**) &(pIdentityProvider->username));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "password",
        (void**) &(pIdentityProvider->password));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "machineAccount",
        (void**) &(pIdentityProvider->machineAccount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "servicePrincipalName",
        (void**) &(pIdentityProvider->servicePrincipalName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "userBaseDN",
        (void**) &(pIdentityProvider->userBaseDN));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "groupBaseDN",
        (void**) &(pIdentityProvider->groupBaseDN));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "connectionStrings",
        (void**) &(pIdentityProvider->connectionStrings));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringMapData,
        "attributesMap",
        (void**) &(pIdentityProvider->attributesMap));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "upnSuffixes",
        (void**) &(pIdentityProvider->upnSuffixes));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToSchemaObjectMappingMapData,
        "schema",
        (void**) &(pIdentityProvider->schema));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "matchingRuleInChainEnabled",
        (void**) &(pIdentityProvider->matchingRuleInChainEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "baseDnForNestedGroupsEnabled",
        (void**) &(pIdentityProvider->baseDnForNestedGroupsEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "directGroupsSearchEnabled",
        (void**) &(pIdentityProvider->directGroupsSearchEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "siteAffinityEnabled",
        (void**) &(pIdentityProvider->siteAffinityEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) RestJsonToCertificateArrayData,
        "certificates",
        (void**) &(pIdentityProvider->certificates));
    BAIL_ON_ERROR(e);

    *ppIdentityProvider = pIdentityProvider;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderDataDelete(pIdentityProvider);
    }

    return e;
}
