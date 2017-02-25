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
IdmEventLogArrayDataNew(
    IDM_EVENT_LOG_ARRAY_DATA** ppEventLogArray,
    IDM_EVENT_LOG_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_ARRAY_DATA* pEventLogArray = NULL;
    size_t i = 0;

    if (ppEventLogArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_ARRAY_DATA), (void**) &pEventLogArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pEventLogArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_EVENT_LOG_DATA*), (void**) &(pEventLogArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmEventLogDataNew(
                &(pEventLogArray->ppEntry[i]),
                ppEntry[i]->type,
                ppEntry[i]->correlationId,
                ppEntry[i]->level,
                ppEntry[i]->start,
                ppEntry[i]->elapsedMillis,
                ppEntry[i]->metadata);
            BAIL_ON_ERROR(e);
        }
    }

    *ppEventLogArray = pEventLogArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogArrayDataDelete(pEventLogArray);
    }

    return e;
}

void
IdmEventLogArrayDataDelete(
    IDM_EVENT_LOG_ARRAY_DATA* pEventLogArray)
{
    if (pEventLogArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pEventLogArray->ppEntry,
            pEventLogArray->length,
            (GenericDestructorFunction) IdmEventLogDataDelete);
        SSOMemoryFree(pEventLogArray, sizeof(IDM_EVENT_LOG_ARRAY_DATA));
    }
}

SSOERROR
IdmEventLogArrayDataToJson(
    const IDM_EVENT_LOG_ARRAY_DATA* pEventLogArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLogArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pEventLogArray,
            (DataObjectToJsonFunc) IdmEventLogDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogArrayData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_ARRAY_DATA** ppEventLogArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_ARRAY_DATA* pEventLogArray = NULL;

    if (pJson == NULL || ppEventLogArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToEventLogData,
        (REST_GENERIC_ARRAY_DATA**) &pEventLogArray);
    BAIL_ON_ERROR(e);

    *ppEventLogArray = pEventLogArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogArrayDataDelete(pEventLogArray);
    }

    return e;
}
