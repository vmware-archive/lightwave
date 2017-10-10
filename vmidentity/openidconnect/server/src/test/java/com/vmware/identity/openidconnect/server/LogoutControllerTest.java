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

import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_CERT_SUBJECT_DN;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_ID;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_PRIVATE_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.ISSUER;
import static com.vmware.identity.openidconnect.server.TestContext.LOGOUT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.PERSON_USER;
import static com.vmware.identity.openidconnect.server.TestContext.POST_LOGOUT_REDIRECT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_COOKIE_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_ID;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_PRIVATE_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.clientAssertionClaims;
import static com.vmware.identity.openidconnect.server.TestContext.idTokenClaims;
import static com.vmware.identity.openidconnect.server.TestContext.idmClientBuilder;
import static com.vmware.identity.openidconnect.server.TestContext.initialize;
import static com.vmware.identity.openidconnect.server.TestContext.logoutController;
import static com.vmware.identity.openidconnect.server.TestContext.logoutRequestParameters;
import static com.vmware.identity.openidconnect.server.TestContext.sessionManager;
import static com.vmware.identity.openidconnect.server.TestContext.validateLogoutErrorResponse;
import static com.vmware.identity.openidconnect.server.TestContext.validateLogoutSuccessResponse;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
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
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class LogoutControllerTest {
    @BeforeClass
    public static void setup() throws Exception {
        initialize();
    }

    @Test
    public void testLogoutUsingPost() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        MockHttpServletResponse response = doRequest(params, (SessionID) null, logoutController(), true /* isPost */);
        validateLogoutSuccessResponse(
                response,
                false /* expectingLogoutUriLinks */);
    }

    @Test
    public void testLogoutWithRedirectWithState() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        assertSuccessResponse(
                params,
                false /* expectingLogoutUriLinks */);
    }

    @Test
    public void testLogoutWithMatchingSessionCookie() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        SessionID sessionId = new SessionID(SESSION_ID);
        SessionManager sessionManager = sessionManager();
        LogoutController controller = logoutController();
        controller.setSessionManager(sessionManager);
        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionId) != null);
        assertSuccessResponse(
                params,
                true /* expectingLogoutUriLinks */,
                sessionId,
                controller);
        Assert.assertTrue("sessionManager.get(sessionId)==null", sessionManager.get(sessionId) == null);
    }

    @Test
    public void testLogoutWithNonMatchingSessionCookie() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        SessionID sessionIdMatching = new SessionID(SESSION_ID);
        SessionID sessionIdNonMatching = new SessionID(SESSION_ID + "non_matching");
        SessionManager sessionManager = sessionManager();
        LogoutController controller = logoutController();
        controller.setSessionManager(sessionManager);
        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionIdMatching) != null);
        assertSuccessResponse(
                params,
                false /* expectingLogoutUriLinks */,
                sessionIdNonMatching,
                controller);
        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionIdMatching) != null);
    }

    @Test
    public void testLogoutFromPersonUserCertLogin() throws Exception {
        Map<String, String> params = logoutRequestParameters();

        SessionManager sessionManager = new SessionManager();
        SessionID sessionId = new SessionID(SESSION_ID);
        ClientInfo clientInfo = new ClientInfo(
                new ClientID(CLIENT_ID),
                Collections.singleton(REDIRECT_URI),
                Collections.singleton(POST_LOGOUT_REDIRECT_URI),
                LOGOUT_URI,
                CLIENT_CERT_SUBJECT_DN,
                0L /* authnRequestClientAssertionLifetimeMs */);
        sessionManager.add(sessionId, PERSON_USER, LoginMethod.PERSON_USER_CERTIFICATE, clientInfo);

        LogoutController controller = logoutController();
        controller.setSessionManager(sessionManager);

        Assert.assertTrue("sessionManager.get(sessionId)!=null", sessionManager.get(sessionId) != null);
        MockHttpServletResponse response = doRequest(params, sessionId, controller, false /* isPost */);
        validateLogoutSuccessResponse(response, false /* expectingLogoutUriLinks */);
        Assert.assertTrue("sessionManager.get(sessionId)==null", sessionManager.get(sessionId) == null);
        Assert.assertNotNull("person_user_cert_logged_out", response.getCookie(SessionManager.getPersonUserCertificateLoggedOutCookieName(TENANT_NAME)));
    }

    @Test
    public void testLogoutWithoutCorrelationId() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.remove("correlation_id");
        assertSuccessResponse(
                params,
                false /* expectingLogoutUriLinks */);
    }

    @Test
    public void testLogoutClientAssertionNotRequired() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.remove("client_assertion");

        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        LogoutController controller = logoutController(idmClient);

        assertSuccessResponse(
                params,
                false /* expectingLogoutUriLinks */,
                (SessionID) null,
                controller);
    }

    @Test
    public void testLogoutClientAssertionMissing() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.remove("client_assertion");
        assertErrorResponse(params, "invalid_client", "client_assertion parameter is required since client has registered a cert");
    }

    @Test
    public void testLogoutClientAssertionInvalidAudience() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.audience("https://sso.com/invalid_logout_endpoint");
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(params, "invalid_client", "client_assertion audience does not match request URI");
    }

    @Test
    public void testLogoutClientAssertionStale() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        Date now = new Date();
        Date issuedAt = new Date(now.getTime() - (5 * 60 * 1000L)); // issued 5 mins ago
        claimsBuilder = claimsBuilder.issueTime(issuedAt);
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(params, "invalid_client", "client_assertion has expired");
    }

    @Test
    public void testLogoutClientAssertionIncorrectTokenClass() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "solution_user_assertion");
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(params, "invalid_request", "client_assertion has incorrect token_class claim");
    }

    @Test
    public void testNonExistentTenant() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();

        LogoutController controller = logoutController();
        controller.logout(request, response, "non_matching_tenant");

        String expectedErrorMessage = "invalid_request: non-existent tenant";

        Assert.assertEquals("expectedErrorMessage", expectedErrorMessage, response.getErrorMessage());
        Assert.assertTrue("response.getStatus", response.getStatus() == 400);
        Assert.assertNull("sessionCookie", response.getCookie(SessionManager.getSessionCookieName(TENANT_NAME)));
    }

    @Test
    public void testUnregisteredClient() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        CasIdmClient idmClient = idmClientBuilder().clientId(CLIENT_ID + "non_matching").build();
        LogoutController controller = logoutController(idmClient);
        assertErrorResponseMessage(params, "invalid_client: unregistered client", null, controller);
    }

    @Test
    public void testUnregisteredRedirectUri() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.put("post_logout_redirect_uri", params.get("post_logout_redirect_uri") + "non_matching");
        String expectedErrorMessage = "invalid_request: unregistered post_logout_redirect_uri";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testInvalidRedirectUri() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.put("post_logout_redirect_uri", "a.com/redirect");
        String expectedErrorMessage = "invalid_request: invalid post_logout_redirect_uri parameter";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testRedirectUriMissing() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.remove("post_logout_redirect_uri");
        String expectedErrorMessage = "invalid_request: missing post_logout_redirect_uri parameter";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testStateMissing() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.remove("state");
        String expectedErrorMessage = "invalid_request: missing state parameter";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testStateInvalid() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.put("state", "\"");
        String expectedErrorMessage = "invalid_request: state parameter must be html friendly";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissing() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.remove("id_token_hint");
        String expectedErrorMessage = "invalid_request: missing id_token_hint parameter";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalid() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        params.put("id_token_hint", "invalid_id_token_hint_jwt");
        String expectedErrorMessage = "invalid_request: failed to parse id_token_hint parameter";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintPlainJwt() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        PlainJWT idToken = new PlainJWT(claimsBuilder.build());
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: failed to parse id_token_hint parameter";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalidTokenType() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_type", "Not-Bearer");
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token has invalid token_type claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalidTokenClassClaimType() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_class", true); // token_class should be a string claim
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token has non-string token_class claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintIncorrectTokenClass() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "access_token"); // should be id_token
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token has incorrect token_class claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintInvalidTokenClass() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "not_a_valid_token_class");
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token has invalid token_class claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintEmptyTokenClass() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "");
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing token_class claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingTokenClass() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_class", null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing token_class claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingIssuer() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.issuer(null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing iss claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingSubject() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.subject(null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing sub claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingAudience() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.audience((String) null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing aud claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingIssuedAt() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.issueTime(null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing iat claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingExpiration() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.expirationTime(null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing exp claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintMissingJwtId() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.jwtID(null);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token is missing jti claim";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintIncorrectIssuer() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.issuer(ISSUER + "non_matching");
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorDescription = "id_token has incorrect issuer";
        assertErrorResponse(params, "invalid_request", expectedErrorDescription);
    }

    @Test
    public void testIdTokenHintIncorrectTenant() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.claim("tenant", TENANT_NAME + "non_matching");
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorDescription = "id_token has incorrect tenant";
        assertErrorResponse(params, "invalid_request", expectedErrorDescription);
    }

    @Test
    public void testIdTokenHintNonMatchingSubject() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        claimsBuilder = claimsBuilder.subject("non_matching_subject");
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorDescription = "id_token subject does not match the session user";
        assertErrorResponse(params, "invalid_request", expectedErrorDescription, new SessionID(SESSION_ID), logoutController());
    }

    @Test
    public void testIdTokenHintInvalidAudience() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        List<String> audience = new ArrayList<String>();
        audience.add("aud1");
        audience.add("aud2");
        claimsBuilder = claimsBuilder.audience(audience);
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY);
        params.put("id_token_hint", idToken.serialize());
        String expectedErrorMessage = "invalid_request: id_token_hint must have a single audience value containing the client_id";
        assertErrorResponseMessage(params, expectedErrorMessage);
    }

    @Test
    public void testIdTokenHintClientIdMismatch() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.issuer(CLIENT_ID + "non_matching");
        claimsBuilder = claimsBuilder.subject(CLIENT_ID + "non_matching");
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        String expectedErrorDescription = "client_assertion issuer must match client_id";
        assertErrorResponse(params, "invalid_client", expectedErrorDescription);
    }

    @Test
    public void testIdTokenHintInvalidSignature() throws Exception {
        Map<String, String> params = logoutRequestParameters();
        JWTClaimsSet.Builder claimsBuilder = idTokenClaims();
        SignedJWT idToken = TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY); // should be signed using server private key
        params.put("id_token_hint", idToken.serialize());
        assertErrorResponse(params, "invalid_request", "id_token has an invalid signature");
    }

    private static void assertSuccessResponse(
            Map<String, String> params,
            boolean expectingLogoutUriLinks) throws Exception {
        assertSuccessResponse(params, expectingLogoutUriLinks, null, logoutController());
    }

    private static void assertSuccessResponse(
            Map<String, String> params,
            boolean expectingLogoutUriLinks,
            SessionID sessionId,
            LogoutController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, sessionId, controller, false /* isPost */);
        validateLogoutSuccessResponse(
                response,
                expectingLogoutUriLinks);
        Assert.assertNull("person_user_cert_logged_out", response.getCookie(SessionManager.getPersonUserCertificateLoggedOutCookieName(TENANT_NAME)));
    }

    private static void assertErrorResponse(
            Map<String, String> params,
            String      expectedError,
            String      expectedErrorDescription) throws Exception {
        assertErrorResponse(params, expectedError, expectedErrorDescription, (SessionID) null, logoutController());
    }

    private static void assertErrorResponse(
            Map<String, String> params,
            String      expectedError,
            String      expectedErrorDescription,
            SessionID   sessionId,
            LogoutController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, sessionId, controller, false /* isPost */);
        validateLogoutErrorResponse(response, expectedError, expectedErrorDescription);
    }

    private static void assertErrorResponseMessage(
            Map<String, String> params,
            String expectedErrorMessage) throws Exception {
        assertErrorResponseMessage(params, expectedErrorMessage, null, logoutController());
    }

    private static void assertErrorResponseMessage(
            Map<String, String> params,
            String expectedErrorMessage,
            SessionID sessionId,
            LogoutController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, sessionId, controller, false /* isPost */);
        Assert.assertEquals("expectedErrorMessage", expectedErrorMessage, response.getErrorMessage());
        Assert.assertTrue("response.getStatus", response.getStatus() == 400 || response.getStatus() == 401);
        Assert.assertNull("sessionCookie", response.getCookie(SessionManager.getSessionCookieName(TENANT_NAME)));
    }

    private static MockHttpServletResponse doRequest(
            Map<String, String> params,
            SessionID sessionId,
            LogoutController controller,
            boolean isPost) throws Exception {
        MockHttpServletRequest request = isPost ? TestUtil.createPostRequest(params) : TestUtil.createGetRequest(params);
        if (sessionId != null) {
            request.setCookies(new Cookie(SESSION_COOKIE_NAME, sessionId.getValue()));
        }
        MockHttpServletResponse response = new MockHttpServletResponse();
        controller.logout(request, response, TENANT_NAME);
        return response;
    }
}