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
RestParseHttpResponse(
    PCSTRING httpResponse,
    long httpStatusCode,
    JsonToDataObjectFunc fJsonToDataObject,
    void** ppDataObjectReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    void* pDataObjectReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;
    bool isObject = false;

    if (ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (httpStatusCode == 204)
    {
        // do nothing
    }
    else
    {
        PSSO_JSON pJson = NULL;

        e = SSOJsonParse(&pJson, httpResponse);
        BAIL_ON_ERROR(e);

        if (httpStatusCode == 200)
        {
            e = fJsonToDataObject(pJson, &pDataObjectReturn);
            BAIL_ON_ERROR(e);

            *ppDataObjectReturn = pDataObjectReturn;

            // debug
            if (DEBUG)
            {
                fprintf(stdout, "%s\n", "raw http response:");
                fprintf(stdout, "%s\n\n", httpResponse);

                e = SSOJsonObjectNew(&pJsonFromHttpResponse);
                BAIL_ON_ERROR(e);

                e = SSOJsonReset(pJsonFromHttpResponse, pJson);
                BAIL_ON_ERROR(e);
            }
        }
        else
        {
            e = SSOMemoryAllocate(sizeof(REST_SERVER_ERROR), (void**) &pError);
            BAIL_ON_ERROR(e);

            e = SSOJsonIsObject(pJson, &isObject);
            BAIL_ON_ERROR(e);

            if (isObject)
            {
                e = RestJsonToServerError(pJson, &pError);
                BAIL_ON_ERROR(e);
            }
            else
            {
                // put HTML response in details
                pError->details = (PSTRING) httpResponse;
            }

            pError->httpStatusCode = httpStatusCode;

            // set server error code
            e = RestServerErrorGetSSOErrorCode(pError->error);
        }

        // cleanup
        SSOJsonDelete(pJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pDataObjectReturn, 0);
        *ppError = pError;
    }

    return e;
}

SSOERROR
RestBuildUPN(
    PCSTRING name,
    PCSTRING domain,
    PSTRING* ppUpn)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_STRING_BUILDER sb = NULL;
    PSTRING pUpn = NULL;

    if (IS_NULL_OR_EMPTY_STRING(
        name) || IS_NULL_OR_EMPTY_STRING(domain) || ppUpn == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, name);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, "@");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, domain);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pUpn);
    BAIL_ON_ERROR(e);

    *ppUpn = pUpn;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pUpn);
    }

    // cleanup
    SSOStringBuilderDelete(sb);

    return e;
}

SSOERROR
RestBuildAndExecuteHttp(
    const void* pDataObject,
    DataObjectToJsonFunc fDataObjectToJson,
    const REST_ACCESS_TOKEN* pAccessToken,
    PCSTRING resourceUri,
    REST_HTTP_METHOD_TYPE httpMethodType,
    JsonToDataObjectFunc fJsonToDataObject,
    void** ppDataObjectReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJson = NULL;
    PSTRING httpFormattedDate = NULL;
    PSTRING httpFormattedDateHeader = NULL;
    PSTRING post = NULL;
    void* pDataObjectReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    // HTTP
    PSSO_HTTP_CLIENT pHttpClient = NULL;
    PSTRING httpResponse = NULL;
    long httpStatusCode = 0;

    if ((pDataObject != NULL && fDataObjectToJson == NULL) || IS_NULL_OR_EMPTY_STRING(resourceUri)
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pDataObject != NULL)
    {
        e = SSOJsonObjectNew(&pJson);
        BAIL_ON_ERROR(e);

        e = fDataObjectToJson(pDataObject, pJson);
        BAIL_ON_ERROR(e);
    }

    // get HTTP formatted time.
    e = RestGetHttpFormattedDate(&httpFormattedDate, &httpFormattedDateHeader);
    BAIL_ON_ERROR(e);

    e = RestBuildPostEntity(pAccessToken, httpMethodType, pJson, httpFormattedDate, resourceUri, &post);
    BAIL_ON_ERROR(e);

    e = SSOHttpClientNew(&pHttpClient);
    BAIL_ON_ERROR(e);

    if (httpMethodType == REST_HTTP_METHOD_TYPE_POST)
    {
        e = SSOHttpClientSendPostJson(
            pHttpClient,
            resourceUri,
            (PCSTRING*) &httpFormattedDateHeader,
            1,
            post,
            &httpResponse,
            &httpStatusCode);
        BAIL_ON_ERROR(e);
    }
    else if (httpMethodType == REST_HTTP_METHOD_TYPE_GET)
    {
        e = SSOHttpClientSendGet(
            pHttpClient,
            resourceUri,
            (PCSTRING*) &httpFormattedDateHeader,
            1,
            &httpResponse,
            &httpStatusCode);
        BAIL_ON_ERROR(e);
    }
    else if (httpMethodType == REST_HTTP_METHOD_TYPE_PUT)
    {
        e = SSOHttpClientSendPutJson(
            pHttpClient,
            resourceUri,
            (PCSTRING*) &httpFormattedDateHeader,
            1,
            post,
            &httpResponse,
            &httpStatusCode);
        BAIL_ON_ERROR(e);
    }
    else if (httpMethodType == REST_HTTP_METHOD_TYPE_DELETE)
    {
        e = SSOHttpClientSendDelete(
            pHttpClient,
            resourceUri,
            (PCSTRING*) &httpFormattedDateHeader,
            1,
            post,
            &httpResponse,
            &httpStatusCode);
        BAIL_ON_ERROR(e);
    }

    e = RestParseHttpResponse(httpResponse, httpStatusCode, fJsonToDataObject, &pDataObjectReturn, &pError);
    BAIL_ON_ERROR(e);

    if (pDataObjectReturn != NULL)
    {
        *ppDataObjectReturn = pDataObjectReturn;
    }

    error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pDataObjectReturn, 0);
        *ppError = pError;
    }

    // cleanup
    SSOHttpClientDelete(pHttpClient);
    SSOJsonDelete(pJson);
    SSOStringFree(httpFormattedDate);
    SSOStringFree(httpFormattedDateHeader);
    SSOStringFree(post);
    SSOStringFree(httpResponse);

    return e;
}
