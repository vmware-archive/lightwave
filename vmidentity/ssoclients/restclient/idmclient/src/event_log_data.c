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
IdmEventLogDataNew(
    IDM_EVENT_LOG_DATA** ppEventLog,
    PCSTRING type,
    PCSTRING correlationId,
    PCSTRING level,
    const SSO_LONG* start,
    const SSO_LONG* elapsedMillis,
    const IDM_EVENT_LOG_METADATA_DATA* metadata)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_DATA* pEventLog = NULL;

    if (ppEventLog == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_DATA), (void**) &pEventLog);
    BAIL_ON_ERROR(e);

    if (type != NULL)
    {
        e = RestStringDataNew(&(pEventLog->type), type);
        BAIL_ON_ERROR(e);
    }

    if (correlationId != NULL)
    {
        e = RestStringDataNew(&(pEventLog->correlationId), correlationId);
        BAIL_ON_ERROR(e);
    }

    if (level != NULL)
    {
        e = RestStringDataNew(&(pEventLog->level), level);
        BAIL_ON_ERROR(e);
    }

    if (start != NULL)
    {
        e = RestLongDataNew(&(pEventLog->start), *start);
        BAIL_ON_ERROR(e);
    }

    if (elapsedMillis != NULL)
    {
        e = RestLongDataNew(&(pEventLog->elapsedMillis), *elapsedMillis);
        BAIL_ON_ERROR(e);
    }

    if (metadata != NULL)
    {
        e = IdmEventLogMetadataDataNew(
            &(pEventLog->metadata),
            metadata->username,
            metadata->provider,
            metadata->ldapQueryStats,
            metadata->extensions);
        BAIL_ON_ERROR(e);
    }

    *ppEventLog = pEventLog;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogDataDelete(pEventLog);
    }

    return e;
}

void
IdmEventLogDataDelete(
    IDM_EVENT_LOG_DATA* pEventLog)
{
    if (pEventLog != NULL)
    {
        RestStringDataDelete(pEventLog->type);
        RestStringDataDelete(pEventLog->correlationId);
        RestStringDataDelete(pEventLog->level);
        RestLongDataDelete(pEventLog->start);
        RestLongDataDelete(pEventLog->elapsedMillis);
        IdmEventLogMetadataDataDelete(pEventLog->metadata);
        SSOMemoryFree(pEventLog, sizeof(IDM_EVENT_LOG_DATA));
    }
}

SSOERROR
IdmEventLogDataToJson(
    const IDM_EVENT_LOG_DATA* pEventLog,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLog != NULL)
    {
        e = RestDataToJson(pEventLog->type, REST_JSON_OBJECT_TYPE_STRING, NULL, "type", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLog->correlationId, REST_JSON_OBJECT_TYPE_STRING, NULL, "correlationId", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLog->level, REST_JSON_OBJECT_TYPE_STRING, NULL, "level", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLog->start, REST_JSON_OBJECT_TYPE_LONG, NULL, "start", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLog->elapsedMillis, REST_JSON_OBJECT_TYPE_LONG, NULL, "elapsedMillis", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLog->metadata,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmEventLogMetadataDataToJson,
            "metadata",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_DATA** ppEventLog)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_DATA* pEventLog = NULL;

    if (pJson == NULL || ppEventLog == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_DATA), (void**) &pEventLog);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "type", (void**) &(pEventLog->type));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "correlationId",
        (void**) &(pEventLog->correlationId));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "level", (void**) &(pEventLog->level));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_LONG, NULL, "start", (void**) &(pEventLog->start));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "elapsedMillis",
        (void**) &(pEventLog->elapsedMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToEventLogMetadataData,
        "metadata",
        (void**) &(pEventLog->metadata));
    BAIL_ON_ERROR(e);

    *ppEventLog = pEventLog;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogDataDelete(pEventLog);
    }

    return e;
}
