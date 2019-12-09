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
SSOHttpClientResponseNew(
    PSSO_HTTP_CLIENT_RESPONSE* pp)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_HTTP_CLIENT_RESPONSE p = NULL;

    ASSERT_NOT_NULL(pp);

    e = SSOMemoryAllocate(sizeof(SSO_HTTP_CLIENT_RESPONSE), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocate("", &p->pszBuffer);
    BAIL_ON_ERROR(e);

    p->bufferSize = 1;

    *pp = p;

error:
    if (e != SSOERROR_NONE)
    {
        SSOHttpClientResponseDelete(p);
    }
    return e;
}

void
SSOHttpClientResponseDelete(
    PSSO_HTTP_CLIENT_RESPONSE p)
{
    if (p != NULL)
    {
        SSOStringFree(p->pszBuffer);
        SSOMemoryFree(p, sizeof(SSO_HTTP_CLIENT_RESPONSE));
    }
}

PCSTRING
SSOHttpClientResponseGetString(
    PCSSO_HTTP_CLIENT_RESPONSE p)
{
    ASSERT_NOT_NULL(p);
    return p->pszBuffer;
}

// this signature is defined by curl
size_t
SSOHttpClientResponseWriteCallback(
    PSTRING contents,
    size_t size,
    size_t nmemb,
    void* userp)
{
    size_t numBytesHandled = 0;
    size_t totalSize = size * nmemb;
    PSSO_HTTP_CLIENT_RESPONSE p = NULL;
    SSOERROR e = SSOERROR_NONE;

    if (NULL == contents || NULL == userp)
    {
        return 0;
    }

    p = (PSSO_HTTP_CLIENT_RESPONSE) userp;

    e = SSOMemoryReallocate(p->bufferSize + totalSize, (void**) &p->pszBuffer);
    if (SSOERROR_NONE == e)
    {
        SSOMemoryCopy(p->pszBuffer + p->bufferSize - 1, contents, totalSize);
        p->bufferSize += totalSize;
        p->pszBuffer[p->bufferSize - 1] = '\0';
        numBytesHandled = totalSize;
    }
    else
    {
        numBytesHandled = 0;
    }

    return numBytesHandled;
}
