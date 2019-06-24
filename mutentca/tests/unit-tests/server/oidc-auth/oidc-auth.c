/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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


static
DWORD
_LwCAMakeReqCtx(
    PLWCA_REQ_CONTEXT   *ppReqCtx
    );

static
BOOLEAN
_LwCAVerifyGroups(
    PLWCA_STRING_ARRAY  pActualGroups,
    PSTR                *ppszExpectedGroups,
    DWORD               dwNumExpectedGroups
    );


DWORD
__wrap_LwCAGetDomainName(
    PSTR        *ppszDomainName
    )
{
    DWORD       dwError = 0;

    assert_non_null(ppszDomainName);

    dwError = LwCAAllocateStringA(TEST_TENANT, ppszDomainName);
    assert_int_equal(dwError, 0);

    return mock();
}

DWORD
__wrap_LwCAGetDCName(
    PSTR        *ppszDCName
    )
{
    DWORD       dwError = 0;

    assert_non_null(ppszDCName);

    dwError = LwCAAllocateStringA(TEST_DC_NAME, ppszDCName);
    assert_int_equal(dwError, 0);

    return mock();
}

DWORD
__wrap_OidcServerMetadataAcquire(
    POIDC_SERVER_METADATA*  pp,
    PCSTRING                pszServer,
    int                     portNumber,
    PCSTRING                pszTenant,
    PCSTRING                pszTlsCAPath
    )
{
    return mock();
}

PCSTR
__wrap_OidcServerMetadataGetSigningCertificatePEM(
    POIDC_SERVER_METADATA   p
    )
{
    return (PCSTR)mock();
}

DWORD
__wrap_LwCAUPNToDN(
    PCSTR       pcszUPN,
    PSTR        *ppszDN
    )
{
    DWORD       dwError = 0;

    assert_non_null(pcszUPN);
    assert_non_null(ppszDN);

    if (LwCAStringCompareA(pcszUPN, TEST_UPN1, FALSE) == 0)
    {
        dwError = LwCAAllocateStringA(TEST_UPN1_DN, ppszDN);
        assert_int_equal(dwError, 0);
    }
    else if (LwCAStringCompareA(pcszUPN, TEST_SRV_UPN1, FALSE) == 0)
    {
        dwError = LwCAAllocateStringA(TEST_SRV_UPN1_DN, ppszDN);
        assert_int_equal(dwError, 0);
    }
    else
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        assert_int_equal(dwError, 0);
    }

    return mock();
}

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;
    char                    *expectedGroups[] = TEST_UPN1_GROUPS;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);
    will_return(__wrap_LwCAUPNToDN, 0);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_BEARER_AUTH_HDR(TEST_UPN1_BEARER_JWT_VALID),
                        "",
                        "",
                        "",
                        "",
                        "",
                        &bAuthenticated);
    assert_int_equal(dwError, 0);
    assert_true(bAuthenticated);

    assert_string_equal(pReqCtx->pszBindUPNTenant, TEST_TENANT);
    assert_string_equal(pReqCtx->pszBindUPN, TEST_UPN1);
    assert_string_equal(pReqCtx->pszBindUPNDN, TEST_UPN1_DN);
    assert_true(_LwCAVerifyGroups(pReqCtx->pBindUPNGroups, expectedGroups, TEST_UPN1_NUM_GROUPS));

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_InvalidIssuer(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT2_OIDC_SIGNING_CERT);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_BEARER_AUTH_HDR(TEST_UPN1_BEARER_JWT_VALID),
                        "",
                        "",
                        "",
                        "",
                        "",
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_InvalidAud(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_BEARER_AUTH_HDR(TEST_UPN1_BEARER_JWT_BAD_AUD),
                        "",
                        "",
                        "",
                        "",
                        "",
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_Expired(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_BEARER_AUTH_HDR(TEST_UPN1_BEARER_JWT_EXPIRED),
                        "",
                        "",
                        "",
                        "",
                        "",
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_BearerToken_NotInHeader(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_HTTP_TOK_TYPE_BEARER,
                        "",
                        "",
                        "",
                        "",
                        "",
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_Valid(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;
    char                    *expectedGroups[] = TEST_SRV_UPN1_GROUPS;

    will_return_always(__wrap_LwCAGetDomainName, 0);
    will_return_always(__wrap_LwCAGetDCName, 0);
    will_return_always(__wrap_OidcServerMetadataAcquire, 0);
    will_return_always(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);
    will_return_always(__wrap_LwCAUPNToDN, 0);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    // Test GET request
    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ1_POP),
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ1_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ1_DATE,
                        TEST_SRV_UPN1_REQ1_BODY,
                        TEST_SRV_UPN1_REQ1_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, 0);
    assert_true(bAuthenticated);

    assert_string_equal(pReqCtx->pszBindUPNTenant, TEST_TENANT);
    assert_string_equal(pReqCtx->pszBindUPN, TEST_SRV_UPN1);
    assert_string_equal(pReqCtx->pszBindUPNDN, TEST_SRV_UPN1_DN);
    assert_true(_LwCAVerifyGroups(pReqCtx->pBindUPNGroups, expectedGroups, TEST_SRV_UPN1_NUM_GROUPS));

    LwCARequestContextFree(pReqCtx);
    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    // Test POST request
    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ2_POP),
                        TEST_SRV_UPN1_REQ2_METHOD,
                        TEST_SRV_UPN1_REQ2_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ2_DATE,
                        TEST_SRV_UPN1_REQ2_BODY,
                        TEST_SRV_UPN1_REQ2_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, 0);
    assert_true(bAuthenticated);

    assert_string_equal(pReqCtx->pszBindUPNTenant, TEST_TENANT);
    assert_string_equal(pReqCtx->pszBindUPN, TEST_SRV_UPN1);
    assert_string_equal(pReqCtx->pszBindUPNDN, TEST_SRV_UPN1_DN);
    assert_true(_LwCAVerifyGroups(pReqCtx->pBindUPNGroups, expectedGroups, TEST_SRV_UPN1_NUM_GROUPS));

    LwCARequestContextFree(pReqCtx);
    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    // Test POST request with un-sanitized whitespace
    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ3_POP),
                        TEST_SRV_UPN1_REQ3_METHOD,
                        TEST_SRV_UPN1_REQ3_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ3_DATE,
                        TEST_SRV_UPN1_REQ3_BODY,
                        TEST_SRV_UPN1_REQ3_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, 0);
    assert_true(bAuthenticated);

    assert_string_equal(pReqCtx->pszBindUPNTenant, TEST_TENANT);
    assert_string_equal(pReqCtx->pszBindUPN, TEST_SRV_UPN1);
    assert_string_equal(pReqCtx->pszBindUPNDN, TEST_SRV_UPN1_DN);
    assert_true(_LwCAVerifyGroups(pReqCtx->pBindUPNGroups, expectedGroups, TEST_SRV_UPN1_NUM_GROUPS));

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidIssuer(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT2_OIDC_SIGNING_CERT);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ1_POP),
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ1_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ1_DATE,
                        TEST_SRV_UPN1_REQ1_BODY,
                        TEST_SRV_UPN1_REQ1_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidAud(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT_BAD_AUD, TEST_SRV_UPN1_REQ1_POP),
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ1_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ1_DATE,
                        TEST_SRV_UPN1_REQ1_BODY,
                        TEST_SRV_UPN1_REQ1_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_Expired(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return(__wrap_LwCAGetDomainName, 0);
    will_return(__wrap_LwCAGetDCName, 0);
    will_return(__wrap_OidcServerMetadataAcquire, 0);
    will_return(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT_EXPIRED, TEST_SRV_UPN1_REQ1_POP),
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ1_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ1_DATE,
                        TEST_SRV_UPN1_REQ1_BODY,
                        TEST_SRV_UPN1_REQ1_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_NoPOP(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_SRV_UPN1_HOTK_JWT,
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ1_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ1_DATE,
                        TEST_SRV_UPN1_REQ1_BODY,
                        TEST_SRV_UPN1_REQ1_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidPOPData(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    will_return_always(__wrap_LwCAGetDomainName, 0);
    will_return_always(__wrap_LwCAGetDCName, 0);
    will_return_always(__wrap_OidcServerMetadataAcquire, 0);
    will_return_always(__wrap_OidcServerMetadataGetSigningCertificatePEM, TEST_TENANT_OIDC_SIGNING_CERT);
    will_return_always(__wrap_LwCAUPNToDN, 0);

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    // Request 1 POP, but with incorrect request values
    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ1_POP),
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ1_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ2_DATE,
                        TEST_SRV_UPN1_REQ2_BODY,
                        TEST_SRV_UPN1_REQ1_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    // Request 2 POP, but with incorrect request values
    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ2_POP),
                        TEST_SRV_UPN1_REQ1_METHOD,
                        TEST_SRV_UPN1_REQ2_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ3_DATE,
                        TEST_SRV_UPN1_REQ2_BODY,
                        TEST_SRV_UPN1_REQ2_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    // Bad POP for request 3, but correct request values
    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_HOTK_AUTH_HDR(TEST_SRV_UPN1_HOTK_JWT, TEST_SRV_UPN1_REQ3_BAD_POP),
                        TEST_SRV_UPN1_REQ3_METHOD,
                        TEST_SRV_UPN1_REQ3_CONTENTTYPE,
                        TEST_SRV_UPN1_REQ3_DATE,
                        TEST_SRV_UPN1_REQ3_BODY,
                        TEST_SRV_UPN1_REQ3_URI,
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_BAD_AUTH_DATA);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}

VOID
Test_LwCAOIDCTokenAuthenticate_UnknownTokType(
    VOID                    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    dwError = _LwCAMakeReqCtx(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    dwError = LwCAOIDCTokenAuthenticate(
                        pReqCtx,
                        TEST_BUILD_AUTH_HDR(TEST_HTTP_TOK_TYPE_UNKNOWN, TEST_HTTP_UNKNOWN_TOK_VALUE),
                        "",
                        "",
                        "",
                        "",
                        "",
                        &bAuthenticated);
    assert_int_equal(dwError, LWCA_ERROR_OIDC_UNKNOWN_TOKEN);
    assert_false(bAuthenticated);

    LwCARequestContextFree(pReqCtx);
}


static
DWORD
_LwCAMakeReqCtx(
    PLWCA_REQ_CONTEXT   *ppReqCtx
    )
{
    DWORD               dwError = 0;
    PLWCA_REQ_CONTEXT   pReqCtx = NULL;

    assert_non_null(ppReqCtx);

    dwError = LwCARequestContextCreate(&pReqCtx);
    assert_int_equal(dwError, 0);
    assert_non_null(pReqCtx);

    *ppReqCtx = pReqCtx;

    return dwError;
}

static
BOOLEAN
_LwCAVerifyGroups(
    PLWCA_STRING_ARRAY  pActualGroups,
    PSTR                *ppszExpectedGroups,
    DWORD               dwNumExpectedGroups
    )
{
    DWORD               dwIdx = 0;

    assert_non_null(pActualGroups);

    assert_int_equal(pActualGroups->dwCount, dwNumExpectedGroups);

    for (; dwIdx < dwNumExpectedGroups; ++dwIdx)
    {
        assert_string_equal(pActualGroups->ppData[dwIdx], ppszExpectedGroups[dwIdx]);
    }

    return TRUE;
}
