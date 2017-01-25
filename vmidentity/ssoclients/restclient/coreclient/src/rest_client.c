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
RestClientGlobalInit()
{
    return SSOHttpClientGlobalInit();
}

void
RestClientGlobalCleanup()
{
    SSOHttpClientGlobalCleanup();
}

SSOERROR
RestClientNew(
    PREST_CLIENT* ppClient,
    PCSTRING serverHost,
    bool highAvailabilityEnabled,
    size_t serverPort,
    REST_SCHEME_TYPE schemeType,
    PCREST_ACCESS_TOKEN pAccessToken)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CLIENT* pClient = NULL;

    if (ppClient == NULL
        || (IS_NULL_OR_EMPTY_STRING(serverHost) && highAvailabilityEnabled == false) /* || check scheme is enum */)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_CLIENT), (void**) &pClient);
    BAIL_ON_ERROR(e);

    pClient->highAvailabilityEnabled = highAvailabilityEnabled;

    if (highAvailabilityEnabled)
    {
        e = SSOCdcNew(&(pClient->pCdc));
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = RestStringDataNew(&(pClient->serverHost), serverHost);
        BAIL_ON_ERROR(e);
    }

    pClient->serverPort = serverPort;

    pClient->schemeType = schemeType;

    if (pAccessToken != NULL)
    {
        e = RestAccessTokenNew(
            &(pClient->pAccessToken),
            pAccessToken->value,
            pAccessToken->type,
            pAccessToken->privateKey);
        BAIL_ON_ERROR(e);
    }

    *ppClient = pClient;

    error:

    if (e != SSOERROR_NONE)
    {
        RestClientDelete(pClient);
    }

    return e;
}

void
RestClientDelete(
    PREST_CLIENT pClient)
{
    if (pClient != NULL)
    {
        RestStringDataDelete(pClient->serverHost);
        SSOCdcDelete(pClient->pCdc);
        RestAccessTokenDelete(pClient->pAccessToken);
        SSOMemoryFree(pClient, sizeof(REST_CLIENT));
    }
}

SSOERROR
RestClientSetAccessToken(
    PREST_CLIENT pClient,
    PCREST_ACCESS_TOKEN pAccessToken)
{
    SSOERROR e = SSOERROR_NONE;

    if (pClient == NULL || pAccessToken == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    RestAccessTokenDelete(pClient->pAccessToken);

    e = RestAccessTokenNew(&(pClient->pAccessToken), pAccessToken->value, pAccessToken->type, pAccessToken->privateKey);
    BAIL_ON_ERROR(e);

    error:

    return e;
}
