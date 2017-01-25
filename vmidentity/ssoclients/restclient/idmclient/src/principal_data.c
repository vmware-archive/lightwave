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
IdmPrincipalDataNew(
    IDM_PRINCIPAL_DATA** ppPrincipal,
    PCSTRING name,
    PCSTRING domain)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PRINCIPAL_DATA* pPrincipal = NULL;

    if (ppPrincipal == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PRINCIPAL_DATA), (void**) &pPrincipal);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pPrincipal->name), name);
        BAIL_ON_ERROR(e);
    }

    if (domain != NULL)
    {
        e = RestStringDataNew(&(pPrincipal->domain), domain);
        BAIL_ON_ERROR(e);
    }

    *ppPrincipal = pPrincipal;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPrincipalDataDelete(pPrincipal);
    }

    return e;
}

void
IdmPrincipalDataDelete(
    IDM_PRINCIPAL_DATA* pPrincipal)
{
    if (pPrincipal != NULL)
    {
        RestStringDataDelete(pPrincipal->name);
        RestStringDataDelete(pPrincipal->domain);
        SSOMemoryFree(pPrincipal, sizeof(IDM_PRINCIPAL_DATA));
    }
}

SSOERROR
IdmPrincipalDataToJson(
    const IDM_PRINCIPAL_DATA* pPrincipal,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pPrincipal != NULL)
    {
        e = RestDataToJson(pPrincipal->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pPrincipal->domain, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToPrincipalData(
    PCSSO_JSON pJson,
    IDM_PRINCIPAL_DATA** ppPrincipal)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_PRINCIPAL_DATA* pPrincipal = NULL;

    if (pJson == NULL || ppPrincipal == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_PRINCIPAL_DATA), (void**) &pPrincipal);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pPrincipal->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", (void**) &(pPrincipal->domain));
    BAIL_ON_ERROR(e);

    *ppPrincipal = pPrincipal;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmPrincipalDataDelete(pPrincipal);
    }

    return e;
}
