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
void
RestClientSSLLockCallback(
    int         mode,
    int         lockNum,
    const char *file,
    int         line)
{

    SSOClientSSLLock(
                  mode,
                  &(gpRestClientContext->pCurlInitCtx->pMutexBuffer[lockNum]));
}


/*
 * IMPORTANT: you must call this function at process startup while there is only a single thread running
 * This is a wrapper for curl_global_init, from its documentation:
 * This function is not thread safe.
 * You must not call it when any other thread in the program (i.e. a thread sharing the same memory) is running.
 * This doesn't just mean no other thread that is using libcurl.
 * Because curl_global_init calls functions of other libraries that are similarly thread unsafe,
 * it could conflict with any other thread that uses these other libraries.
 */
SSOERROR
RestClientGlobalInit()
{
    SSOERROR e = SSOERROR_NONE;

    if (!gpRestClientContext->bIsCurlInitialized)
    {
        e = SSOHttpClientGlobalInit(RestClientSSLLockCallback, 
                                    &gpRestClientContext->pCurlInitCtx);
        BAIL_ON_ERROR(e);

        gpRestClientContext->bIsCurlInitialized = 1;
    }

error:
    return e;
}

// this function is not thread safe. Call it right before process exit
void
RestClientGlobalCleanup()
{
    if (gpRestClientContext->bIsCurlInitialized)
    {
        SSOHttpClientGlobalCleanup(gpRestClientContext->pCurlInitCtx);
        gpRestClientContext->bIsCurlInitialized = 0;
    }
}

// make sure you call RestClientGlobalInit once per process before calling this
// tlsCAPath: NULL means skip tls validation, otherwise LIGHTWAVE_TLS_CA_PATH will work on lightwave client and server
SSOERROR
RestClientNew(
    PREST_CLIENT* ppClient,
    PCSTRING serverHost,
    bool highAvailabilityEnabled,
    size_t serverPort,
    REST_SCHEME_TYPE schemeType,
    PCSTRING tlsCAPath, // optional, see comment above
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

    if (tlsCAPath != NULL)
    {
        e = RestStringDataNew(&(pClient->tlsCAPath), tlsCAPath);
        BAIL_ON_ERROR(e);
    }

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
        RestStringDataDelete(pClient->tlsCAPath);
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
