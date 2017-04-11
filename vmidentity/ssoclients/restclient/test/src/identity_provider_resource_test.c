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

static PCSTRING provider = "test_privider_rest_c_client";

PCSTRING
IdmIdentityProviderCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;

    IDM_STRING_ARRAY_DATA* pConnectionStrings = NULL;
    PSTRING pConnectionStringArray[] = {NULL};

    IDM_STRING_MAP_DATA* pAttributesMap = NULL;
    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntryData1 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntryData2 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntry[] = {NULL, NULL};

    IDM_STRING_ARRAY_DATA* pUpnSuffixes = NULL;

    PSTRING pUpnSuffixArray[] =
    {
        "upnSuffixes1",
        "upnSuffixes2"
    };

    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchema = NULL;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMappingData1 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntryData1 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMappingData2 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntryData2 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntry[] = {NULL, NULL};

    REST_CERTIFICATE_DATA* pCertificate1 = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    REST_CERTIFICATE_DATA* pCertificate2 = NULL;

    REST_CERTIFICATE_DATA* pCertificateArray[] = {NULL, NULL};

    REST_CERTIFICATE_ARRAY_DATA* pCertificates = NULL;

    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider = NULL;
    PCSTRING domainType = "test_domainType";
    PCSTRING alias = "test_alias";
    PCSTRING type = "IDENTITY_STORE_TYPE_LDAP";
    PCSTRING authenticationType = "PASSWORD";
    PCSTRING friendlyName = "test_friendlyName";
    SSO_LONG searchTimeOutInSeconds = 10000;
    PCSTRING username = "cn=Administrator, cn=users, dc=vSphere, dc=local";
    PCSTRING password = "Admin!23";
    bool machineAccount = false;
    PCSTRING servicePrincipalName = "test_servicePrincipalName";
    PCSTRING userBaseDN = "cn=users, dc=vSphere, dc=local";
    PCSTRING groupBaseDN = "cn=solutionusers, dc=vSphere, dc=local";
    bool matchingRuleInChainEnabled = true;
    bool baseDnForNestedGroupsEnabled = true;
    bool directGroupsSearchEnabled = true;
    bool siteAffinityEnabled = true;

    PSSO_STRING_BUILDER sb = NULL;

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, "ldap://");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, pscHost);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, ":389");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pConnectionStringArray[0]);
    BAIL_ON_ERROR(e);

    SSOStringBuilderDelete(sb);

    e = IdmStringArrayDataNew(
        &pConnectionStrings,
        pConnectionStringArray,
        (size_t) (sizeof(pConnectionStringArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    SSOStringFree(pConnectionStringArray[0]);

    e = IdmStringMapEntryDataNew(&pAttributesMapEntryData1, "attribute1", "value1");
    BAIL_ON_ERROR(e);

    e = IdmStringMapEntryDataNew(&pAttributesMapEntryData2, "attribute2", "value2");
    BAIL_ON_ERROR(e);

    pAttributesMapEntry[0] = pAttributesMapEntryData1;
    pAttributesMapEntry[1] = pAttributesMapEntryData2;

    e = IdmStringMapDataNew(
        &pAttributesMap,
        pAttributesMapEntry,
        (size_t) (sizeof(pAttributesMapEntry) / sizeof(IDM_STRING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmStringMapEntryDataDelete(pAttributesMapEntryData1);
    IdmStringMapEntryDataDelete(pAttributesMapEntryData2);

    e = IdmStringArrayDataNew(&pUpnSuffixes, pUpnSuffixArray, (size_t) (sizeof(pUpnSuffixArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingDataNew(&pSchemaObjectMappingData1, "objectClass1", pAttributesMap);
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingMapEntryDataNew(&pSchemaEntryData1, "schema1", pSchemaObjectMappingData1);
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingData1);

    e = IdmSchemaObjectMappingDataNew(&pSchemaObjectMappingData2, "objectClass2", pAttributesMap);
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingMapEntryDataNew(&pSchemaEntryData2, "schema2", pSchemaObjectMappingData2);
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingData2);

    pSchemaEntry[0] = pSchemaEntryData1;
    pSchemaEntry[1] = pSchemaEntryData2;

    e = IdmSchemaObjectMappingMapDataNew(
        &pSchema,
        pSchemaEntry,
        (size_t) (sizeof(pSchemaEntry) / sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingMapEntryDataDelete(pSchemaEntryData1);
    IdmSchemaObjectMappingMapEntryDataDelete(pSchemaEntryData2);

    // create certificate objects
    e = RestCertificateDataNew(&pCertificate1, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDnjCCAoagAwIBAgIJAOValQs66yOrMA0GCSqGSIb3DQEBCwUAMH0x\nCzAJBgNVBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMG\nCgmSJomT8ixkARkWBWxvY2FsMQswCQYDVQQGEwJVUzExMC8GA1UECgwo\nc2MtcmRvcHMtdm0wOC1kaGNwLTIzMC00Ni5lbmcudm13YXJlLmNvbTAe\nFw0xNTAxMDYyMTQ4MjdaFw0yNDEyMzEyMTU4MTNaMBgxFjAUBgNVBAMM\nDXNzb3NlcnZlclNpZ24wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\nAoIBAQCi9yZSituevZoQzfqr/n+QyPWWUn2b2kiIn9Q/pWlZdaSTyMn9\nnsFZvxUFxFz+klqPyoqWP/++4BffEzClByMEm4mdSfhsSOaIAZ6LVJxR\n+38Cy0LT3YsHcPMGtPmk7Olxel+R2lmTr+1a7K56/CY15LzSR214JaTt\neAJ5XFgud8yqjOEFx/R2fkKS7phbGvvTEtfFPXEVGXe02Wc3UstuY8uV\nImnhF2xYud8LjgAQXqGHae2Jf08AlphD5FpU33GQcQMnIVto3AG6iFT/\nKj6Btfa8DfcVWCUB2w3G52KU0ndMrewBL5XzAWfle5JQMDup8vDxdAcI\nQ87iLsygB/XtAgMBAAGjgYUwgYIwCwYDVR0PBAQDAgXgMDMGA1UdEQQs\nMCqCKHNjLXJkb3BzLXZtMDgtZGhjcC0yMzAtNDYuZW5nLnZtd2FyZS5j\nb20wHQYDVR0OBBYEFNQH1ogZAwUFGP6PbJB5LJw/CQbtMB8GA1UdIwQY\nMBaAFN3YC312m4W23Uu2kzKP/zzPuewpMA0GCSqGSIb3DQEBCwUAA4IB\nAQCtBR+Hl1ttC9Q/CucM8//cDygpJzVMJdc5ZP6jO50twQ93ssTSTHTb\na21yk7M9ilJKBc2J5mnkps1il//goAFniAIHNxKmArrYcMlrItJWJZ5s\nt3osjaiLwj/RxQZg4oHfCXsCdaY+hIPjMMam1covOdJazsUMYdL1Wdbx\nU71VccERxRuO5k3OmIS+nWcCS3z/7DV7UgXvnNScmtFTUDp3Vxs/4viS\nMgrLppXIqkISpDboPklQPiJbiRZHd09a2jJL4tXU7K0NvwVBJfjPwBKr\nmj36c1dg+2HvjymtunzJbOifzzcbgkxv+7EyVBeihAmYj/t3w7XswGNQ\nB9Xj4rI/\n-----END CERTIFICATE-----";
    e = RestCertificateDataNew(&pCertificate2, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateArray[0] = pCertificate1;
    pCertificateArray[1] = pCertificate2;

    // create certificate array object
    e = RestCertificateArrayDataNew(
        &pCertificates,
        pCertificateArray,
        sizeof(pCertificateArray) / sizeof(REST_CERTIFICATE_DATA*));
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate1);
    RestCertificateDataDelete(pCertificate2);

    e = IdmIdentityProviderDataNew(
        &pIdentityProvider,
        domainType,
        provider,
        alias,
        type,
        authenticationType,
        friendlyName,
        &searchTimeOutInSeconds,
        username,
        password,
        &machineAccount,
        servicePrincipalName,
        userBaseDN,
        groupBaseDN,
        pConnectionStrings,
        pAttributesMap,
        pUpnSuffixes,
        pSchema,
        &matchingRuleInChainEnabled,
        &baseDnForNestedGroupsEnabled,
        &directGroupsSearchEnabled,
        &siteAffinityEnabled,
        pCertificates);
    BAIL_ON_ERROR(e);

    IdmStringArrayDataDelete(pConnectionStrings);
    IdmStringArrayDataDelete(pUpnSuffixes);
    IdmStringMapDataDelete(pAttributesMap);
    IdmSchemaObjectMappingMapDataDelete(pSchema);
    RestCertificateArrayDataDelete(pCertificates);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmIdentityProviderCreate(pRestClient, testTenant, pIdentityProvider, &pIdentityProviderReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmIdentityProviderDataDelete(pIdentityProviderReturn);
    IdmIdentityProviderDataDelete(pIdentityProvider);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmIdentityProviderProbeTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_STRING_ARRAY_DATA* pConnectionStrings = NULL;
    PSTRING pConnectionStringArray[] = {NULL};

    IDM_STRING_MAP_DATA* pAttributesMap = NULL;
    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntryData1 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntryData2 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntry[] = {NULL, NULL};

    IDM_STRING_ARRAY_DATA* pUpnSuffixes = NULL;
    PSTRING pUpnSuffixArray[] =
    {
        "upnSuffixes1",
        "upnSuffixes2"
    };

    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchema = NULL;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMappingData1 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntryData1 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMappingData2 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntryData2 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntry[] = {NULL, NULL};

    REST_CERTIFICATE_DATA* pCertificate1 = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    REST_CERTIFICATE_DATA* pCertificate2 = NULL;

    REST_CERTIFICATE_DATA* pCertificateArray[] = {NULL, NULL};

    REST_CERTIFICATE_ARRAY_DATA* pCertificates = NULL;

    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider = NULL;
    PCSTRING domainType = "test_domainType";
    PCSTRING alias = "test_alias";
    PCSTRING type = "IDENTITY_STORE_TYPE_LDAP";
    PCSTRING authenticationType = "PASSWORD";
    PCSTRING friendlyName = "test_friendlyName";
    SSO_LONG searchTimeOutInSeconds = 10000;
    PCSTRING username = "cn=Administrator, cn=users, dc=vSphere, dc=local";
    PCSTRING password = "Admin!23";
    bool machineAccount = false;
    PCSTRING servicePrincipalName = "test_servicePrincipalName";
    PCSTRING userBaseDN = "cn=users, dc=vSphere, dc=local";
    PCSTRING groupBaseDN = "cn=solutionusers, dc=vSphere, dc=local";
    bool matchingRuleInChainEnabled = true;
    bool baseDnForNestedGroupsEnabled = true;
    bool directGroupsSearchEnabled = true;
    bool siteAffinityEnabled = true;

    PSSO_STRING_BUILDER sb = NULL;

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, "ldap://");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, pscHost);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, ":389");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pConnectionStringArray[0]);
    BAIL_ON_ERROR(e);

    SSOStringBuilderDelete(sb);

    e = IdmStringArrayDataNew(
        &pConnectionStrings,
        pConnectionStringArray,
        (size_t) (sizeof(pConnectionStringArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    SSOStringFree(pConnectionStringArray[0]);

    e = IdmStringMapEntryDataNew(&pAttributesMapEntryData1, "attribute1", "value1");
    BAIL_ON_ERROR(e);

    e = IdmStringMapEntryDataNew(&pAttributesMapEntryData2, "attribute2", "value2");
    BAIL_ON_ERROR(e);

    pAttributesMapEntry[0] = pAttributesMapEntryData1;
    pAttributesMapEntry[1] = pAttributesMapEntryData2;

    e = IdmStringMapDataNew(
        &pAttributesMap,
        pAttributesMapEntry,
        (size_t) (sizeof(pAttributesMapEntry) / sizeof(IDM_STRING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmStringMapEntryDataDelete(pAttributesMapEntryData1);
    IdmStringMapEntryDataDelete(pAttributesMapEntryData2);

    e = IdmStringArrayDataNew(&pUpnSuffixes, pUpnSuffixArray, (size_t) (sizeof(pUpnSuffixArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingDataNew(&pSchemaObjectMappingData1, "objectClass1", pAttributesMap);
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingMapEntryDataNew(&pSchemaEntryData1, "schema1", pSchemaObjectMappingData1);
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingData1);

    e = IdmSchemaObjectMappingDataNew(&pSchemaObjectMappingData2, "objectClass2", pAttributesMap);
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingMapEntryDataNew(&pSchemaEntryData2, "schema2", pSchemaObjectMappingData2);
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingData2);

    pSchemaEntry[0] = pSchemaEntryData1;
    pSchemaEntry[1] = pSchemaEntryData2;

    e = IdmSchemaObjectMappingMapDataNew(
        &pSchema,
        pSchemaEntry,
        (size_t) (sizeof(pSchemaEntry) / sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingMapEntryDataDelete(pSchemaEntryData1);
    IdmSchemaObjectMappingMapEntryDataDelete(pSchemaEntryData2);

    // create certificate objects
    e = RestCertificateDataNew(&pCertificate1, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDnjCCAoagAwIBAgIJAOValQs66yOrMA0GCSqGSIb3DQEBCwUAMH0x\nCzAJBgNVBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMG\nCgmSJomT8ixkARkWBWxvY2FsMQswCQYDVQQGEwJVUzExMC8GA1UECgwo\nc2MtcmRvcHMtdm0wOC1kaGNwLTIzMC00Ni5lbmcudm13YXJlLmNvbTAe\nFw0xNTAxMDYyMTQ4MjdaFw0yNDEyMzEyMTU4MTNaMBgxFjAUBgNVBAMM\nDXNzb3NlcnZlclNpZ24wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\nAoIBAQCi9yZSituevZoQzfqr/n+QyPWWUn2b2kiIn9Q/pWlZdaSTyMn9\nnsFZvxUFxFz+klqPyoqWP/++4BffEzClByMEm4mdSfhsSOaIAZ6LVJxR\n+38Cy0LT3YsHcPMGtPmk7Olxel+R2lmTr+1a7K56/CY15LzSR214JaTt\neAJ5XFgud8yqjOEFx/R2fkKS7phbGvvTEtfFPXEVGXe02Wc3UstuY8uV\nImnhF2xYud8LjgAQXqGHae2Jf08AlphD5FpU33GQcQMnIVto3AG6iFT/\nKj6Btfa8DfcVWCUB2w3G52KU0ndMrewBL5XzAWfle5JQMDup8vDxdAcI\nQ87iLsygB/XtAgMBAAGjgYUwgYIwCwYDVR0PBAQDAgXgMDMGA1UdEQQs\nMCqCKHNjLXJkb3BzLXZtMDgtZGhjcC0yMzAtNDYuZW5nLnZtd2FyZS5j\nb20wHQYDVR0OBBYEFNQH1ogZAwUFGP6PbJB5LJw/CQbtMB8GA1UdIwQY\nMBaAFN3YC312m4W23Uu2kzKP/zzPuewpMA0GCSqGSIb3DQEBCwUAA4IB\nAQCtBR+Hl1ttC9Q/CucM8//cDygpJzVMJdc5ZP6jO50twQ93ssTSTHTb\na21yk7M9ilJKBc2J5mnkps1il//goAFniAIHNxKmArrYcMlrItJWJZ5s\nt3osjaiLwj/RxQZg4oHfCXsCdaY+hIPjMMam1covOdJazsUMYdL1Wdbx\nU71VccERxRuO5k3OmIS+nWcCS3z/7DV7UgXvnNScmtFTUDp3Vxs/4viS\nMgrLppXIqkISpDboPklQPiJbiRZHd09a2jJL4tXU7K0NvwVBJfjPwBKr\nmj36c1dg+2HvjymtunzJbOifzzcbgkxv+7EyVBeihAmYj/t3w7XswGNQ\nB9Xj4rI/\n-----END CERTIFICATE-----";
    e = RestCertificateDataNew(&pCertificate2, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateArray[0] = pCertificate1;
    pCertificateArray[1] = pCertificate2;

    // create certificate array object
    e = RestCertificateArrayDataNew(
        &pCertificates,
        pCertificateArray,
        sizeof(pCertificateArray) / sizeof(REST_CERTIFICATE_DATA*));
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate1);
    RestCertificateDataDelete(pCertificate2);

    e = IdmIdentityProviderDataNew(
        &pIdentityProvider,
        domainType,
        provider,
        alias,
        type,
        authenticationType,
        friendlyName,
        &searchTimeOutInSeconds,
        username,
        password,
        &machineAccount,
        servicePrincipalName,
        userBaseDN,
        groupBaseDN,
        pConnectionStrings,
        pAttributesMap,
        pUpnSuffixes,
        pSchema,
        &matchingRuleInChainEnabled,
        &baseDnForNestedGroupsEnabled,
        &directGroupsSearchEnabled,
        &siteAffinityEnabled,
        pCertificates);
    BAIL_ON_ERROR(e);

    IdmStringArrayDataDelete(pConnectionStrings);
    IdmStringArrayDataDelete(pUpnSuffixes);
    IdmStringMapDataDelete(pAttributesMap);
    IdmSchemaObjectMappingMapDataDelete(pSchema);
    RestCertificateArrayDataDelete(pCertificates);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmIdentityProviderProbe(pRestClient, testTenant, pIdentityProvider, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmIdentityProviderDataDelete(pIdentityProvider);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmIdentityProviderGetAllTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArrayReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmIdentityProviderGetAll(pRestClient, testTenant, &pIdentityProviderArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmIdentityProviderArrayDataDelete(pIdentityProviderArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmIdentityProviderGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmIdentityProviderGet(pRestClient, testTenant, provider, &pIdentityProviderReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmIdentityProviderDataDelete(pIdentityProviderReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmIdentityProviderUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;

    IDM_STRING_ARRAY_DATA* pConnectionStrings = NULL;
    PSTRING pConnectionStringArray[] = {NULL};

    IDM_STRING_MAP_DATA* pAttributesMap = NULL;
    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntryData1 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntryData2 = NULL;

    IDM_STRING_MAP_ENTRY_DATA* pAttributesMapEntry[] = {NULL, NULL};

    IDM_STRING_ARRAY_DATA* pUpnSuffixes = NULL;
    PSTRING pUpnSuffixArray[] =
    {
        "upnSuffixes1",
        "upnSuffixes2"
    };

    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchema = NULL;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMappingData1 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntryData1 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMappingData2 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntryData2 = NULL;

    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaEntry[] = {NULL, NULL};

    REST_CERTIFICATE_DATA* pCertificate1 = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    REST_CERTIFICATE_DATA* pCertificate2 = NULL;

    REST_CERTIFICATE_DATA* pCertificateArray[] = {NULL, NULL};

    REST_CERTIFICATE_ARRAY_DATA* pCertificates = NULL;

    IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider = NULL;
    PCSTRING domainType = "test_domainType";
    PCSTRING alias = "test_alias";
    PCSTRING type = "IDENTITY_STORE_TYPE_LDAP";
    PCSTRING authenticationType = "PASSWORD";
    PCSTRING friendlyName = "test_friendlyName";
    SSO_LONG searchTimeOutInSeconds = 10000;
    PCSTRING username = "cn=Administrator, cn=users, dc=vSphere, dc=local";
    PCSTRING password = "Admin!23";
    bool machineAccount = false;
    PCSTRING servicePrincipalName = "test_servicePrincipalName";
    PCSTRING userBaseDN = "cn=users, dc=vSphere, dc=local";
    PCSTRING groupBaseDN = "cn=solutionusers, dc=vSphere, dc=local";
    bool matchingRuleInChainEnabled = true;
    bool baseDnForNestedGroupsEnabled = true;
    bool directGroupsSearchEnabled = true;
    bool siteAffinityEnabled = true;

    PSSO_STRING_BUILDER sb = NULL;

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, "ldap://");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, pscHost);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, ":389");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pConnectionStringArray[0]);
    BAIL_ON_ERROR(e);

    SSOStringBuilderDelete(sb);

    e = IdmStringArrayDataNew(
        &pConnectionStrings,
        pConnectionStringArray,
        (size_t) (sizeof(pConnectionStringArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    SSOStringFree(pConnectionStringArray[0]);

    e = IdmStringMapEntryDataNew(&pAttributesMapEntryData1, "attribute1", "value1");
    BAIL_ON_ERROR(e);

    e = IdmStringMapEntryDataNew(&pAttributesMapEntryData2, "attribute2", "value2");
    BAIL_ON_ERROR(e);

    pAttributesMapEntry[0] = pAttributesMapEntryData1;
    pAttributesMapEntry[1] = pAttributesMapEntryData2;

    e = IdmStringMapDataNew(
        &pAttributesMap,
        pAttributesMapEntry,
        (size_t) (sizeof(pAttributesMapEntry) / sizeof(IDM_STRING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmStringMapEntryDataDelete(pAttributesMapEntryData1);
    IdmStringMapEntryDataDelete(pAttributesMapEntryData2);

    e = IdmStringArrayDataNew(&pUpnSuffixes, pUpnSuffixArray, (size_t) (sizeof(pUpnSuffixArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingDataNew(&pSchemaObjectMappingData1, "objectClass1", pAttributesMap);
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingMapEntryDataNew(&pSchemaEntryData1, "schema1", pSchemaObjectMappingData1);
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingData1);

    e = IdmSchemaObjectMappingDataNew(&pSchemaObjectMappingData2, "objectClass2", pAttributesMap);
    BAIL_ON_ERROR(e);

    e = IdmSchemaObjectMappingMapEntryDataNew(&pSchemaEntryData2, "schema2", pSchemaObjectMappingData2);
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingData2);

    pSchemaEntry[0] = pSchemaEntryData1;
    pSchemaEntry[1] = pSchemaEntryData2;

    e = IdmSchemaObjectMappingMapDataNew(
        &pSchema,
        pSchemaEntry,
        (size_t) (sizeof(pSchemaEntry) / sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA*)));
    BAIL_ON_ERROR(e);

    IdmSchemaObjectMappingMapEntryDataDelete(pSchemaEntryData1);
    IdmSchemaObjectMappingMapEntryDataDelete(pSchemaEntryData2);

    // create certificate objects
    e = RestCertificateDataNew(&pCertificate1, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDnjCCAoagAwIBAgIJAOValQs66yOrMA0GCSqGSIb3DQEBCwUAMH0x\nCzAJBgNVBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMG\nCgmSJomT8ixkARkWBWxvY2FsMQswCQYDVQQGEwJVUzExMC8GA1UECgwo\nc2MtcmRvcHMtdm0wOC1kaGNwLTIzMC00Ni5lbmcudm13YXJlLmNvbTAe\nFw0xNTAxMDYyMTQ4MjdaFw0yNDEyMzEyMTU4MTNaMBgxFjAUBgNVBAMM\nDXNzb3NlcnZlclNpZ24wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\nAoIBAQCi9yZSituevZoQzfqr/n+QyPWWUn2b2kiIn9Q/pWlZdaSTyMn9\nnsFZvxUFxFz+klqPyoqWP/++4BffEzClByMEm4mdSfhsSOaIAZ6LVJxR\n+38Cy0LT3YsHcPMGtPmk7Olxel+R2lmTr+1a7K56/CY15LzSR214JaTt\neAJ5XFgud8yqjOEFx/R2fkKS7phbGvvTEtfFPXEVGXe02Wc3UstuY8uV\nImnhF2xYud8LjgAQXqGHae2Jf08AlphD5FpU33GQcQMnIVto3AG6iFT/\nKj6Btfa8DfcVWCUB2w3G52KU0ndMrewBL5XzAWfle5JQMDup8vDxdAcI\nQ87iLsygB/XtAgMBAAGjgYUwgYIwCwYDVR0PBAQDAgXgMDMGA1UdEQQs\nMCqCKHNjLXJkb3BzLXZtMDgtZGhjcC0yMzAtNDYuZW5nLnZtd2FyZS5j\nb20wHQYDVR0OBBYEFNQH1ogZAwUFGP6PbJB5LJw/CQbtMB8GA1UdIwQY\nMBaAFN3YC312m4W23Uu2kzKP/zzPuewpMA0GCSqGSIb3DQEBCwUAA4IB\nAQCtBR+Hl1ttC9Q/CucM8//cDygpJzVMJdc5ZP6jO50twQ93ssTSTHTb\na21yk7M9ilJKBc2J5mnkps1il//goAFniAIHNxKmArrYcMlrItJWJZ5s\nt3osjaiLwj/RxQZg4oHfCXsCdaY+hIPjMMam1covOdJazsUMYdL1Wdbx\nU71VccERxRuO5k3OmIS+nWcCS3z/7DV7UgXvnNScmtFTUDp3Vxs/4viS\nMgrLppXIqkISpDboPklQPiJbiRZHd09a2jJL4tXU7K0NvwVBJfjPwBKr\nmj36c1dg+2HvjymtunzJbOifzzcbgkxv+7EyVBeihAmYj/t3w7XswGNQ\nB9Xj4rI/\n-----END CERTIFICATE-----";
    e = RestCertificateDataNew(&pCertificate2, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    pCertificateArray[0] = pCertificate1;
    pCertificateArray[1] = pCertificate2;

    // create certificate array object
    e = RestCertificateArrayDataNew(
        &pCertificates,
        pCertificateArray,
        sizeof(pCertificateArray) / sizeof(REST_CERTIFICATE_DATA*));
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate1);
    RestCertificateDataDelete(pCertificate2);

    e = IdmIdentityProviderDataNew(
        &pIdentityProvider,
        domainType,
        provider,
        alias,
        type,
        authenticationType,
        friendlyName,
        &searchTimeOutInSeconds,
        username,
        password,
        &machineAccount,
        servicePrincipalName,
        userBaseDN,
        groupBaseDN,
        pConnectionStrings,
        pAttributesMap,
        pUpnSuffixes,
        pSchema,
        &matchingRuleInChainEnabled,
        &baseDnForNestedGroupsEnabled,
        &directGroupsSearchEnabled,
        &siteAffinityEnabled,
        pCertificates);
    BAIL_ON_ERROR(e);

    IdmStringArrayDataDelete(pConnectionStrings);
    IdmStringArrayDataDelete(pUpnSuffixes);
    IdmStringMapDataDelete(pAttributesMap);
    IdmSchemaObjectMappingMapDataDelete(pSchema);
    RestCertificateArrayDataDelete(pCertificates);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmIdentityProviderUpdate(pRestClient, testTenant, provider, pIdentityProvider, &pIdentityProviderReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmIdentityProviderDataDelete(pIdentityProviderReturn);
    IdmIdentityProviderDataDelete(pIdentityProvider);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmIdentityProviderDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmIdentityProviderDelete(pRestClient, testTenant, provider, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
