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

static PCSTRING entityID = "test_entityid_rest_c_client";

PCSTRING
IdmExternalIdpRegisterTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_EXTERNAL_IDP_DATA* pExternalIdpReturn = NULL;

    PSTRING pNameIDFormatArray[] =
    {
        "format1",
        "format2"
    };
    IDM_STRING_ARRAY_DATA* pNameIDFormats = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSsoService1 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSsoService2 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSsoServiceArray[] = {NULL, NULL};

    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pSsoServices = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSloService1 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSloService2 = NULL;

    IDM_SERVICE_ENDPOINT_DATA* pSloServiceArray[] = {NULL, NULL};

    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pSloServices = NULL;

    REST_CERTIFICATE_DATA* pCertificate1;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    REST_CERTIFICATE_DATA* pCertificateArray[] = {NULL};

    REST_CERTIFICATE_ARRAY_DATA* pCertificates = NULL;

    IDM_CERTIFICATE_CHAIN_DATA* pSigningCertificates = NULL;

    IDM_STRING_MAP_DATA* pSubjectFormats = NULL;
    IDM_STRING_MAP_ENTRY_DATA* pSubjectFormatsEntryData1 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pSubjectFormatsEntryData2 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pSubjectFormatsEntry[] = {NULL, NULL};

    IDM_STRING_ARRAY_DATA* pGroup1 = NULL;
    PSTRING pGroupArray1[] =
    {
        "group1.1",
        "group1.2"
    };

    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup1 = NULL;

    IDM_STRING_ARRAY_DATA* pGroup2 = NULL;
    PSTRING pGroupArray2[] =
    {
        "group2.1",
        "group2.2"
    };

    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup2 = NULL;

    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroupArray[] = {NULL, NULL};

    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroups = NULL;

    PCSTRING alias = "test_alias";
    bool jitEnabled = true;
    PCSTRING upnSuffix = "@upnSuffix";
    IDM_EXTERNAL_IDP_DATA* pExternalIdp = NULL;


    e = IdmStringArrayDataNew(
        &pNameIDFormats,
        pNameIDFormatArray,
        (size_t) (sizeof(pNameIDFormatArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmServiceEndpointDataNew(&pSsoService1, "sso1", "https://test.com/sso1", "HTTP");
    BAIL_ON_ERROR(e);

    e = IdmServiceEndpointDataNew(&pSsoService2, "sso2", "https://test.com/sso2", "HTTP");
    BAIL_ON_ERROR(e);

    pSsoServiceArray[0] = pSsoService1;
    pSsoServiceArray[1] = pSsoService2;

    e = IdmServiceEndpointArrayDataNew(
        &pSsoServices,
        pSsoServiceArray,
        (size_t) (sizeof(pSsoServiceArray) / sizeof(IDM_SERVICE_ENDPOINT_DATA*)));
    BAIL_ON_ERROR(e);

    IdmServiceEndpointDataDelete(pSsoService1);
    IdmServiceEndpointDataDelete(pSsoService2);

    e = IdmServiceEndpointDataNew(&pSloService1, "slo1", "https://test.com/slo1", "HTTP");
    BAIL_ON_ERROR(e);

    e = IdmServiceEndpointDataNew(&pSloService2, "slo2", "https://test.com/slo2", "HTTP");
    BAIL_ON_ERROR(e);

    pSloServiceArray[0] = pSloService1;
    pSloServiceArray[1] = pSloService2;

    e = IdmServiceEndpointArrayDataNew(
        &pSloServices,
        pSloServiceArray,
        (size_t) (sizeof(pSloServiceArray) / sizeof(IDM_SERVICE_ENDPOINT_DATA*)));
    BAIL_ON_ERROR(e);

    IdmServiceEndpointDataDelete(pSloService1);
    IdmServiceEndpointDataDelete(pSloService2);

    // create certificate objects
    e = RestCertificateDataNew(&pCertificate1, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateArray[0] = pCertificate1;

    // create certificate array object
    e = RestCertificateArrayDataNew(
        &pCertificates,
        pCertificateArray,
        sizeof(pCertificateArray) / sizeof(REST_CERTIFICATE_DATA*));
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate1);

    e = IdmCertificateChainDataNew(&pSigningCertificates, pCertificates);
    BAIL_ON_ERROR(e);

    RestCertificateArrayDataDelete(pCertificates);

    e = IdmStringMapEntryDataNew(&pSubjectFormatsEntryData1, "attribute1", "value1");
    BAIL_ON_ERROR(e);

    e = IdmStringMapEntryDataNew(&pSubjectFormatsEntryData2, "attribute2", "value2");
    BAIL_ON_ERROR(e);

    pSubjectFormatsEntry[0] =  pSubjectFormatsEntryData1;
    pSubjectFormatsEntry[1] =  pSubjectFormatsEntryData2;

    e = IdmStringMapDataNew(
        &pSubjectFormats,
        pSubjectFormatsEntry,
        (size_t) (sizeof(pSubjectFormatsEntry) / sizeof(IDM_STRING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmStringMapEntryDataDelete(pSubjectFormatsEntryData1);
    IdmStringMapEntryDataDelete(pSubjectFormatsEntryData2);

    e = IdmStringArrayDataNew(&pGroup1, pGroupArray1, (size_t) (sizeof(pGroupArray1) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmTokenClaimGroupDataNew(&pTokenClaimGroup1, "TokenLifeTime", "500", pGroup1);
    BAIL_ON_ERROR(e);

    IdmStringArrayDataDelete(pGroup1);

    e = IdmStringArrayDataNew(&pGroup2, pGroupArray2, (size_t) (sizeof(pGroupArray2) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmTokenClaimGroupDataNew(&pTokenClaimGroup2, "TokenLifeTime", "500", pGroup2);
    BAIL_ON_ERROR(e);

    IdmStringArrayDataDelete(pGroup2);

    pTokenClaimGroupArray[0] = pTokenClaimGroup1;
    pTokenClaimGroupArray[1] = pTokenClaimGroup2;

    e = IdmTokenClaimGroupArrayDataNew(
        &pTokenClaimGroups,
        pTokenClaimGroupArray,
        (size_t) (sizeof(pTokenClaimGroupArray) / sizeof(IDM_TOKEN_CLAIM_GROUP_DATA*)));
    BAIL_ON_ERROR(e);

    IdmTokenClaimGroupDataDelete(pTokenClaimGroup1);
    IdmTokenClaimGroupDataDelete(pTokenClaimGroup2);

    e = IdmExternalIdpDataNew(
        &pExternalIdp,
        entityID,
        alias,
        pNameIDFormats,
        pSsoServices,
        pSloServices,
        pSigningCertificates,
        pSubjectFormats,
        pTokenClaimGroups,
        &jitEnabled,
        upnSuffix);

    IdmStringArrayDataDelete(pNameIDFormats);
    IdmServiceEndpointArrayDataDelete(pSsoServices);
    IdmServiceEndpointArrayDataDelete(pSloServices);
    IdmStringMapDataDelete(pSubjectFormats);
    IdmTokenClaimGroupArrayDataDelete(pTokenClaimGroups);
    IdmCertificateChainDataDelete(pSigningCertificates);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalIdpRegister(pRestClient, testTenant, pExternalIdp, &pExternalIdpReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmExternalIdpDataDelete(pExternalIdpReturn);
    IdmExternalIdpDataDelete(pExternalIdp);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmExternalIdpRegisterByMetadataTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PCSTRING metadata = "ABC";
    IDM_EXTERNAL_IDP_DATA* pExternalIdpReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalIdpRegisterByMetadata(pRestClient, testTenant, metadata, &pExternalIdpReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmExternalIdpDataDelete(pExternalIdpReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmExternalIdpGetAllTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArrayReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalIdpGetAll(pRestClient, testTenant, &pExternalIdpArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmExternalIdpArrayDataDelete(pExternalIdpArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmExternalIdpGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_EXTERNAL_IDP_DATA* pExternalIdpReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalIdpGet(pRestClient, testTenant, entityID, &pExternalIdpReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmExternalIdpDataDelete(pExternalIdpReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmExternalIdpDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalIdpDelete(pRestClient, testTenant, entityID, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
