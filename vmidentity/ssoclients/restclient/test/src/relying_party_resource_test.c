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

static PCSTRING name = "test_relying_party_rest_c_client";
static PSTRING url = "http://vmware.com/test_relying_party_rest_c_client";

PCSTRING
IdmRelyingPartyRegisterTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RELYING_PARTY_DATA* pRelyingPartyReturn = NULL;

    INTEGER maxKeySize1 = 1024;
    INTEGER minKeySize1 = 256;
    INTEGER priority1 = 1;
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm1 = NULL;

    INTEGER maxKeySize2 = 512;
    INTEGER minKeySize2 = 128;
    INTEGER priority2 = 2;
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm2 = NULL;

    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithmArray[] = {NULL, NULL};

    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithms = NULL;

    INTEGER index1 = 1;
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService1 = NULL;

    INTEGER index2 = 2;

    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService2 = NULL;

    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerServiceArray[] = {NULL, NULL};

    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServices = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute1 = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute2 = NULL;

    IDM_ATTRIBUTE_DATA* pAttributeArray1[] = {NULL, NULL};

    IDM_ATTRIBUTE_ARRAY_DATA* pAttributes1 = NULL;

    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService1 = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute3 = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute4 = NULL;

    IDM_ATTRIBUTE_DATA* pAttributeArray2[] = {NULL, NULL};

    IDM_ATTRIBUTE_ARRAY_DATA* pAttributes2 = NULL;

    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService2 = NULL;

    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerServiceArray[] = {NULL, NULL};

    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServices = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSingleLogoutService1 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSingleLogoutService2 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSingleLogoutServiceArray[] = {NULL, NULL};

    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pSingleLogoutServices = NULL;

    REST_CERTIFICATE_DATA* pCertificate = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    PSTRING defaultAssertionConsumerService = "defaultAssertionConsumerService";
    PSTRING defaultAttributeConsumerService = "defaultAttributeConsumerService";
    bool authnRequestsSigned = true;

    IDM_RELYING_PARTY_DATA* pRelyingParty = NULL;


    e = IdmSignatureAlgorithmDataNew(&pSignatureAlgorithm1, &maxKeySize1, &minKeySize1, &priority1);
    BAIL_ON_ERROR(e);

    e = IdmSignatureAlgorithmDataNew(&pSignatureAlgorithm2, &maxKeySize2, &minKeySize2, &priority2);
    BAIL_ON_ERROR(e);

    pSignatureAlgorithmArray[0] = pSignatureAlgorithm1;
    pSignatureAlgorithmArray[1] = pSignatureAlgorithm2;

    e = IdmSignatureAlgorithmArrayDataNew(
        &pSignatureAlgorithms,
        pSignatureAlgorithmArray,
        (size_t) (sizeof(pSignatureAlgorithmArray) / sizeof(IDM_SIGNATURE_ALGORITHM_DATA*)));
    BAIL_ON_ERROR(e);

    IdmSignatureAlgorithmDataDelete(pSignatureAlgorithm1);
    IdmSignatureAlgorithmDataDelete(pSignatureAlgorithm2);

    e = IdmAssertionConsumerServiceDataNew(
        &pAssertionConsumerService1,
        "AssertionConsumerService1",
        "https://abc.com/AssertionConsumerService1",
        "HTTP",
        &index1);
    BAIL_ON_ERROR(e);

    e = IdmAssertionConsumerServiceDataNew(
        &pAssertionConsumerService2,
        "AssertionConsumerService2",
        "https://abc.com/AssertionConsumerService2",
        "HTTP",
        &index2);
    BAIL_ON_ERROR(e);

    pAssertionConsumerServiceArray[0] = pAssertionConsumerService1;
    pAssertionConsumerServiceArray[1] = pAssertionConsumerService2;

    e = IdmAssertionConsumerServiceArrayDataNew(
        &pAssertionConsumerServices,
        pAssertionConsumerServiceArray,
        (size_t) (sizeof(pAssertionConsumerServiceArray) / sizeof(IDM_ASSERTION_CONSUMER_SERVICE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAssertionConsumerServiceDataDelete(pAssertionConsumerService1);
    IdmAssertionConsumerServiceDataDelete(pAssertionConsumerService2);

    e = IdmAttributeDataNew(&pAttribute1, "Attribute1", "Attribute1_friendlyName", "Format1");
    BAIL_ON_ERROR(e);

    e = IdmAttributeDataNew(&pAttribute2, "Attribute2", "Attribute2_friendlyName", "Format2");
    BAIL_ON_ERROR(e);

    pAttributeArray1[0] = pAttribute1;
    pAttributeArray1[1] = pAttribute2;

    e = IdmAttributeArrayDataNew(
        &pAttributes1,
        pAttributeArray1,
        (size_t) (sizeof(pAttributeArray1) / sizeof(IDM_ATTRIBUTE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAttributeDataDelete(pAttribute1);
    IdmAttributeDataDelete(pAttribute2);

    e = IdmAttributeConsumerServiceDataNew(
        &pAttributeConsumerService1,
        "pAttributeConsumerService1",
        &index1,
        pAttributes1);
    BAIL_ON_ERROR(e);

    IdmAttributeArrayDataDelete(pAttributes1);

    e = IdmAttributeDataNew(&pAttribute3, "Attribute3", "Attribute3_friendlyName", "Format3");
    BAIL_ON_ERROR(e);

    e = IdmAttributeDataNew(&pAttribute4, "Attribute4", "Attribute4_friendlyName", "Format4");
    BAIL_ON_ERROR(e);

    pAttributeArray2[0] = pAttribute3;
    pAttributeArray2[1] = pAttribute4;

    e = IdmAttributeArrayDataNew(
        &pAttributes2,
        pAttributeArray2,
        (size_t) (sizeof(pAttributeArray2) / sizeof(IDM_ATTRIBUTE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAttributeDataDelete(pAttribute3);
    IdmAttributeDataDelete(pAttribute4);

    e = IdmAttributeConsumerServiceDataNew(
        &pAttributeConsumerService2,
        "pAttributeConsumerService2",
        &index2,
        pAttributes2);
    BAIL_ON_ERROR(e);

    IdmAttributeArrayDataDelete(pAttributes2);

    pAttributeConsumerServiceArray[0] = pAttributeConsumerService1;
    pAttributeConsumerServiceArray[1] = pAttributeConsumerService2;

    e = IdmAttributeConsumerServiceArrayDataNew(
        &pAttributeConsumerServices,
        pAttributeConsumerServiceArray,
        (size_t) (sizeof(pAttributeConsumerServiceArray) / sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAttributeConsumerServiceDataDelete(pAttributeConsumerService1);
    IdmAttributeConsumerServiceDataDelete(pAttributeConsumerService2);

    e = IdmServiceEndpointDataNew(
        &pSingleLogoutService1,
        "SingleLogoutService1",
        "https:://abc.com/SingleLogoutService1",
        "HTTP");
    BAIL_ON_ERROR(e);

    e = IdmServiceEndpointDataNew(
        &pSingleLogoutService2,
        "SingleLogoutService2",
        "https:://abc.com/SingleLogoutService2",
        "HTTP");
    BAIL_ON_ERROR(e);

    pSingleLogoutServiceArray[0] = pSingleLogoutService1;
    pSingleLogoutServiceArray[1] = pSingleLogoutService2;

    e = IdmServiceEndpointArrayDataNew(
        &pSingleLogoutServices,
        pSingleLogoutServiceArray,
        (size_t) (sizeof(pSingleLogoutServiceArray) / sizeof(IDM_SERVICE_ENDPOINT_DATA*)));
    BAIL_ON_ERROR(e);

    IdmServiceEndpointDataDelete(pSingleLogoutService1);
    IdmServiceEndpointDataDelete(pSingleLogoutService2);

    e = RestCertificateDataNew(&pCertificate, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyDataNew(
        &pRelyingParty,
        name,
        url,
        pSignatureAlgorithms,
        pAssertionConsumerServices,
        pAttributeConsumerServices,
        pSingleLogoutServices,
        pCertificate,
        defaultAssertionConsumerService,
        defaultAttributeConsumerService,
        &authnRequestsSigned);
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate);

    IdmServiceEndpointArrayDataDelete(pSingleLogoutServices);
    IdmSignatureAlgorithmArrayDataDelete(pSignatureAlgorithms);
    IdmAssertionConsumerServiceArrayDataDelete(pAssertionConsumerServices);
    IdmAttributeConsumerServiceArrayDataDelete(pAttributeConsumerServices);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyRegister(pRestClient, testTenant, pRelyingParty, &pRelyingPartyReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmRelyingPartyDataDelete(pRelyingPartyReturn);
    IdmRelyingPartyDataDelete(pRelyingParty);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmRelyingPartyGetAllTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArrayReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyGetAll(pRestClient, testTenant, &pRelyingPartyArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmRelyingPartyArrayDataDelete(pRelyingPartyArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmRelyingPartyGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RELYING_PARTY_DATA* pRelyingPartyReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyGet(pRestClient, testTenant, name, &pRelyingPartyReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmRelyingPartyDataDelete(pRelyingPartyReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmRelyingPartyUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_RELYING_PARTY_DATA* pRelyingPartyReturn = NULL;

    INTEGER maxKeySize1 = 1024;
    INTEGER minKeySize1 = 256;
    INTEGER priority1 = 1;
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm1 = NULL;

    INTEGER maxKeySize2 = 512;
    INTEGER minKeySize2 = 128;
    INTEGER priority2 = 2;
    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithm2 = NULL;

    IDM_SIGNATURE_ALGORITHM_DATA* pSignatureAlgorithmArray[] = {NULL, NULL};

    IDM_SIGNATURE_ALGORITHM_ARRAY_DATA* pSignatureAlgorithms = NULL;

    INTEGER index1 = 1;
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService1 = NULL;

    INTEGER index2 = 2;
    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerService2 = NULL;

    IDM_ASSERTION_CONSUMER_SERVICE_DATA* pAssertionConsumerServiceArray[] = {NULL, NULL};

    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServices = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute1 = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute2 = NULL;

    IDM_ATTRIBUTE_DATA* pAttributeArray1[] = {NULL, NULL};

    IDM_ATTRIBUTE_ARRAY_DATA* pAttributes1 = NULL;

    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService1 = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute3 = NULL;

    IDM_ATTRIBUTE_DATA* pAttribute4 = NULL;

    IDM_ATTRIBUTE_DATA* pAttributeArray2[] = {NULL, NULL};

    IDM_ATTRIBUTE_ARRAY_DATA* pAttributes2 = NULL;

    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerService2 = NULL;

    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA* pAttributeConsumerServiceArray[] = {NULL, NULL};

    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServices = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSingleLogoutService1 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSingleLogoutService2 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSingleLogoutServiceArray[] = {NULL, NULL};

    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pSingleLogoutServices = NULL;

    REST_CERTIFICATE_DATA* pCertificate = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    PSTRING defaultAssertionConsumerService = "defaultAssertionConsumerService";
    PSTRING defaultAttributeConsumerService = "defaultAttributeConsumerService";
    bool authnRequestsSigned = true;

    IDM_RELYING_PARTY_DATA* pRelyingParty = NULL;


    e = IdmSignatureAlgorithmDataNew(&pSignatureAlgorithm1, &maxKeySize1, &minKeySize1, &priority1);
    BAIL_ON_ERROR(e);

    e = IdmSignatureAlgorithmDataNew(&pSignatureAlgorithm2, &maxKeySize2, &minKeySize2, &priority2);
    BAIL_ON_ERROR(e);

    pSignatureAlgorithmArray[0] = pSignatureAlgorithm1;
    pSignatureAlgorithmArray[1] = pSignatureAlgorithm2;

    e = IdmSignatureAlgorithmArrayDataNew(
        &pSignatureAlgorithms,
        pSignatureAlgorithmArray,
        (size_t) (sizeof(pSignatureAlgorithmArray) / sizeof(IDM_SIGNATURE_ALGORITHM_DATA*)));
    BAIL_ON_ERROR(e);

    IdmSignatureAlgorithmDataDelete(pSignatureAlgorithm1);
    IdmSignatureAlgorithmDataDelete(pSignatureAlgorithm2);

    e = IdmAssertionConsumerServiceDataNew(
        &pAssertionConsumerService1,
        "AssertionConsumerService1",
        "https://def.com/AssertionConsumerService1",
        "HTTP",
        &index1);
    BAIL_ON_ERROR(e);

    e = IdmAssertionConsumerServiceDataNew(
        &pAssertionConsumerService2,
        "AssertionConsumerService2",
        "https://def.com/AssertionConsumerService2",
        "HTTP",
        &index2);
    BAIL_ON_ERROR(e);

    pAssertionConsumerServiceArray[0] = pAssertionConsumerService1;
    pAssertionConsumerServiceArray[1] = pAssertionConsumerService2;

    e = IdmAssertionConsumerServiceArrayDataNew(
        &pAssertionConsumerServices,
        pAssertionConsumerServiceArray,
        (size_t) (sizeof(pAssertionConsumerServiceArray) / sizeof(IDM_ASSERTION_CONSUMER_SERVICE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAssertionConsumerServiceDataDelete(pAssertionConsumerService1);
    IdmAssertionConsumerServiceDataDelete(pAssertionConsumerService2);

    e = IdmAttributeDataNew(&pAttribute1, "Attribute1", "Attribute1_friendlyName", "Format1");
    BAIL_ON_ERROR(e);

    e = IdmAttributeDataNew(&pAttribute2, "Attribute2", "Attribute2_friendlyName", "Format2");
    BAIL_ON_ERROR(e);

    pAttributeArray1[0] = pAttribute1;
    pAttributeArray1[1] = pAttribute2;

    e = IdmAttributeArrayDataNew(
        &pAttributes1,
        pAttributeArray1,
        (size_t) (sizeof(pAttributeArray1) / sizeof(IDM_ATTRIBUTE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAttributeDataDelete(pAttribute1);
    IdmAttributeDataDelete(pAttribute2);

    e = IdmAttributeConsumerServiceDataNew(
        &pAttributeConsumerService1,
        "pAttributeConsumerService1",
        &index1,
        pAttributes1);
    BAIL_ON_ERROR(e);

    IdmAttributeArrayDataDelete(pAttributes1);

    e = IdmAttributeDataNew(&pAttribute3, "Attribute3", "Attribute3_friendlyName", "Format3");
    BAIL_ON_ERROR(e);

    e = IdmAttributeDataNew(&pAttribute4, "Attribute4", "Attribute4_friendlyName", "Format4");
    BAIL_ON_ERROR(e);

    pAttributeArray2[0] = pAttribute3;
    pAttributeArray2[1] = pAttribute4;

    e = IdmAttributeArrayDataNew(
        &pAttributes2,
        pAttributeArray2,
        (size_t) (sizeof(pAttributeArray2) / sizeof(IDM_ATTRIBUTE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAttributeDataDelete(pAttribute3);
    IdmAttributeDataDelete(pAttribute4);

    e = IdmAttributeConsumerServiceDataNew(
        &pAttributeConsumerService2,
        "pAttributeConsumerService2",
        &index2,
        pAttributes2);
    BAIL_ON_ERROR(e);

    IdmAttributeArrayDataDelete(pAttributes2);

    pAttributeConsumerServiceArray[0] = pAttributeConsumerService1;
    pAttributeConsumerServiceArray[1] = pAttributeConsumerService2;

    e = IdmAttributeConsumerServiceArrayDataNew(
        &pAttributeConsumerServices,
        pAttributeConsumerServiceArray,
        (size_t) (sizeof(pAttributeConsumerServiceArray) / sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA*)));
    BAIL_ON_ERROR(e);

    IdmAttributeConsumerServiceDataDelete(pAttributeConsumerService1);
    IdmAttributeConsumerServiceDataDelete(pAttributeConsumerService2);

    e = IdmServiceEndpointDataNew(
        &pSingleLogoutService1,
        "SingleLogoutService1",
        "https:://def.com/SingleLogoutService1",
        "HTTP");
    BAIL_ON_ERROR(e);

    e = IdmServiceEndpointDataNew(
        &pSingleLogoutService2,
        "SingleLogoutService2",
        "https:://def.com/SingleLogoutService2",
        "HTTP");
    BAIL_ON_ERROR(e);

    pSingleLogoutServiceArray[0] = pSingleLogoutService1;
    pSingleLogoutServiceArray[1] = pSingleLogoutService2;

    e = IdmServiceEndpointArrayDataNew(
        &pSingleLogoutServices,
        pSingleLogoutServiceArray,
        (size_t) (sizeof(pSingleLogoutServiceArray) / sizeof(IDM_SERVICE_ENDPOINT_DATA*)));
    BAIL_ON_ERROR(e);

    IdmServiceEndpointDataDelete(pSingleLogoutService1);
    IdmServiceEndpointDataDelete(pSingleLogoutService2);

    e = RestCertificateDataNew(&pCertificate, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyDataNew(
        &pRelyingParty,
        name,
        url,
        pSignatureAlgorithms,
        pAssertionConsumerServices,
        pAttributeConsumerServices,
        pSingleLogoutServices,
        pCertificate,
        defaultAssertionConsumerService,
        defaultAttributeConsumerService,
        &authnRequestsSigned);
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate);

    IdmSignatureAlgorithmArrayDataDelete(pSignatureAlgorithms);
    IdmServiceEndpointArrayDataDelete(pSingleLogoutServices);
    IdmAssertionConsumerServiceArrayDataDelete(pAssertionConsumerServices);
    IdmAttributeConsumerServiceArrayDataDelete(pAttributeConsumerServices);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyUpdate(pRestClient, testTenant, name, pRelyingParty, &pRelyingPartyReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmRelyingPartyDataDelete(pRelyingPartyReturn);
    IdmRelyingPartyDataDelete(pRelyingParty);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmRelyingPartyDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmRelyingPartyDelete(pRestClient, testTenant, name, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
