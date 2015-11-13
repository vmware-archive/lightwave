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

import java.net.URI;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;

import javax.servlet.http.Cookie;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.context.MessageSource;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.ui.ExtendedModelMap;
import org.springframework.web.servlet.ModelAndView;

import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.oauth2.sdk.Scope;
import com.vmware.identity.idm.OIDCClient;

/**
 * @author Yehia Zayour
 */
public class AuthorizationCodeFlowTest {
    private static final String CLIENT_ID = "_client_id_ABC_";
    private static URI POST_LOGOUT_REDIRECT_URI;
    private static URI LOGOUT_URI;

    @BeforeClass
    public static void initialize() throws Exception {
        TestContext.initialize();
        POST_LOGOUT_REDIRECT_URI = new URI("https://relyingparty.com/postlogout");
        LOGOUT_URI = new URI("https://relyingparty.com/logout");
    }

    @Test
    public void test() throws Exception {
        OIDCClient client = new OIDCClient.Builder(CLIENT_ID).
                redirectUris(Arrays.asList(TestContext.REDIRECT_URI.toString())).
                postLogoutRedirectUris(Arrays.asList(POST_LOGOUT_REDIRECT_URI.toString())).
                logoutUri(LOGOUT_URI.toString()).
                tokenEndpointAuthMethod("private_key_jwt").
                tokenEndpointAuthSigningAlg("RS256").
                certSubjectDN(TestContext.CLIENT_CERT_SUBJECT_DN).build();
        IdmClient idmClient = TestContext.idmClientBuilder().additionalClient(client).build();

        AuthorizationCodeManager authzCodeManager = new AuthorizationCodeManager();
        SessionManager sessionManager = new SessionManager();
        MessageSource messageSource = TestContext.messageSource();

        AuthorizationController authzEndpoint = new AuthorizationController(idmClient, authzCodeManager, sessionManager, messageSource);
        TokenController tokenEndpoint = new TokenController(idmClient, authzCodeManager);
        EndSessionController logoutEndpoint = new EndSessionController(idmClient, sessionManager);

        String scope = "openid offline_access id_groups at_groups rs_admin_server";
        String state = UUID.randomUUID().toString();
        String nonce = UUID.randomUUID().toString();

        String[] result = doAuthnRequest(authzEndpoint, state, nonce, scope);
        doTokenRequest(tokenEndpoint, result[0], nonce, scope);
        doLogoutRequest(logoutEndpoint, result[1]);
    }

    // returns authzCode and sessionId
    private static String[] doAuthnRequest(
            AuthorizationController authzEndpoint,
            String state,
            String nonce,
            String scope) throws Exception {
        Map<String, String> params = TestContext.authnRequestParameters(Flow.AUTHZ_CODE);
        params.put("state", state);
        params.put("nonce", nonce);
        params.put("scope", scope);

        // request with no session and no login string results in login form
        MockHttpServletRequest request = TestUtil.createGetRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        ModelAndView modelView = authzEndpoint.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response);
        Assert.assertEquals("status", 200, response.getStatus());
        Assert.assertTrue("ModelAndView!=null", modelView != null); // login form is returned

        // submit non-matching username/password should result in 401 Unauthorized
        Map<String, String> formParams = new HashMap<String, String>();
        formParams.put("CastleAuthorization", TestContext.passwordLoginString("invalid_username0", "invalid_password0"));
        request = TestUtil.createPostRequestWithQueryString(formParams, params);
        response = new MockHttpServletResponse();
        modelView = authzEndpoint.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response);
        Assert.assertEquals("status", 401, response.getStatus());
        Assert.assertTrue("ModelAndView==null", modelView == null);

        // submit correct credentials should work, returns 302 redirect to requested redirect_uri
        formParams = new HashMap<String, String>();
        formParams.put("CastleAuthorization", TestContext.passwordLoginString());
        request = TestUtil.createPostRequestWithQueryString(formParams, params);
        response = new MockHttpServletResponse();
        modelView = authzEndpoint.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response);
        Assert.assertTrue("ModelAndView==null", modelView == null);
        String authzCode = TestContext.validateAuthnSuccessResponse(Flow.AUTHZ_CODE, response, Scope.parse(scope), state, nonce, false, true);
        Cookie sessionCookie = response.getCookie(TestContext.SESSION_COOKIE_NAME);
        Assert.assertTrue("sessionCookie != null", sessionCookie != null);

        // SSO: login to another client using the session cookie
        JWTClaimsSet clientAssertionClaims = TestContext.clientAssertionClaims();
        clientAssertionClaims.setIssuer(CLIENT_ID);
        clientAssertionClaims.setSubject(CLIENT_ID);
        params.put("client_assertion", Shared.sign(clientAssertionClaims, TestContext.CLIENT_PRIVATE_KEY).serialize());
        params.put("client_id", CLIENT_ID);
        request = TestUtil.createGetRequest(params);
        request.setCookies(new Cookie(TestContext.SESSION_COOKIE_NAME, sessionCookie.getValue()));
        response = new MockHttpServletResponse();
        modelView = authzEndpoint.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response);
        Assert.assertTrue("ModelAndView==null", modelView == null);
        String secondAuthzCode = TestContext.validateAuthnSuccessResponse(Flow.AUTHZ_CODE, response, Scope.parse(scope), state, nonce, false, false);
        Assert.assertTrue("!authzCode.equals(secondAuthzCode)", !authzCode.equals(secondAuthzCode));

        return new String[] { authzCode, sessionCookie.getValue() };
    }

    private static void doTokenRequest(
            TokenController tokenEndpoint,
            String authzCode,
            String nonce,
            String scope) throws Exception {
        Map<String, String> params = TestContext.tokenRequestParametersClient(Flow.AUTHZ_CODE);
        params.put("code", authzCode);

        // making a request using the previously issued authzCode should succeed
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        tokenEndpoint.token(request, response);
        String refreshToken = TestContext.validateTokenSuccessResponse(
                Flow.AUTHZ_CODE,
                response,
                Scope.parse(scope),
                false /* wSltnAssertion */,
                true /* wClientAssertion */,
                nonce);

        // making another request using the same authzCode should fail (authzCode can only be used once)
        request = TestUtil.createPostRequest(params);
        response = new MockHttpServletResponse();
        tokenEndpoint.token(request, response);
        TestContext.validateTokenErrorResponse(Flow.AUTHZ_CODE, response, "invalid_grant", "invalid authorization code");

        // making a request using the previously issued refreshToken should succeed
        params = TestContext.tokenRequestParametersClient(Flow.REFRESH_TOKEN);
        params.put("refresh_token", refreshToken);

        request = TestUtil.createPostRequest(params);
        response = new MockHttpServletResponse();
        tokenEndpoint.token(request, response);
        String secondRefreshToken = TestContext.validateTokenSuccessResponse(
                Flow.REFRESH_TOKEN,
                response,
                Scope.parse(scope),
                false /* wSltnAssertion */,
                true /* wClientAssertion */,
                null /* nonce */);
        Assert.assertNull("secondRefreshToken", secondRefreshToken);

        // making another request using the same refreshToken should succeed
        request = TestUtil.createPostRequest(params);
        response = new MockHttpServletResponse();
        tokenEndpoint.token(request, response);
        TestContext.validateTokenSuccessResponse(
                Flow.REFRESH_TOKEN,
                response,
                Scope.parse(scope),
                false /* wSltnAssertion */,
                true /* wClientAssertion */,
                null /* nonce */);
    }

    private static void doLogoutRequest(
            EndSessionController logoutEndpoint,
            String sessionId) throws Exception {
        Map<String, String> params = TestContext.logoutRequestParameters();
        MockHttpServletRequest request = TestUtil.createGetRequest(params);
        request.setCookies(new Cookie(TestContext.SESSION_COOKIE_NAME, sessionId));
        MockHttpServletResponse response = new MockHttpServletResponse();
        logoutEndpoint.logout(request, response);
        TestContext.validateLogoutSuccessResponse(
                response,
                true /* redirect */,
                true /* withState */,
                true /* imageLinksExpected*/,
                true /* sessionCookieExpected */,
                sessionId,
                new URI[] { LOGOUT_URI, TestContext.LOGOUT_URI }); // SLO: logged out of both clients
    }
}
