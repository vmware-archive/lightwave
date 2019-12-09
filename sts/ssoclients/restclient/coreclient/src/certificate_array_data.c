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
RestCertificateArrayDataNew(
    REST_CERTIFICATE_ARRAY_DATA** ppCertificateArray,
    REST_CERTIFICATE_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CERTIFICATE_ARRAY_DATA* pCertificateArray = NULL;
    size_t i = 0;

    if (ppCertificateArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_CERTIFICATE_ARRAY_DATA), (void**) &pCertificateArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pCertificateArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(REST_CERTIFICATE_DATA*), (void**) &(pCertificateArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = RestCertificateDataNew(&(pCertificateArray->ppEntry[i]), ppEntry[i]->encoded);
            BAIL_ON_ERROR(e);
        }
    }

    *ppCertificateArray = pCertificateArray;

    error:

    if (e != SSOERROR_NONE)
    {
        RestCertificateArrayDataDelete(pCertificateArray);
    }

    return e;
}

void
RestCertificateArrayDataDelete(
    REST_CERTIFICATE_ARRAY_DATA* pCertificateArray)
{
    if (pCertificateArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pCertificateArray->ppEntry,
            pCertificateArray->length,
            (GenericDestructorFunction) RestCertificateDataDelete);
        SSOMemoryFree(pCertificateArray, sizeof(REST_CERTIFICATE_ARRAY_DATA));
    }
}

SSOERROR
RestCertificateArrayDataToJson(
    const REST_CERTIFICATE_ARRAY_DATA* pCertificateArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pCertificateArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pCertificateArray,
            (DataObjectToJsonFunc) RestCertificateDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
RestJsonToCertificateArrayData(
    PCSSO_JSON pJson,
    REST_CERTIFICATE_ARRAY_DATA** ppCertificateArray)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CERTIFICATE_ARRAY_DATA* pCertificateArray = NULL;

    if (pJson == NULL || ppCertificateArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) RestJsonToCertificateData,
        (REST_GENERIC_ARRAY_DATA**) &pCertificateArray);
    BAIL_ON_ERROR(e);

    *ppCertificateArray = pCertificateArray;

    error:

    if (e != SSOERROR_NONE)
    {
        RestCertificateArrayDataDelete(pCertificateArray);
    }

    return e;
}
