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
IdmAttributeConsumerServiceDataNew(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppAttributeConsumerService,
    PCSTRING name,
    const INTEGER* index,
    const IDM_ATTRIBUTE_ARRAY_DATA* attributes)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService = NULL;

    if (ppAttributeConsumerService == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA), (void**) &pAttributeConsumerService);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pAttributeConsumerService->name), name);
        BAIL_ON_ERROR(e);
    }

    if (index != NULL)
    {
        e = RestIntegerDataNew(&(pAttributeConsumerService->index), *index);
        BAIL_ON_ERROR(e);
    }

    if (attributes != NULL)
    {
        e = IdmAttributeArrayDataNew(&(pAttributeConsumerService->attributes), attributes->ppEntry, attributes->length);
        BAIL_ON_ERROR(e);
    }

    *ppAttributeConsumerService = pAttributeConsumerService;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeConsumerServiceDataDelete(pAttributeConsumerService);
    }

    return e;
}

void
IdmAttributeConsumerServiceDataDelete(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService)
{
    if (pAttributeConsumerService != NULL)
    {
        RestStringDataDelete(pAttributeConsumerService->name);
        RestIntegerDataDelete(pAttributeConsumerService->index);
        IdmAttributeArrayDataDelete(pAttributeConsumerService->attributes);
        SSOMemoryFree(pAttributeConsumerService, sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA));
    }
}

SSOERROR
IdmAttributeConsumerServiceDataToJson(
    const IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAttributeConsumerService != NULL)
    {
        e = RestDataToJson(pAttributeConsumerService->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pAttributeConsumerService->index, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "index", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pAttributeConsumerService->attributes,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmAttributeArrayDataToJson,
            "attributes",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAttributeConsumerServiceData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppAttributeConsumerService)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService = NULL;

    if (pJson == NULL || ppAttributeConsumerService == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA), (void**) &pAttributeConsumerService);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "name",
        (void**) &(pAttributeConsumerService->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "index",
        (void**) &(pAttributeConsumerService->index));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToAttributeArrayData,
        "attributes",
        (void**) &(pAttributeConsumerService->attributes));
    BAIL_ON_ERROR(e);

    *ppAttributeConsumerService = pAttributeConsumerService;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeConsumerServiceDataDelete(pAttributeConsumerService);
    }

    return e;
}
