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
IdmAttributeDataNew(
    IDM_ATTRIBUTE_DATA** ppAttribute,
    PCSTRING name,
    PCSTRING friendlyName,
    PCSTRING nameFormat)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_DATA* pAttribute = NULL;

    if (ppAttribute == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ATTRIBUTE_DATA), (void**) &pAttribute);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pAttribute->name), name);
        BAIL_ON_ERROR(e);
    }

    if (friendlyName != NULL)
    {
        e = RestStringDataNew(&(pAttribute->friendlyName), friendlyName);
        BAIL_ON_ERROR(e);
    }

    if (nameFormat != NULL)
    {
        e = RestStringDataNew(&(pAttribute->nameFormat), nameFormat);
        BAIL_ON_ERROR(e);
    }

    *ppAttribute = pAttribute;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeDataDelete(pAttribute);
    }

    return e;
}

void
IdmAttributeDataDelete(
    IDM_ATTRIBUTE_DATA* pAttribute)
{
    if (pAttribute != NULL)
    {
        RestStringDataDelete(pAttribute->name);
        RestStringDataDelete(pAttribute->friendlyName);
        RestStringDataDelete(pAttribute->nameFormat);
        SSOMemoryFree(pAttribute, sizeof(IDM_ATTRIBUTE_DATA));
    }
}

SSOERROR
IdmAttributeDataToJson(
    const IDM_ATTRIBUTE_DATA* pAttribute,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAttribute != NULL)
    {
        e = RestDataToJson(pAttribute->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pAttribute->friendlyName, REST_JSON_OBJECT_TYPE_STRING, NULL, "friendlyName", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pAttribute->nameFormat, REST_JSON_OBJECT_TYPE_STRING, NULL, "nameFormat", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAttributeData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_DATA** ppAttribute)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_DATA* pAttribute = NULL;

    if (pJson == NULL || ppAttribute == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ATTRIBUTE_DATA), (void**) &pAttribute);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pAttribute->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "friendlyName",
        (void**) &(pAttribute->friendlyName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "nameFormat",
        (void**) &(pAttribute->nameFormat));
    BAIL_ON_ERROR(e);

    *ppAttribute = pAttribute;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeDataDelete(pAttribute);
    }

    return e;
}
