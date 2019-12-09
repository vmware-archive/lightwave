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
RestCredentialsDataNew(
    REST_CREDENTIALS_DATA** ppCredentials,
    PCSTRING username,
    PCSTRING password)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CREDENTIALS_DATA* pCredentials = NULL;

    if (ppCredentials == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_CREDENTIALS_DATA), (void**) &pCredentials);
    BAIL_ON_ERROR(e);

    if (username != NULL)
    {
        e = RestStringDataNew(&(pCredentials->username), username);
        BAIL_ON_ERROR(e);
    }

    if (password != NULL)
    {
        e = RestStringDataNew(&(pCredentials->password), password);
        BAIL_ON_ERROR(e);
    }

    *ppCredentials = pCredentials;

    error:

    if (e != SSOERROR_NONE)
    {
        RestCredentialsDataDelete(pCredentials);
    }

    return e;
}

void
RestCredentialsDataDelete(
    REST_CREDENTIALS_DATA* pCredentials)
{
    if (pCredentials != NULL)
    {
        RestStringDataDelete(pCredentials->username);
        RestStringDataDelete(pCredentials->password);
        SSOMemoryFree(pCredentials, sizeof(REST_CREDENTIALS_DATA));
    }
}

SSOERROR
RestCredentialsDataToJson(
    const REST_CREDENTIALS_DATA* pCredentials,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pCredentials != NULL)
    {
        e = RestDataToJson(pCredentials->username, REST_JSON_OBJECT_TYPE_STRING, NULL, "username", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pCredentials->password, REST_JSON_OBJECT_TYPE_STRING, NULL, "password", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
RestJsonToCredentialsData(
    PCSSO_JSON pJson,
    REST_CREDENTIALS_DATA** ppCredentials)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CREDENTIALS_DATA* pCredentials = NULL;

    if (pJson == NULL || ppCredentials == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_CREDENTIALS_DATA), (void**) &pCredentials);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "username", (void**) &(pCredentials->username));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "password", (void**) &(pCredentials->password));
    BAIL_ON_ERROR(e);

    *ppCredentials = pCredentials;

    error:

    if (e != SSOERROR_NONE)
    {
        RestCredentialsDataDelete(pCredentials);
    }

    return e;
}
