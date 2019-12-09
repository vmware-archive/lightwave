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
IdmEventLogProviderMetadataDataNew(
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA** ppEventLogProviderMetadata,
    PCSTRING name,
    PCSTRING type,
    const bool* matchingRuleInChainEnabled,
    const bool* baseDnForNestedGroupsEnabled,
    const bool* directGroupsSearchEnabled,
    const bool* siteAffinityEnabled)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA* pEventLogProviderMetadata = NULL;

    if (ppEventLogProviderMetadata == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_PROVIDER_METADATA_DATA), (void**) &pEventLogProviderMetadata);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pEventLogProviderMetadata->name), name);
        BAIL_ON_ERROR(e);
    }

    if (type != NULL)
    {
        e = RestStringDataNew(&(pEventLogProviderMetadata->type), type);
        BAIL_ON_ERROR(e);
    }

    if (matchingRuleInChainEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pEventLogProviderMetadata->matchingRuleInChainEnabled), *matchingRuleInChainEnabled);
        BAIL_ON_ERROR(e);
    }

    if (baseDnForNestedGroupsEnabled != NULL)
    {
        e = RestBooleanDataNew(
            &(pEventLogProviderMetadata->baseDnForNestedGroupsEnabled),
            *baseDnForNestedGroupsEnabled);
        BAIL_ON_ERROR(e);
    }

    if (directGroupsSearchEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pEventLogProviderMetadata->directGroupsSearchEnabled), *directGroupsSearchEnabled);
        BAIL_ON_ERROR(e);
    }

    if (siteAffinityEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pEventLogProviderMetadata->siteAffinityEnabled), *siteAffinityEnabled);
        BAIL_ON_ERROR(e);
    }

    *ppEventLogProviderMetadata = pEventLogProviderMetadata;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogProviderMetadataDataDelete(pEventLogProviderMetadata);
    }

    return e;
}

void
IdmEventLogProviderMetadataDataDelete(
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA* pEventLogProviderMetadata)
{
    if (pEventLogProviderMetadata != NULL)
    {
        RestStringDataDelete(pEventLogProviderMetadata->name);
        RestStringDataDelete(pEventLogProviderMetadata->type);
        RestBooleanDataDelete(pEventLogProviderMetadata->matchingRuleInChainEnabled);
        RestBooleanDataDelete(pEventLogProviderMetadata->baseDnForNestedGroupsEnabled);
        RestBooleanDataDelete(pEventLogProviderMetadata->directGroupsSearchEnabled);
        RestBooleanDataDelete(pEventLogProviderMetadata->siteAffinityEnabled);
        SSOMemoryFree(pEventLogProviderMetadata, sizeof(IDM_EVENT_LOG_PROVIDER_METADATA_DATA));
    }
}

SSOERROR
IdmEventLogProviderMetadataDataToJson(
    const IDM_EVENT_LOG_PROVIDER_METADATA_DATA* pEventLogProviderMetadata,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLogProviderMetadata != NULL)
    {
        e = RestDataToJson(pEventLogProviderMetadata->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLogProviderMetadata->type, REST_JSON_OBJECT_TYPE_STRING, NULL, "type", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogProviderMetadata->matchingRuleInChainEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "matchingRuleInChainEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogProviderMetadata->baseDnForNestedGroupsEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "baseDnForNestedGroupsEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogProviderMetadata->directGroupsSearchEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "directGroupsSearchEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogProviderMetadata->siteAffinityEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "siteAffinityEnabled",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogProviderMetadataData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA** ppEventLogProviderMetadata)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_PROVIDER_METADATA_DATA* pEventLogProviderMetadata = NULL;

    if (pJson == NULL || ppEventLogProviderMetadata == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_PROVIDER_METADATA_DATA), (void**) &pEventLogProviderMetadata);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "name",
        (void**) &(pEventLogProviderMetadata->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "type",
        (void**) &(pEventLogProviderMetadata->type));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "matchingRuleInChainEnabled",
        (void**) &(pEventLogProviderMetadata->matchingRuleInChainEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "baseDnForNestedGroupsEnabled",
        (void**) &(pEventLogProviderMetadata->baseDnForNestedGroupsEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "directGroupsSearchEnabled",
        (void**) &(pEventLogProviderMetadata->directGroupsSearchEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "siteAffinityEnabled",
        (void**) &(pEventLogProviderMetadata->siteAffinityEnabled));
    BAIL_ON_ERROR(e);

    *ppEventLogProviderMetadata = pEventLogProviderMetadata;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogProviderMetadataDataDelete(pEventLogProviderMetadata);
    }

    return e;
}
