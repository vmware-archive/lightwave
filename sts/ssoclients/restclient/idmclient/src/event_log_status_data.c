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
IdmEventLogStatusDataNew(
    IDM_EVENT_LOG_STATUS_DATA** ppEventLogStatus,
    const bool* enabled,
    const SSO_LONG* size)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_STATUS_DATA* pEventLogStatus = NULL;

    if (ppEventLogStatus == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_STATUS_DATA), (void**) &pEventLogStatus);
    BAIL_ON_ERROR(e);

    if (enabled != NULL)
    {
        e = RestBooleanDataNew(&(pEventLogStatus->enabled), *enabled);
        BAIL_ON_ERROR(e);
    }

    if (size != NULL)
    {
        e = RestLongDataNew(&(pEventLogStatus->size), *size);
        BAIL_ON_ERROR(e);
    }

    *ppEventLogStatus = pEventLogStatus;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogStatusDataDelete(pEventLogStatus);
    }

    return e;
}

void
IdmEventLogStatusDataDelete(
    IDM_EVENT_LOG_STATUS_DATA* pEventLogStatus)
{
    if (pEventLogStatus != NULL)
    {
        RestBooleanDataDelete(pEventLogStatus->enabled);
        RestLongDataDelete(pEventLogStatus->size);
        SSOMemoryFree(pEventLogStatus, sizeof(IDM_EVENT_LOG_STATUS_DATA));
    }
}

SSOERROR
IdmEventLogStatusDataToJson(
    const IDM_EVENT_LOG_STATUS_DATA* pEventLogStatus,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLogStatus != NULL)
    {
        e = RestDataToJson(pEventLogStatus->enabled, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "enabled", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLogStatus->size, REST_JSON_OBJECT_TYPE_LONG, NULL, "size", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogStatusData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_STATUS_DATA** ppEventLogStatus)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_STATUS_DATA* pEventLogStatus = NULL;

    if (pJson == NULL || ppEventLogStatus == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_STATUS_DATA), (void**) &pEventLogStatus);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "enabled",
        (void**) &(pEventLogStatus->enabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_LONG, NULL, "size", (void**) &(pEventLogStatus->size));
    BAIL_ON_ERROR(e);

    *ppEventLogStatus = pEventLogStatus;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogStatusDataDelete(pEventLogStatus);
    }

    return e;
}
