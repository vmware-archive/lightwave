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

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.servlet.http.Cookie;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.PlainJWT;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class EndSessionControllerTest {
    @BeforeClass
    public static void initialize() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testLogoutWithRedirectWithState() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        assertSuccessResponse(
                params,
                true /* redirect */,
                true /* withState */,
                false /* imageLinksExpected */,
                false /* sessionCookieExpected */);
    }

    @Test
    public void testLogoutWithRedirectWithoutState() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.remove("state");
        assertSuccessResponse(
                params,
                true /* redirect */,
                false /* withState */,
                false /* imageLinksExpected */,
                false /* sessionCookieExpected */);
    }

    @Test
    public void testLogoutWithoutRedirect() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.remove("post_logout_redirect_uri");
        params.remove("state");
        assertSuccessResponse(
                params,
                false /* redirect */,
                false /* withState */,
                false /* imageLinksExpected */,
                false /* sessionCookieExpected */);
    }

    @Test
    public void testLogoutWithMatchingSessionCookie() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        SessionID sessionId = new SessionID(TestContext.SESSION_ID);
        SessionManager sessionManager = TestContext.sessionManager();
        EndSessionController controller = TestContext.endSessionController();
        controller.setSessionManager(sessionManager);
        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionId) != null);
        assertSuccessResponse(
                params,
                true /* redirect */,
                true /* withState */,
                true /* imageLinksExpected */,
                true /* sessionCookieExpected */,
                sessionId,
                controller);
        Assert.assertTrue("sessionManager.get(sessionId)==null", sessionManager.get(sessionId) == null);
    }

    @Test
    public void testLogoutWithNonMatchingSessionCookie() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        SessionID sessionIdMatching = new SessionID(TestContext.SESSION_ID);
        SessionID sessionIdNonMatching = new SessionID(TestContext.SESSION_ID + "non_matching");
        SessionManager sessionManager = TestContext.sessionManager();
        EndSessionController controller = TestContext.endSessionController();
        controller.setSessionManager(sessionManager);
        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionIdMatching) != null);
        assertSuccessResponse(
                params,
                true /* redirect */,
                true /* withState */,
                false /* imageLinksExpected */,
                true /* sessionCookieExpected */,
                sessionIdNonMatching,
                controller);
        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionIdMatching) != null);
    }

    @Test
    public void testLogoutWithoutCorrelationId() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.remove("correlation_id");
        assertSuccessResponse(
                params,
                true /* redirect */,
                true /* withState */,
                false /* imageLinksExpected */,
                false /* sessionCookieExpected */);
    }

    @Test
    public void testLogoutClientAssertionNotRequired() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.remove("client_assertion");

        IdmClient idmClient = TestContext.idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        EndSessionController controller = TestContext.endSessionController(idmClient);

        assertSuccessResponse(
                params,
                true /* redirect */,
                true /* withState */,
                false /* imageLinksExpected */,
                false /* sessionCookieExpected */,
                (SessionID) null,
                controller);
    }

    @Test
    public void testLogoutClientAssertionMissing() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.remove("client_assertion");
        assertErrorResponse(params, "invalid_client: client_assertion parameter is required since client has registered a cert");
    }

    @Test
    public void testLogoutClientAssertionInvalidAudience() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet clientAssertionClaims = TestContext.clientAssertionClaims();
        clientAssertionClaims.setAudience(clientAssertionClaims.getAudience() + "non_matching");
        params.put("client_assertion", Shared.sign(clientAssertionClaims, TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(params, "invalid_client: jwt audience does not match request URL");
    }

    @Test
    public void testNonExistentTenant() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();

        EndSessionController logoutEndpoint = TestContext.endSessionController();
        logoutEndpoint.logout(request, response, "non_matching_tenant");

        String expectedErrorMessage = "invalid_request: non-existent tenant";
        TestContext.validateLogoutErrorResponse(response, expectedErrorMessage);
    }

    @Test
    public void testUnregisteredClient() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        IdmClient idmClient = TestContext.idmClientBuilder().clientId(TestContext.CLIENT_ID + "non_matching").build();
        EndSessionController controller = TestContext.endSessionController(idmClient);
        assertErrorResponse(params, "invalid_request: unregistered client", null, controller);
    }

    @Test
    public void testUnregisteredRedirectUri() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.put("post_logout_redirect_uri", params.get("post_logout_redirect_uri") + "non_matching");
        String expectedErrorMessage = "invalid_request: unregistered post_logout_redirect_uri";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testInvalidRedirectUri() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.put("post_logout_redirect_uri", "http://a.com/redirect"); // should be https
        String expectedErrorMessage = "invalid_request: logout request parse error: invalid post_logout_redirect_uri";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissing() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.remove("id_token_hint");
        String expectedErrorMessage = "invalid_request: logout request parse error: Missing \"id_token_hint\" parameter";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalid() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        params.put("id_token_hint", "invalid_id_token_hint_jwt");
        String expectedErrorMessage = "invalid_request: logout request parse error: Invalid ID token hint: Invalid JWT serialization: Missing dot delimiter(s)";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintPlainJwt() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        PlainJWT idToken = new PlainJWT(claimsSet);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: logout request parse error: id_token_hint must be a signed jwt";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalidTokenClass() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        claimsSet.setClaim("token_class", true); // token_class should be a string claim
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: failed to parse token_class or token_type claims out of jwt";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintIncorrectTokenClass() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        claimsSet.setClaim("token_class", "access_token"); // should be id_token
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: jwt must have token_class=id_token";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintIncorrectIssuer() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        claimsSet.setIssuer(claimsSet.getIssuer() + "non_matching");
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: invalid issuer";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingIssuer() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        claimsSet.setIssuer(null);
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: invalid issuer";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingSubject() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        claimsSet.setSubject(null);
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: jwt is missing a sub (subject) claim";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintNonMatchingSubject() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        claimsSet.setSubject(claimsSet.getSubject() + "non_matching");
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: jwt subject does not match the session user";
        assertErrorResponse(params, expectedErrorMessage, new SessionID(TestContext.SESSION_ID), TestContext.endSessionController());
    }

    @Test
    public void testIdTokenHintInvalidAudience() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        List<String> audience = new ArrayList<String>();
        audience.add("aud1");
        audience.add("aud2");
        claimsSet.setAudience(audience);
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint validation error: jwt must have a single audience value containing the client_id";
        assertErrorResponse(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalidSignature() throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        JWTClaimsSet claimsSet = TestContext.idTokenClaims();
        SignedJWT idToken = Shared.sign(claimsSet, TestContext.CLIENT_PRIVATE_KEY); // should be signed using server private key
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint has an invalid signature";
        assertErrorResponse(params, expectedErrorMessage);
    }

    private static void assertSuccessResponse(
            Map<String, String> params,
            boolean redirect,
            boolean withState,
            boolean imageLinksExpected,
            boolean sessionCookieExpected) throws Exception {
        assertSuccessResponse(params, redirect, withState, imageLinksExpected, sessionCookieExpected, null, TestContext.endSessionController());
    }

    private static void assertSuccessResponse(
            Map<String, String> params,
            boolean redirect,
            boolean withState,
            boolean imageLinksExpected,
            boolean sessionCookieExpected,
            SessionID sessionId,
            EndSessionController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, sessionId, controller);
        TestContext.validateLogoutSuccessResponse(
                response,
                redirect,
                withState,
                imageLinksExpected,
                sessionCookieExpected);
    }

    private static void assertErrorResponse(
            Map<String, String> params,
            String expectedErrorMessage) throws Exception {
        assertErrorResponse(params, expectedErrorMessage, null, TestContext.endSessionController());
    }

    private static void assertErrorResponse(
            Map<String, String> params,
            String expectedErrorMessage,
            SessionID sessionId,
            EndSessionController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, sessionId, controller);
        TestContext.validateLogoutErrorResponse(response, expectedErrorMessage);
    }

    private static MockHttpServletResponse doRequest(
            Map<String, String> params,
            SessionID sessionId,
            EndSessionController controller) throws Exception {
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        if (sessionId != null) {
            request.setCookies(new Cookie(TestContext.SESSION_COOKIE_NAME, sessionId.getValue()));
        }
        MockHttpServletResponse response = new MockHttpServletResponse();
        controller.logout(request, response);
        return response;
    }
}
