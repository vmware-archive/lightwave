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
IdmCertificateChainArrayDataNew(
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA** ppCertificateChainArray,
    IDM_CERTIFICATE_CHAIN_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArray = NULL;
    size_t i = 0;

    if (ppCertificateChainArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_CERTIFICATE_CHAIN_ARRAY_DATA), (void**) &pCertificateChainArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pCertificateChainArray->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_CERTIFICATE_CHAIN_DATA*),
            (void**) &(pCertificateChainArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmCertificateChainDataNew(&(pCertificateChainArray->ppEntry[i]), ppEntry[i]->certificates);
            BAIL_ON_ERROR(e);
        }
    }

    *ppCertificateChainArray = pCertificateChainArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmCertificateChainArrayDataDelete(pCertificateChainArray);
    }

    return e;
}

void
IdmCertificateChainArrayDataDelete(
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArray)
{
    if (pCertificateChainArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pCertificateChainArray->ppEntry,
            pCertificateChainArray->length,
            (GenericDestructorFunction) IdmCertificateChainDataDelete);
        SSOMemoryFree(pCertificateChainArray, sizeof(IDM_CERTIFICATE_CHAIN_ARRAY_DATA));
    }
}

SSOERROR
IdmCertificateChainArrayDataToJson(
    const IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pCertificateChainArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pCertificateChainArray,
            (DataObjectToJsonFunc) IdmCertificateChainDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToCertificateChainArrayData(
    PCSSO_JSON pJson,
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA** ppCertificateChainArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_CERTIFICATE_CHAIN_ARRAY_DATA* pCertificateChainArray = NULL;

    if (pJson == NULL || ppCertificateChainArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToCertificateChainData,
        (REST_GENERIC_ARRAY_DATA**) &pCertificateChainArray);
    BAIL_ON_ERROR(e);

    *ppCertificateChainArray = pCertificateChainArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmCertificateChainArrayDataDelete(pCertificateChainArray);
    }

    return e;
}
