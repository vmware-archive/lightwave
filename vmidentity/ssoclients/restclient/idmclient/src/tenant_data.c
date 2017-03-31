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
IdmTenantDataNew(
    IDM_TENANT_DATA** ppTenant,
    PCSTRING name,
    PCSTRING longName,
    PCSTRING key,
    PCSTRING guid,
    PCSTRING issuer,
    const IDM_TENANT_CREDENTIALS_DATA* credentials,
    PCSTRING username,
    PCSTRING password)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TENANT_DATA* pTenant = NULL;

    if (ppTenant == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TENANT_DATA), (void**) &pTenant);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pTenant->name), name);
        BAIL_ON_ERROR(e);
    }

    if (longName != NULL)
    {
        e = RestStringDataNew(&(pTenant->longName), longName);
        BAIL_ON_ERROR(e);
    }

    if (key != NULL)
    {
        e = RestStringDataNew(&(pTenant->key), key);
        BAIL_ON_ERROR(e);
    }

    if (guid != NULL)
    {
        e = RestStringDataNew(&(pTenant->guid), guid);
        BAIL_ON_ERROR(e);
    }

    if (issuer != NULL)
    {
        e = RestStringDataNew(&(pTenant->issuer), issuer);
        BAIL_ON_ERROR(e);
    }

    if (credentials != NULL)
    {
        e = IdmTenantCredentialsDataNew(&(pTenant->credentials), credentials->privateKey, credentials->certificates);
        BAIL_ON_ERROR(e);
    }

    if (username != NULL)
    {
        e = RestStringDataNew(&(pTenant->username), username);
        BAIL_ON_ERROR(e);
    }

    if (password != NULL)
    {
        e = RestStringDataNew(&(pTenant->password), password);
        BAIL_ON_ERROR(e);
    }

    *ppTenant = pTenant;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantDataDelete(pTenant);
    }

    return e;
}

void
IdmTenantDataDelete(
    IDM_TENANT_DATA* pTenant)
{
    if (pTenant != NULL)
    {
        RestStringDataDelete(pTenant->name);
        RestStringDataDelete(pTenant->longName);
        RestStringDataDelete(pTenant->key);
        RestStringDataDelete(pTenant->guid);
        RestStringDataDelete(pTenant->issuer);
        IdmTenantCredentialsDataDelete(pTenant->credentials);
        RestStringDataDelete(pTenant->username);
        RestStringDataDelete(pTenant->password);
        SSOMemoryFree(pTenant, sizeof(IDM_TENANT_DATA));
    }
}

SSOERROR
IdmTenantDataToJson(
    const IDM_TENANT_DATA* pTenant,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pTenant != NULL)
    {
        e = RestDataToJson(pTenant->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTenant->longName, REST_JSON_OBJECT_TYPE_STRING, NULL, "longName", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTenant->key, REST_JSON_OBJECT_TYPE_STRING, NULL, "key", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTenant->guid, REST_JSON_OBJECT_TYPE_STRING, NULL, "guid", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTenant->issuer, REST_JSON_OBJECT_TYPE_STRING, NULL, "issuer", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenant->credentials,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmTenantCredentialsDataToJson,
            "credentials",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTenant->username, REST_JSON_OBJECT_TYPE_STRING, NULL, "username", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTenant->password, REST_JSON_OBJECT_TYPE_STRING, NULL, "password", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToTenantData(
    PCSSO_JSON pJson,
    IDM_TENANT_DATA** ppTenant)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TENANT_DATA* pTenant = NULL;

    if (pJson == NULL || ppTenant == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TENANT_DATA), (void**) &pTenant);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pTenant->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "longName", (void**) &(pTenant->longName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "key", (void**) &(pTenant->key));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "guid", (void**) &(pTenant->guid));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "issuer", (void**) &(pTenant->issuer));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToTenantCredentialsData,
        "credentials",
        (void**) &(pTenant->credentials));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "username", (void**) &(pTenant->username));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "password", (void**) &(pTenant->password));
    BAIL_ON_ERROR(e);

    *ppTenant = pTenant;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantDataDelete(pTenant);
    }

    return e;
}
