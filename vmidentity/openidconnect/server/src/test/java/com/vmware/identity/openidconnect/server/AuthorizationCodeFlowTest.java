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
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_PRIVATE_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_COOKIE_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.authnRequestParameters;
import static com.vmware.identity.openidconnect.server.TestContext.clientAssertionClaims;
import static com.vmware.identity.openidconnect.server.TestContext.idmClientBuilder;
import static com.vmware.identity.openidconnect.server.TestContext.initialize;
import static com.vmware.identity.openidconnect.server.TestContext.logoutRequestParameters;
import static com.vmware.identity.openidconnect.server.TestContext.messageSource;
import static com.vmware.identity.openidconnect.server.TestContext.passwordLoginString;
import static com.vmware.identity.openidconnect.server.TestContext.tokenRequestParametersClient;
import static com.vmware.identity.openidconnect.server.TestContext.validateAuthnSuccessResponse;
import static com.vmware.identity.openidconnect.server.TestContext.validateLogoutSuccessResponse;
import static com.vmware.identity.openidconnect.server.TestContext.validateTokenErrorResponse;
import static com.vmware.identity.openidconnect.server.TestContext.validateTokenSuccessResponse;

import java.net.URI;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
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
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.server.TestContext.AuthnResponse;
import com.vmware.identity.openidconnect.server.TestContext.TokenResponse;

/**
 * @author Yehia Zayour
 */
public class AuthorizationCodeFlowTest {
    private static final String CLIENT_ID = "_client_id_ABC_";
    private static final URI POST_LOGOUT_REDIRECT_URI = URI.create("https://relyingparty.com/postlogout");
    private static final URI LOGOUT_URI = URI.create("https://relyingparty.com/logout");

    @BeforeClass
    public static void setup() throws Exception {
        initialize();
    }

    @Test
    public void test() throws Exception {
        OIDCClient client = new OIDCClient.Builder(AuthorizationCodeFlowTest.CLIENT_ID).
                redirectUris(Arrays.asList(REDIRECT_URI.toString())).
                postLogoutRedirectUris(Arrays.asList(AuthorizationCodeFlowTest.POST_LOGOUT_REDIRECT_URI.toString())).
                logoutUri(AuthorizationCodeFlowTest.LOGOUT_URI.toString()).
                tokenEndpointAuthMethod("private_key_jwt").
                tokenEndpointAuthSigningAlg("RS256").
                certSubjectDN(CLIENT_CERT_SUBJECT_DN).build();
        CasIdmClient idmClient = idmClientBuilder().additionalClient(client).build();

        AuthorizationCodeManager authzCodeManager = new AuthorizationCodeManager();
        SessionManager sessionManager = new SessionManager();
        MessageSource messageSource = messageSource();

        AuthenticationController authnController = new AuthenticationController(idmClient, authzCodeManager, sessionManager, messageSource);
        TokenController tokenController = new TokenController(idmClient, authzCodeManager);
        LogoutController logoutController = new LogoutController(idmClient, sessionManager);

        String scope = "openid offline_access id_groups at_groups rs_admin_server";
        String state = UUID.randomUUID().toString();
        String nonce = UUID.randomUUID().toString();

        String[] result = doAuthnRequest(authnController, state, nonce, scope);
        doTokenRequest(tokenController, result[0], nonce, scope);
        doLogoutRequest(logoutController, result[1]);
    }

    // returns authzCode and sessionId
    private static String[] doAuthnRequest(
            AuthenticationController authnController,
            String state,
            String nonce,
            String scope) throws Exception {
        Map<String, String> params = authnRequestParameters(Flow.AUTHZ_CODE);
        params.put("state", state);
        params.put("nonce", nonce);
        params.put("scope", scope);

        // request with no session and no login string results in login form
        MockHttpServletRequest request = TestUtil.createGetRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        ModelAndView modelView = authnController.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        Assert.assertEquals("status", 200, response.getStatus());
        Assert.assertTrue("ModelAndView!=null", modelView != null); // login form is returned

        // submit non-matching username/password should result in 401 Unauthorized
        Map<String, String> formParams = new HashMap<String, String>();
        formParams.put("CastleAuthorization", passwordLoginString("invalid_username0", "invalid_password0"));
        request = TestUtil.createPostRequestWithQueryString(formParams, params);
        response = new MockHttpServletResponse();
        modelView = authnController.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        Assert.assertEquals("status", 401, response.getStatus());
        Assert.assertTrue("ModelAndView==null", modelView == null);

        // submit correct credentials should work, returns 302 redirect to requested redirect_uri
        formParams = new HashMap<String, String>();
        formParams.put("CastleAuthorization", passwordLoginString());
        request = TestUtil.createPostRequestWithQueryString(formParams, params);
        response = new MockHttpServletResponse();
        modelView = authnController.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        Assert.assertTrue("ModelAndView==null", modelView == null);
        AuthnResponse authnResponse = validateAuthnSuccessResponse(
                response,
                Flow.AUTHZ_CODE,
                Scope.parse(scope),
                false /* redirectResponseMode */,
                true /* ajaxRequest */,
                state,
                nonce);
        Cookie sessionCookie = response.getCookie(SESSION_COOKIE_NAME);
        Assert.assertTrue("sessionCookie != null", sessionCookie != null);

        // SSO: login to another client using the session cookie
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.issuer(AuthorizationCodeFlowTest.CLIENT_ID);
        claimsBuilder = claimsBuilder.subject(AuthorizationCodeFlowTest.CLIENT_ID);
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        params.put("client_id", AuthorizationCodeFlowTest.CLIENT_ID);
        request = TestUtil.createGetRequest(params);
        request.setCookies(new Cookie(SESSION_COOKIE_NAME, sessionCookie.getValue()));
        response = new MockHttpServletResponse();
        modelView = authnController.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        Assert.assertTrue("ModelAndView==null", modelView == null);
        AuthnResponse secondAuthnResponse = validateAuthnSuccessResponse(
                response,
                Flow.AUTHZ_CODE,
                Scope.parse(scope),
                false /* redirectResponseMode */,
                false /* ajaxRequest */,
                state,
                nonce);
        Assert.assertTrue("authzCode != secondAuthzCode", !Objects.equals(authnResponse.getAuthzCode(), secondAuthnResponse.getAuthzCode()));

        return new String[] { authnResponse.getAuthzCode(), sessionCookie.getValue() };
    }

    private static void doTokenRequest(
            TokenController tokenController,
            String authzCode,
            String nonce,
            String scope) throws Exception {
        Map<String, String> params = tokenRequestParametersClient(Flow.AUTHZ_CODE);
        params.put("code", authzCode);

        // making a request using the previously issued authzCode should succeed
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        tokenController.acquireTokens(request, response);
        TokenResponse tokenResponse = validateTokenSuccessResponse(
                response,
                Flow.AUTHZ_CODE,
                Scope.parse(scope),
                false /* wSltnAssertion */,
                true /* wClientAssertion */,
                nonce);

        // making another request using the same authzCode should fail (authzCode can only be used once)
        request = TestUtil.createPostRequest(params);
        response = new MockHttpServletResponse();
        tokenController.acquireTokens(request, response);
        validateTokenErrorResponse(response, Flow.AUTHZ_CODE, "invalid_grant", "invalid authorization code");

        // making a request using the previously issued refreshToken should succeed
        params = tokenRequestParametersClient(Flow.REFRESH_TOKEN);
        params.put("refresh_token", tokenResponse.getRefreshToken().serialize());

        request = TestUtil.createPostRequest(params);
        response = new MockHttpServletResponse();
        tokenController.acquireTokens(request, response);
        TokenResponse secondTokenResponse = validateTokenSuccessResponse(
                response,
                Flow.REFRESH_TOKEN,
                Scope.parse(scope),
                false /* wSltnAssertion */,
                true /* wClientAssertion */,
                null /* expecteNonce */);
        Assert.assertNull("secondRefreshToken", secondTokenResponse.getRefreshToken());

        // making another request using the same refreshToken should succeed
        request = TestUtil.createPostRequest(params);
        response = new MockHttpServletResponse();
        tokenController.acquireTokens(request, response);
        validateTokenSuccessResponse(
                response,
                Flow.REFRESH_TOKEN,
                Scope.parse(scope),
                false /* wSltnAssertion */,
                true /* wClientAssertion */,
                null /* expectedNonce */);
    }

    private static void doLogoutRequest(
            LogoutController logoutController,
            String sessionId) throws Exception {
        Map<String, String> params = logoutRequestParameters();
        MockHttpServletRequest request = TestUtil.createGetRequest(params);
        request.setCookies(new Cookie(SESSION_COOKIE_NAME, sessionId));
        MockHttpServletResponse response = new MockHttpServletResponse();
        logoutController.logout(request, response, TENANT_NAME);
        validateLogoutSuccessResponse(
                response,
                true /* expectingLogoutUriLinks */,
                sessionId,
                new URI[] { AuthorizationCodeFlowTest.LOGOUT_URI }); // SLO: logout request goes to the other client
    }
}