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
IdmProviderPolicyDataNew(
    IDM_PROVIDER_POLICY_DATA** ppProviderPolicy,
    PCSTRING defaultProvider,
    PCSTRING defaultProviderAlias,
    const bool* providerSelectionEnabled)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PROVIDER_POLICY_DATA* pProviderPolicy = NULL;

    if (ppProviderPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PROVIDER_POLICY_DATA), (void**) &pProviderPolicy);
    BAIL_ON_ERROR(e);

    if (defaultProvider != NULL)
    {
        e = RestStringDataNew(&(pProviderPolicy->defaultProvider), defaultProvider);
        BAIL_ON_ERROR(e);
    }

    if (defaultProviderAlias != NULL)
    {
        e = RestStringDataNew(&(pProviderPolicy->defaultProviderAlias), defaultProviderAlias);
        BAIL_ON_ERROR(e);
    }

    if (providerSelectionEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pProviderPolicy->providerSelectionEnabled), *providerSelectionEnabled);
        BAIL_ON_ERROR(e);
    }

    *ppProviderPolicy = pProviderPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmProviderPolicyDataDelete(pProviderPolicy);
    }

    return e;
}

void
IdmProviderPolicyDataDelete(
    IDM_PROVIDER_POLICY_DATA* pProviderPolicy)
{
    if (pProviderPolicy != NULL)
    {
        RestStringDataDelete(pProviderPolicy->defaultProvider);
        RestStringDataDelete(pProviderPolicy->defaultProviderAlias);
        RestBooleanDataDelete(pProviderPolicy->providerSelectionEnabled);
        SSOMemoryFree(pProviderPolicy, sizeof(IDM_PROVIDER_POLICY_DATA));
    }
}

SSOERROR
IdmProviderPolicyDataToJson(
    const IDM_PROVIDER_POLICY_DATA* pProviderPolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pProviderPolicy != NULL)
    {
        e = RestDataToJson(
            pProviderPolicy->defaultProvider,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "defaultProvider",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pProviderPolicy->defaultProviderAlias,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "defaultProviderAlias",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pProviderPolicy->providerSelectionEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "providerSelectionEnabled",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToProviderPolicyData(
    PCSSO_JSON pJson,
    IDM_PROVIDER_POLICY_DATA** ppProviderPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PROVIDER_POLICY_DATA* pProviderPolicy = NULL;

    if (pJson == NULL || ppProviderPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PROVIDER_POLICY_DATA), (void**) &pProviderPolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "defaultProvider",
        (void**) &(pProviderPolicy->defaultProvider));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "defaultProviderAlias",
        (void**) &(pProviderPolicy->defaultProviderAlias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "providerSelectionEnabled",
        (void**) &(pProviderPolicy->providerSelectionEnabled));
    BAIL_ON_ERROR(e);

    *ppProviderPolicy = pProviderPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmProviderPolicyDataDelete(pProviderPolicy);
    }

    return e;
}
