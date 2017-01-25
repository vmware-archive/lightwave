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

static PCSTRING IDM_COMPUTER_TYPE_ENUMS[] =
{
    "DC",
    "COMPUTER",
    "ALL"
};

static PCSTRING const SERVER_POST_URI = "/idm/post/server";

SSOERROR
IdmServerGetComputers(
    PCREST_CLIENT pClient,
    IDM_COMPUTER_TYPE type,
    IDM_SERVER_DETAILS_ARRAY_DATA** ppServerDetailsArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || ppServerDetailsArrayReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        SERVER_POST_URI,
        "computers",
        NULL,
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("type", IDM_COMPUTER_TYPE_ENUMS[type], true, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToServerDetailsArrayData,
        (void**) &pServerDetailsArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppServerDetailsArrayReturn = pServerDetailsArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppServerDetailsArrayReturn, (DataObjectToJsonFunc) IdmServerDetailsArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServerDetailsArrayDataDelete(pServerDetailsArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}
