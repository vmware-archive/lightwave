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

#include "test_cases.h"

static POIDC_CLIENT             s_pClient = NULL;
static PCSTRING                 s_pszServer = NULL;
static PCSTRING                 s_pszTenant = NULL;
static PCSTRING                 s_pszUsername = NULL;
static PCSTRING                 s_pszPassword = NULL;

static const PCSTRING s_pszIDTokenWithGroups = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJpc3MiOiJodHRwczpcL1wvc2MtcmRvcHMtdm0xOC1kaGNwLTYyLTY0LmVuZy52bXdhcmUuY29tXC9vcGVuaWRjb25uZWN0XC92c3BoZXJlLmxvY2FsIiwiZ3JvdXBzIjpbInZzcGhlcmUubG9jYWxcXFVzZXJzIiwidnNwaGVyZS5sb2NhbFxcQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxDQUFkbWlucyIsInZzcGhlcmUubG9jYWxcXENvbXBvbmVudE1hbmFnZXIuQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxTeXN0ZW1Db25maWd1cmF0aW9uLkJhc2hTaGVsbEFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcU3lzdGVtQ29uZmlndXJhdGlvbi5BZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXExpY2Vuc2VTZXJ2aWNlLkFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcRXZlcnlvbmUiXSwidG9rZW5fY2xhc3MiOiJpZF90b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJnaXZlbl9uYW1lIjoiQWRtaW5pc3RyYXRvciIsImF1ZCI6ImFkbWluaXN0cmF0b3JAdnNwaGVyZS5sb2NhbCIsInNjb3BlIjoicnNfYWRtaW5fc2VydmVyIGF0X2dyb3VwcyBvcGVuaWQgaWRfZ3JvdXBzIiwiZXhwIjoxNDc3NTI2NTEwLCJpYXQiOjE0Nzc1MjYyMTAsImZhbWlseV9uYW1lIjoidnNwaGVyZS5sb2NhbCIsImp0aSI6IjlqZUpQXzg2dmt1QnhNMl96Z0s5dXBENDFzRms1cXRZbUVnMTNWRTdHSk0iLCJ0ZW5hbnQiOiJ2c3BoZXJlLmxvY2FsIn0.PE4ddqkx6sly9J6wfV9ZcquYp-0xrSBPSP_toZe6v4PP9DS0zGJDJvmgK7uEKJKyCSEIGe0F9IfZXhWW4fbh7ishztiqY8U9utPB01ciPotA7uLTn8jJsydZQt70IoqCh1zhwEXmr1MfOLTtM5uPn3BlHoZREdeLoQ7BLgbwNwcl3j4SDZQKqOQCzIXwVA4KBg20cVqLoy_hxbO2ri3WpzfFbgs5xV5hIL-8FUldW2AZ0rp-PGu7CUxwYPLVSyuwEOjI8_IaUv6M7YSXDILNBNqSstNturDlDg6utw2P-wSZnsHL1fvj9hJ3u8uWVu-vevwwgHSte4swxr0P0LOTGA";
static const PCSTRING s_pszIDTokenWithoutGroups = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJhdWQiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJzY29wZSI6InJzX2FkbWluX3NlcnZlciBvcGVuaWQiLCJpc3MiOiJodHRwczpcL1wvc2MtcmRvcHMtdm0xOC1kaGNwLTYyLTY0LmVuZy52bXdhcmUuY29tXC9vcGVuaWRjb25uZWN0XC92c3BoZXJlLmxvY2FsIiwidG9rZW5fY2xhc3MiOiJpZF90b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJleHAiOjE0Nzc1MjY0NzMsImdpdmVuX25hbWUiOiJBZG1pbmlzdHJhdG9yIiwiaWF0IjoxNDc3NTI2MTczLCJmYW1pbHlfbmFtZSI6InZzcGhlcmUubG9jYWwiLCJqdGkiOiItS1RUODNoX0pJbXNKWTVTeTBSS2M0aVlWbEJyc1ZVNlI3ek9yZWJURElNIiwidGVuYW50IjoidnNwaGVyZS5sb2NhbCJ9.P0s2Nr_IvrJ_5TFcyRbFeuIz0vthSdo0rQKYtaXjfaU0qQcB318xeMVOsFGGoxxvZaNE7iGBA61K-LARUJ1MBvw1LqthSBRi8lyplJNEG3xByS-kuBNyjdXIx3FdGf7fpDLQqKYih8SSehckMT7Rqwdms5yPMq816Nw0Ctx7FpOl1Md4Enosp9DQ-CMjOM9ceoFE7Rg8JQmOHTHWa6Io_d253gZThUafvsBuAD00INTZjanqmiJXYyQkbisnDYX7S8cIv8-Rqg7IP_tFwSJ6F8F9EKlff56MPs2cSvUastjOLrJxqMCvLFjM2IheqAlp-d03GjidlRzN-M4vXGiOlQ";
static const PCSTRING s_pszAccessTokenWithGroups = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJhdWQiOlsiYWRtaW5pc3RyYXRvckB2c3BoZXJlLmxvY2FsIiwicnNfYWRtaW5fc2VydmVyIl0sInNjb3BlIjoicnNfYWRtaW5fc2VydmVyIGF0X2dyb3VwcyBvcGVuaWQgaWRfZ3JvdXBzIiwiaXNzIjoiaHR0cHM6XC9cL3NjLXJkb3BzLXZtMTgtZGhjcC02Mi02NC5lbmcudm13YXJlLmNvbVwvb3BlbmlkY29ubmVjdFwvdnNwaGVyZS5sb2NhbCIsImdyb3VwcyI6WyJ2c3BoZXJlLmxvY2FsXFxVc2VycyIsInZzcGhlcmUubG9jYWxcXEFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcQ0FBZG1pbnMiLCJ2c3BoZXJlLmxvY2FsXFxDb21wb25lbnRNYW5hZ2VyLkFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcU3lzdGVtQ29uZmlndXJhdGlvbi5CYXNoU2hlbGxBZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXFN5c3RlbUNvbmZpZ3VyYXRpb24uQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxMaWNlbnNlU2VydmljZS5BZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXEV2ZXJ5b25lIl0sInRva2VuX2NsYXNzIjoiYWNjZXNzX3Rva2VuIiwidG9rZW5fdHlwZSI6IkJlYXJlciIsImV4cCI6MTQ3NzUyNjUxMCwiaWF0IjoxNDc3NTI2MjEwLCJqdGkiOiJ0YlhFcXNaSExZbWU0WV9KRS1tenNuWmxPNkpNQ2FiSXlnb2VhMkNoX1c4IiwidGVuYW50IjoidnNwaGVyZS5sb2NhbCIsImFkbWluX3NlcnZlcl9yb2xlIjoiQWRtaW5pc3RyYXRvciJ9.qPA4DBtYuMXlMgUex-icX9VHqJN02CwIMtqkcUPEj_BIDe8JNF_axOy8WjKQOcw2bLW2RxMjVB-zRWgxchRnbebSQl-BIPO6t7wLHLdh-sz2mvmSUbM2qAdLTmQjuTjbVZ2Nutz84DrwLqeHP6F_Z0ln5SSpCOaH5zPoRufJvtC6nWCNm9w7RmwT7yK9WuZjMti6VarUYs0stOJhEkVdLmgNkXdqVe4f2yaOO02t9luspKzmQ-O61u_3zIqtpsxRExrptQ73Yguax541JJ0Pyd-8wZYiw9E7isA62Um5dDzmNkFhfjOdWhh0WZ2bVJONG7Aep_SlukTMk473hRg3_Q";
static const PCSTRING s_pszAccessTokenWithoutGroups = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJhdWQiOlsiYWRtaW5pc3RyYXRvckB2c3BoZXJlLmxvY2FsIiwicnNfYWRtaW5fc2VydmVyIl0sInNjb3BlIjoicnNfYWRtaW5fc2VydmVyIG9wZW5pZCIsImlzcyI6Imh0dHBzOlwvXC9zYy1yZG9wcy12bTE4LWRoY3AtNjItNjQuZW5nLnZtd2FyZS5jb21cL29wZW5pZGNvbm5lY3RcL3ZzcGhlcmUubG9jYWwiLCJ0b2tlbl9jbGFzcyI6ImFjY2Vzc190b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJleHAiOjE0Nzc1MjY0NzMsImlhdCI6MTQ3NzUyNjE3MywianRpIjoiTG00MzJPQ2VBYVZtdnZPMkZ2X0R6VkFQMEdKWjJaVWRkQ0NaMGlZUFNNcyIsInRlbmFudCI6InZzcGhlcmUubG9jYWwiLCJhZG1pbl9zZXJ2ZXJfcm9sZSI6IkFkbWluaXN0cmF0b3IifQ.HpWsmEAcDoFz1YHZQzvN2JkB0TGWY0QCmBXS8l9V80jumoI1IWe_lOGobW0d-BXxPzXcNhEgCULwO7Vjfkcltcj5Juytx_NFE3iYD0j1s3A7890LHthInNeqDnQtmjWKCSnguCWKZE5bcIhkHknLTe-IIm2oSCzOn8NolGEmpgHn3mnJrOQyn0Mt8DN0iLeOSs_m-vnj9p6ZepiD4y9F1-uqSNJWIdFfVsWwZyF5l4sfApjVQZJlzDlHaKTtIxf7aItiiKR-5X6luPvhv0sJe5x67NkwIEpk0pAqVOLpElRUHkxEOrPZgwnZzpvwVpNXsy6UhfOXBnOpnGQV_ACD-A";
static const PCSTRING s_pszSigningCertificatePEM = "-----BEGIN CERTIFICATE-----\nMIIDzzCCAregAwIBAgIJAN3wKoGrohUNMA0GCSqGSIb3DQEBCwUAMIGu\nMQswCQYDVQQDDAJDQTEXMBUGCgmSJomT8ixkARkWB3ZzcGhlcmUxFTAT\nBgoJkiaJk/IsZAEZFgVsb2NhbDELMAkGA1UEBhMCVVMxEzARBgNVBAgM\nCkNhbGlmb3JuaWExMDAuBgNVBAoMJ3NjLXJkb3BzLXZtMTgtZGhjcC02\nMi02NC5lbmcudm13YXJlLmNvbTEbMBkGA1UECwwSVk13YXJlIEVuZ2lu\nZWVyaW5nMB4XDTE2MTAyMDE3Mjk0MVoXDTI2MTAxNTE3MzkyNFowGDEW\nMBQGA1UEAwwNc3Nvc2VydmVyU2lnbjCCASIwDQYJKoZIhvcNAQEBBQAD\nggEPADCCAQoCggEBANfcD1DjMhqoMwv5lCFGnQOso3sXTDHCyPDemOVL\n8Mch3rImiaKa1Tnx814mhSqP8GSOA5hmHzytzqYQBKYLGnpeF41ku2Ba\n+GwBm/1wTpjRhzqlRYtg8iggfZve6DzW7PqTLHcICC4hx8Xuoy2bwolM\nssTlczyKKtuX3Ys64FFPjse9u8lpAcVAIDbj92QsNVSzrB6fF87L3hFg\nK9VgYdcwDpp2LpXe+MZ1IV26Lpzz1blOqhrBBZxyRwjS8ovq1eS/e6hM\nruL6PCfik+EAUbb0rJ0A29utjDTWR3rst6gxHIGHaZF/anKC6ctQWt/U\nD1Nys6nVRQkvnxpc4tlW9PsCAwEAAaOBhDCBgTALBgNVHQ8EBAMCBeAw\nMgYDVR0RBCswKYInc2MtcmRvcHMtdm0xOC1kaGNwLTYyLTY0LmVuZy52\nbXdhcmUuY29tMB0GA1UdDgQWBBRoINBDReKwKk0MmSlH8RZsJkXgGzAf\nBgNVHSMEGDAWgBRbVxEsqv9GgoPYrqpm1u0YeSX2+TANBgkqhkiG9w0B\nAQsFAAOCAQEAMS7KvsRsmIrIUyqQeUK6d9IwVBi/koPeLNXgPReFxHHq\nHFcpMD7xlMqwxEzY2ZiA4C6pHnS8xGAYRhh0m7Ba9S61Ty65GBFgNSVg\nnhPFh/XRngofbVJAwN1EuLv3nOrDma52S3q677jub06JpEm1cunvJ5Ya\nRwIXwGOLXsUOVfS2Cm2Z0g250Qi/SDJw4dKwG05huCXEqhBuRuE3VN+O\nQyUnYQ4mh0kM1j351afuGyhLJ8X0J6YLMRlE/txh9b8ewDDw3uzf3pcW\nvak8I+T3LpxeH7//s8DBpggIucPttuzhPzmwM7ZoTutpy4xn5Bh2jiLA\nK6sDrnZEN/xDqywDYA==\n-----END CERTIFICATE-----";
static const PCSTRING s_pszWrongCertificatePEM = "-----BEGIN CERTIFICATE-----\nMIIDhjCCAm4CCQCJFCY/GQuKxzANBgkqhkiG9w0BAQUFADCBhDETMBEGA1UEAwwK\ndm13YXJlLmNvbTEPMA0GA1UECgwGVk1XYXJlMQwwCgYDVQQLDANTU08xDTALBgNV\nBAcMBEtpbmcxEzARBgNVBAgMCldhc2hpbmd0b24xCzAJBgNVBAYTAlVTMR0wGwYJ\nKoZIhvcNAQkBFg5zc29Adm13YXJlLmNvbTAeFw0xNjEwMjExOTA1MDJaFw0xNzEw\nMjExOTA1MDJaMIGEMRMwEQYDVQQDDAp2bXdhcmUuY29tMQ8wDQYDVQQKDAZWTVdh\ncmUxDDAKBgNVBAsMA1NTTzENMAsGA1UEBwwES2luZzETMBEGA1UECAwKV2FzaGlu\nZ3RvbjELMAkGA1UEBhMCVVMxHTAbBgkqhkiG9w0BCQEWDnNzb0B2bXdhcmUuY29t\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqvqORpuhbNW4HUumNAr9\ndpY3SonTVvo7IDWxt/BtlG/Rce+9E+xyqgF66LA+XyNaSpciv0z/VkOl3eUZIxkP\nj1KdkmrKPyjbhDrbDLrXxbOBYEITtGg59NXOLb9J91EHH1F/oHhGywA6m61Ce//z\n2QD1UPm8BJkdGe918OfyhTS4MjXlce564GejwvX1O0pliprncng3rUwNaSM4g0Zv\n+PNQ2my2HuWMrbPJ2A51t0bhRG/X3cZyqiO/w+eszt/eRGe74NCmcYrY2B6NtHtq\n6YKvYv95NAiXHsYTcydATEwhVVEIuyunDipr/FgBgRkQdqGuah6pNgKqQPY+H58B\nSwIDAQABMA0GCSqGSIb3DQEBBQUAA4IBAQCenaBW2cRmoaaHMet9cxTvh8hvX58v\nDRodhnJtx9FoyZ76IkC5aky+ElHUVDM29DHcJO1CveyMXNve+0OdwYY4hAfO0cK8\nMYgQiLT39HZ7WSQ60OZ1Iv8W1wpyhIg7cbmOSkZfUSXIis7bk+WKQMlxj8LQx6jH\nEjunn8dFPkBaT5ZKBOYijgdDjRJslumw+lKQeoOn/ZkyYGGO3olg3jVgOZB5eIqf\nhTfMmPwvCV/sS9RXFjncSkNHDu6sQK6lIB5v/NdPjCHnMAktMqb9Z+5mENCvO+cG\nuxRVbOCas3H2HR6NaBHcbNdUI0c7KNzSd8NWmsTNNGMKfZGP6FGcFjkU\n-----END CERTIFICATE-----";
static const SSO_LONG CLOCK_TOLERANCE_IN_SECONDS = 5 * 60L; // 5 mins

SSOERROR
TestInit(
    PCSTRING pszServer,
    PCSTRING pszTenant,
    PCSTRING pszUsername,
    PCSTRING pszPassword,
    bool highAvailabilityEnabled)
{
    SSOERROR e = SSOERROR_NONE;

    s_pszServer = pszServer;
    s_pszTenant = pszTenant;
    s_pszUsername = pszUsername;
    s_pszPassword = pszPassword;

    e = OidcClientGlobalInit();
    BAIL_ON_ERROR(e);

    e = OidcClientBuild(&s_pClient, highAvailabilityEnabled ? NULL : s_pszServer, 443, s_pszTenant, CLOCK_TOLERANCE_IN_SECONDS);
    BAIL_ON_ERROR(e);

error:

    return e;
}

void
TestCleanup()
{
    OidcClientDelete(s_pClient);
    OidcClientGlobalCleanup();
}

bool
TestPasswordGrantSuccessResponse()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;
    PCOIDC_ID_TOKEN pIDToken = NULL;
    POIDC_ACCESS_TOKEN pAccessToken = NULL;
    PSTRING pszStringClaim = NULL;
    const PSTRING* ppszAudience = NULL;
    size_t audienceSize = 0;

    e = OidcClientAcquireTokensByPassword(
        s_pClient,
        s_pszUsername,
        s_pszPassword,
        "openid offline_access",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    TEST_ASSERT_SUCCESS(e);

    pIDToken = OidcTokenSuccessResponseGetIDToken(pTokenSuccessResponse);
    TEST_ASSERT_TRUE(OidcIDTokenGetIssuer(pIDToken) != NULL);
    TEST_ASSERT_EQUAL_STRINGS(s_pszUsername, OidcIDTokenGetSubject(pIDToken));

    OidcIDTokenGetAudience(pIDToken, &ppszAudience, &audienceSize);
    TEST_ASSERT_TRUE(ppszAudience != NULL);
    TEST_ASSERT_TRUE(audienceSize == 1);
    TEST_ASSERT_EQUAL_STRINGS(s_pszUsername, ppszAudience[0]);

    TEST_ASSERT_TRUE(OidcIDTokenGetIssueTime(pIDToken) > 0);
    TEST_ASSERT_TRUE(OidcIDTokenGetExpirationTime(pIDToken) > 0);

    e = OidcIDTokenGetStringClaim(pIDToken, "token_class", &pszStringClaim);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("id_token", pszStringClaim);
    SSOStringFree(pszStringClaim);

    e = OidcAccessTokenBuild(
        &pAccessToken,
        OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse),
        OidcClientGetSigningCertificatePEM(s_pClient),
        "issuer",
        NULL, // pszResourceServerName
        CLOCK_TOLERANCE_IN_SECONDS);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(OidcAccessTokenGetIssuer(pAccessToken) != NULL);
    TEST_ASSERT_EQUAL_STRINGS(s_pszUsername, OidcAccessTokenGetSubject(pAccessToken));

    OidcAccessTokenGetAudience(pAccessToken, &ppszAudience, &audienceSize);
    TEST_ASSERT_TRUE(ppszAudience != NULL);
    TEST_ASSERT_TRUE(audienceSize == 1);
    TEST_ASSERT_EQUAL_STRINGS(s_pszUsername, ppszAudience[0]);

    e = OidcAccessTokenGetStringClaim(pAccessToken, "token_class", &pszStringClaim);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("access_token", pszStringClaim);
    SSOStringFree(pszStringClaim);

    TEST_ASSERT_TRUE(OidcAccessTokenGetIssueTime(pAccessToken) > 0);
    TEST_ASSERT_TRUE(OidcAccessTokenGetExpirationTime(pAccessToken) > 0);

    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);
    OidcAccessTokenDelete(pAccessToken);

    return true;
}

bool
TestPasswordGrantSuccessResponseNoRefreshToken()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;

    e = OidcClientAcquireTokensByPassword(
        s_pClient,
        s_pszUsername,
        s_pszPassword,
        "openid", // offline_access missing
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    TEST_ASSERT_SUCCESS(e);

    TEST_ASSERT_TRUE(OidcTokenSuccessResponseGetRefreshToken(pTokenSuccessResponse) == NULL);

    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);

    return true;
}

bool
TestPasswordGrantErrorResponse()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;
    PSTRING pszWrongPassword = NULL;

    e = SSOStringConcatenate(s_pszPassword, "_non_matching_", &pszWrongPassword);
    TEST_ASSERT_SUCCESS(e);

    e = OidcClientAcquireTokensByPassword(
        s_pClient,
        s_pszUsername,
        pszWrongPassword,
        "openid offline_access",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    TEST_ASSERT_EQUAL(SSOERROR_OIDC_SERVER_INVALID_GRANT, e);

    SSOStringFree(pszWrongPassword);
    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);

    return true;
}

bool
TestRefreshTokenGrantSuccessResponse()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;
    PSTRING pszRefreshToken = NULL;

    e = OidcClientAcquireTokensByPassword(
        s_pClient,
        s_pszUsername,
        s_pszPassword,
        "openid offline_access",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    TEST_ASSERT_SUCCESS(e);

    e = SSOStringAllocate(OidcTokenSuccessResponseGetRefreshToken(pTokenSuccessResponse), &pszRefreshToken);
    TEST_ASSERT_SUCCESS(e);

    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);

    e = OidcClientAcquireTokensByRefreshToken(
        s_pClient,
        pszRefreshToken,
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    TEST_ASSERT_SUCCESS(e);

    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);
    SSOStringFree(pszRefreshToken);

    return true;
}

bool
TestRefreshTokenGrantErrorResponse()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;

    e = OidcClientAcquireTokensByRefreshToken(
        s_pClient,
        "invalid_refresh_token",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    TEST_ASSERT_EQUAL(SSOERROR_OIDC_SERVER_INVALID_REQUEST, e);

    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);

    return true;
}

bool
TestIDTokenParseSuccessWithGroups()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ID_TOKEN p = NULL;
    const PSTRING* ppszGroups = NULL;
    size_t groupsSize = 0;

    e = OidcIDTokenParse(&p, s_pszIDTokenWithGroups);
    TEST_ASSERT_SUCCESS(e);

    OidcIDTokenGetGroups(p, &ppszGroups, &groupsSize);
    TEST_ASSERT_TRUE(NULL != ppszGroups);
    TEST_ASSERT_TRUE(groupsSize > 0);

    OidcIDTokenDelete(p);

    return true;
}

bool
TestIDTokenParseSuccessWithoutGroups()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ID_TOKEN p = NULL;
    const PSTRING* ppszGroups = NULL;
    size_t groupsSize = 0;

    e = OidcIDTokenParse(&p, s_pszIDTokenWithoutGroups);
    TEST_ASSERT_SUCCESS(e);

    OidcIDTokenGetGroups(p, &ppszGroups, &groupsSize);
    TEST_ASSERT_TRUE(NULL == ppszGroups);
    TEST_ASSERT_TRUE(0 == groupsSize);

    OidcIDTokenDelete(p);

    return true;
}

bool
TestIDTokenParseFail()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ID_TOKEN p = NULL;

    // parsing an access_token as an id_token should fail
    e = OidcIDTokenParse(&p, s_pszAccessTokenWithGroups);
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    OidcIDTokenDelete(p);

    return true;
}

bool
TestIDTokenBuildFailInvalidSignature()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ID_TOKEN p = NULL;

    e = OidcIDTokenBuild(&p, s_pszIDTokenWithGroups, s_pszWrongCertificatePEM, NULL /* pszIssuer */, CLOCK_TOLERANCE_IN_SECONDS);
    TEST_ASSERT_EQUAL(SSOERROR_TOKEN_INVALID_SIGNATURE, e);

    OidcIDTokenDelete(p);

    return true;
}

bool
TestIDTokenBuildFailExpired()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ID_TOKEN p = NULL;

    e = OidcIDTokenBuild(&p, s_pszIDTokenWithGroups, s_pszSigningCertificatePEM, NULL /* pszIssuer */, CLOCK_TOLERANCE_IN_SECONDS);
    TEST_ASSERT_EQUAL(SSOERROR_TOKEN_EXPIRED, e);

    OidcIDTokenDelete(p);

    return true;
}

bool
TestAccessTokenParseSuccessWithGroups()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ACCESS_TOKEN p = NULL;
    const PSTRING* ppszGroups = NULL;
    size_t groupsSize = 0;

    e = OidcAccessTokenParse(&p, s_pszAccessTokenWithGroups);
    TEST_ASSERT_SUCCESS(e);

    OidcAccessTokenGetGroups(p, &ppszGroups, &groupsSize);
    TEST_ASSERT_TRUE(NULL != ppszGroups);
    TEST_ASSERT_TRUE(groupsSize > 0);

    OidcAccessTokenDelete(p);

    return true;
}

bool
TestAccessTokenParseSuccessWithoutGroups()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ACCESS_TOKEN p = NULL;
    const PSTRING* ppszGroups = NULL;
    size_t groupsSize = 0;

    e = OidcAccessTokenParse(&p, s_pszAccessTokenWithoutGroups);
    TEST_ASSERT_SUCCESS(e);

    OidcAccessTokenGetGroups(p, &ppszGroups, &groupsSize);
    TEST_ASSERT_TRUE(NULL == ppszGroups);
    TEST_ASSERT_TRUE(0 == groupsSize);

    OidcAccessTokenDelete(p);

    return true;
}

bool
TestAccessTokenParseFail()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ACCESS_TOKEN p = NULL;

    // parsing an id_token as an access_token should fail
    e = OidcAccessTokenParse(&p, s_pszIDTokenWithGroups);
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    OidcAccessTokenDelete(p);

    return true;
}

bool
TestAccessTokenBuildFailInvalidSignature()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ACCESS_TOKEN p = NULL;

    e = OidcAccessTokenBuild(
        &p,
        s_pszAccessTokenWithGroups,
        s_pszWrongCertificatePEM,
        "issuer",
        "rs_admin_server",
        CLOCK_TOLERANCE_IN_SECONDS);
    TEST_ASSERT_EQUAL(SSOERROR_TOKEN_INVALID_SIGNATURE, e);

    OidcAccessTokenDelete(p);

    return true;
}

bool
TestAccessTokenBuildFailInvalidAudience()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ACCESS_TOKEN p = NULL;

    e = OidcAccessTokenBuild(
        &p,
        s_pszAccessTokenWithGroups,
        s_pszSigningCertificatePEM,
        "issuer",
        "rs_NOT_admin_server",
        CLOCK_TOLERANCE_IN_SECONDS);
    TEST_ASSERT_EQUAL(SSOERROR_TOKEN_INVALID_AUDIENCE, e);

    OidcAccessTokenDelete(p);

    return true;
}

bool
TestAccessTokenBuildFailExpired()
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_ACCESS_TOKEN p = NULL;

    e = OidcAccessTokenBuild(
        &p,
        s_pszAccessTokenWithGroups,
        s_pszSigningCertificatePEM,
        "issuer",
        "rs_admin_server",
        CLOCK_TOLERANCE_IN_SECONDS);
    TEST_ASSERT_EQUAL(SSOERROR_TOKEN_EXPIRED, e);

    OidcAccessTokenDelete(p);

    return true;
}
