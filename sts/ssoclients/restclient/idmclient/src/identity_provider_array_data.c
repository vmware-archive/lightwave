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
IdmIdentityProviderArrayDataNew(
    IDM_IDENTITY_PROVIDER_ARRAY_DATA** ppIdentityProviderArray,
    IDM_IDENTITY_PROVIDER_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArray = NULL;
    size_t i = 0;

    if (ppIdentityProviderArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_IDENTITY_PROVIDER_ARRAY_DATA), (void**) &pIdentityProviderArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pIdentityProviderArray->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_IDENTITY_PROVIDER_DATA*),
            (void**) &(pIdentityProviderArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmIdentityProviderDataNew(
                &(pIdentityProviderArray->ppEntry[i]),
                ppEntry[i]->domainType,
                ppEntry[i]->name,
                ppEntry[i]->alias,
                ppEntry[i]->type,
                ppEntry[i]->authenticationType,
                ppEntry[i]->friendlyName,
                ppEntry[i]->searchTimeOutInSeconds,
                ppEntry[i]->username,
                ppEntry[i]->password,
                ppEntry[i]->machineAccount,
                ppEntry[i]->servicePrincipalName,
                ppEntry[i]->userBaseDN,
                ppEntry[i]->groupBaseDN,
                ppEntry[i]->connectionStrings,
                ppEntry[i]->attributesMap,
                ppEntry[i]->upnSuffixes,
                ppEntry[i]->schema,
                ppEntry[i]->matchingRuleInChainEnabled,
                ppEntry[i]->baseDnForNestedGroupsEnabled,
                ppEntry[i]->directGroupsSearchEnabled,
                ppEntry[i]->siteAffinityEnabled,
                ppEntry[i]->certificates);
            BAIL_ON_ERROR(e);
        }
    }

    *ppIdentityProviderArray = pIdentityProviderArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderArrayDataDelete(pIdentityProviderArray);
    }

    return e;
}

void
IdmIdentityProviderArrayDataDelete(
    IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArray)
{
    if (pIdentityProviderArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pIdentityProviderArray->ppEntry,
            pIdentityProviderArray->length,
            (GenericDestructorFunction) IdmIdentityProviderDataDelete);
        SSOMemoryFree(pIdentityProviderArray, sizeof(IDM_IDENTITY_PROVIDER_ARRAY_DATA));
    }
}

SSOERROR
IdmIdentityProviderArrayDataToJson(
    const IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pIdentityProviderArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pIdentityProviderArray,
            (DataObjectToJsonFunc) IdmIdentityProviderDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToIdentityProviderArrayData(
    PCSSO_JSON pJson,
    IDM_IDENTITY_PROVIDER_ARRAY_DATA** ppIdentityProviderArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArray = NULL;

    if (pJson == NULL || ppIdentityProviderArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToIdentityProviderData,
        (REST_GENERIC_ARRAY_DATA**) &pIdentityProviderArray);
    BAIL_ON_ERROR(e);

    *ppIdentityProviderArray = pIdentityProviderArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderArrayDataDelete(pIdentityProviderArray);
    }

    return e;
}
