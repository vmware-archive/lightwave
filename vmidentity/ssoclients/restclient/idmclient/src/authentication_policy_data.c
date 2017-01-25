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
IdmAuthenticationPolicyDataNew(
    IDM_AUTHENTICATION_POLICY_DATA** ppAuthenticationPolicy,
    const bool* passwordBasedAuthenticationEnabled,
    const bool* windowsBasedAuthenticationEnabled,
    const bool* certificateBasedAuthenticationEnabled,
    const IDM_CLIENT_CERTIFICATE_POLICY_DATA* clientCertificatePolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_AUTHENTICATION_POLICY_DATA* pAuthenticationPolicy = NULL;

    if (ppAuthenticationPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_AUTHENTICATION_POLICY_DATA), (void**) &pAuthenticationPolicy);
    BAIL_ON_ERROR(e);

    if (passwordBasedAuthenticationEnabled != NULL)
    {
        e = RestBooleanDataNew(
            &(pAuthenticationPolicy->passwordBasedAuthenticationEnabled),
            *passwordBasedAuthenticationEnabled);
        BAIL_ON_ERROR(e);
    }

    if (windowsBasedAuthenticationEnabled != NULL)
    {
        e = RestBooleanDataNew(
            &(pAuthenticationPolicy->windowsBasedAuthenticationEnabled),
            *windowsBasedAuthenticationEnabled);
        BAIL_ON_ERROR(e);
    }

    if (certificateBasedAuthenticationEnabled != NULL)
    {
        e = RestBooleanDataNew(
            &(pAuthenticationPolicy->certificateBasedAuthenticationEnabled),
            *certificateBasedAuthenticationEnabled);
        BAIL_ON_ERROR(e);
    }

    if (clientCertificatePolicy != NULL)
    {
        e = IdmClientCertificatePolicyDataNew(
            &(pAuthenticationPolicy->clientCertificatePolicy),
            clientCertificatePolicy->certPolicyOIDs,
            clientCertificatePolicy->trustedCACertificates,
            clientCertificatePolicy->revocationCheckEnabled,
            clientCertificatePolicy->ocspEnabled,
            clientCertificatePolicy->failOverToCrlEnabled,
            clientCertificatePolicy->ocspUrlOverride,
            clientCertificatePolicy->crlDistributionPointUsageEnabled,
            clientCertificatePolicy->crlDistributionPointOverride);
        BAIL_ON_ERROR(e);
    }

    *ppAuthenticationPolicy = pAuthenticationPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAuthenticationPolicyDataDelete(pAuthenticationPolicy);
    }

    return e;
}

void
IdmAuthenticationPolicyDataDelete(
    IDM_AUTHENTICATION_POLICY_DATA* pAuthenticationPolicy)
{
    if (pAuthenticationPolicy != NULL)
    {
        RestBooleanDataDelete(pAuthenticationPolicy->passwordBasedAuthenticationEnabled);
        RestBooleanDataDelete(pAuthenticationPolicy->windowsBasedAuthenticationEnabled);
        RestBooleanDataDelete(pAuthenticationPolicy->certificateBasedAuthenticationEnabled);
        IdmClientCertificatePolicyDataDelete(pAuthenticationPolicy->clientCertificatePolicy);
        SSOMemoryFree(pAuthenticationPolicy, sizeof(IDM_AUTHENTICATION_POLICY_DATA));
    }
}

SSOERROR
IdmAuthenticationPolicyDataToJson(
    const IDM_AUTHENTICATION_POLICY_DATA* pAuthenticationPolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAuthenticationPolicy != NULL)
    {
        e = RestDataToJson(
            pAuthenticationPolicy->passwordBasedAuthenticationEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "passwordBasedAuthenticationEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pAuthenticationPolicy->windowsBasedAuthenticationEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "windowsBasedAuthenticationEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pAuthenticationPolicy->certificateBasedAuthenticationEnabled,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "certificateBasedAuthenticationEnabled",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pAuthenticationPolicy->clientCertificatePolicy,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmClientCertificatePolicyDataToJson,
            "clientCertificatePolicy",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAuthenticationPolicyData(
    PCSSO_JSON pJson,
    IDM_AUTHENTICATION_POLICY_DATA** ppAuthenticationPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_AUTHENTICATION_POLICY_DATA* pAuthenticationPolicy = NULL;

    if (pJson == NULL || ppAuthenticationPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_AUTHENTICATION_POLICY_DATA), (void**) &pAuthenticationPolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "passwordBasedAuthenticationEnabled",
        (void**) &(pAuthenticationPolicy->passwordBasedAuthenticationEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "windowsBasedAuthenticationEnabled",
        (void**) &(pAuthenticationPolicy->windowsBasedAuthenticationEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "certificateBasedAuthenticationEnabled",
        (void**) &(pAuthenticationPolicy->certificateBasedAuthenticationEnabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToClientCertificatePolicyData,
        "clientCertificatePolicy",
        (void**) &(pAuthenticationPolicy->clientCertificatePolicy));
    BAIL_ON_ERROR(e);

    *ppAuthenticationPolicy = pAuthenticationPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAuthenticationPolicyDataDelete(pAuthenticationPolicy);
    }

    return e;
}
