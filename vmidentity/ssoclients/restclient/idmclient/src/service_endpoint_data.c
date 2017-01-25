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
IdmServiceEndpointDataNew(
    IDM_SERVICE_ENDPOINT_DATA** ppServiceEndpoint,
    PCSTRING name,
    PCSTRING endpoint,
    PCSTRING binding)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVICE_ENDPOINT_DATA* pServiceEndpoint = NULL;

    if (ppServiceEndpoint == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SERVICE_ENDPOINT_DATA), (void**) &pServiceEndpoint);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pServiceEndpoint->name), name);
        BAIL_ON_ERROR(e);
    }

    if (endpoint != NULL)
    {
        e = RestStringDataNew(&(pServiceEndpoint->endpoint), endpoint);
        BAIL_ON_ERROR(e);
    }

    if (binding != NULL)
    {
        e = RestStringDataNew(&(pServiceEndpoint->binding), binding);
        BAIL_ON_ERROR(e);
    }

    *ppServiceEndpoint = pServiceEndpoint;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServiceEndpointDataDelete(pServiceEndpoint);
    }

    return e;
}

void
IdmServiceEndpointDataDelete(
    IDM_SERVICE_ENDPOINT_DATA* pServiceEndpoint)
{
    if (pServiceEndpoint != NULL)
    {
        RestStringDataDelete(pServiceEndpoint->name);
        RestStringDataDelete(pServiceEndpoint->endpoint);
        RestStringDataDelete(pServiceEndpoint->binding);
        SSOMemoryFree(pServiceEndpoint, sizeof(IDM_SERVICE_ENDPOINT_DATA));
    }
}

SSOERROR
IdmServiceEndpointDataToJson(
    const IDM_SERVICE_ENDPOINT_DATA* pServiceEndpoint,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pServiceEndpoint != NULL)
    {
        e = RestDataToJson(pServiceEndpoint->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pServiceEndpoint->endpoint, REST_JSON_OBJECT_TYPE_STRING, NULL, "endpoint", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pServiceEndpoint->binding, REST_JSON_OBJECT_TYPE_STRING, NULL, "binding", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToServiceEndpointData(
    PCSSO_JSON pJson,
    IDM_SERVICE_ENDPOINT_DATA** ppServiceEndpoint)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVICE_ENDPOINT_DATA* pServiceEndpoint = NULL;

    if (pJson == NULL || ppServiceEndpoint == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SERVICE_ENDPOINT_DATA), (void**) &pServiceEndpoint);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pServiceEndpoint->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "endpoint",
        (void**) &(pServiceEndpoint->endpoint));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "binding",
        (void**) &(pServiceEndpoint->binding));
    BAIL_ON_ERROR(e);

    *ppServiceEndpoint = pServiceEndpoint;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServiceEndpointDataDelete(pServiceEndpoint);
    }

    return e;
}
