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
IdmAssertionConsumerServiceDataNew(
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppAssertionConsumerService,
    PCSTRING name,
    PCSTRING endpoint,
    PCSTRING binding,
    const INTEGER* index)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService = NULL;

    if (ppAssertionConsumerService == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ASSERTION_CONSUMER_SERVICE_DATA), (void**) &pAssertionConsumerService);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pAssertionConsumerService->name), name);
        BAIL_ON_ERROR(e);
    }

    if (endpoint != NULL)
    {
        e = RestStringDataNew(&(pAssertionConsumerService->endpoint), endpoint);
        BAIL_ON_ERROR(e);
    }

    if (binding != NULL)
    {
        e = RestStringDataNew(&(pAssertionConsumerService->binding), binding);
        BAIL_ON_ERROR(e);
    }

    if (index != NULL)
    {
        e = RestIntegerDataNew(&(pAssertionConsumerService->index), *index);
        BAIL_ON_ERROR(e);
    }

    *ppAssertionConsumerService = pAssertionConsumerService;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAssertionConsumerServiceDataDelete(pAssertionConsumerService);
    }

    return e;
}

void
IdmAssertionConsumerServiceDataDelete(
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService)
{
    if (pAssertionConsumerService != NULL)
    {
        RestStringDataDelete(pAssertionConsumerService->name);
        RestStringDataDelete(pAssertionConsumerService->endpoint);
        RestStringDataDelete(pAssertionConsumerService->binding);
        RestIntegerDataDelete(pAssertionConsumerService->index);
        SSOMemoryFree(pAssertionConsumerService, sizeof(IDM_ASSERTION_CONSUMER_SERVICE_DATA));
    }
}

SSOERROR
IdmAssertionConsumerServiceDataToJson(
    const IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAssertionConsumerService != NULL)
    {
        e = RestDataToJson(pAssertionConsumerService->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pAssertionConsumerService->endpoint,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "endpoint",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pAssertionConsumerService->binding,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "binding",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pAssertionConsumerService->index, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "index", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAssertionConsumerServiceData(
    PCSSO_JSON pJson,
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppAssertionConsumerService)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService = NULL;

    if (pJson == NULL || ppAssertionConsumerService == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ASSERTION_CONSUMER_SERVICE_DATA), (void**) &pAssertionConsumerService);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "name",
        (void**) &(pAssertionConsumerService->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "endpoint",
        (void**) &(pAssertionConsumerService->endpoint));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "binding",
        (void**) &(pAssertionConsumerService->binding));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "index",
        (void**) &(pAssertionConsumerService->index));
    BAIL_ON_ERROR(e);

    *ppAssertionConsumerService = pAssertionConsumerService;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAssertionConsumerServiceDataDelete(pAssertionConsumerService);
    }

    return e;
}
