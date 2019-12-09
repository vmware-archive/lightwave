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
IdmTenantCredentialsDataNew(
    IDM_TENANT_CREDENTIALS_DATA** ppTenantCredentials,
    const IDM_PRIVATE_KEY_DATA* privateKey,
    const REST_CERTIFICATE_ARRAY_DATA* certificates)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials = NULL;

    if (ppTenantCredentials == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TENANT_CREDENTIALS_DATA), (void**) &pTenantCredentials);
    BAIL_ON_ERROR(e);

    if (privateKey != NULL)
    {
        e = IdmPrivateKeyDataNew(&(pTenantCredentials->privateKey), privateKey->encoded, privateKey->algorithm);
        BAIL_ON_ERROR(e);
    }

    if (certificates != NULL)
    {
        e = RestCertificateArrayDataNew(
            &(pTenantCredentials->certificates),
            certificates->ppEntry,
            certificates->length);
        BAIL_ON_ERROR(e);
    }

    *ppTenantCredentials = pTenantCredentials;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantCredentialsDataDelete(pTenantCredentials);
    }

    return e;
}

void
IdmTenantCredentialsDataDelete(
    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials)
{
    if (pTenantCredentials != NULL)
    {
        IdmPrivateKeyDataDelete(pTenantCredentials->privateKey);
        RestCertificateArrayDataDelete(pTenantCredentials->certificates);
        SSOMemoryFree(pTenantCredentials, sizeof(IDM_TENANT_CREDENTIALS_DATA));
    }
}

SSOERROR
IdmTenantCredentialsDataToJson(
    const IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pTenantCredentials != NULL)
    {
        e = RestDataToJson(
            pTenantCredentials->privateKey,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmPrivateKeyDataToJson,
            "privateKey",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTenantCredentials->certificates,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) RestCertificateArrayDataToJson,
            "certificates",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToTenantCredentialsData(
    PCSSO_JSON pJson,
    IDM_TENANT_CREDENTIALS_DATA** ppTenantCredentials)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials = NULL;

    if (pJson == NULL || ppTenantCredentials == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TENANT_CREDENTIALS_DATA), (void**) &pTenantCredentials);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToPrivateKeyData,
        "privateKey",
        (void**) &(pTenantCredentials->privateKey));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) RestJsonToCertificateArrayData,
        "certificates",
        (void**) &(pTenantCredentials->certificates));
    BAIL_ON_ERROR(e);

    *ppTenantCredentials = pTenantCredentials;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantCredentialsDataDelete(pTenantCredentials);
    }

    return e;
}
