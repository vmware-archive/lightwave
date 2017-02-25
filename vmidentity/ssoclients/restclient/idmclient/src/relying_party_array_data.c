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
IdmRelyingPartyArrayDataNew(
    IDM_RELYING_PARTY_ARRAY_DATA** ppRelyingPartyArray,
    IDM_RELYING_PARTY_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArray = NULL;
    size_t i = 0;

    if (ppRelyingPartyArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_RELYING_PARTY_ARRAY_DATA), (void**) &pRelyingPartyArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pRelyingPartyArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_RELYING_PARTY_DATA*), (void**) &(pRelyingPartyArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmRelyingPartyDataNew(
                &(pRelyingPartyArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->url,
                ppEntry[i]->signatureAlgorithms,
                ppEntry[i]->assertionConsumerServices,
                ppEntry[i]->attributeConsumerServices,
                ppEntry[i]->singleLogoutServices,
                ppEntry[i]->certificate,
                ppEntry[i]->defaultAssertionConsumerService,
                ppEntry[i]->defaultAttributeConsumerService,
                ppEntry[i]->authnRequestsSigned);
            BAIL_ON_ERROR(e);
        }
    }

    *ppRelyingPartyArray = pRelyingPartyArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyArrayDataDelete(pRelyingPartyArray);
    }

    return e;
}

void
IdmRelyingPartyArrayDataDelete(
    IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArray)
{
    if (pRelyingPartyArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pRelyingPartyArray->ppEntry,
            pRelyingPartyArray->length,
            (GenericDestructorFunction) IdmRelyingPartyDataDelete);
        SSOMemoryFree(pRelyingPartyArray, sizeof(IDM_RELYING_PARTY_ARRAY_DATA));
    }
}

SSOERROR
IdmRelyingPartyArrayDataToJson(
    const IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pRelyingPartyArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pRelyingPartyArray,
            (DataObjectToJsonFunc) IdmRelyingPartyDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToRelyingPartyArrayData(
    PCSSO_JSON pJson,
    IDM_RELYING_PARTY_ARRAY_DATA** ppRelyingPartyArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArray = NULL;

    if (pJson == NULL || ppRelyingPartyArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToRelyingPartyData,
        (REST_GENERIC_ARRAY_DATA**) &pRelyingPartyArray);
    BAIL_ON_ERROR(e);

    *ppRelyingPartyArray = pRelyingPartyArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyArrayDataDelete(pRelyingPartyArray);
    }

    return e;
}
