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
IdmExternalIdpArrayDataNew(
    IDM_EXTERNAL_IDP_ARRAY_DATA** ppExternalIdpArray,
    IDM_EXTERNAL_IDP_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArray = NULL;
    size_t i = 0;

    if (ppExternalIdpArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EXTERNAL_IDP_ARRAY_DATA), (void**) &pExternalIdpArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pExternalIdpArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_EXTERNAL_IDP_DATA*), (void**) &(pExternalIdpArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmExternalIdpDataNew(
                &(pExternalIdpArray->ppEntry[i]),
                ppEntry[i]->entityID,
                ppEntry[i]->alias,
                ppEntry[i]->nameIDFormats,
                ppEntry[i]->ssoServices,
                ppEntry[i]->sloServices,
                ppEntry[i]->signingCertificates,
                ppEntry[i]->subjectFormats,
                ppEntry[i]->tokenClaimGroups,
                ppEntry[i]->jitEnabled,
                ppEntry[i]->upnSuffix);
            BAIL_ON_ERROR(e);
        }
    }

    *ppExternalIdpArray = pExternalIdpArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpArrayDataDelete(pExternalIdpArray);
    }

    return e;
}

void
IdmExternalIdpArrayDataDelete(
    IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArray)
{
    if (pExternalIdpArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pExternalIdpArray->ppEntry,
            pExternalIdpArray->length,
            (GenericDestructorFunction) IdmExternalIdpDataDelete);
        SSOMemoryFree(pExternalIdpArray, sizeof(IDM_EXTERNAL_IDP_ARRAY_DATA));
    }
}

SSOERROR
IdmExternalIdpArrayDataToJson(
    const IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pExternalIdpArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pExternalIdpArray,
            (DataObjectToJsonFunc) IdmExternalIdpDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToExternalIdpArrayData(
    PCSSO_JSON pJson,
    IDM_EXTERNAL_IDP_ARRAY_DATA** ppExternalIdpArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArray = NULL;

    if (pJson == NULL || ppExternalIdpArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToExternalIdpData,
        (REST_GENERIC_ARRAY_DATA**) &pExternalIdpArray);
    BAIL_ON_ERROR(e);

    *ppExternalIdpArray = pExternalIdpArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpArrayDataDelete(pExternalIdpArray);
    }

    return e;
}
