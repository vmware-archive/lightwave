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
IdmTokenPolicyDataNew(
    IDM_TOKEN_POLICY_DATA** ppTokenPolicy,
    const SSO_LONG* clockToleranceMillis,
    const INTEGER* delegationCount,
    const SSO_LONG* maxBearerTokenLifeTimeMillis,
    const SSO_LONG* maxHOKTokenLifeTimeMillis,
    const SSO_LONG* maxBearerRefreshTokenLifeTimeMillis,
    const SSO_LONG* maxHOKRefreshTokenLifeTimeMillis,
    const INTEGER* renewCount)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TOKEN_POLICY_DATA* pTokenPolicy = NULL;

    if (ppTokenPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TOKEN_POLICY_DATA), (void**) &pTokenPolicy);
    BAIL_ON_ERROR(e);

    if (clockToleranceMillis != NULL)
    {
        e = RestLongDataNew(&(pTokenPolicy->clockToleranceMillis), *clockToleranceMillis);
        BAIL_ON_ERROR(e);
    }

    if (delegationCount != NULL)
    {
        e = RestIntegerDataNew(&(pTokenPolicy->delegationCount), *delegationCount);
        BAIL_ON_ERROR(e);
    }

    if (maxBearerTokenLifeTimeMillis != NULL)
    {
        e = RestLongDataNew(&(pTokenPolicy->maxBearerTokenLifeTimeMillis), *maxBearerTokenLifeTimeMillis);
        BAIL_ON_ERROR(e);
    }

    if (maxHOKTokenLifeTimeMillis != NULL)
    {
        e = RestLongDataNew(&(pTokenPolicy->maxHOKTokenLifeTimeMillis), *maxHOKTokenLifeTimeMillis);
        BAIL_ON_ERROR(e);
    }

    if (maxBearerRefreshTokenLifeTimeMillis != NULL)
    {
        e = RestLongDataNew(&(pTokenPolicy->maxBearerRefreshTokenLifeTimeMillis), *maxBearerRefreshTokenLifeTimeMillis);
        BAIL_ON_ERROR(e);
    }

    if (maxHOKRefreshTokenLifeTimeMillis != NULL)
    {
        e = RestLongDataNew(&(pTokenPolicy->maxHOKRefreshTokenLifeTimeMillis), *maxHOKRefreshTokenLifeTimeMillis);
        BAIL_ON_ERROR(e);
    }

    if (renewCount != NULL)
    {
        e = RestIntegerDataNew(&(pTokenPolicy->renewCount), *renewCount);
        BAIL_ON_ERROR(e);
    }

    *ppTokenPolicy = pTokenPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTokenPolicyDataDelete(pTokenPolicy);
    }

    return e;
}

void
IdmTokenPolicyDataDelete(
    IDM_TOKEN_POLICY_DATA* pTokenPolicy)
{
    if (pTokenPolicy != NULL)
    {
        RestLongDataDelete(pTokenPolicy->clockToleranceMillis);
        RestIntegerDataDelete(pTokenPolicy->delegationCount);
        RestLongDataDelete(pTokenPolicy->maxBearerTokenLifeTimeMillis);
        RestLongDataDelete(pTokenPolicy->maxHOKTokenLifeTimeMillis);
        RestLongDataDelete(pTokenPolicy->maxBearerRefreshTokenLifeTimeMillis);
        RestLongDataDelete(pTokenPolicy->maxHOKRefreshTokenLifeTimeMillis);
        RestIntegerDataDelete(pTokenPolicy->renewCount);
        SSOMemoryFree(pTokenPolicy, sizeof(IDM_TOKEN_POLICY_DATA));
    }
}

SSOERROR
IdmTokenPolicyDataToJson(
    const IDM_TOKEN_POLICY_DATA* pTokenPolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pTokenPolicy != NULL)
    {
        e = RestDataToJson(
            pTokenPolicy->clockToleranceMillis,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "clockToleranceMillis",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTokenPolicy->delegationCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "delegationCount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTokenPolicy->maxBearerTokenLifeTimeMillis,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "maxBearerTokenLifeTimeMillis",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTokenPolicy->maxHOKTokenLifeTimeMillis,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "maxHOKTokenLifeTimeMillis",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTokenPolicy->maxBearerRefreshTokenLifeTimeMillis,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "maxBearerRefreshTokenLifeTimeMillis",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTokenPolicy->maxHOKRefreshTokenLifeTimeMillis,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "maxHOKRefreshTokenLifeTimeMillis",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTokenPolicy->renewCount, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "renewCount", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToTokenPolicyData(
    PCSSO_JSON pJson,
    IDM_TOKEN_POLICY_DATA** ppTokenPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TOKEN_POLICY_DATA* pTokenPolicy = NULL;

    if (pJson == NULL || ppTokenPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TOKEN_POLICY_DATA), (void**) &pTokenPolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "clockToleranceMillis",
        (void**) &(pTokenPolicy->clockToleranceMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "delegationCount",
        (void**) &(pTokenPolicy->delegationCount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "maxBearerTokenLifeTimeMillis",
        (void**) &(pTokenPolicy->maxBearerTokenLifeTimeMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "maxHOKTokenLifeTimeMillis",
        (void**) &(pTokenPolicy->maxHOKTokenLifeTimeMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "maxBearerRefreshTokenLifeTimeMillis",
        (void**) &(pTokenPolicy->maxBearerRefreshTokenLifeTimeMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "maxHOKRefreshTokenLifeTimeMillis",
        (void**) &(pTokenPolicy->maxHOKRefreshTokenLifeTimeMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "renewCount",
        (void**) &(pTokenPolicy->renewCount));
    BAIL_ON_ERROR(e);

    *ppTokenPolicy = pTokenPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTokenPolicyDataDelete(pTokenPolicy);
    }

    return e;
}
