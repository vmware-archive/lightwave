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
IdmBrandPolicyDataNew(
    IDM_BRAND_POLICY_DATA** ppBrandPolicy,
    PCSTRING name,
    PCSTRING logonBannerTitle,
    PCSTRING logonBannerContent,
    const bool* logonBannerCheckboxEnabled,
    const bool* logonBannerDisabled)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_BRAND_POLICY_DATA* pBrandPolicy = NULL;

    if (ppBrandPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_BRAND_POLICY_DATA), (void**) &pBrandPolicy);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pBrandPolicy->name), name);
        BAIL_ON_ERROR(e);
    }

    if (logonBannerTitle != NULL)
    {
        e = RestStringDataNew(&(pBrandPolicy->logonBannerTitle), logonBannerTitle);
        BAIL_ON_ERROR(e);
    }

    if (logonBannerContent != NULL)
    {
        e = RestStringDataNew(&(pBrandPolicy->logonBannerContent), logonBannerContent);
        BAIL_ON_ERROR(e);
    }

    if (logonBannerCheckboxEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pBrandPolicy->logonBannerCheckboxEnabled), *logonBannerCheckboxEnabled);
        BAIL_ON_ERROR(e);
    }

    if (logonBannerDisabled != NULL)
    {
        e = RestBooleanDataNew(&(pBrandPolicy->logonBannerDisabled), *logonBannerDisabled);
        BAIL_ON_ERROR(e);
    }

    *ppBrandPolicy = pBrandPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmBrandPolicyDataDelete(pBrandPolicy);
    }

    return e;
}

void
IdmBrandPolicyDataDelete(
    IDM_BRAND_POLICY_DATA* pBrandPolicy)
{
    if (pBrandPolicy != NULL)
    {
        RestStringDataDelete(pBrandPolicy->name);
        RestStringDataDelete(pBrandPolicy->logonBannerTitle);
        RestStringDataDelete(pBrandPolicy->logonBannerContent);
        RestBooleanDataDelete(pBrandPolicy->logonBannerCheckboxEnabled);
        RestBooleanDataDelete(pBrandPolicy->logonBannerDisabled);
        SSOMemoryFree(pBrandPolicy, sizeof(IDM_BRAND_POLICY_DATA));
    }
}

SSOERROR
IdmBrandPolicyDataToJson(
    const IDM_BRAND_POLICY_DATA* pBrandPolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pBrandPolicy != NULL)
    {
        e = RestDataToJson(pBrandPolicy->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pBrandPolicy->logonBannerTitle,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "logonBannerTitle",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pBrandPolicy->logonBannerContent,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "logonBannerContent",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pBrandPolicy->logonBannerCheckboxEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "logonBannerCheckboxEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pBrandPolicy->logonBannerDisabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "logonBannerDisabled",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToBrandPolicyData(
    PCSSO_JSON pJson,
    IDM_BRAND_POLICY_DATA** ppBrandPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_BRAND_POLICY_DATA* pBrandPolicy = NULL;

    if (pJson == NULL || ppBrandPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_BRAND_POLICY_DATA), (void**) &pBrandPolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pBrandPolicy->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "logonBannerTitle",
        (void**) &(pBrandPolicy->logonBannerTitle));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "logonBannerContent",
        (void**) &(pBrandPolicy->logonBannerContent));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "logonBannerCheckboxEnabled",
        (void**) &(pBrandPolicy->logonBannerCheckboxEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "logonBannerDisabled",
        (void**) &(pBrandPolicy->logonBannerDisabled));
    BAIL_ON_ERROR(e);

    *ppBrandPolicy = pBrandPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmBrandPolicyDataDelete(pBrandPolicy);
    }

    return e;
}
