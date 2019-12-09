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
IdmEventLogMetadataDataNew(
    IDM_EVENT_LOG_METADATA_DATA** ppEventLogMetadata,
    PCSTRING username,
    const IDM_EVENT_LOG_PROVIDER_METADATA_DATA* provider,
    const IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* ldapQueryStats,
    const IDM_STRING_MAP_DATA* extensions)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_METADATA_DATA* pEventLogMetadata = NULL;

    if (ppEventLogMetadata == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_METADATA_DATA), (void**) &pEventLogMetadata);
    BAIL_ON_ERROR(e);

    if (username != NULL)
    {
        e = RestStringDataNew(&(pEventLogMetadata->username), username);
        BAIL_ON_ERROR(e);
    }

    if (provider != NULL)
    {
        e = IdmEventLogProviderMetadataDataNew(
            &(pEventLogMetadata->provider),
            provider->name,
            provider->type,
            provider->matchingRuleInChainEnabled,
            provider->baseDnForNestedGroupsEnabled,
            provider->directGroupsSearchEnabled,
            provider->siteAffinityEnabled);
        BAIL_ON_ERROR(e);
    }

    if (ldapQueryStats != NULL)
    {
        e = IdmEventLogLdapQueryStatArrayDataNew(
            &(pEventLogMetadata->ldapQueryStats),
            ldapQueryStats->ppEntry,
            ldapQueryStats->length);
        BAIL_ON_ERROR(e);
    }

    if (extensions != NULL)
    {
        e = IdmStringMapDataNew(&(pEventLogMetadata->extensions), extensions->ppEntry, extensions->length);
        BAIL_ON_ERROR(e);
    }

    *ppEventLogMetadata = pEventLogMetadata;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogMetadataDataDelete(pEventLogMetadata);
    }

    return e;
}

void
IdmEventLogMetadataDataDelete(
    IDM_EVENT_LOG_METADATA_DATA* pEventLogMetadata)
{
    if (pEventLogMetadata != NULL)
    {
        RestStringDataDelete(pEventLogMetadata->username);
        IdmEventLogProviderMetadataDataDelete(pEventLogMetadata->provider);
        IdmEventLogLdapQueryStatArrayDataDelete(pEventLogMetadata->ldapQueryStats);
        IdmStringMapDataDelete(pEventLogMetadata->extensions);
        SSOMemoryFree(pEventLogMetadata, sizeof(IDM_EVENT_LOG_METADATA_DATA));
    }
}

SSOERROR
IdmEventLogMetadataDataToJson(
    const IDM_EVENT_LOG_METADATA_DATA* pEventLogMetadata,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLogMetadata != NULL)
    {
        e = RestDataToJson(pEventLogMetadata->username, REST_JSON_OBJECT_TYPE_STRING, NULL, "username", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogMetadata->provider,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmEventLogProviderMetadataDataToJson,
            "provider",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogMetadata->ldapQueryStats,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmEventLogLdapQueryStatArrayDataToJson,
            "ldapQueryStats",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogMetadata->extensions,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmStringMapDataToJson,
            "extensions",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogMetadataData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_METADATA_DATA** ppEventLogMetadata)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_METADATA_DATA* pEventLogMetadata = NULL;

    if (pJson == NULL || ppEventLogMetadata == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_METADATA_DATA), (void**) &pEventLogMetadata);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "username",
        (void**) &(pEventLogMetadata->username));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToEventLogProviderMetadataData,
        "provider",
        (void**) &(pEventLogMetadata->provider));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToEventLogLdapQueryStatArrayData,
        "ldapQueryStats",
        (void**) &(pEventLogMetadata->ldapQueryStats));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringMapData,
        "extensions",
        (void**) &(pEventLogMetadata->extensions));
    BAIL_ON_ERROR(e);

    *ppEventLogMetadata = pEventLogMetadata;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogMetadataDataDelete(pEventLogMetadata);
    }

    return e;
}
