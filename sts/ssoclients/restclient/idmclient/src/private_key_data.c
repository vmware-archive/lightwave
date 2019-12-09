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
IdmPrivateKeyDataNew(
    IDM_PRIVATE_KEY_DATA** ppPrivateKey,
    PCSTRING encoded,
    PCSTRING algorithm)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PRIVATE_KEY_DATA* pPrivateKey = NULL;

    if (ppPrivateKey == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PRIVATE_KEY_DATA), (void**) &pPrivateKey);
    BAIL_ON_ERROR(e);

    if (encoded != NULL)
    {
        e = RestStringDataNew(&(pPrivateKey->encoded), encoded);
        BAIL_ON_ERROR(e);
    }

    if (algorithm != NULL)
    {
        e = RestStringDataNew(&(pPrivateKey->algorithm), algorithm);
        BAIL_ON_ERROR(e);
    }

    *ppPrivateKey = pPrivateKey;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPrivateKeyDataDelete(pPrivateKey);
    }

    return e;
}

void
IdmPrivateKeyDataDelete(
    IDM_PRIVATE_KEY_DATA* pPrivateKey)
{
    if (pPrivateKey != NULL)
    {
        RestStringDataDelete(pPrivateKey->encoded);
        RestStringDataDelete(pPrivateKey->algorithm);
        SSOMemoryFree(pPrivateKey, sizeof(IDM_PRIVATE_KEY_DATA));
    }
}

SSOERROR
IdmPrivateKeyDataToJson(
    const IDM_PRIVATE_KEY_DATA* pPrivateKey,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pPrivateKey != NULL)
    {
        e = RestDataToJson(pPrivateKey->encoded, REST_JSON_OBJECT_TYPE_STRING, NULL, "encoded", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pPrivateKey->algorithm, REST_JSON_OBJECT_TYPE_STRING, NULL, "algorithm", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToPrivateKeyData(
    PCSSO_JSON pJson,
    IDM_PRIVATE_KEY_DATA** ppPrivateKey)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PRIVATE_KEY_DATA* pPrivateKey = NULL;

    if (pJson == NULL || ppPrivateKey == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PRIVATE_KEY_DATA), (void**) &pPrivateKey);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "encoded", (void**) &(pPrivateKey->encoded));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "algorithm",
        (void**) &(pPrivateKey->algorithm));
    BAIL_ON_ERROR(e);

    *ppPrivateKey = pPrivateKey;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPrivateKeyDataDelete(pPrivateKey);
    }

    return e;
}
