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
IdmSolutionUserDataNew(
    IDM_SOLUTION_USER_DATA** ppSolutionUser,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING description,
    const IDM_PRINCIPAL_DATA* alias,
    const REST_CERTIFICATE_DATA* certificate,
    const bool* disabled,
    PCSTRING objectId)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SOLUTION_USER_DATA* pSolutionUser = NULL;

    if (ppSolutionUser == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SOLUTION_USER_DATA), (void**) &pSolutionUser);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pSolutionUser->name), name);
        BAIL_ON_ERROR(e);
    }

    if (domain != NULL)
    {
        e = RestStringDataNew(&(pSolutionUser->domain), domain);
        BAIL_ON_ERROR(e);
    }

    if (description != NULL)
    {
        e = RestStringDataNew(&(pSolutionUser->description), description);
        BAIL_ON_ERROR(e);
    }

    if (alias != NULL)
    {
        e = IdmPrincipalDataNew(&(pSolutionUser->alias), alias->name, alias->domain);
        BAIL_ON_ERROR(e);
    }

    if (certificate != NULL)
    {
        e = RestCertificateDataNew(&(pSolutionUser->certificate), certificate->encoded);
        BAIL_ON_ERROR(e);
    }

    if (disabled != NULL)
    {
        e = RestBooleanDataNew(&(pSolutionUser->disabled), *disabled);
        BAIL_ON_ERROR(e);
    }

    if (objectId != NULL)
    {
        e = RestStringDataNew(&(pSolutionUser->objectId), objectId);
        BAIL_ON_ERROR(e);
    }

    *ppSolutionUser = pSolutionUser;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSolutionUserDataDelete(pSolutionUser);
    }

    return e;
}

void
IdmSolutionUserDataDelete(
    IDM_SOLUTION_USER_DATA* pSolutionUser)
{
    if (pSolutionUser != NULL)
    {
        RestStringDataDelete(pSolutionUser->name);
        RestStringDataDelete(pSolutionUser->domain);
        RestStringDataDelete(pSolutionUser->description);
        IdmPrincipalDataDelete(pSolutionUser->alias);
        RestCertificateDataDelete(pSolutionUser->certificate);
        RestBooleanDataDelete(pSolutionUser->disabled);
        RestStringDataDelete(pSolutionUser->objectId);
        SSOMemoryFree(pSolutionUser, sizeof(IDM_SOLUTION_USER_DATA));
    }
}

SSOERROR
IdmSolutionUserDataToJson(
    const IDM_SOLUTION_USER_DATA* pSolutionUser,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSolutionUser != NULL)
    {
        e = RestDataToJson(pSolutionUser->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pSolutionUser->domain, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pSolutionUser->description, REST_JSON_OBJECT_TYPE_STRING, NULL, "description", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pSolutionUser->alias,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmPrincipalDataToJson,
            "alias",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pSolutionUser->certificate,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) RestCertificateDataToJson,
            "certificate",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pSolutionUser->disabled, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "disabled", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pSolutionUser->objectId, REST_JSON_OBJECT_TYPE_STRING, NULL, "objectId", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToSolutionUserData(
    PCSSO_JSON pJson,
    IDM_SOLUTION_USER_DATA** ppSolutionUser)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SOLUTION_USER_DATA* pSolutionUser = NULL;

    if (pJson == NULL || ppSolutionUser == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SOLUTION_USER_DATA), (void**) &pSolutionUser);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pSolutionUser->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", (void**) &(pSolutionUser->domain));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "description",
        (void**) &(pSolutionUser->description));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToPrincipalData,
        "alias",
        (void**) &(pSolutionUser->alias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) RestJsonToCertificateData,
        "certificate",
        (void**) &(pSolutionUser->certificate));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "disabled",
        (void**) &(pSolutionUser->disabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "objectId",
        (void**) &(pSolutionUser->objectId));
    BAIL_ON_ERROR(e);

    *ppSolutionUser = pSolutionUser;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSolutionUserDataDelete(pSolutionUser);
    }

    return e;
}
