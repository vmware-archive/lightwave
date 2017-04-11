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
IdmSignatureAlgorithmDataNew(
    IDM_SIGNATURE_ALGORITHM_DATA** ppSignatureAlgorithm,
    const INTEGER* maxKeySize,
    const INTEGER* minKeySize,
    const INTEGER* priority)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm = NULL;

    if (ppSignatureAlgorithm == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SIGNATURE_ALGORITHM_DATA), (void**) &pSignatureAlgorithm);
    BAIL_ON_ERROR(e);

    if (maxKeySize != NULL)
    {
        e = RestIntegerDataNew(&(pSignatureAlgorithm->maxKeySize), *maxKeySize);
        BAIL_ON_ERROR(e);
    }

    if (minKeySize != NULL)
    {
        e = RestIntegerDataNew(&(pSignatureAlgorithm->minKeySize), *minKeySize);
        BAIL_ON_ERROR(e);
    }

    if (priority != NULL)
    {
        e = RestIntegerDataNew(&(pSignatureAlgorithm->priority), *priority);
        BAIL_ON_ERROR(e);
    }

    *ppSignatureAlgorithm = pSignatureAlgorithm;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSignatureAlgorithmDataDelete(pSignatureAlgorithm);
    }

    return e;
}

void
IdmSignatureAlgorithmDataDelete(
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm)
{
    if (pSignatureAlgorithm != NULL)
    {
        RestIntegerDataDelete(pSignatureAlgorithm->maxKeySize);
        RestIntegerDataDelete(pSignatureAlgorithm->minKeySize);
        RestIntegerDataDelete(pSignatureAlgorithm->priority);
        SSOMemoryFree(pSignatureAlgorithm, sizeof(IDM_SIGNATURE_ALGORITHM_DATA));
    }
}

SSOERROR
IdmSignatureAlgorithmDataToJson(
    const IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSignatureAlgorithm != NULL)
    {
        e = RestDataToJson(
            pSignatureAlgorithm->maxKeySize,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "maxKeySize",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pSignatureAlgorithm->minKeySize,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "minKeySize",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pSignatureAlgorithm->priority, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "priority", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToSignatureAlgorithmData(
    PCSSO_JSON pJson,
    IDM_SIGNATURE_ALGORITHM_DATA** ppSignatureAlgorithm)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm = NULL;

    if (pJson == NULL || ppSignatureAlgorithm == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SIGNATURE_ALGORITHM_DATA), (void**) &pSignatureAlgorithm);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "maxKeySize",
        (void**) &(pSignatureAlgorithm->maxKeySize));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "minKeySize",
        (void**) &(pSignatureAlgorithm->minKeySize));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "priority",
        (void**) &(pSignatureAlgorithm->priority));
    BAIL_ON_ERROR(e);

    *ppSignatureAlgorithm = pSignatureAlgorithm;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSignatureAlgorithmDataDelete(pSignatureAlgorithm);
    }

    return e;
}
