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
IdmPasswordPolicyDataNew(
    IDM_PASSWORD_POLICY_DATA** ppPasswordPolicy,
    PCSTRING description,
    const INTEGER* maxIdenticalAdjacentCharacters,
    const INTEGER* maxLength,
    const INTEGER* minAlphabeticCount,
    const INTEGER* minLength,
    const INTEGER* minLowercaseCount,
    const INTEGER* minNumericCount,
    const INTEGER* minSpecialCharCount,
    const INTEGER* minUppercaseCount,
    const INTEGER* passwordLifetimeDays,
    const INTEGER* prohibitedPreviousPasswordCount)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PASSWORD_POLICY_DATA* pPasswordPolicy = NULL;

    if (ppPasswordPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PASSWORD_POLICY_DATA), (void**) &pPasswordPolicy);
    BAIL_ON_ERROR(e);

    if (description != NULL)
    {
        e = RestStringDataNew(&(pPasswordPolicy->description), description);
        BAIL_ON_ERROR(e);
    }

    if (maxIdenticalAdjacentCharacters != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->maxIdenticalAdjacentCharacters), *maxIdenticalAdjacentCharacters);
        BAIL_ON_ERROR(e);
    }

    if (maxLength != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->maxLength), *maxLength);
        BAIL_ON_ERROR(e);
    }

    if (minAlphabeticCount != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->minAlphabeticCount), *minAlphabeticCount);
        BAIL_ON_ERROR(e);
    }

    if (minLength != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->minLength), *minLength);
        BAIL_ON_ERROR(e);
    }

    if (minLowercaseCount != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->minLowercaseCount), *minLowercaseCount);
        BAIL_ON_ERROR(e);
    }

    if (minNumericCount != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->minNumericCount), *minNumericCount);
        BAIL_ON_ERROR(e);
    }

    if (minSpecialCharCount != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->minSpecialCharCount), *minSpecialCharCount);
        BAIL_ON_ERROR(e);
    }

    if (minUppercaseCount != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->minUppercaseCount), *minUppercaseCount);
        BAIL_ON_ERROR(e);
    }

    if (passwordLifetimeDays != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->passwordLifetimeDays), *passwordLifetimeDays);
        BAIL_ON_ERROR(e);
    }

    if (prohibitedPreviousPasswordCount != NULL)
    {
        e = RestIntegerDataNew(&(pPasswordPolicy->prohibitedPreviousPasswordCount), *prohibitedPreviousPasswordCount);
        BAIL_ON_ERROR(e);
    }

    *ppPasswordPolicy = pPasswordPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPasswordPolicyDataDelete(pPasswordPolicy);
    }

    return e;
}

void
IdmPasswordPolicyDataDelete(
    IDM_PASSWORD_POLICY_DATA* pPasswordPolicy)
{
    if (pPasswordPolicy != NULL)
    {
        RestStringDataDelete(pPasswordPolicy->description);
        RestIntegerDataDelete(pPasswordPolicy->maxIdenticalAdjacentCharacters);
        RestIntegerDataDelete(pPasswordPolicy->maxLength);
        RestIntegerDataDelete(pPasswordPolicy->minAlphabeticCount);
        RestIntegerDataDelete(pPasswordPolicy->minLength);
        RestIntegerDataDelete(pPasswordPolicy->minLowercaseCount);
        RestIntegerDataDelete(pPasswordPolicy->minNumericCount);
        RestIntegerDataDelete(pPasswordPolicy->minSpecialCharCount);
        RestIntegerDataDelete(pPasswordPolicy->minUppercaseCount);
        RestIntegerDataDelete(pPasswordPolicy->passwordLifetimeDays);
        RestIntegerDataDelete(pPasswordPolicy->prohibitedPreviousPasswordCount);
        SSOMemoryFree(pPasswordPolicy, sizeof(IDM_PASSWORD_POLICY_DATA));
    }
}

SSOERROR
IdmPasswordPolicyDataToJson(
    const IDM_PASSWORD_POLICY_DATA* pPasswordPolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pPasswordPolicy != NULL)
    {
        e = RestDataToJson(
            pPasswordPolicy->description,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "description",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->maxIdenticalAdjacentCharacters,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "maxIdenticalAdjacentCharacters",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pPasswordPolicy->maxLength, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "maxLength", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->minAlphabeticCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "minAlphabeticCount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pPasswordPolicy->minLength, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "minLength", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->minLowercaseCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "minLowercaseCount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->minNumericCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "minNumericCount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->minSpecialCharCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "minSpecialCharCount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->minUppercaseCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "minUppercaseCount",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->passwordLifetimeDays,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "passwordLifetimeDays",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pPasswordPolicy->prohibitedPreviousPasswordCount,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "prohibitedPreviousPasswordCount",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToPasswordPolicyData(
    PCSSO_JSON pJson,
    IDM_PASSWORD_POLICY_DATA** ppPasswordPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PASSWORD_POLICY_DATA* pPasswordPolicy = NULL;

    if (pJson == NULL || ppPasswordPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PASSWORD_POLICY_DATA), (void**) &pPasswordPolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "description",
        (void**) &(pPasswordPolicy->description));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "maxIdenticalAdjacentCharacters",
        (void**) &(pPasswordPolicy->maxIdenticalAdjacentCharacters));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "maxLength",
        (void**) &(pPasswordPolicy->maxLength));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minAlphabeticCount",
        (void**) &(pPasswordPolicy->minAlphabeticCount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minLength",
        (void**) &(pPasswordPolicy->minLength));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minLowercaseCount",
        (void**) &(pPasswordPolicy->minLowercaseCount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minNumericCount",
        (void**) &(pPasswordPolicy->minNumericCount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minSpecialCharCount",
        (void**) &(pPasswordPolicy->minSpecialCharCount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minUppercaseCount",
        (void**) &(pPasswordPolicy->minUppercaseCount));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "passwordLifetimeDays",
        (void**) &(pPasswordPolicy->passwordLifetimeDays));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "prohibitedPreviousPasswordCount",
        (void**) &(pPasswordPolicy->prohibitedPreviousPasswordCount));
    BAIL_ON_ERROR(e);

    *ppPasswordPolicy = pPasswordPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPasswordPolicyDataDelete(pPasswordPolicy);
    }

    return e;
}
