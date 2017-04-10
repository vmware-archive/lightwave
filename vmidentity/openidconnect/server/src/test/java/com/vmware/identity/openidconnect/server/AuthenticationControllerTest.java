/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

package com.vmware.identity.openidconnect.server;

import static com.vmware.identity.openidconnect.server.TestContext.ADMIN_SERVER_ROLE;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_ID;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_PRIVATE_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.GROUP_MEMBERSHIP;
import static com.vmware.identity.openidconnect.server.TestContext.GROUP_MEMBERSHIP_FILTERED;
import static com.vmware.identity.openidconnect.server.TestContext.NONCE;
import static com.vmware.identity.openidconnect.server.TestContext.SCOPE_VALUE_RSX;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_COOKIE_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_ID;
import static com.vmware.identity.openidconnect.server.TestContext.STATE;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.authnController;
import static com.vmware.identity.openidconnect.server.TestContext.authnRequestParameters;
import static com.vmware.identity.openidconnect.server.TestContext.clientAssertionClaims;
import static com.vmware.identity.openidconnect.server.TestContext.idmClientBuilder;
import static com.vmware.identity.openidconnect.server.TestContext.initialize;
import static com.vmware.identity.openidconnect.server.TestContext.passwordLoginString;
import static com.vmware.identity.openidconnect.server.TestContext.qualifyGroupMembership;
import static com.vmware.identity.openidconnect.server.TestContext.validateAuthnErrorResponse;
import static com.vmware.identity.openidconnect.server.TestContext.validateAuthnSuccessResponse;

import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import javax.servlet.http.Cookie;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.ui.ExtendedModelMap;

import com.nimbusds.jwt.JWTClaimsSet;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.Scope;

/**
 * @author Yehia Zayour
 */
public class AuthenticationControllerTest {
    @BeforeClass
    public static void setup() throws Exception {
        initialize();
    }

    @Test
    public void testAuthzCodeFlowGet() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowAjax() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        assertSuccessResponse(flow, params, authnController(), true);
    }

    @Test
    public void testAuthzCodeFlowQueryResponseMode() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow, "query");
        assertSuccessResponse(flow, params, authnController(), false, true);
    }

    @Test
    public void testAuthzCodeFlowQueryResponseModeAjax() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow, "query");
        assertSuccessResponse(flow, params, authnController(), true, true);
    }

    @Test
    public void testAuthzCodeFlowCorrelationIdMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("correlation_id");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowScopeMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("scope");
        assertErrorResponse(flow, params, "invalid_request", "missing scope parameter");
    }

    @Test
    public void testAuthzCodeFlowScopeMissingOpenId() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "missing openid scope value");
    }

    @Test
    public void testAuthzCodeFlowStateMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("state");
        assertErrorResponseMessage(flow, params, "invalid_request: missing state parameter");
    }

    @Test
    public void testAuthzCodeFlowStateInvalid() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("state", "<");
        assertErrorResponseMessage(flow, params, "invalid_request: state parameter must be html friendly");
    }

    @Test
    public void testAuthzCodeFlowNonceMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("nonce");
        assertErrorResponse(flow, params, "invalid_request", "missing nonce parameter");
    }

    @Test
    public void testAuthzCodeFlowResponseModeMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("response_mode");
        assertErrorResponseMessage(flow, params, "invalid_request: missing response_mode parameter");
    }

    @Test
    public void testAuthzCodeFlowResponseModeEmpty() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("response_mode", "");
        assertErrorResponseMessage(flow, params, "invalid_request: missing response_mode parameter");
    }

    @Test
    public void testAuthzCodeFlowResponseModeInvalid() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("response_mode", "invalid_response_mode");
        assertErrorResponseMessage(flow, params, "invalid_request: invalid response_mode parameter");
    }

    @Test
    public void testAuthzCodeFlowResponseModeNotAllowed() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("response_mode", "fragment");
        MockHttpServletResponse response = doRequest(params, authnController(), false);
        Assert.assertTrue(response.getRedirectedUrl().contains(TestUtil.urlEncode("invalid_request")));
        Assert.assertTrue(response.getRedirectedUrl().contains(TestUtil.urlEncode("response_mode=fragment is not allowed for authz code flow")));
    }

    @Test
    public void testAuthzCodeFlowUnrecognizedScopeValue() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile");
    }

    @Test
    public void testAuthzCodeFlowUnrecognizedScopeValueQueryResponse() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow, "query");
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", authnController(), false, true);
    }

    @Test
    public void testAuthzCodeFlowUnrecognizedScopeValueQueryResponseAjax() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow, "query");
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", authnController(), true, true);
    }

    @Test
    public void testImplicitFlowGet() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowAjax() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        assertSuccessResponse(flow, params, authnController(), true);
    }

    @Test
    public void testImplicitFlowFragmentResponseMode() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow, "fragment");
        assertSuccessResponse(flow, params, authnController(), false, true);
    }

    @Test
    public void testImplicitFlowFragmentResponseModeAjax() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow, "fragment");
        assertSuccessResponse(flow, params, authnController(), true, true);
    }

    @Test
    public void testImplicitFlowIdTokenOnly() throws Exception {
        Flow flow = Flow.IMPLICIT_ID_TOKEN_ONLY;
        Map<String, String> params = authnRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeIdTokenGroups() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid id_groups");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeIdTokenGroupsFiltered() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP_FILTERED;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, authnController(), false, false, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testImplicitFlowScopeAccessTokenGroups() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid at_groups rs_x rs_y");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeAccessTokenGroupsFiltered() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid at_groups_filtered rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP_FILTERED;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, authnController(), false, false, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testImplicitFlowScopeAdminServer() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeAdminServerUserIsAdministrator() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        Set<String> groupMembership = qualifyGroupMembership(Arrays.asList("x", "users", "systemconfiguration.administrators", "administrators"));
        CasIdmClient idmClient = idmClientBuilder().groupMembership(groupMembership).build();
        AuthenticationController controller = authnController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = "Administrator";
        assertSuccessResponse(flow, params, controller, false, false, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testImplicitFlowScopeResourceServerX() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid " + SCOPE_VALUE_RSX);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowClientAssertionNotRequired() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("client_assertion");
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        AuthenticationController controller = authnController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testImplicitFlowClientAssertionMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("client_assertion");
        assertErrorResponse(flow, params, "invalid_client", "client_assertion parameter is required since client has registered a cert");
    }

    @Test
    public void testImplicitFlowClientAssertionInvalidAudience() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.audience("https://sso.com/invalid_authz_endpoint");
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_client", "client_assertion audience does not match request URI");
    }

    @Test
    public void testImplicitFlowClientAssertionInvalidIssuer() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        String nonMatchingClientId = CLIENT_ID + "non_matching";
        claimsBuilder = claimsBuilder.issuer(nonMatchingClientId);
        claimsBuilder = claimsBuilder.subject(nonMatchingClientId);
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_client", "client_assertion issuer must match client_id");
    }

    @Test
    public void testImplicitFlowClientAssertionStale() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        Date now = new Date();
        Date issuedAt = new Date(now.getTime() - (2 * 60 * 60 * 1000L)); // issued 2 hrs ago
        claimsBuilder = claimsBuilder.issueTime(issuedAt);
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_client", "client_assertion has expired");
    }

    @Test
    public void testImplicitFlowRefreshTokenDisallowed() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "refresh token (offline_access) is not allowed for this grant_type");
    }

    @Test
    public void testImplicitFlowScopeMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("scope");
        assertErrorResponse(flow, params, "invalid_request", "missing scope parameter");
    }

    @Test
    public void testImplicitFlowScopeMissingOpenId() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "missing openid scope value");
    }

    @Test
    public void testImplicitFlowStateMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("state");
        assertErrorResponseMessage(flow, params, "invalid_request: missing state parameter");
    }

    @Test
    public void testImplicitFlowStateInvalid() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("state", ">");
        assertErrorResponseMessage(flow, params, "invalid_request: state parameter must be html friendly");
    }

    @Test
    public void testImplicitFlowNonceMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("nonce");
        assertErrorResponse(flow, params, "invalid_request", "missing nonce parameter");
    }

    @Test
    public void testImplicitFlowResponseModeNotAllowed() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("response_mode", "query");
        MockHttpServletResponse response = doRequest(params, authnController(), false);
        Assert.assertTrue(response.getRedirectedUrl().contains(TestUtil.urlEncode("invalid_request")));
        Assert.assertTrue(response.getRedirectedUrl().contains(TestUtil.urlEncode("response_mode=query is not allowed for implicit flow")));
    }

    @Test
    public void testImplicitFlowUnrecognizedScopeValue() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile");
    }

    @Test
    public void testImplicitFlowUnrecognizedScopeValueFragmentResponse() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow, "fragment");
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", authnController(), false, true);
    }

    @Test
    public void testImplicitFlowUnrecognizedScopeValueFragmentResponseAjax() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow, "fragment");
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", authnController(), true, true);
    }

    @Test
    public void testInvalidResponseType() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("response_type", "invalid_response_type_value");
        assertErrorResponse(flow, params, "invalid_request", "invalid response_type value");
    }

    @Test
    public void testUnsupportedResponseType() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("response_type", "code id_token");
        assertErrorResponse(flow, params, "unsupported_response_type", "unsupported response_type");
    }

    @Test
    public void testInvalidRedirectUri() throws Exception {
        // when redirect_uri is not a valid uri, we should get a 400 Bad Request as opposed to a 302 Redirect containing an error parameter
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("redirect_uri", "this_is_not_a_url");
        assertErrorResponseMessage(flow, params, "invalid_request: invalid redirect_uri parameter");
    }

    @Test
    public void testUnregisteredRedirectUri() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("redirect_uri", params.get("redirect_uri") + "non_matching");
        assertErrorResponseMessage(flow, params, "invalid_request: unregistered redirect_uri");
    }

    @Test
    public void testUnregisteredClient() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.put("client_id", params.get("client_id") + "non_matching");
        assertErrorResponseMessage(flow, params, "invalid_client: unregistered client");
    }

    @Test
    public void testNonExistentTenant() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        MockHttpServletRequest request = TestUtil.createGetRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        AuthenticationController controller = authnController();
        controller.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME + "_non_matching");
        Assert.assertEquals("errorMessage", "invalid_request: non-existent tenant", response.getErrorMessage());
        Assert.assertEquals("status", 400, response.getStatus());
    }

    @Test
    public void testDisabledPersonUser() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = authnRequestParameters(flow);
        CasIdmClient idmClient = idmClientBuilder().personUserEnabled(false).build();
        AuthenticationController controller = authnController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "user has been disabled or deleted", controller);
    }

    @Test
    public void testInvalidRequestUnregisteredClient() throws Exception {
        // force an error response to be returned as a 400 instead of 302 because of unregistered client
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = authnRequestParameters(flow);
        params.remove("scope"); // a request missing scope parameter will fail
        params.put("client_id", params.get("client_id") + "non_matching");
        assertErrorResponseMessage(flow, params, "invalid_client: unregistered client");
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params) throws Exception {
        assertSuccessResponse(flow, params, authnController(), false, false);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthenticationController controller) throws Exception {
        assertSuccessResponse(flow, params, controller, false, false);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthenticationController controller,
            boolean ajaxRequest) throws Exception {
        assertSuccessResponse(flow, params, controller, ajaxRequest, false);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthenticationController controller,
            boolean ajaxRequest,
            boolean redirectResponseMode) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller, ajaxRequest);
        validateAuthnSuccessResponse(
                response,
                flow,
                Scope.parse(params.get("scope")),
                redirectResponseMode,
                ajaxRequest,
                STATE,
                NONCE);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthenticationController controller,
            boolean ajaxRequest,
            boolean redirectResponseMode,
            Set<String> expectedIdTokenGroups,
            Set<String> expectedAccessTokenGroups,
            String expectedAdminServerRole) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller, ajaxRequest);
        validateAuthnSuccessResponse(
                response,
                flow,
                Scope.parse(params.get("scope")),
                redirectResponseMode,
                ajaxRequest,
                STATE,
                NONCE,
                expectedIdTokenGroups,
                expectedAccessTokenGroups,
                expectedAdminServerRole);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription) throws Exception {
        assertErrorResponse(flow, params, expectedError, expectedErrorDescription, authnController(), false, false);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription,
            AuthenticationController controller) throws Exception {
        assertErrorResponse(flow, params, expectedError, expectedErrorDescription, controller, false, false);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription,
            AuthenticationController controller,
            boolean ajaxRequest,
            boolean redirectResponseMode) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller, ajaxRequest);
        validateAuthnErrorResponse(response, flow, redirectResponseMode, ajaxRequest, expectedError, expectedErrorDescription);
    }

    private static void assertErrorResponseMessage(
            Flow flow /* ignored */,
            Map<String, String> params,
            String expectedErrorMessage) throws Exception {
        MockHttpServletResponse response = doRequest(params, authnController(), false);
        Assert.assertEquals("errorMessage", expectedErrorMessage, response.getErrorMessage());
        Assert.assertTrue("status", response.getStatus() == 400 || response.getStatus() == 401);
    }

    private static MockHttpServletResponse doRequest(
            Map<String, String> params,
            AuthenticationController controller,
            boolean ajaxRequest) throws Exception {
        MockHttpServletRequest request;
        if (ajaxRequest) {
            Map<String, String> formParams = new HashMap<String, String>();
            formParams.put("CastleAuthorization", passwordLoginString());
            request = TestUtil.createPostRequestWithQueryString(formParams, params);
        } else {
            request = TestUtil.createGetRequest(params);
            request.setCookies(new Cookie(SESSION_COOKIE_NAME, SESSION_ID));
        }
        MockHttpServletResponse response = new MockHttpServletResponse();
        controller.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        return response;
    }
}