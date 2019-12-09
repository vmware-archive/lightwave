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
IdmSearchResultDataNew(
    IDM_SEARCH_RESULT_DATA** ppSearchResult,
    const IDM_USER_ARRAY_DATA* users,
    const IDM_GROUP_ARRAY_DATA* groups,
    const IDM_SOLUTION_USER_ARRAY_DATA* solutionUsers)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SEARCH_RESULT_DATA* pSearchResult = NULL;

    if (ppSearchResult == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SEARCH_RESULT_DATA), (void**) &pSearchResult);
    BAIL_ON_ERROR(e);

    if (users != NULL)
    {
        e = IdmUserArrayDataNew(&(pSearchResult->users), users->ppEntry, users->length);
        BAIL_ON_ERROR(e);
    }

    if (groups != NULL)
    {
        e = IdmGroupArrayDataNew(&(pSearchResult->groups), groups->ppEntry, groups->length);
        BAIL_ON_ERROR(e);
    }

    if (solutionUsers != NULL)
    {
        e = IdmSolutionUserArrayDataNew(&(pSearchResult->solutionUsers), solutionUsers->ppEntry, solutionUsers->length);
        BAIL_ON_ERROR(e);
    }

    *ppSearchResult = pSearchResult;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSearchResultDataDelete(pSearchResult);
    }

    return e;
}

void
IdmSearchResultDataDelete(
    IDM_SEARCH_RESULT_DATA* pSearchResult)
{
    if (pSearchResult != NULL)
    {
        IdmUserArrayDataDelete(pSearchResult->users);
        IdmGroupArrayDataDelete(pSearchResult->groups);
        IdmSolutionUserArrayDataDelete(pSearchResult->solutionUsers);
        SSOMemoryFree(pSearchResult, sizeof(IDM_SEARCH_RESULT_DATA));
    }
}

SSOERROR
IdmSearchResultDataToJson(
    const IDM_SEARCH_RESULT_DATA* pSearchResult,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSearchResult != NULL)
    {
        e = RestDataToJson(
            pSearchResult->users,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmUserArrayDataToJson,
            "users",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pSearchResult->groups,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmGroupArrayDataToJson,
            "groups",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pSearchResult->solutionUsers,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmSolutionUserArrayDataToJson,
            "solutionUsers",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToSearchResultData(
    PCSSO_JSON pJson,
    IDM_SEARCH_RESULT_DATA** ppSearchResult)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SEARCH_RESULT_DATA* pSearchResult = NULL;

    if (pJson == NULL || ppSearchResult == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SEARCH_RESULT_DATA), (void**) &pSearchResult);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToUserArrayData,
        "users",
        (void**) &(pSearchResult->users));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToGroupArrayData,
        "groups",
        (void**) &(pSearchResult->groups));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToSolutionUserArrayData,
        "solutionUsers",
        (void**) &(pSearchResult->solutionUsers));
    BAIL_ON_ERROR(e);

    *ppSearchResult = pSearchResult;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSearchResultDataDelete(pSearchResult);
    }

    return e;
}
