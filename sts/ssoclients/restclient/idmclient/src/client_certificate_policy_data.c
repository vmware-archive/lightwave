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
IdmClientCertificatePolicyDataNew(
    IDM_CLIENT_CERTIFICATE_POLICY_DATA** ppClientCertificatePolicy,
    const IDM_STRING_ARRAY_DATA* certPolicyOIDs,
    const REST_CERTIFICATE_ARRAY_DATA* trustedCACertificates,
    const bool* revocationCheckEnabled,
    const bool* ocspEnabled,
    const bool* failOverToCrlEnabled,
    PCSTRING ocspUrlOverride,
    const bool* crlDistributionPointUsageEnabled,
    PCSTRING crlDistributionPointOverride)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_CLIENT_CERTIFICATE_POLICY_DATA* pClientCertificatePolicy = NULL;

    if (ppClientCertificatePolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_CLIENT_CERTIFICATE_POLICY_DATA), (void**) &pClientCertificatePolicy);
    BAIL_ON_ERROR(e);

    if (certPolicyOIDs != NULL)
    {
        e = IdmStringArrayDataNew(
            &(pClientCertificatePolicy->certPolicyOIDs),
            certPolicyOIDs->ppEntry,
            certPolicyOIDs->length);
        BAIL_ON_ERROR(e);
    }

    if (trustedCACertificates != NULL)
    {
        e = RestCertificateArrayDataNew(
            &(pClientCertificatePolicy->trustedCACertificates),
            trustedCACertificates->ppEntry,
            trustedCACertificates->length);
        BAIL_ON_ERROR(e);
    }

    if (revocationCheckEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pClientCertificatePolicy->revocationCheckEnabled), *revocationCheckEnabled);
        BAIL_ON_ERROR(e);
    }

    if (ocspEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pClientCertificatePolicy->ocspEnabled), *ocspEnabled);
        BAIL_ON_ERROR(e);
    }

    if (failOverToCrlEnabled != NULL)
    {
        e = RestBooleanDataNew(&(pClientCertificatePolicy->failOverToCrlEnabled), *failOverToCrlEnabled);
        BAIL_ON_ERROR(e);
    }

    if (ocspUrlOverride != NULL)
    {
        e = RestStringDataNew(&(pClientCertificatePolicy->ocspUrlOverride), ocspUrlOverride);
        BAIL_ON_ERROR(e);
    }

    if (crlDistributionPointUsageEnabled != NULL)
    {
        e = RestBooleanDataNew(
            &(pClientCertificatePolicy->crlDistributionPointUsageEnabled),
            *crlDistributionPointUsageEnabled);
        BAIL_ON_ERROR(e);
    }

    if (crlDistributionPointOverride != NULL)
    {
        e = RestStringDataNew(&(pClientCertificatePolicy->crlDistributionPointOverride), crlDistributionPointOverride);
        BAIL_ON_ERROR(e);
    }

    *ppClientCertificatePolicy = pClientCertificatePolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmClientCertificatePolicyDataDelete(pClientCertificatePolicy);
    }

    return e;
}

void
IdmClientCertificatePolicyDataDelete(
    IDM_CLIENT_CERTIFICATE_POLICY_DATA* pClientCertificatePolicy)
{
    if (pClientCertificatePolicy != NULL)
    {
        IdmStringArrayDataDelete(pClientCertificatePolicy->certPolicyOIDs);
        RestCertificateArrayDataDelete(pClientCertificatePolicy->trustedCACertificates);
        RestBooleanDataDelete(pClientCertificatePolicy->revocationCheckEnabled);
        RestBooleanDataDelete(pClientCertificatePolicy->ocspEnabled);
        RestBooleanDataDelete(pClientCertificatePolicy->failOverToCrlEnabled);
        RestStringDataDelete(pClientCertificatePolicy->ocspUrlOverride);
        RestBooleanDataDelete(pClientCertificatePolicy->crlDistributionPointUsageEnabled);
        RestStringDataDelete(pClientCertificatePolicy->crlDistributionPointOverride);
        SSOMemoryFree(pClientCertificatePolicy, sizeof(IDM_CLIENT_CERTIFICATE_POLICY_DATA));
    }
}

SSOERROR
IdmClientCertificatePolicyDataToJson(
    const IDM_CLIENT_CERTIFICATE_POLICY_DATA* pClientCertificatePolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pClientCertificatePolicy != NULL)
    {
        e = RestDataToJson(
            pClientCertificatePolicy->certPolicyOIDs,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "certPolicyOIDs",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->trustedCACertificates,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) RestCertificateArrayDataToJson,
            "trustedCACertificates",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->revocationCheckEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "revocationCheckEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->ocspEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "ocspEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->failOverToCrlEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "failOverToCrlEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->ocspUrlOverride,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "ocspUrlOverride",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->crlDistributionPointUsageEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "crlDistributionPointUsageEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pClientCertificatePolicy->crlDistributionPointOverride,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "crlDistributionPointOverride",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToClientCertificatePolicyData(
    PCSSO_JSON pJson,
    IDM_CLIENT_CERTIFICATE_POLICY_DATA** ppClientCertificatePolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_CLIENT_CERTIFICATE_POLICY_DATA* pClientCertificatePolicy = NULL;

    if (pJson == NULL || ppClientCertificatePolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_CLIENT_CERTIFICATE_POLICY_DATA), (void**) &pClientCertificatePolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "certPolicyOIDs",
        (void**) &(pClientCertificatePolicy->certPolicyOIDs));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) RestJsonToCertificateArrayData,
        "trustedCACertificates",
        (void**) &(pClientCertificatePolicy->trustedCACertificates));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "revocationCheckEnabled",
        (void**) &(pClientCertificatePolicy->revocationCheckEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "ocspEnabled",
        (void**) &(pClientCertificatePolicy->ocspEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "failOverToCrlEnabled",
        (void**) &(pClientCertificatePolicy->failOverToCrlEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "ocspUrlOverride",
        (void**) &(pClientCertificatePolicy->ocspUrlOverride));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "crlDistributionPointUsageEnabled",
        (void**) &(pClientCertificatePolicy->crlDistributionPointUsageEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "crlDistributionPointOverride",
        (void**) &(pClientCertificatePolicy->crlDistributionPointOverride));
    BAIL_ON_ERROR(e);

    *ppClientCertificatePolicy = pClientCertificatePolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmClientCertificatePolicyDataDelete(pClientCertificatePolicy);
    }

    return e;
}
