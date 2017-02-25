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

static PCSTRING name = "test_solution_user_rest_c_client";

PCSTRING
IdmSolutionUserGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmSolutionUserGet(pRestClient, testTenant, name, &pSolutionUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmSolutionUserDataDelete(pSolutionUserReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirSolutionUserCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;

    PSTRING description = "description";

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;

    REST_CERTIFICATE_DATA* pCertificate = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDoDCCAoigAwIBAgIJAM7FLbqdb8JNMA0GCSqGSIb3DQEBCwUAMH4xCzAJBgNV\nBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMGCgmSJomT8ixkARkW\nBWxvY2FsMQswCQYDVQQGEwJVUzEyMDAGA1UECgwpc2MtcmRvcHMtdm0wNC1kaGNw\nLTExOS0xMTIuZW5nLnZtd2FyZS5jb20wHhcNMTYwMTEyMTczMzA3WhcNMjYwMTA2\nMTc0MjUyWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\nAQEFAAOCAQ8AMIIBCgKCAQEAvXd5cJRlatT13iHhy4neTuu/u/wF90ubcYSOn6rz\nAOSJf3XXLUSh+hN+H1vsWgageOAbLpS23uwiWdonbu0uBLzyGcvhxtmTsi4Gb+Zv\nBlWQuwNUEedmqpRDASEtm23DlMQ6C8mzgm2LJbF6G5tkzgIVuIWOLQyaNZUGEjtH\nKjkqndbejquLVppNXUlRV17LZsfBdt32WD7iFH2Hw2iwzmlxrnMTioXo92zKJ1G4\nIeptqbey6TRA70aVSy4vsg90j28qkoRkRTEfNYCBIAWTbmizt+JxhcfCHAdfStlm\nmLqsnN6kkKBqLXv3aUWcarLUINVQ5Qstsv/5zjfIE74V3wIDAQABo4GGMIGDMAsG\nA1UdDwQEAwIF4DA0BgNVHREELTArgilzYy1yZG9wcy12bTA0LWRoY3AtMTE5LTEx\nMi5lbmcudm13YXJlLmNvbTAdBgNVHQ4EFgQUXqkBMn1AvOw/E4VFu6dxNJv17MYw\nHwYDVR0jBBgwFoAU5/f1UhAXyqTFJupgdGRRYeUwYcQwDQYJKoZIhvcNAQELBQAD\nggEBAJMZ7HI2zVqkchremBWi5TmasQGeAuzAOlVCGUlvSzY26dUKB6DEbvPA969U\nfGmcuJIt/cLmePeHD7bS0gYkfq0QPVp1aD3cZTeCzgSEslNzB73KPmx8ZKz7tMn/\nWTH74tvOPY1whCZfv1WwdKSNplMVycepdKFMRXYPy2RavhWMUriycvDrZUr4Klti\nvBW/M2HnsWhmibTn5hUK7geyXsS4YMaajYhfTiMAapHWf/IYe1HyMmD1K+DljkIU\n1IyMnJhWM8H/fDqzSDXTDPsThlGN9L4bz/Ij328G9jS60JCfwiHsU11zr7Au9ySL\n1cmkk5GsG0q+xLXOPmvfgOH3Y30=\n-----END CERTIFICATE-----";

    bool disabled = true;
    PSTRING objectId = "objectId";

    VMDIR_SOLUTION_USER_DATA* pSolutionUser = NULL;

    PCSTRING domain = testTenant;

    e = VmdirPrincipalDataNew(&pAlias, "name", "domain");
    BAIL_ON_ERROR(e);

    e = RestCertificateDataNew(&pCertificate, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserDataNew(&pSolutionUser, name, domain, description, pAlias, pCertificate, &disabled, objectId);
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate);

    VmdirPrincipalDataDelete(pAlias);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserCreate(pRestClient, testTenant, pSolutionUser, &pSolutionUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirSolutionUserDataDelete(pSolutionUserReturn);
    VmdirSolutionUserDataDelete(pSolutionUser);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirSolutionUserDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserDelete(pRestClient, testTenant, name, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirSolutionUserGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserGet(pRestClient, testTenant, name, &pSolutionUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirSolutionUserDataDelete(pSolutionUserReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirSolutionUserUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;

    PSTRING description = "description";

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;

    REST_CERTIFICATE_DATA* pCertificate = NULL;
    PSTRING pCertificateEncoded =
        "-----BEGIN CERTIFICATE-----\nMIIDnjCCAoagAwIBAgIJAOValQs66yOrMA0GCSqGSIb3DQEBCwUAMH0x\nCzAJBgNVBAMMAkNBMRcwFQYKCZImiZPyLGQBGRYHdnNwaGVyZTEVMBMG\nCgmSJomT8ixkARkWBWxvY2FsMQswCQYDVQQGEwJVUzExMC8GA1UECgwo\nc2MtcmRvcHMtdm0wOC1kaGNwLTIzMC00Ni5lbmcudm13YXJlLmNvbTAe\nFw0xNTAxMDYyMTQ4MjdaFw0yNDEyMzEyMTU4MTNaMBgxFjAUBgNVBAMM\nDXNzb3NlcnZlclNpZ24wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\nAoIBAQCi9yZSituevZoQzfqr/n+QyPWWUn2b2kiIn9Q/pWlZdaSTyMn9\nnsFZvxUFxFz+klqPyoqWP/++4BffEzClByMEm4mdSfhsSOaIAZ6LVJxR\n+38Cy0LT3YsHcPMGtPmk7Olxel+R2lmTr+1a7K56/CY15LzSR214JaTt\neAJ5XFgud8yqjOEFx/R2fkKS7phbGvvTEtfFPXEVGXe02Wc3UstuY8uV\nImnhF2xYud8LjgAQXqGHae2Jf08AlphD5FpU33GQcQMnIVto3AG6iFT/\nKj6Btfa8DfcVWCUB2w3G52KU0ndMrewBL5XzAWfle5JQMDup8vDxdAcI\nQ87iLsygB/XtAgMBAAGjgYUwgYIwCwYDVR0PBAQDAgXgMDMGA1UdEQQs\nMCqCKHNjLXJkb3BzLXZtMDgtZGhjcC0yMzAtNDYuZW5nLnZtd2FyZS5j\nb20wHQYDVR0OBBYEFNQH1ogZAwUFGP6PbJB5LJw/CQbtMB8GA1UdIwQY\nMBaAFN3YC312m4W23Uu2kzKP/zzPuewpMA0GCSqGSIb3DQEBCwUAA4IB\nAQCtBR+Hl1ttC9Q/CucM8//cDygpJzVMJdc5ZP6jO50twQ93ssTSTHTb\na21yk7M9ilJKBc2J5mnkps1il//goAFniAIHNxKmArrYcMlrItJWJZ5s\nt3osjaiLwj/RxQZg4oHfCXsCdaY+hIPjMMam1covOdJazsUMYdL1Wdbx\nU71VccERxRuO5k3OmIS+nWcCS3z/7DV7UgXvnNScmtFTUDp3Vxs/4viS\nMgrLppXIqkISpDboPklQPiJbiRZHd09a2jJL4tXU7K0NvwVBJfjPwBKr\nmj36c1dg+2HvjymtunzJbOifzzcbgkxv+7EyVBeihAmYj/t3w7XswGNQ\nB9Xj4rI/\n-----END CERTIFICATE-----";

    bool disabled = true;
    PSTRING objectId = "objectId";

    VMDIR_SOLUTION_USER_DATA* pSolutionUser = NULL;

    PCSTRING domain = testTenant;

    e = VmdirPrincipalDataNew(&pAlias, "name", "domain");
    BAIL_ON_ERROR(e);

    e = RestCertificateDataNew(&pCertificate, pCertificateEncoded);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserDataNew(&pSolutionUser, name, domain, description, pAlias, pCertificate, &disabled, objectId);
    BAIL_ON_ERROR(e);

    RestCertificateDataDelete(pCertificate);

    VmdirPrincipalDataDelete(pAlias);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserUpdate(pRestClient, testTenant, name, pSolutionUser, &pSolutionUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirSolutionUserDataDelete(pSolutionUserReturn);
    VmdirSolutionUserDataDelete(pSolutionUser);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
