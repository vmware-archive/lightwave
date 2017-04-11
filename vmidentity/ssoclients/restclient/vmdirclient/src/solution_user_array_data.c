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
VmdirSolutionUserArrayDataNew(
    VMDIR_SOLUTION_USER_ARRAY_DATA** ppSolutionUserArray,
    VMDIR_SOLUTION_USER_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray = NULL;
    size_t i = 0;

    if (ppSolutionUserArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_SOLUTION_USER_ARRAY_DATA), (void**) &pSolutionUserArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pSolutionUserArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(VMDIR_SOLUTION_USER_DATA*), (void**) &(pSolutionUserArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = VmdirSolutionUserDataNew(
                &(pSolutionUserArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->domain,
                ppEntry[i]->description,
                ppEntry[i]->alias,
                ppEntry[i]->certificate,
                ppEntry[i]->disabled,
                ppEntry[i]->objectId);
            BAIL_ON_ERROR(e);
        }
    }

    *ppSolutionUserArray = pSolutionUserArray;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirSolutionUserArrayDataDelete(pSolutionUserArray);
    }

    return e;
}

void
VmdirSolutionUserArrayDataDelete(
    VMDIR_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray)
{
    if (pSolutionUserArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pSolutionUserArray->ppEntry,
            pSolutionUserArray->length,
            (GenericDestructorFunction) VmdirSolutionUserDataDelete);
        SSOMemoryFree(pSolutionUserArray, sizeof(VMDIR_SOLUTION_USER_ARRAY_DATA));
    }
}

SSOERROR
VmdirSolutionUserArrayDataToJson(
    const VMDIR_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSolutionUserArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pSolutionUserArray,
            (DataObjectToJsonFunc) VmdirSolutionUserDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
VmdirJsonToSolutionUserArrayData(
    PCSSO_JSON pJson,
    VMDIR_SOLUTION_USER_ARRAY_DATA** ppSolutionUserArray)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray = NULL;

    if (pJson == NULL || ppSolutionUserArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) VmdirJsonToSolutionUserData,
        (REST_GENERIC_ARRAY_DATA**) &pSolutionUserArray);
    BAIL_ON_ERROR(e);

    *ppSolutionUserArray = pSolutionUserArray;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirSolutionUserArrayDataDelete(pSolutionUserArray);
    }

    return e;
}
