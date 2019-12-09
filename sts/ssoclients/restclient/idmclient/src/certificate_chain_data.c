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
IdmCertificateChainDataNew(
    IDM_CERTIFICATE_CHAIN_DATA** ppCertificateChain,
    const REST_CERTIFICATE_ARRAY_DATA* certificates)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_CERTIFICATE_CHAIN_DATA* pCertificateChain = NULL;

    if (ppCertificateChain == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_CERTIFICATE_CHAIN_DATA), (void**) &pCertificateChain);
    BAIL_ON_ERROR(e);

    if (certificates != NULL)
    {
        e = RestCertificateArrayDataNew(&(pCertificateChain->certificates), certificates->ppEntry, certificates->length);
        BAIL_ON_ERROR(e);
    }

    *ppCertificateChain = pCertificateChain;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmCertificateChainDataDelete(pCertificateChain);
    }

    return e;
}

void
IdmCertificateChainDataDelete(
    IDM_CERTIFICATE_CHAIN_DATA* pCertificateChain)
{
    if (pCertificateChain != NULL)
    {
        RestCertificateArrayDataDelete(pCertificateChain->certificates);
        SSOMemoryFree(pCertificateChain, sizeof(IDM_CERTIFICATE_CHAIN_DATA));
    }
}

SSOERROR
IdmCertificateChainDataToJson(
    const IDM_CERTIFICATE_CHAIN_DATA* pCertificateChain,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pCertificateChain != NULL)
    {
        e = RestDataToJson(
            pCertificateChain->certificates,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) RestCertificateArrayDataToJson,
            "certificates",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToCertificateChainData(
    PCSSO_JSON pJson,
    IDM_CERTIFICATE_CHAIN_DATA** ppCertificateChain)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_CERTIFICATE_CHAIN_DATA* pCertificateChain = NULL;

    if (pJson == NULL || ppCertificateChain == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_CERTIFICATE_CHAIN_DATA), (void**) &pCertificateChain);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) RestJsonToCertificateArrayData,
        "certificates",
        (void**) &(pCertificateChain->certificates));
    BAIL_ON_ERROR(e);

    *ppCertificateChain = pCertificateChain;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmCertificateChainDataDelete(pCertificateChain);
    }

    return e;
}
