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

static
SSOERROR
SSOHttpClientSetCurlOptionInt(
    PCSSO_HTTP_CLIENT p,
    CURLoption option,
    int intValue)
{
    SSOERROR e = SSOERROR_NONE;

    CURLcode code = curl_easy_setopt(p->pCurl, option, intValue);
    if (code != CURLE_OK)
    {
        e = SSOERROR_CURL_FAILURE;
        BAIL_ON_ERROR(e);
    }

error:
    return e;
}

static
SSOERROR
SSOHttpClientSetCurlOptionPointer(
    PCSSO_HTTP_CLIENT p,
    CURLoption option,
    const void* pValue)
{
    SSOERROR e = SSOERROR_NONE;

    CURLcode code = curl_easy_setopt(p->pCurl, option, pValue);
    if (code != CURLE_OK)
    {
        e = SSOERROR_CURL_FAILURE;
        BAIL_ON_ERROR(e);
    }

error:
    return e;
}

static
SSOERROR
SSOHttpClientBuildForm(
    PCSSO_HTTP_CLIENT p,
    PSSO_KEY_VALUE_PAIR* ppPairs,
    size_t numPairs,
    PSTRING* ppszOut /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING pszOut = NULL;
    PSTRING pszUrlEncodedValue = NULL;
    size_t i = 0;

    PSSO_STRING_BUILDER pStringBuilder = NULL;
    e = SSOStringBuilderNew(&pStringBuilder);
    BAIL_ON_ERROR(e);

    for (i = 0; i < numPairs; i++)
    {
        if (i > 0)
        {
            e = SSOStringBuilderAppend(pStringBuilder, "&");
            BAIL_ON_ERROR(e);
        }

        e = SSOStringBuilderAppend(pStringBuilder, SSOKeyValuePairGetKey(ppPairs[i]));
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(pStringBuilder, "=");
        BAIL_ON_ERROR(e);

        pszUrlEncodedValue = curl_easy_escape(p->pCurl, SSOKeyValuePairGetValue(ppPairs[i]), 0);
        if (NULL == pszUrlEncodedValue)
        {
            e = SSOERROR_CURL_FAILURE;
            BAIL_ON_ERROR(e);
        }
        e = SSOStringBuilderAppend(pStringBuilder, pszUrlEncodedValue);
        curl_free(pszUrlEncodedValue);
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderGetString(pStringBuilder, &pszOut);
    BAIL_ON_ERROR(e);

    *ppszOut = pszOut;

error:
    SSOStringBuilderDelete(pStringBuilder);
    return e;
}

SSOERROR
SSOHttpClientGlobalInit()
{
    SSOERROR e = SSOERROR_NONE;

    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK)
    {
        e = SSOERROR_CURL_INIT_FAILURE;
        BAIL_ON_ERROR(e);
    }

error:
    return e;
}

void
SSOHttpClientGlobalCleanup()
{
    curl_global_cleanup();
}

SSOERROR
SSOHttpClientNew(
    PSSO_HTTP_CLIENT* pp)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_HTTP_CLIENT p = NULL;

    ASSERT_NOT_NULL(pp);

    e = SSOMemoryAllocate(sizeof(SSO_HTTP_CLIENT), (void**) &p);
    BAIL_ON_ERROR(e);

    p->pCurl = curl_easy_init();
    if (NULL == p->pCurl)
    {
        e = SSOERROR_CURL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *pp = p;

error:
    if (e != SSOERROR_NONE)
    {
        SSOHttpClientDelete(p);
    }
    return e;
}

void
SSOHttpClientDelete(
    PSSO_HTTP_CLIENT p)
{
    if (p != NULL)
    {
        if (p->pCurl != NULL)
        {
            curl_easy_cleanup(p->pCurl);
        }
        SSOMemoryFree(p, sizeof(SSO_HTTP_CLIENT));
    }
}

static
SSOERROR
SSOHttpClientSend(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszBody,
    PCSTRING pszContentTypeHeader,
    PCSTRING pszHttpMethod,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING pszOutResponse = NULL;
    PCSTRING pszHttpClientResponse = NULL;
    PSSO_HTTP_CLIENT_RESPONSE pHttpClientResponse = NULL;
    long statusCode = 0;
    size_t i = 0;
    struct curl_slist* pHeaderList = NULL;
    CURLcode code = CURLE_OK;

    if (pszContentTypeHeader != NULL)
    {
        pHeaderList = curl_slist_append(pHeaderList, pszContentTypeHeader);
        if (NULL == pHeaderList)
        {
            e = SSOERROR_CURL_FAILURE;
            BAIL_ON_ERROR(e);
        }
    }

    for (i = 0; i < headerCount; i++)
    {
        pHeaderList = curl_slist_append(pHeaderList, ppszHeaders[i]);
        if (NULL == pHeaderList)
        {
            e = SSOERROR_CURL_FAILURE;
            BAIL_ON_ERROR(e);
        }
    }

    e = SSOHttpClientResponseNew(&pHttpClientResponse);
    BAIL_ON_ERROR(e);

    e = SSOHttpClientSetCurlOptionPointer(p, CURLOPT_CUSTOMREQUEST, pszHttpMethod);
    BAIL_ON_ERROR(e);
    if (pszBody != NULL)
    {
        e = SSOHttpClientSetCurlOptionPointer(p, CURLOPT_POSTFIELDS, pszBody);
        BAIL_ON_ERROR(e);
    }
    e = SSOHttpClientSetCurlOptionPointer(p, CURLOPT_WRITEDATA, pHttpClientResponse);
    BAIL_ON_ERROR(e);
    e = SSOHttpClientSetCurlOptionPointer(p, CURLOPT_WRITEFUNCTION, SSOHttpClientResponseWriteCallback);
    BAIL_ON_ERROR(e);
    e = SSOHttpClientSetCurlOptionPointer(p, CURLOPT_URL, pszUrl);
    BAIL_ON_ERROR(e);
    e = SSOHttpClientSetCurlOptionPointer(p, CURLOPT_HTTPHEADER, pHeaderList);
    BAIL_ON_ERROR(e);
    e = SSOHttpClientSetCurlOptionInt(p, CURLOPT_SSL_VERIFYPEER, 0); // TODO: enable TLS verification
    BAIL_ON_ERROR(e);
    e = SSOHttpClientSetCurlOptionInt(p, CURLOPT_SSL_VERIFYHOST, 0);
    BAIL_ON_ERROR(e);

    code = curl_easy_perform(p->pCurl);
    if (code != CURLE_OK)
    {
        e = SSOERROR_HTTP_SEND_FAILURE;
        BAIL_ON_ERROR(e);
    }

    code = curl_easy_getinfo(p->pCurl, CURLINFO_RESPONSE_CODE, &statusCode);
    if (code != CURLE_OK)
    {
        e = SSOERROR_CURL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    pszHttpClientResponse = SSOHttpClientResponseGetString(pHttpClientResponse);
    e = SSOStringAllocate(pszHttpClientResponse, &pszOutResponse);
    BAIL_ON_ERROR(e);

    *ppszOutResponse = pszOutResponse;
    *pOutHttpStatusCode = statusCode;

error:
    SSOHttpClientResponseDelete(pHttpClientResponse);
    curl_slist_free_all(pHeaderList);
    return e;
}

SSOERROR
SSOHttpClientSendPostForm(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PSSO_KEY_VALUE_PAIR* ppPairs,
    size_t numPairs,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pszForm = NULL;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszUrl);
    ASSERT_NOT_NULL(ppPairs);
    ASSERT_NOT_NULL(ppszOutResponse);
    ASSERT_NOT_NULL(pOutHttpStatusCode);

    e = SSOHttpClientBuildForm(p, ppPairs, numPairs, &pszForm);
    BAIL_ON_ERROR(e);

    e = SSOHttpClientSend(
        p,
        pszUrl,
        ppszHeaders,
        headerCount,
        pszForm,
        "Content-Type: application/x-www-form-urlencoded; charset=utf-8",
        "POST",
        ppszOutResponse,
        pOutHttpStatusCode);
    BAIL_ON_ERROR(e);

error:
    SSOStringFree(pszForm);
    return e;
}

SSOERROR
SSOHttpClientSendPostJson(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszJsonBody,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszUrl);
    ASSERT_NOT_NULL(pszJsonBody);
    ASSERT_NOT_NULL(ppszOutResponse);
    ASSERT_NOT_NULL(pOutHttpStatusCode);

    e = SSOHttpClientSend(
        p,
        pszUrl,
        ppszHeaders,
        headerCount,
        pszJsonBody,
        "Content-Type: application/json; charset=utf-8",
        "POST",
        ppszOutResponse,
        pOutHttpStatusCode);
    BAIL_ON_ERROR(e);

error:
    return e;
}

SSOERROR
SSOHttpClientSendPutJson(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszJsonBody,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszUrl);
    ASSERT_NOT_NULL(pszJsonBody);
    ASSERT_NOT_NULL(ppszOutResponse);
    ASSERT_NOT_NULL(pOutHttpStatusCode);

    e = SSOHttpClientSend(
        p,
        pszUrl,
        ppszHeaders,
        headerCount,
        pszJsonBody,
        "Content-Type: application/json; charset=utf-8",
        "PUT",
        ppszOutResponse,
        pOutHttpStatusCode);
    BAIL_ON_ERROR(e);

error:
    return e;
}

SSOERROR
SSOHttpClientSendDelete(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszJsonBody,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszUrl);
    ASSERT_NOT_NULL(ppszOutResponse);
    ASSERT_NOT_NULL(pOutHttpStatusCode);

    e = SSOHttpClientSend(
        p,
        pszUrl,
        ppszHeaders,
        headerCount,
        pszJsonBody,
        "Content-Type: application/json; charset=utf-8",
        "DELETE",
        ppszOutResponse,
        pOutHttpStatusCode);
    BAIL_ON_ERROR(e);

error:
    return e;
}

SSOERROR
SSOHttpClientSendGet(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszUrl);
    ASSERT_NOT_NULL(ppszOutResponse);
    ASSERT_NOT_NULL(pOutHttpStatusCode);

    e = SSOHttpClientSend(
        p,
        pszUrl,
        ppszHeaders,
        headerCount,
        NULL,
        NULL,
        "GET",
        ppszOutResponse,
        pOutHttpStatusCode);
    BAIL_ON_ERROR(e);

error:
    return e;
}
