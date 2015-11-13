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

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import javax.servlet.http.Cookie;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.ui.ExtendedModelMap;

import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.openid.connect.sdk.ResponseMode;

/**
 * @author Yehia Zayour
 */
public class AuthorizationControllerTest {
    @BeforeClass
    public static void initialize() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testAuthzCodeFlowGet() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowAjax() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        assertSuccessResponse(flow, params, TestContext.authzController(), true);
    }

    @Test
    public void testAuthzCodeFlowQueryResponseMode() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.QUERY);
        assertSuccessResponse(flow, params, TestContext.authzController(), false, true);
    }

    @Test
    public void testAuthzCodeFlowQueryResponseModeAjax() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.QUERY);
        assertSuccessResponse(flow, params, TestContext.authzController(), true, true);
    }

    @Test
    public void testAuthzCodeFlowCorrelationIdMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("correlation_id");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowScopeMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("scope");
        assertErrorResponseMessage(flow, params, "invalid_request: Missing \"scope\" parameter");
    }

    @Test
    public void testAuthzCodeFlowScopeMissingOpenId() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "profile");
        assertErrorResponseMessage(flow, params, "invalid_request: The scope must include an \"openid\" value");
    }

    @Test
    public void testAuthzCodeFlowStateMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("state");
        assertErrorResponseMessage(flow, params, "invalid_request: missing state parameter");
    }

    @Test
    public void testAuthzCodeFlowNonceMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("nonce");
        assertErrorResponseMessage(flow, params, "invalid_request: missing nonce parameter");
    }

    @Test
    public void testAuthzCodeFlowResponseModeMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("response_mode");
        assertErrorResponseMessage(flow, params, "invalid_request: missing response_mode parameter");
    }

    @Test
    public void testAuthzCodeFlowResponseModeNotAllowed() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("response_mode", "fragment");
        assertErrorResponseMessage(flow, params, "invalid_request: response_mode=fragment is not allowed for authz code flow");
    }

    @Test
    public void testAuthzCodeFlowUnrecognizedScopeValue() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile");
    }

    @Test
    public void testAuthzCodeFlowUnrecognizedScopeValueQueryResponse() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.QUERY);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", TestContext.authzController(), false, true);
    }

    @Test
    public void testAuthzCodeFlowUnrecognizedScopeValueQueryResponseAjax() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.QUERY);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", TestContext.authzController(), true, true);
    }

    @Test
    public void testImplicitFlowGet() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowAjax() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        assertSuccessResponse(flow, params, TestContext.authzController(), true);
    }

    @Test
    public void testImplicitFlowFragmentResponseMode() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.FRAGMENT);
        assertSuccessResponse(flow, params, TestContext.authzController(), false, true);
    }

    @Test
    public void testImplicitFlowFragmentResponseModeAjax() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.FRAGMENT);
        assertSuccessResponse(flow, params, TestContext.authzController(), true, true);
    }

    @Test
    public void testImplicitFlowIdTokenOnly() throws Exception {
        Flow flow = Flow.IMPLICIT_ID_TOKEN_ONLY;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeIdTokenGroups() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid id_groups");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeAccessTokenGroups() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid at_groups");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeAdminServer() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowScopeResourceServerX() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid " + TestContext.SCOPE_VALUE_RSX);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testImplicitFlowClientAssertionNotRequired() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("client_assertion");
        IdmClient idmClient = TestContext.idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        AuthorizationController controller = TestContext.authzController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testImplicitFlowClientAssertionMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("client_assertion");
        assertErrorResponseMessage(flow, params, "invalid_client: client_assertion parameter is required since client has registered a cert");
    }

    @Test
    public void testImplicitFlowClientAssertionInvalidAudience() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        JWTClaimsSet clientAssertionClaims = TestContext.clientAssertionClaims();
        clientAssertionClaims.setAudience(clientAssertionClaims.getAudience() + "non_matching");
        params.put("client_assertion", Shared.sign(clientAssertionClaims, TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponseMessage(flow, params, "invalid_client: jwt audience does not match request URL");
    }

    @Test
    public void testImplicitFlowClientAssertionInvalidIssuer() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        JWTClaimsSet clientAssertionClaims = TestContext.clientAssertionClaims();
        String nonMatchingClientId = TestContext.CLIENT_ID + "non_matching";
        clientAssertionClaims.setIssuer(nonMatchingClientId);
        clientAssertionClaims.setSubject(nonMatchingClientId);
        params.put("client_assertion", Shared.sign(clientAssertionClaims, TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponseMessage(flow, params, "invalid_client: client_assertion issuer must match client_id");
    }

    @Test
    public void testImplicitFlowRefreshTokenDisallowed() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "refresh token (offline_access) is not allowed for this grant_type");
    }

    @Test
    public void testImplicitFlowScopeMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("scope");
        assertErrorResponseMessage(flow, params, "invalid_request: Missing \"scope\" parameter");
    }

    @Test
    public void testImplicitFlowScopeMissingOpenId() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "profile");
        assertErrorResponseMessage(flow, params, "invalid_request: The scope must include an \"openid\" value");
    }

    @Test
    public void testImplicitFlowStateMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("state");
        assertErrorResponseMessage(flow, params, "invalid_request: missing state parameter");
    }

    @Test
    public void testImplicitFlowNonceMissing() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("nonce");
        assertErrorResponseMessage(flow, params, "invalid_request: Missing \"nonce\" parameter: Required in implicit flow");
    }

    @Test
    public void testImplicitFlowResponseModeNotAllowed() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("response_mode", "query");
        assertErrorResponseMessage(flow, params, "invalid_request: response_mode=query is not allowed for implicit flow");
    }

    @Test
    public void testImplicitFlowUnrecognizedScopeValue() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile");
    }

    @Test
    public void testImplicitFlowUnrecognizedScopeValueFragmentResponse() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.FRAGMENT);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", TestContext.authzController(), false, true);
    }

    @Test
    public void testImplicitFlowUnrecognizedScopeValueFragmentResponseAjax() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow, ResponseMode.FRAGMENT);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile", TestContext.authzController(), true, true);
    }

    @Test
    public void testUnsupportedResponseType() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("response_type", "code id_token");
        assertErrorResponse(flow, params, "unsupported_response_type", "Unsupported response type");
    }

    @Test
    public void testInvalidRedirectUri() throws Exception {
        // when redirect_uri is not a valid uri, we should get a 400 Bad Request as opposed to a 302 Redirect containing an error parameter
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("redirect_uri", "this_is_not_a_url");
        assertErrorResponseMessage(flow, params, "invalid_request: invalid redirect_uri");
    }

    @Test
    public void testUnregisteredRedirectUri() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("redirect_uri", params.get("redirect_uri") + "non_matching");
        assertErrorResponseMessage(flow, params, "invalid_request: unregistered redirect_uri");
    }

    @Test
    public void testUnregisteredClient() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.put("client_id", params.get("client_id") + "non_matching");
        assertErrorResponseMessage(flow, params, "invalid_request: unregistered client");
    }

    @Test
    public void testNonExistentTenant() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        MockHttpServletRequest request = TestUtil.createGetRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        AuthorizationController controller = TestContext.authzController();
        controller.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response, TestContext.TENANT_NAME + "_non_matching");
        Assert.assertEquals("errorMessage", "invalid_request: non-existent tenant", response.getErrorMessage());
        Assert.assertEquals("status", 400, response.getStatus());
    }

    @Test
    public void testDisabledPersonUser() throws Exception {
        Flow flow = Flow.IMPLICIT;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().personUserEnabled(false).build();
        AuthorizationController controller = TestContext.authzController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "user has been disabled or deleted", controller);
    }

    @Test
    public void testInvalidRequestUnregisteredClient() throws Exception {
        // force an error response to be returned as a 400 instead of 302 because of unregistered client
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.authnRequestParameters(flow);
        params.remove("scope"); // a request missing scope parameter will fail
        params.put("client_id", params.get("client_id") + "non_matching");
        assertErrorResponseMessage(flow, params, "invalid_request: Missing \"scope\" parameter");
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params) throws Exception {
        assertSuccessResponse(flow, params, TestContext.authzController(), false, false);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthorizationController controller) throws Exception {
        assertSuccessResponse(flow, params, controller, false, false);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthorizationController controller,
            boolean ajaxRequest) throws Exception {
        assertSuccessResponse(flow, params, controller, ajaxRequest, false);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            AuthorizationController controller,
            boolean ajaxRequest,
            boolean redirectResponseMode) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller, ajaxRequest);
        TestContext.validateAuthnSuccessResponse(flow, response, Scope.parse(params.get("scope")), TestContext.STATE, TestContext.NONCE, redirectResponseMode, ajaxRequest);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription) throws Exception {
        assertErrorResponse(flow, params, expectedError, expectedErrorDescription, TestContext.authzController(), false, false);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription,
            AuthorizationController controller) throws Exception {
        assertErrorResponse(flow, params, expectedError, expectedErrorDescription, controller, false, false);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription,
            AuthorizationController controller,
            boolean ajaxRequest,
            boolean redirectResponseMode) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller, ajaxRequest);
        TestContext.validateAuthnErrorResponse(flow, response, expectedError, expectedErrorDescription, redirectResponseMode, ajaxRequest);
    }

    private static void assertErrorResponseMessage(
            Flow flow,
            Map<String, String> params,
            String expectedErrorMessage) throws Exception {
        MockHttpServletResponse response = doRequest(params, TestContext.authzController(), false);
        Assert.assertEquals("errorMessage", expectedErrorMessage, response.getErrorMessage());
        Assert.assertTrue("status", response.getStatus() == 400 || response.getStatus() == 401);
    }

    private static MockHttpServletResponse doRequest(
            Map<String, String> params,
            AuthorizationController controller,
            boolean ajaxRequest) throws Exception {
        MockHttpServletRequest request;
        if (ajaxRequest) {
            Map<String, String> formParams = new HashMap<String, String>();
            formParams.put("CastleAuthorization", TestContext.passwordLoginString());
            request = TestUtil.createPostRequestWithQueryString(formParams, params);
        } else {
            request = TestUtil.createGetRequest(params);
            request.setCookies(new Cookie(TestContext.SESSION_COOKIE_NAME, TestContext.SESSION_ID));
        }
        MockHttpServletResponse response = new MockHttpServletResponse();
        controller.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response);
        return response;
    }
}
