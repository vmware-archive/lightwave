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
IdmRelyingPartyDataNew(
    IDM_RELYING_PARTY_DATA** ppRelyingParty,
    PCSTRING name,
    PCSTRING url,
    const IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* signatureAlgorithms,
    const IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* assertionConsumerServices,
    const IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* attributeConsumerServices,
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* singleLogoutServices,
    const REST_CERTIFICATE_DATA* certificate,
    PCSTRING defaultAssertionConsumerService,
    PCSTRING defaultAttributeConsumerService,
    const bool* authnRequestsSigned)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RELYING_PARTY_DATA* pRelyingParty = NULL;

    if (ppRelyingParty == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_RELYING_PARTY_DATA), (void**) &pRelyingParty);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pRelyingParty->name), name);
        BAIL_ON_ERROR(e);
    }

    if (url != NULL)
    {
        e = RestStringDataNew(&(pRelyingParty->url), url);
        BAIL_ON_ERROR(e);
    }

    if (signatureAlgorithms != NULL)
    {
        e = IdmSignatureAlgorithmArrayDataNew(
            &(pRelyingParty->signatureAlgorithms),
            signatureAlgorithms->ppEntry,
            signatureAlgorithms->length);
        BAIL_ON_ERROR(e);
    }

    if (assertionConsumerServices != NULL)
    {
        e = IdmAssertionConsumerServiceArrayDataNew(
            &(pRelyingParty->assertionConsumerServices),
            assertionConsumerServices->ppEntry,
            assertionConsumerServices->length);
        BAIL_ON_ERROR(e);
    }

    if (attributeConsumerServices != NULL)
    {
        e = IdmAttributeConsumerServiceArrayDataNew(
            &(pRelyingParty->attributeConsumerServices),
            attributeConsumerServices->ppEntry,
            attributeConsumerServices->length);
        BAIL_ON_ERROR(e);
    }

    if (singleLogoutServices != NULL)
    {
        e = IdmServiceEndpointArrayDataNew(
            &(pRelyingParty->singleLogoutServices),
            singleLogoutServices->ppEntry,
            singleLogoutServices->length);
        BAIL_ON_ERROR(e);
    }

    if (certificate != NULL)
    {
        e = RestCertificateDataNew(&(pRelyingParty->certificate), certificate->encoded);
        BAIL_ON_ERROR(e);
    }

    if (defaultAssertionConsumerService != NULL)
    {
        e = RestStringDataNew(&(pRelyingParty->defaultAssertionConsumerService), defaultAssertionConsumerService);
        BAIL_ON_ERROR(e);
    }

    if (defaultAttributeConsumerService != NULL)
    {
        e = RestStringDataNew(&(pRelyingParty->defaultAttributeConsumerService), defaultAttributeConsumerService);
        BAIL_ON_ERROR(e);
    }

    if (authnRequestsSigned != NULL)
    {
        e = RestBooleanDataNew(&(pRelyingParty->authnRequestsSigned), *authnRequestsSigned);
        BAIL_ON_ERROR(e);
    }

    *ppRelyingParty = pRelyingParty;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyDataDelete(pRelyingParty);
    }

    return e;
}

void
IdmRelyingPartyDataDelete(
    IDM_RELYING_PARTY_DATA* pRelyingParty)
{
    if (pRelyingParty != NULL)
    {
        RestStringDataDelete(pRelyingParty->name);
        RestStringDataDelete(pRelyingParty->url);
        IdmSignatureAlgorithmArrayDataDelete(pRelyingParty->signatureAlgorithms);
        IdmAssertionConsumerServiceArrayDataDelete(pRelyingParty->assertionConsumerServices);
        IdmAttributeConsumerServiceArrayDataDelete(pRelyingParty->attributeConsumerServices);
        IdmServiceEndpointArrayDataDelete(pRelyingParty->singleLogoutServices);
        RestCertificateDataDelete(pRelyingParty->certificate);
        RestStringDataDelete(pRelyingParty->defaultAssertionConsumerService);
        RestStringDataDelete(pRelyingParty->defaultAttributeConsumerService);
        RestBooleanDataDelete(pRelyingParty->authnRequestsSigned);
        SSOMemoryFree(pRelyingParty, sizeof(IDM_RELYING_PARTY_DATA));
    }
}

SSOERROR
IdmRelyingPartyDataToJson(
    const IDM_RELYING_PARTY_DATA* pRelyingParty,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pRelyingParty != NULL)
    {
        e = RestDataToJson(pRelyingParty->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pRelyingParty->url, REST_JSON_OBJECT_TYPE_STRING, NULL, "url", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->signatureAlgorithms,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmSignatureAlgorithmArrayDataToJson,
            "signatureAlgorithms",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->assertionConsumerServices,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmAssertionConsumerServiceArrayDataToJson,
            "assertionConsumerServices",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->attributeConsumerServices,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmAttributeConsumerServiceArrayDataToJson,
            "attributeConsumerServices",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->singleLogoutServices,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmServiceEndpointArrayDataToJson,
            "singleLogoutServices",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->certificate,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) RestCertificateDataToJson,
            "certificate",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->defaultAssertionConsumerService,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "defaultAssertionConsumerService",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->defaultAttributeConsumerService,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "defaultAttributeConsumerService",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pRelyingParty->authnRequestsSigned,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "authnRequestsSigned",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToRelyingPartyData(
    PCSSO_JSON pJson,
    IDM_RELYING_PARTY_DATA** ppRelyingParty)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RELYING_PARTY_DATA* pRelyingParty = NULL;

    if (pJson == NULL || ppRelyingParty == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_RELYING_PARTY_DATA), (void**) &pRelyingParty);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pRelyingParty->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "url", (void**) &(pRelyingParty->url));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToSignatureAlgorithmArrayData,
        "signatureAlgorithms",
        (void**) &(pRelyingParty->signatureAlgorithms));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToAssertionConsumerServiceArrayData,
        "assertionConsumerServices",
        (void**) &(pRelyingParty->assertionConsumerServices));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToAttributeConsumerServiceArrayData,
        "attributeConsumerServices",
        (void**) &(pRelyingParty->attributeConsumerServices));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToServiceEndpointArrayData,
        "singleLogoutServices",
        (void**) &(pRelyingParty->singleLogoutServices));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) RestJsonToCertificateData,
        "certificate",
        (void**) &(pRelyingParty->certificate));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "defaultAssertionConsumerService",
        (void**) &(pRelyingParty->defaultAssertionConsumerService));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "defaultAttributeConsumerService",
        (void**) &(pRelyingParty->defaultAttributeConsumerService));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "authnRequestsSigned",
        (void**) &(pRelyingParty->authnRequestsSigned));
    BAIL_ON_ERROR(e);

    *ppRelyingParty = pRelyingParty;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyDataDelete(pRelyingParty);
    }

    return e;
}
