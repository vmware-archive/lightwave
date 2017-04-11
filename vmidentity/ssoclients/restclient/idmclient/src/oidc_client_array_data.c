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
IdmOidcClientArrayDataNew(
    IDM_OIDC_CLIENT_ARRAY_DATA** ppOidcClientArray,
    IDM_OIDC_CLIENT_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArray = NULL;
    size_t i = 0;

    if (ppOidcClientArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_OIDC_CLIENT_ARRAY_DATA), (void**) &pOidcClientArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pOidcClientArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_OIDC_CLIENT_DATA*), (void**) &(pOidcClientArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmOidcClientDataNew(
                &(pOidcClientArray->ppEntry[i]),
                ppEntry[i]->clientId,
                ppEntry[i]->oidcclientMetadataDTO);
            BAIL_ON_ERROR(e);
        }
    }

    *ppOidcClientArray = pOidcClientArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientArrayDataDelete(pOidcClientArray);
    }

    return e;
}

void
IdmOidcClientArrayDataDelete(
    IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArray)
{
    if (pOidcClientArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pOidcClientArray->ppEntry,
            pOidcClientArray->length,
            (GenericDestructorFunction) IdmOidcClientDataDelete);
        SSOMemoryFree(pOidcClientArray, sizeof(IDM_OIDC_CLIENT_ARRAY_DATA));
    }
}

SSOERROR
IdmOidcClientArrayDataToJson(
    const IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pOidcClientArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pOidcClientArray,
            (DataObjectToJsonFunc) IdmOidcClientDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToOidcClientArrayData(
    PCSSO_JSON pJson,
    IDM_OIDC_CLIENT_ARRAY_DATA** ppOidcClientArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArray = NULL;

    if (pJson == NULL || ppOidcClientArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToOidcClientData,
        (REST_GENERIC_ARRAY_DATA**) &pOidcClientArray);
    BAIL_ON_ERROR(e);

    *ppOidcClientArray = pOidcClientArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientArrayDataDelete(pOidcClientArray);
    }

    return e;
}
