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
IdmResourceServerDataNew(
    IDM_RESOURCE_SERVER_DATA** ppResourceServer,
    PCSTRING name,
    const IDM_STRING_ARRAY_DATA* groupFilter)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RESOURCE_SERVER_DATA* pResourceServer = NULL;

    if (ppResourceServer == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_RESOURCE_SERVER_DATA), (void**) &pResourceServer);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pResourceServer->name), name);
        BAIL_ON_ERROR(e);
    }

    if (groupFilter != NULL)
    {
        e = IdmStringArrayDataNew(&(pResourceServer->groupFilter), groupFilter->ppEntry, groupFilter->length);
        BAIL_ON_ERROR(e);
    }

    *ppResourceServer = pResourceServer;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerDataDelete(pResourceServer);
    }

    return e;
}

void
IdmResourceServerDataDelete(
    IDM_RESOURCE_SERVER_DATA* pResourceServer)
{
    if (pResourceServer != NULL)
    {
        RestStringDataDelete(pResourceServer->name);
        IdmStringArrayDataDelete(pResourceServer->groupFilter);
        SSOMemoryFree(pResourceServer, sizeof(IDM_RESOURCE_SERVER_DATA));
    }
}

SSOERROR
IdmResourceServerDataToJson(
    const IDM_RESOURCE_SERVER_DATA* pResourceServer,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pResourceServer != NULL)
    {
        e = RestDataToJson(pResourceServer->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pResourceServer->groupFilter,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "groupFilter",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToResourceServerData(
    PCSSO_JSON pJson,
    IDM_RESOURCE_SERVER_DATA** ppResourceServer)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RESOURCE_SERVER_DATA* pResourceServer = NULL;

    if (pJson == NULL || ppResourceServer == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_RESOURCE_SERVER_DATA), (void**) &pResourceServer);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pResourceServer->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "groupFilter",
        (void**) &(pResourceServer->groupFilter));
    BAIL_ON_ERROR(e);

    *ppResourceServer = pResourceServer;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerDataDelete(pResourceServer);
    }

    return e;
}
