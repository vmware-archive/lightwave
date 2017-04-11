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

static PCSTRING REST_HTTP_METHOD_TYPE_ENUMS[] =
{
    "POST",
    "GET",
    "PUT",
    "DELETE"
};

SSOERROR
RestGetHttpFormattedDate(
    PSTRING* ppDate,
    PSTRING* ppDateHeader)
{
    SSOERROR e = SSOERROR_NONE;
    time_t now = time(NULL);
    struct tm tm = {0};

    size_t HTTP_FORMATTED_TIME_SIZE = 30;
    PSTRING pDate;
    PSTRING pDateHeader;
    PSSO_STRING_BUILDER sb = NULL;

    e = SSOMemoryAllocate(HTTP_FORMATTED_TIME_SIZE, (void**) &pDate);
    BAIL_ON_ERROR(e);

    now = time(0);
    tm = *gmtime(&now);
    strftime(pDate, HTTP_FORMATTED_TIME_SIZE, "%a, %d %b %Y %H:%M:%S %Z", &tm);

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, "Date: ");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, pDate);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pDateHeader);
    BAIL_ON_ERROR(e);

    *ppDate = pDate;
    *ppDateHeader = pDateHeader;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pDate);
        SSOStringFree(pDateHeader);
    }

    // cleanup
    SSOStringBuilderDelete(sb);

    return e;
}

SSOERROR
RestEncodeHex(
    const unsigned char data[],
    const size_t length,
    PSTRING* ppHex)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pHex = NULL;
    int i = 0;

    e = SSOMemoryAllocate(length * 2 + 1, (void**) &pHex);
    BAIL_ON_ERROR(e);

    for (i = 0; i < length; i++)
    {
        sprintf(&pHex[i * 2], "%02x", data[i]);
    }

    *ppHex = pHex;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pHex);
    }

    return e;
}

SSOERROR
RestBuildSigningBytes(
    REST_HTTP_METHOD_TYPE httpMethodType,
    PCSTRING md5Hex,
    PCSTRING mediaType,
    PCSTRING httpFormattedDate,
    PCSTRING resourceUri,
    unsigned char** ppData,
    size_t* pDataSize)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pData = NULL;

    PCSTRING newLine = "\n";

    PSSO_STRING_BUILDER sb = NULL;

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, REST_HTTP_METHOD_TYPE_ENUMS[httpMethodType]);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, newLine);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, md5Hex);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, newLine);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, mediaType);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, newLine);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, httpFormattedDate);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, newLine);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, resourceUri);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pData);
    BAIL_ON_ERROR(e);

    *ppData = (unsigned char*) pData;
    *pDataSize = strlen(pData);

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pData);
    }

    // cleanup
    SSOStringBuilderDelete(sb);

    return e;
}
