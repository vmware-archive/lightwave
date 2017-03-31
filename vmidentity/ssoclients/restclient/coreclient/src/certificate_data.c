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
RestCertificateDataNew(
    REST_CERTIFICATE_DATA** ppCertificate,
    PCSTRING encoded)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CERTIFICATE_DATA* pCertificate = NULL;

    if (ppCertificate == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_CERTIFICATE_DATA), (void**) &pCertificate);
    BAIL_ON_ERROR(e);

    if (encoded != NULL)
    {
        e = RestStringDataNew(&(pCertificate->encoded), encoded);
        BAIL_ON_ERROR(e);
    }

    *ppCertificate = pCertificate;

    error:

    if (e != SSOERROR_NONE)
    {
        RestCertificateDataDelete(pCertificate);
    }

    return e;
}

void
RestCertificateDataDelete(
    REST_CERTIFICATE_DATA* pCertificate)
{
    if (pCertificate != NULL)
    {
        RestStringDataDelete(pCertificate->encoded);
        SSOMemoryFree(pCertificate, sizeof(REST_CERTIFICATE_DATA));
    }
}

SSOERROR
RestCertificateDataToJson(
    const REST_CERTIFICATE_DATA* pCertificate,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pCertificate != NULL)
    {
        e = RestDataToJson(pCertificate->encoded, REST_JSON_OBJECT_TYPE_STRING, NULL, "encoded", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
RestJsonToCertificateData(
    PCSSO_JSON pJson,
    REST_CERTIFICATE_DATA** ppCertificate)
{
    SSOERROR e = SSOERROR_NONE;
    REST_CERTIFICATE_DATA* pCertificate = NULL;

    if (pJson == NULL || ppCertificate == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_CERTIFICATE_DATA), (void**) &pCertificate);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "encoded", (void**) &(pCertificate->encoded));
    BAIL_ON_ERROR(e);

    *ppCertificate = pCertificate;

    error:

    if (e != SSOERROR_NONE)
    {
        RestCertificateDataDelete(pCertificate);
    }

    return e;
}
