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
IdmSignatureAlgorithmArrayDataNew(
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA** ppSignatureAlgorithmArray,
    IDM_SIGNATURE_ALGORITHM_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithmArray = NULL;
    size_t i = 0;

    if (ppSignatureAlgorithmArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SIGNATURE_ALGORITHM_ARRAY_DATA), (void**) &pSignatureAlgorithmArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pSignatureAlgorithmArray->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_SIGNATURE_ALGORITHM_DATA*),
            (void**) &(pSignatureAlgorithmArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmSignatureAlgorithmDataNew(
                &(pSignatureAlgorithmArray->ppEntry[i]),
                ppEntry[i]->maxKeySize,
                ppEntry[i]->minKeySize,
                ppEntry[i]->priority);
            BAIL_ON_ERROR(e);
        }
    }

    *ppSignatureAlgorithmArray = pSignatureAlgorithmArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSignatureAlgorithmArrayDataDelete(pSignatureAlgorithmArray);
    }

    return e;
}

void
IdmSignatureAlgorithmArrayDataDelete(
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithmArray)
{
    if (pSignatureAlgorithmArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pSignatureAlgorithmArray->ppEntry,
            pSignatureAlgorithmArray->length,
            (GenericDestructorFunction) IdmSignatureAlgorithmDataDelete);
        SSOMemoryFree(pSignatureAlgorithmArray, sizeof(IDM_SIGNATURE_ALGORITHM_ARRAY_DATA));
    }
}

SSOERROR
IdmSignatureAlgorithmArrayDataToJson(
    const IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithmArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSignatureAlgorithmArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pSignatureAlgorithmArray,
            (DataObjectToJsonFunc) IdmSignatureAlgorithmDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToSignatureAlgorithmArrayData(
    PCSSO_JSON pJson,
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA** ppSignatureAlgorithmArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithmArray = NULL;

    if (pJson == NULL || ppSignatureAlgorithmArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToSignatureAlgorithmData,
        (REST_GENERIC_ARRAY_DATA**) &pSignatureAlgorithmArray);
    BAIL_ON_ERROR(e);

    *ppSignatureAlgorithmArray = pSignatureAlgorithmArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSignatureAlgorithmArrayDataDelete(pSignatureAlgorithmArray);
    }

    return e;
}
