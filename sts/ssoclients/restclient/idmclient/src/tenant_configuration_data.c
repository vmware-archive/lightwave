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
IdmTenantConfigurationDataNew(
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfiguration,
    const IDM_PASSWORD_POLICY_DATA* passwordPolicy,
    const IDM_LOCKOUT_POLICY_DATA* lockoutPolicy,
    const IDM_TOKEN_POLICY_DATA* tokenPolicy,
    const IDM_PROVIDER_POLICY_DATA* providerPolicy,
    const IDM_BRAND_POLICY_DATA* brandPolicy,
    const IDM_AUTHENTICATION_POLICY_DATA* authenticationPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration = NULL;

    if (ppTenantConfiguration == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TENANT_CONFIGURATION_DATA), (void**) &pTenantConfiguration);
    BAIL_ON_ERROR(e);

    if (passwordPolicy != NULL)
    {
        e = IdmPasswordPolicyDataNew(
            &(pTenantConfiguration->passwordPolicy),
            passwordPolicy->description,
            passwordPolicy->maxIdenticalAdjacentCharacters,
            passwordPolicy->maxLength,
            passwordPolicy->minAlphabeticCount,
            passwordPolicy->minLength,
            passwordPolicy->minLowercaseCount,
            passwordPolicy->minNumericCount,
            passwordPolicy->minSpecialCharCount,
            passwordPolicy->minUppercaseCount,
            passwordPolicy->passwordLifetimeDays,
            passwordPolicy->prohibitedPreviousPasswordCount);
        BAIL_ON_ERROR(e);
    }

    if (lockoutPolicy != NULL)
    {
        e = IdmLockoutPolicyDataNew(
            &(pTenantConfiguration->lockoutPolicy),
            lockoutPolicy->description,
            lockoutPolicy->failedAttemptIntervalSec,
            lockoutPolicy->maxFailedAttempts,
            lockoutPolicy->autoUnlockIntervalSec);
        BAIL_ON_ERROR(e);
    }

    if (tokenPolicy != NULL)
    {
        e = IdmTokenPolicyDataNew(
            &(pTenantConfiguration->tokenPolicy),
            tokenPolicy->clockToleranceMillis,
            tokenPolicy->delegationCount,
            tokenPolicy->maxBearerTokenLifeTimeMillis,
            tokenPolicy->maxHOKTokenLifeTimeMillis,
            tokenPolicy->maxBearerRefreshTokenLifeTimeMillis,
            tokenPolicy->maxHOKRefreshTokenLifeTimeMillis,
            tokenPolicy->renewCount);
        BAIL_ON_ERROR(e);
    }

    if (providerPolicy != NULL)
    {
        e = IdmProviderPolicyDataNew(
            &(pTenantConfiguration->providerPolicy),
            providerPolicy->defaultProvider,
            providerPolicy->defaultProviderAlias,
            providerPolicy->providerSelectionEnabled);
        BAIL_ON_ERROR(e);
    }

    if (brandPolicy != NULL)
    {
        e = IdmBrandPolicyDataNew(
            &(pTenantConfiguration->brandPolicy),
            brandPolicy->name,
            brandPolicy->logonBannerTitle,
            brandPolicy->logonBannerContent,
            brandPolicy->logonBannerCheckboxEnabled,
            brandPolicy->logonBannerDisabled);
        BAIL_ON_ERROR(e);
    }

    if (authenticationPolicy != NULL)
    {
        e = IdmAuthenticationPolicyDataNew(
            &(pTenantConfiguration->authenticationPolicy),
            authenticationPolicy->passwordBasedAuthenticationEnabled,
            authenticationPolicy->windowsBasedAuthenticationEnabled,
            authenticationPolicy->certificateBasedAuthenticationEnabled,
            authenticationPolicy->clientCertificatePolicy);
        BAIL_ON_ERROR(e);
    }

    *ppTenantConfiguration = pTenantConfiguration;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantConfigurationDataDelete(pTenantConfiguration);
    }

    return e;
}

void
IdmTenantConfigurationDataDelete(
    IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration)
{
    if (pTenantConfiguration != NULL)
    {
        IdmPasswordPolicyDataDelete(pTenantConfiguration->passwordPolicy);
        IdmLockoutPolicyDataDelete(pTenantConfiguration->lockoutPolicy);
        IdmTokenPolicyDataDelete(pTenantConfiguration->tokenPolicy);
        IdmProviderPolicyDataDelete(pTenantConfiguration->providerPolicy);
        IdmBrandPolicyDataDelete(pTenantConfiguration->brandPolicy);
        IdmAuthenticationPolicyDataDelete(pTenantConfiguration->authenticationPolicy);
        SSOMemoryFree(pTenantConfiguration, sizeof(IDM_TENANT_CONFIGURATION_DATA));
    }
}

SSOERROR
IdmTenantConfigurationDataToJson(
    const IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pTenantConfiguration != NULL)
    {
        e = RestDataToJson(
            pTenantConfiguration->passwordPolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmPasswordPolicyDataToJson,
            "passwordPolicy",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenantConfiguration->lockoutPolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmLockoutPolicyDataToJson,
            "lockoutPolicy",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenantConfiguration->tokenPolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmTokenPolicyDataToJson,
            "tokenPolicy",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenantConfiguration->providerPolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmProviderPolicyDataToJson,
            "providerPolicy",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenantConfiguration->brandPolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmBrandPolicyDataToJson,
            "brandPolicy",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenantConfiguration->authenticationPolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmAuthenticationPolicyDataToJson,
            "authenticationPolicy",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToTenantConfigurationData(
    PCSSO_JSON pJson,
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfiguration)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration = NULL;

    if (pJson == NULL || ppTenantConfiguration == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TENANT_CONFIGURATION_DATA), (void**) &pTenantConfiguration);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToPasswordPolicyData,
        "passwordPolicy",
        (void**) &(pTenantConfiguration->passwordPolicy));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToLockoutPolicyData,
        "lockoutPolicy",
        (void**) &(pTenantConfiguration->lockoutPolicy));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToTokenPolicyData,
        "tokenPolicy",
        (void**) &(pTenantConfiguration->tokenPolicy));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToProviderPolicyData,
        "providerPolicy",
        (void**) &(pTenantConfiguration->providerPolicy));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToBrandPolicyData,
        "brandPolicy",
        (void**) &(pTenantConfiguration->brandPolicy));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToAuthenticationPolicyData,
        "authenticationPolicy",
        (void**) &(pTenantConfiguration->authenticationPolicy));
    BAIL_ON_ERROR(e);

    *ppTenantConfiguration = pTenantConfiguration;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantConfigurationDataDelete(pTenantConfiguration);
    }

    return e;
}
