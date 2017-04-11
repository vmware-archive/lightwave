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

PCSTRING
IdmTenantCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;
    IDM_TENANT_DATA* pTenantReturn = NULL;

    IDM_PRIVATE_KEY_DATA* pPrivateKey = NULL;
    PCSTRING privateKeyEncoded =
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC9d3lwlGVq1PXeIeHLid5O67+7/AX3S5txhI6fqvMA5Il/ddctRKH6E34fW+xaBqB44BsulLbe7CJZ2idu7S4EvPIZy+HG2ZOyLgZv5m8GVZC7A1QR52aqlEMBIS2bbcOUxDoLybOCbYslsXobm2TOAhW4hY4tDJo1lQYSO0cqOSqd1t6Oq4tWmk1dSVFXXstmx8F23fZYPuIUfYfDaLDOaXGucxOKhej3bMonUbgh6m2pt7LpNEDvRpVLLi+yD3SPbyqShGRFMR81gIEgBZNuaLO34nGFx8IcB19K2WaYuqyc3qSQoGote/dpRZxqstQg1VDlCy2y//nON8gTvhXfAgMBAAECggEBALc2pzpUZdXu5WrOfNgkE9NhaWFRDjva4w/czUZdOutsFAIrY86khbWbz+RnMaDEs7zviZl7uXLMCVO1/1fnBw8R31NOsp9lmss8coEW5dnMOE5/o3ZEvCTLhhe7i6y3cd+T+UWY4HuJjnr+qkOz+sCzh2mNzLSHcT/LQbmzlNegEPtLzuMufnkEYnklNWwDdJi/NNddDwAWaQWj+taEXiOdSl+howPYNcWe6IijEqiuzET2KDVq/DbTfBYKfHP/8wbkCqyfPaRkT0H7/n98ztn83vpxvmOkFXal1F2fHCKvNEbB8ewqMztxEIz+HK5/LVrcR5N0Azx2s3FIEAbG8PECgYEA7f8N2IVKu1BZ3CjGIcww00CEa/fTRMuY0u+d9jKAmsa/7wBq4HgW2ZTJBSROoo9NRdjKkGPHJ8tcpjpCm2XcwLL5mYvI8OhkTD5SEcB3do3YZ7hqi5SbNCPZ9DaORT3e7pedRXOw4p66R+Glr02j3j4mhxPsQIBqHqEOfVma7dMCgYEAy8ydRmHOOmtGxSf5nzT9L4vn8AmfJOWmSfmrtRqJNA9KgmCN3URM+euHJy6LHRZSACg8DdlD9AapL1QcdJPTNmP7BH9BZEW6Amuo4PdnNf/vhTly1l5DX5OlGMnfI5PVlQkqKnQZiOiFexavdJaTWh8sANw/TfM2ud3eIKHKlEUCgYA/NpJ34xdxXysu9cmCapjBU46Yms+Lo0QpKqnbHZjZA1cxZPv+OQdgrUsjSXx1YviUR7ut1EKuiC5InIraeZjDugXfyagKjL8vHRJxUpyoaY6EJhBumH8Mv0UBv4fUTlWrK6wDKXJfDufNddqrEEUpH3reP+VtUG5fSknt82HaLwKBgHsA5U/sii8iPlWSmgvoTIPc+kEbXY1Eekgdw/ALsxHTxNHJ+vW4WolhCXKxmc8VgKqNnilxn5zyRDzHlGEcM5eZYpDFSa9+khUR65zUVdv2vBb5pIET5bATCctbVC1B9d/85xE2f47tYFr5Ry6kDw2O0N04EZE3oSkvavjhDcbNAoGBAMtNujGnMlqljr9UqvXBJbGT8lt75BK/lw2UlWuywTrzSzMqeVcjqMdw1+7nI9vWquCmanhD6tan72Li+9ftu6FXjowKv4yKhTfJLuMS0X284fWz2C9Gway0VrZzeCEGyVrYAreDwo8anciWMSzDUaPFywdUATtK4HPiONIU+kwH";
    PCSTRING privateKeyAlgorithm = "RSA";

    REST_CERTIFICATE_DATA* pCertificate1 = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    REST_CERTIFICATE_DATA* pCertificate2 = NULL;

    REST_CERTIFICATE_DATA* pCertificateArray[] = {NULL, NULL};
    REST_CERTIFICATE_ARRAY_DATA* pCertificates = NULL;

    IDM_TENANT_CREDENTIALS_DATA* pTenantCredentials = NULL;

    IDM_TENANT_DATA* pTenant = NULL;
    PCSTRING longName = "test_tenant_long_name";
    PCSTRING key = "test_tenant_key";
    PCSTRING guid = "test_tenant_guid";
    PCSTRING issuer = "test_tenant_issuer";
    PCSTRING username = "administrator@vsphere.local";
    PCSTRING password = "Admin!23";

    // create private key
    e = IdmPrivateKeyDataNew(&pPrivateKey, privateKeyEncoded, privateKeyAlgorithm);
    BAIL_ON_ERROR(e);

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

    // create tenant credentials object
    e = IdmTenantCredentialsDataNew(&pTenantCredentials, pPrivateKey, pCertificates);
    BAIL_ON_ERROR(e);

    IdmPrivateKeyDataDelete(pPrivateKey);
    RestCertificateArrayDataDelete(pCertificates);

    // create tenant
    e = IdmTenantDataNew(&pTenant, testTenant, longName, key, guid, issuer, pTenantCredentials, username, password);
    BAIL_ON_ERROR(e);

    IdmTenantCredentialsDataDelete(pTenantCredentials);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, systemTenant, systemTenantUsername, systemTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantCreate(pRestClient, pTenant, &pTenantReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmTenantDataDelete(pTenantReturn);
    IdmTenantDataDelete(pTenant);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_TENANT_DATA* pTenantReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantGet(pRestClient, testTenant, &pTenantReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmTenantDataDelete(pTenantReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantGetByHOKClientTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestHOKTokenClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_TENANT_DATA* pTenantReturn = NULL;

    e = RestTestHOKTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestHOKTokenClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantGet(pRestHOKTokenClient, testTenant, &pTenantReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestHOKTokenClient);
    IdmTenantDataDelete(pTenantReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantGetByHAClientTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_TENANT_DATA* pTenantReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, true, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantGet(pRestClient, testTenant, &pTenantReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmTenantDataDelete(pTenantReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantDelete(pRestClient, testTenant, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantGetConfigTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_TENANT_CONFIGURATION_DATA* pTenantConfigurationReturn = NULL;

    IDM_TENANT_CONFIG_TYPE type1 = IDM_TENANT_CONFIG_TYPE_PASSWORD;
    IDM_TENANT_CONFIG_TYPE type2 = IDM_TENANT_CONFIG_TYPE_LOCKOUT;
    IDM_TENANT_CONFIG_TYPE type3 = IDM_TENANT_CONFIG_TYPE_TOKEN;
    IDM_TENANT_CONFIG_TYPE type4 = IDM_TENANT_CONFIG_TYPE_PROVIDER;
    IDM_TENANT_CONFIG_TYPE type5 = IDM_TENANT_CONFIG_TYPE_BRAND;
    IDM_TENANT_CONFIG_TYPE type6 = IDM_TENANT_CONFIG_TYPE_AUTHENTICATION;
    IDM_TENANT_CONFIG_TYPE* pType[] =
    {
        &type1,
        &type2,
        &type3,
        &type4,
        &type5,
        &type6
    };
    IDM_TENANT_CONFIG_TYPE_ARRAY tenantConfigTypes =
    {
        pType,
        sizeof(pType) / sizeof(IDM_TENANT_CONFIG_TYPE*)
    };

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantGetConfig(pRestClient, testTenant, &tenantConfigTypes, &pTenantConfigurationReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmTenantConfigurationDataDelete(pTenantConfigurationReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantUpdateConfigTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_TENANT_CONFIGURATION_DATA* pTenantConfigurationReturn = NULL;

    IDM_PASSWORD_POLICY_DATA* pPasswordPolicy = NULL;
    PCSTRING description = "PasswordPolicy description";
    INTEGER maxIdenticalAdjacentCharacters = 3;
    INTEGER maxLength = 20;
    INTEGER minAlphabeticCount = 2;
    INTEGER minLength = 8;
    INTEGER minLowercaseCount = 1;
    INTEGER minNumericCount = 1;
    INTEGER minSpecialCharCount = 1;
    INTEGER minUppercaseCount = 1;
    INTEGER passwordLifetimeDays = 90;
    INTEGER prohibitedPreviousPasswordCount = 5;

    IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy = NULL;
    SSO_LONG failedAttemptIntervalSec = 1;      // SSO_LONG seems not supported in IDM.
    INTEGER maxFailedAttempts = 2;
    SSO_LONG autoUnlockIntervalSec = 3;

    IDM_TOKEN_POLICY_DATA* pTokenPolicy = NULL;
    SSO_LONG clockToleranceMillis = 600000;
    INTEGER delegationCount = 10;
    SSO_LONG maxBearerTokenLifeTimeMillis = 30000000;
    SSO_LONG maxHOKTokenLifeTimeMillis = 259200000000;
    SSO_LONG maxBearerRefreshTokenLifeTimeMillis = 2160000000;
    SSO_LONG maxHOKRefreshTokenLifeTimeMillis = 259200000000;
    INTEGER renewCount = 10;

    IDM_PROVIDER_POLICY_DATA* pProviderPolicy = NULL;
    PSTRING defaultProvider = "defaultProvider";
    PSTRING defaultProviderAlias = "defaultProviderAlias";
    bool providerSelectionEnabled = true;

    IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration = NULL;

    e = IdmPasswordPolicyDataNew(
        &pPasswordPolicy,
        description,
        &maxIdenticalAdjacentCharacters,
        &maxLength,
        &minAlphabeticCount,
        &minLength,
        &minLowercaseCount,
        &minNumericCount,
        &minSpecialCharCount,
        &minUppercaseCount,
        &passwordLifetimeDays,
        &prohibitedPreviousPasswordCount);
    BAIL_ON_ERROR(e);

    description = "LockoutPolicy description";
    e = IdmLockoutPolicyDataNew(
        &pLockoutPolicy,
        description,
        &failedAttemptIntervalSec,
        &maxFailedAttempts,
        &autoUnlockIntervalSec);
    BAIL_ON_ERROR(e);

    e = IdmTokenPolicyDataNew(
        &pTokenPolicy,
        &clockToleranceMillis,
        &delegationCount,
        &maxBearerTokenLifeTimeMillis,
        &maxHOKTokenLifeTimeMillis,
        &maxBearerRefreshTokenLifeTimeMillis,
        &maxHOKRefreshTokenLifeTimeMillis,
        &renewCount);
    BAIL_ON_ERROR(e);

    e = IdmProviderPolicyDataNew(&pProviderPolicy, defaultProvider, defaultProviderAlias, &providerSelectionEnabled);
    BAIL_ON_ERROR(e);

    e = IdmTenantConfigurationDataNew(
        &pTenantConfiguration,
        pPasswordPolicy,
        pLockoutPolicy,
        pTokenPolicy,
        pProviderPolicy,
        NULL,
        NULL);
    BAIL_ON_ERROR(e);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmTenantUpdateConfig(pRestClient, testTenant, pTenantConfiguration, &pTenantConfigurationReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmTenantConfigurationDataDelete(pTenantConfigurationReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmTenantSearchTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PCSTRING domain = "test_tenant_name";
    PCSTRING query = "Administrator";
    IDM_MEMBER_TYPE memberType = IDM_MEMBER_TYPE_ALL;
    IDM_SEARCH_TYPE searchBy = IDM_SEARCH_TYPE_NAME;
    size_t limit = 10;
    IDM_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    IdmTenantSearch(pRestClient, testTenant, domain, query, memberType, searchBy, limit, &pSearchResultReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmSearchResultDataDelete(pSearchResultReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
