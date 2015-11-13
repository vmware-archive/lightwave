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
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Security;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Date;
import java.util.HashSet;
import java.util.Map;

import javax.servlet.http.Cookie;

import net.minidev.json.JSONObject;

import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.junit.Assert;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jose.JWSVerifier;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.util.Base64;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.AuthorizationCode;
import com.nimbusds.oauth2.sdk.AuthorizationCodeGrant;
import com.nimbusds.oauth2.sdk.AuthorizationGrant;
import com.nimbusds.oauth2.sdk.ClientCredentialsGrant;
import com.nimbusds.oauth2.sdk.RefreshTokenGrant;
import com.nimbusds.oauth2.sdk.ResourceOwnerPasswordCredentialsGrant;
import com.nimbusds.oauth2.sdk.ResponseType;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.auth.PrivateKeyJWT;
import com.nimbusds.oauth2.sdk.auth.Secret;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.oauth2.sdk.id.JWTID;
import com.nimbusds.oauth2.sdk.id.State;
import com.nimbusds.oauth2.sdk.token.RefreshToken;
import com.nimbusds.oauth2.sdk.util.URLUtils;
import com.nimbusds.openid.connect.sdk.Nonce;
import com.nimbusds.openid.connect.sdk.OIDCScopeValue;
import com.nimbusds.openid.connect.sdk.ResponseMode;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientMetadata;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.openidconnect.common.AuthenticationRequest;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.GssTicketGrant;
import com.vmware.identity.openidconnect.common.IDToken;
import com.vmware.identity.openidconnect.common.LogoutRequest;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.SolutionUserCredentialsGrant;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenErrorResponse;
import com.vmware.identity.openidconnect.common.TokenRequest;
import com.vmware.identity.openidconnect.common.TokenSuccessResponse;

/**
 * @author Yehia Zayour
 */
public class TestContext {
    public static final String TENANT_NAME = "tenant_name";
    public static final String ISSUER = "https://psc.vmware.com/openidconnect/" + TENANT_NAME;
    public static final String SCOPE_VALUE_RSX = "rs_x";
    public static final String CLIENT_ID = "_client_id_xyz_";
    public static final String STATE = "_state_xyz_";
    public static final String LOGOUT_STATE = "_logout_state_xyz_";
    public static final String NONCE = "_nonce_xyz_";
    public static final String AUTHZ_CODE = "_authz_code_xyz_";
    public static final String USERNAME = "_username_xyz_";
    public static final String PASSWORD = "_password_xyz_";
    public static final String SESSION_ID = "_session_id_xyz_";
    public static final String SOLUTION_USERNAME = "_solution_username_xyz_";
    public static final String CLIENT_CERT_SUBJECT_DN = "OU=abc,C=US,DC=local,DC=vsphere,CN=_solution_username_xyz_";
    public static final String GSS_CONTEXT_ID = "_context_id_xyz_";
    public static final PersonUser PERSON_USER = new PersonUser(new PrincipalId(USERNAME, TENANT_NAME), TENANT_NAME);
    public static URI AUTHZ_ENDPOINT_URI;
    public static URI TOKEN_ENDPOINT_URI;
    public static URI LOGOUT_ENDPOINT_URI;
    public static URI REDIRECT_URI;
    public static URI POST_LOGOUT_REDIRECT_URI;
    public static URI LOGOUT_URI;
    public static SolutionUser SOLUTION_USER;
    public static String SESSION_COOKIE_NAME;
    public static RSAPrivateKey TENANT_PRIVATE_KEY;
    public static RSAPublicKey TENANT_PUBLIC_KEY;
    public static X509Certificate TENANT_CERT;
    public static RSAPrivateKey CLIENT_PRIVATE_KEY;
    public static RSAPublicKey CLIENT_PUBLIC_KEY;
    public static X509Certificate CLIENT_CERT;

    public static void initialize() throws Exception {
        AUTHZ_ENDPOINT_URI          = new URI("https://identity.vmware.com/authz");
        TOKEN_ENDPOINT_URI          = new URI("https://identity.vmware.com/token");
        LOGOUT_ENDPOINT_URI         = new URI("https://identity.vmware.com/logout");
        REDIRECT_URI                = new URI("https://vcenter-server.com/relying-party/redirect");
        POST_LOGOUT_REDIRECT_URI    = new URI("https://vcenter-server.com/relying-party/post-logout-redirect");
        LOGOUT_URI                  = new URI("https://vcenter-server.com/relying-party/logout");

        SESSION_COOKIE_NAME = Shared.getSessionCookieName(TENANT_NAME);

        Security.addProvider(new BouncyCastleProvider());
        KeyPairGenerator keyGenerator = KeyPairGenerator.getInstance("RSA", "BC");
        keyGenerator.initialize(1024, new SecureRandom());

        KeyPair kp = keyGenerator.genKeyPair();
        TENANT_PRIVATE_KEY = (RSAPrivateKey) kp.getPrivate();
        TENANT_PUBLIC_KEY = (RSAPublicKey) kp.getPublic();
        TENANT_CERT = TestUtil.generateCertificate(kp, "CN=server");

        kp = keyGenerator.genKeyPair();
        CLIENT_PRIVATE_KEY = (RSAPrivateKey) kp.getPrivate();
        CLIENT_PUBLIC_KEY = (RSAPublicKey) kp.getPublic();
        CLIENT_CERT = TestUtil.generateCertificate(kp, CLIENT_CERT_SUBJECT_DN);

        SOLUTION_USER = new SolutionUser(
                new PrincipalId(SOLUTION_USERNAME, TENANT_NAME),
                TENANT_NAME,
                CLIENT_CERT);
    }

    public static AuthorizationController authzController() {
        return authzController(idmClient());
    }

    public static AuthorizationController authzController(IdmClient idmClient) {
        return new AuthorizationController(idmClient, authzCodeManager(), sessionManager(), messageSource());
    }

    public static TokenController tokenController() {
        return tokenController(idmClient());
    }

    public static TokenController tokenController(IdmClient idmClient) {
        return new TokenController(idmClient, authzCodeManager());
    }

    public static EndSessionController endSessionController() {
        return endSessionController(idmClient());
    }

    public static EndSessionController endSessionController(IdmClient idmClient) {
        return new EndSessionController(idmClient, sessionManager());
    }

    public static MockIdmClient idmClient() {
        return idmClientBuilder().build();
    }

    public static MockIdmClient.Builder idmClientBuilder() {
        long tokenBearerLifetimeMs        = 1000L * 60 * 5;
        long tokenHokLifetimeMs           = 1000L * 60 * 60 * 2;
        long refreshTokenBearerLifetimeMs = 1000L * 60 * 60 * 6;
        long refreshTokenHokLifetimeMs    = 1000L * 60 * 60 * 24 * 30;
        long clockToleranceMs             = 1000L * 60 * 2;

        return new MockIdmClient.Builder().
                tenantName(TENANT_NAME).
                tenantPrivateKey(TENANT_PRIVATE_KEY).
                tenantCertificate(TENANT_CERT).
                issuer(ISSUER).

                clientId(CLIENT_ID).
                redirectUri(REDIRECT_URI.toString()).
                postLogoutRedirectUri(POST_LOGOUT_REDIRECT_URI.toString()).
                logoutUri(LOGOUT_URI.toString()).
                clientCertSubjectDN(CLIENT_CERT_SUBJECT_DN).
                clientCertificate(CLIENT_CERT).
                tokenEndpointAuthMethod("private_key_jwt").

                username(USERNAME).
                password(PASSWORD).
                gssContextId(GSS_CONTEXT_ID).
                personUserEnabled(true).

                solutionUsername(SOLUTION_USERNAME).
                solutionUserEnabled(true).
                isMemberOfSystemGroup(true).

                maxBearerTokenLifetime(tokenBearerLifetimeMs).
                maxHoKTokenLifetime(tokenHokLifetimeMs).
                maxBearerRefreshTokenLifetime(refreshTokenBearerLifetimeMs).
                maxHoKRefreshTokenLifetime(refreshTokenHokLifetimeMs).
                clockTolerance(clockToleranceMs);
    }

    public static AuthorizationCodeManager authzCodeManager() {
        AuthorizationCodeManager authzCodeManager = new AuthorizationCodeManager();

        AuthenticationRequest originalAuthnRequest = new AuthenticationRequest(
                AUTHZ_ENDPOINT_URI,
                new ResponseType(ResponseType.Value.CODE),
                ResponseMode.FORM_POST,
                new ClientID(CLIENT_ID),
                REDIRECT_URI,
                new Scope(OIDCScopeValue.OPENID),
                new State(STATE),
                new Nonce(NONCE),
                (SignedJWT) null,
                new CorrelationID());
        authzCodeManager.add(
                new AuthorizationCode(AUTHZ_CODE),
                PERSON_USER,
                new SessionID(SESSION_ID),
                originalAuthnRequest);

        return authzCodeManager;
    }

    public static SessionManager sessionManager() {
        SessionManager sessionManager = new SessionManager();

        OIDCClientMetadata clientMetadata = new OIDCClientMetadata();
        clientMetadata.setRedirectionURI(REDIRECT_URI);
        clientMetadata.setPostLogoutRedirectionURIs(new HashSet<URI>(Arrays.asList(POST_LOGOUT_REDIRECT_URI)));
        clientMetadata.setCustomField("logout_uri", LOGOUT_URI.toString());
        clientMetadata.setCustomField("cert_subject_dn", CLIENT_CERT_SUBJECT_DN);

        OIDCClientInformation clientInfo = new OIDCClientInformation(
                new ClientID(CLIENT_ID),
                new Date(),
                clientMetadata,
                new Secret());

        sessionManager.add(new SessionID(SESSION_ID), PERSON_USER, clientInfo);
        return sessionManager;
    }

    public static ResourceBundleMessageSource messageSource() {
        ResourceBundleMessageSource messageSource = new ResourceBundleMessageSource();
        messageSource.setBasename("messages");
        return messageSource;
    }

    public static JWTClaimsSet idTokenClaims() {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", "id_token");
        claimsSet.setClaim("token_type", "Bearer");
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(ISSUER);
        claimsSet.setSubject(PERSON_USER.getSubject().getValue());
        claimsSet.setAudience(CLIENT_ID);
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        return claimsSet;
    }

    public static JWTClaimsSet refreshTokenClaims() {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", "refresh_token");
        claimsSet.setClaim("token_type", "Bearer");
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(ISSUER);
        claimsSet.setSubject(PERSON_USER.getSubject().getValue());
        claimsSet.setAudience(PERSON_USER.getSubject().getValue());
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsSet.setClaim("tenant", TENANT_NAME);
        claimsSet.setClaim("scope", "openid");
        claimsSet.setClaim("sid", SESSION_ID);
        return claimsSet;
    }

    public static JWTClaimsSet refreshTokenClaimsSltn() {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", "refresh_token");
        claimsSet.setClaim("token_type", "hotk-pk");
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(ISSUER);
        claimsSet.setSubject(PERSON_USER.getSubject().getValue());
        claimsSet.setAudience(SOLUTION_USER.getSubject().getValue());
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsSet.setClaim("tenant", TENANT_NAME);
        claimsSet.setClaim("scope", "openid");
        claimsSet.setClaim("sid", SESSION_ID);
        claimsSet.setClaim("act_as", SOLUTION_USER.getSubject().getValue());
        return claimsSet;
    }

    public static JWTClaimsSet refreshTokenClaimsClient() {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", "refresh_token");
        claimsSet.setClaim("token_type", "hotk-pk");
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(ISSUER);
        claimsSet.setSubject(PERSON_USER.getSubject().getValue());
        claimsSet.setAudience(CLIENT_ID);
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsSet.setClaim("tenant", TENANT_NAME);
        claimsSet.setClaim("scope", "openid");
        claimsSet.setClaim("sid", SESSION_ID);
        claimsSet.setClaim("act_as", SOLUTION_USER.getSubject().getValue());
        claimsSet.setClaim("client_id", CLIENT_ID);
        return claimsSet;
    }

    public static JWTClaimsSet sltnAssertionClaims() {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", "solution_assertion");
        claimsSet.setClaim("token_type", "Bearer");
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(CLIENT_CERT_SUBJECT_DN);
        claimsSet.setSubject(CLIENT_CERT_SUBJECT_DN);
        claimsSet.setAudience("https://localhost");
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + 356 * 24 * 60 * 60 * 1000L));
        return claimsSet;
    }

    public static JWTClaimsSet clientAssertionClaims() {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", "client_assertion");
        claimsSet.setClaim("token_type", "Bearer");
        claimsSet.setJWTID((new JWTID()).toString());
        claimsSet.setIssuer(CLIENT_ID);
        claimsSet.setSubject(CLIENT_ID);
        claimsSet.setAudience("https://localhost");
        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        return claimsSet;
    }

    public static String passwordLoginString() {
        return passwordLoginString(USERNAME, PASSWORD);
    }

    public static String passwordLoginString(String username, String password) {
        String unp = username + ":" + password;
        String unp64 = Base64.encode(unp).toString();
        return "Basic " + unp64;
    }

    public static String gssLoginString() {
        return gssLoginString(GSS_CONTEXT_ID);
    }

    public static String gssLoginString(String contextId) {
        return String.format("Negotiate %s _gss_ticket__xyz_", contextId);
    }

    public static Map<String, String> authnRequestParameters(Flow flow) throws Exception {
        return authnRequestParameters(flow, ResponseMode.FORM_POST);
    }

    public static Map<String, String> authnRequestParameters(Flow flow, ResponseMode responseMode) throws Exception {
        assert flow.isAuthzEndpointFlow();

        String responseType = null;
        switch (flow) {
            case AUTHZ_CODE:
                responseType = "code";
                break;
            case IMPLICIT:
                responseType = "token id_token";
                break;
            case IMPLICIT_ID_TOKEN_ONLY:
                responseType = "id_token";
                break;
            default:
                throw new IllegalArgumentException("unrecognized flow value " + flow.toString());
        }

        AuthenticationRequest authnRequest = new AuthenticationRequest(
                AUTHZ_ENDPOINT_URI,
                ResponseType.parse(responseType),
                responseMode,
                new ClientID(CLIENT_ID),
                REDIRECT_URI,
                new Scope("openid"),
                new State(STATE),
                new Nonce(NONCE),
                Shared.sign(clientAssertionClaims(), CLIENT_PRIVATE_KEY),
                new CorrelationID());
        return authnRequest.toParameters();
    }

    public static Map<String, String> tokenRequestParameters(Flow flow) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.AUTHZ_CODE && flow != Flow.CLIENT_CREDS && flow != Flow.SOLUTION_USER_CREDS;

        Object[] grantAndScope = tokenRequestAuthzGrantAndScope(flow, refreshTokenClaims());
        AuthorizationGrant authzGrant = (AuthorizationGrant) grantAndScope[0];
        Scope scope = (Scope) grantAndScope[1];
        TokenRequest tokenRequest = new TokenRequest(TOKEN_ENDPOINT_URI, authzGrant, scope, new CorrelationID());
        return tokenRequest.toHTTPRequest().getQueryParameters();
    }

    public static Map<String, String> tokenRequestParametersSltn(Flow flow) throws Exception {
        return tokenRequestParametersSltn(flow, sltnAssertionClaims());
    }

    public static Map<String, String> tokenRequestParametersSltn(Flow flow, JWTClaimsSet sltnAssertionClaims) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.AUTHZ_CODE && flow != Flow.CLIENT_CREDS;

        SignedJWT solutionAssertion = Shared.sign(sltnAssertionClaims, CLIENT_PRIVATE_KEY);

        Object[] grantAndScope = tokenRequestAuthzGrantAndScope(flow, refreshTokenClaimsSltn());
        AuthorizationGrant authzGrant = (AuthorizationGrant) grantAndScope[0];
        Scope scope = (Scope) grantAndScope[1];
        TokenRequest tokenRequest = new TokenRequest(TOKEN_ENDPOINT_URI, authzGrant, scope, solutionAssertion, new CorrelationID());
        return tokenRequest.toHTTPRequest().getQueryParameters();
    }

    public static Map<String, String> tokenRequestParametersClient(Flow flow) throws Exception {
        return tokenRequestParametersClient(flow, clientAssertionClaims());
    }

    public static Map<String, String> tokenRequestParametersClient(Flow flow, JWTClaimsSet clientAssertionClaims) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.SOLUTION_USER_CREDS;

        SignedJWT clientAssertion = Shared.sign(clientAssertionClaims, CLIENT_PRIVATE_KEY);

        Object[] grantAndScope = tokenRequestAuthzGrantAndScope(flow, refreshTokenClaimsClient());
        AuthorizationGrant authzGrant = (AuthorizationGrant) grantAndScope[0];
        Scope scope = (Scope) grantAndScope[1];
        TokenRequest tokenRequest = new TokenRequest(TOKEN_ENDPOINT_URI, authzGrant, scope, new PrivateKeyJWT(clientAssertion), new CorrelationID());
        return tokenRequest.toHTTPRequest().getQueryParameters();
    }

    public static Map<String, String> logoutRequestParameters() throws Exception {
        LogoutRequest logoutRequest = new LogoutRequest(
                LOGOUT_ENDPOINT_URI,
                new IDToken(Shared.sign(idTokenClaims(), TENANT_PRIVATE_KEY)),
                POST_LOGOUT_REDIRECT_URI,
                new State(LOGOUT_STATE),
                Shared.sign(clientAssertionClaims(), CLIENT_PRIVATE_KEY),
                new CorrelationID());
        return logoutRequest.toParameters();
    }

    // return authzCode if any
    public static String validateAuthnSuccessResponse(
            Flow flow,
            MockHttpServletResponse response,
            Scope scope) throws Exception {
        return validateAuthnSuccessResponse(flow, response, scope, STATE, NONCE, false, false);
    }

    // return authzCode if any
    public static String validateAuthnSuccessResponse(
            Flow flow,
            MockHttpServletResponse response,
            Scope scope,
            String state,
            String nonce,
            boolean redirectResponseMode,
            boolean ajaxRequest) throws Exception {
        assert flow.isAuthzEndpointFlow();

        Assert.assertEquals("status", (redirectResponseMode && !ajaxRequest) ? 302 : 200, response.getStatus());
        Assert.assertEquals("redirectTarget", REDIRECT_URI.toString(), extractAuthnResponseTarget(flow, response, redirectResponseMode, ajaxRequest));
        Assert.assertEquals("error==null", extractAuthnResponseParameter(flow, response, "error", redirectResponseMode, ajaxRequest), null);
        Assert.assertEquals("state", state, extractAuthnResponseParameter(flow, response, "state", redirectResponseMode, ajaxRequest));
        Assert.assertEquals("refreshTokenShouldNotExist", extractAuthnResponseParameter(flow, response, "refresh_token", redirectResponseMode, ajaxRequest), null);

        String authzCode = extractAuthnResponseParameter(flow, response, "code", redirectResponseMode, ajaxRequest);
        if (flow == Flow.AUTHZ_CODE) {
            Assert.assertTrue("authzCode!=null", authzCode != null);
        } else if (flow.isImplicit()) {
            Assert.assertTrue("authzCode==null", authzCode == null);
            Cookie sessionCookie = response.getCookie(Shared.getSessionCookieName(TENANT_NAME));
            String sessionId = (sessionCookie != null) ? sessionCookie.getValue() : SESSION_ID;
            validateIdToken(flow, extractAuthnResponseParameter(flow, response, "id_token", redirectResponseMode, ajaxRequest), nonce, scope, sessionId, false, false);
            if (flow == Flow.IMPLICIT) {
                Assert.assertEquals("token_type==Bearer", "Bearer", extractAuthnResponseParameter(flow, response, "token_type", redirectResponseMode, ajaxRequest));
                Assert.assertEquals("expires_in==300", "300", extractAuthnResponseParameter(flow, response, "expires_in", redirectResponseMode, ajaxRequest));
                validateAccessToken(flow, extractAuthnResponseParameter(flow, response, "access_token", redirectResponseMode, ajaxRequest), nonce, scope, false, false);
            }
            if (flow == Flow.IMPLICIT_ID_TOKEN_ONLY) {
                Assert.assertTrue("access_token==null", extractAuthnResponseParameter(flow, response, "access_token", redirectResponseMode, ajaxRequest) == null);
            }
        }

        Cookie sessionCookie = response.getCookie(SESSION_COOKIE_NAME);
        if (ajaxRequest) {
            Assert.assertNotNull("sessionCookie", sessionCookie);
        } else {
            Assert.assertNull("sessionCookie", sessionCookie);
        }

        return authzCode;
    }

    public static void validateAuthnErrorResponse(
            Flow flow,
            MockHttpServletResponse response,
            String expectedError,
            String expectedErrorDescription) throws Exception {
        validateAuthnErrorResponse(flow, response, expectedError, expectedErrorDescription, false, false);
    }

    public static void validateAuthnErrorResponse(
            Flow flow,
            MockHttpServletResponse response,
            String expectedError,
            String expectedErrorDescription,
            boolean redirectResponseMode,
            boolean ajaxRequest) throws Exception {
        assert flow.isAuthzEndpointFlow();

        Assert.assertEquals("status", (redirectResponseMode && !ajaxRequest) ? 302 : 200, response.getStatus());
        Assert.assertEquals("redirectTarget", REDIRECT_URI.toString(), extractAuthnResponseTarget(flow, response, redirectResponseMode, ajaxRequest));
        Assert.assertEquals("state", STATE, extractAuthnResponseParameter(flow, response, "state", redirectResponseMode, ajaxRequest));
        Assert.assertEquals("error", expectedError, extractAuthnResponseParameter(flow, response, "error", redirectResponseMode, ajaxRequest));
        Assert.assertEquals("error_description", expectedErrorDescription, extractAuthnResponseParameter(flow, response, "error_description", redirectResponseMode, ajaxRequest));
        Assert.assertNull("sessionCookie", response.getCookie(SESSION_COOKIE_NAME));
    }

    // return refresh_token if any
    public static String validateTokenSuccessResponse(
            Flow flow,
            MockHttpServletResponse response,
            Scope scope,
            boolean wSltnAssertion,
            boolean wClientAssertion) throws Exception {
        return validateTokenSuccessResponse(flow, response, scope, wSltnAssertion, wClientAssertion, NONCE);
    }

    // return refresh_token if any
    public static String validateTokenSuccessResponse(
            Flow flow,
            MockHttpServletResponse response,
            Scope scope,
            boolean wSltnAssertion,
            boolean wClientAssertion,
            String nonce) throws Exception {
        assert flow.isTokenEndpointFlow();

        HTTPResponse httpResponse = new HTTPResponse(response.getStatus());
        httpResponse.setContentType(response.getContentType());
        httpResponse.setContent(response.getContentAsString());

        TokenSuccessResponse tokenResponse = TokenSuccessResponse.parse(httpResponse);

        Assert.assertEquals("token_type", (wSltnAssertion || wClientAssertion) ? "hotk-pk" : "Bearer", tokenResponse.getAccessToken().getType().getValue());
        Assert.assertEquals("expires_in", (wSltnAssertion || wClientAssertion) ? 7200 : 300, tokenResponse.getAccessToken().getLifetime());

        validateIdToken(flow, tokenResponse.getIDToken().serialize(), nonce, scope, null /* sessionId */, wSltnAssertion, wClientAssertion);
        validateAccessToken(flow, tokenResponse.getAccessToken().getValue(), nonce, scope, wSltnAssertion, wClientAssertion);

        String refreshToken = (tokenResponse.getRefreshToken() != null) ? tokenResponse.getRefreshToken().getValue() : null;
        boolean refreshTokenShouldExist =
                scope.contains("offline_access") &&
                (flow == Flow.AUTHZ_CODE || flow == Flow.PASSWORD || flow == Flow.GSS_TICKET);
        Assert.assertTrue("refreshTokenShouldExist", refreshTokenShouldExist == (refreshToken != null));
        if (refreshTokenShouldExist) {
            validateRefreshToken(flow, refreshToken, scope, wSltnAssertion, wClientAssertion);
        }

        return refreshToken;
    }

    public static void validateTokenErrorResponse(
            Flow flow,
            MockHttpServletResponse response,
            String expectedError,
            String expectedErrorDescription) throws Exception {
        assert flow.isTokenEndpointFlow();

        HTTPResponse httpResponse = new HTTPResponse(response.getStatus());
        httpResponse.setContentType(response.getContentType());
        httpResponse.setContent(response.getContentAsString());

        TokenErrorResponse tokenResponse = TokenErrorResponse.parse(httpResponse);

        Assert.assertEquals("error", expectedError, tokenResponse.getErrorObject().getCode());
        Assert.assertEquals("error_description", expectedErrorDescription, tokenResponse.getErrorObject().getDescription());
    }

    public static void validateLogoutSuccessResponse(
            MockHttpServletResponse response,
            boolean redirect,
            boolean withState,
            boolean imageLinksExpected,
            boolean sessionCookieExpected) throws Exception {
        validateLogoutSuccessResponse(
                response,
                redirect,
                withState,
                imageLinksExpected,
                sessionCookieExpected,
                SESSION_ID,
                new URI[] { LOGOUT_URI });
    }

    public static void validateLogoutSuccessResponse(
            MockHttpServletResponse response,
            boolean redirect,
            boolean withState,
            boolean imageLinksExpected,
            boolean sessionCookieExpected,
            String sessionId,
            URI[] logoutUris) throws Exception {
        Assert.assertEquals("response.getErrorMessage", null, response.getErrorMessage());
        Assert.assertEquals("response.getStatus", 200, response.getStatus());

        String expectedRedirectTarget;
        if (redirect) {
            if (withState) {
                expectedRedirectTarget = String.format("%s?state=%s", POST_LOGOUT_REDIRECT_URI.toString(), LOGOUT_STATE);
            } else {
                expectedRedirectTarget = POST_LOGOUT_REDIRECT_URI.toString();
            }
        } else {
            expectedRedirectTarget = "";
        }
        String actualRedirectTarget = TestUtil.extractString(response, "var postLogoutRedirectUriWithState = \"", "\"");
        Assert.assertEquals("postLogoutRedirectUriWithState", expectedRedirectTarget, actualRedirectTarget);

        String logoutUriImageLinks = TestUtil.extractString(response, "<!-- logoutUriImageLinks --> ", " <!-- logoutUriImageLinks -->");
        if (imageLinksExpected) {
            int expectedLength = 0;
            for (URI logoutUri : logoutUris) {
                String logoutUriWithSid = String.format("%s?sid=%s", logoutUri, sessionId);
                String imageLink = String.format("<img src=\"%s\">", logoutUriWithSid);
                Assert.assertTrue("logoutUriImageLinks.contains(logoutUriWithSid)", logoutUriImageLinks.contains(imageLink));
                expectedLength += imageLink.length();
            }
            Assert.assertEquals("logoutUriImageLinks.length()", expectedLength, logoutUriImageLinks.length());
        } else {
            Assert.assertEquals("logoutUriImageLinks", "", logoutUriImageLinks);
        }

        Cookie sessionCookie = response.getCookie(Shared.getSessionCookieName(TENANT_NAME));
        Assert.assertTrue("sessionCookieExpected", sessionCookieExpected == (sessionCookie != null));
        if (sessionCookieExpected) {
            Assert.assertTrue("sessionCookie value is empty", sessionCookie.getValue().equals(""));
        }
    }

    public static void validateLogoutErrorResponse(
            MockHttpServletResponse response,
            String expectedErrorMessage) throws Exception {
        Assert.assertEquals("expectedErrorMessage", expectedErrorMessage, response.getErrorMessage());
        Assert.assertTrue("response.getStatus", response.getStatus() == 400 || response.getStatus() == 401);
        Assert.assertNull("sessionCookie", response.getCookie(Shared.getSessionCookieName(TENANT_NAME)));
    }

    private static void validateToken(
            Flow flow,
            String tokenString,
            String nonce,
            Scope scope,
            String sessionId,
            boolean wSltnAssertion,
            boolean wClientAssertion,
            TokenClass tokenClass) throws Exception {
        Assert.assertTrue("tokenString not null or empty", tokenString != null && !tokenString.isEmpty());
        SignedJWT token = SignedJWT.parse(tokenString);
        Assert.assertTrue("token!=null", token != null);

        // verify signature
        JWSVerifier verifier = new RSASSAVerifier(TENANT_PUBLIC_KEY);
        Assert.assertTrue(token.verify(verifier));

        Date now = new Date();
        ReadOnlyJWTClaimsSet claimsSet = token.getJWTClaimsSet();

        Assert.assertEquals("scope", scope.toString(), claimsSet.getStringClaim("scope"));
        Assert.assertEquals("client_id", (wClientAssertion || flow.isImplicit()) ? CLIENT_ID : null, claimsSet.getStringClaim("client_id"));
        Assert.assertEquals("tenant", TENANT_NAME, claimsSet.getStringClaim("tenant"));
        Assert.assertEquals("issuer", ISSUER, claimsSet.getIssuer());

        String expectedSubject = (flow == Flow.SOLUTION_USER_CREDS || flow == Flow.CLIENT_CREDS) ?
                SOLUTION_USER.getSubject().getValue() :
                PERSON_USER.getSubject().getValue();
        Assert.assertEquals("subject", expectedSubject, claimsSet.getSubject());

        String expectedAudience;
        if (wClientAssertion || flow.isImplicit()) {
            expectedAudience = CLIENT_ID;
        } else if (wSltnAssertion) {
            expectedAudience = SOLUTION_USER.getSubject().getValue();
        } else {
            expectedAudience = PERSON_USER.getSubject().getValue();
        }
        Assert.assertTrue("audience", claimsSet.getAudience().contains(expectedAudience));

        Assert.assertTrue("issued at", claimsSet.getIssueTime().before(now));
        Assert.assertTrue("expiration", claimsSet.getExpirationTime().after(now));
        Assert.assertTrue("jwt_id", claimsSet.getJWTID() != null);
        if (flow == Flow.AUTHZ_CODE || flow.isImplicit()) {
            Assert.assertEquals("nonce", nonce, claimsSet.getStringClaim("nonce"));
        } else if (flow == Flow.PASSWORD || flow == Flow.REFRESH_TOKEN) {
            Assert.assertTrue("nonce==null", claimsSet.getStringClaim("nonce") == null);
        }

        Assert.assertEquals("token_type", (wSltnAssertion || wClientAssertion) ? "hotk-pk" : "Bearer", claimsSet.getStringClaim("token_type"));

        if (wSltnAssertion || wClientAssertion) {
            JSONObject hotk = (JSONObject) claimsSet.getClaim("hotk");
            Assert.assertTrue("hotk!=null", hotk != null);
            JWKSet jwkSet = JWKSet.parse(hotk);
            Assert.assertTrue("jwkSet!=null", jwkSet != null);
            RSAPublicKey publicKey = Shared.extractRsa256PublicKey(jwkSet);
            Assert.assertTrue("access_token hotk claim contains CLIENT_PUBLIC_KEY", CLIENT_PUBLIC_KEY.equals(publicKey));

            if (flow != Flow.SOLUTION_USER_CREDS && flow != Flow.CLIENT_CREDS) {
                Assert.assertEquals("act_as", SOLUTION_USER.getSubject().getValue(), claimsSet.getStringClaim("act_as"));
            }
        }

        Assert.assertEquals("token_class", tokenClass.getName(), claimsSet.getStringClaim("token_class"));

        if (tokenClass == TokenClass.ID_TOKEN) {
            boolean idGroupsScope = scope.contains("id_groups");
            boolean idGroupsExist = claimsSet.getClaim("groups") != null;
            Assert.assertTrue("idGroupsScope==idGroupsExist", idGroupsScope == idGroupsExist);

            if (flow.isImplicit() || flow == Flow.AUTHZ_CODE) {
                Assert.assertNotNull("sid", claimsSet.getStringClaim("sid"));
            }
            if (flow.isImplicit()) {
                Assert.assertEquals("sid", sessionId, claimsSet.getStringClaim("sid"));
            }
        } else if (tokenClass == TokenClass.ACCESS_TOKEN) {
            boolean atGroupsScope = scope.contains("at_groups");
            boolean atGroupsExist = claimsSet.getClaim("groups") != null;
            Assert.assertTrue("atGroupsScope==atGroupsExist", atGroupsScope == atGroupsExist);

            boolean adminServerScope = scope.contains("rs_admin_server");
            boolean adminServerAudience = claimsSet.getAudience().contains("rs_admin_server");
            boolean adminServerRoleExists = claimsSet.getStringClaim("admin_server_role") != null;
            boolean adminServerRoleGuestUser = ("GuestUser").equals(claimsSet.getStringClaim("admin_server_role"));
            Assert.assertTrue("adminServerScope==adminServerAudience", adminServerScope == adminServerAudience);
            Assert.assertTrue("adminServerScope==adminServerRoleExists", adminServerScope == adminServerRoleExists);
            Assert.assertTrue("adminServerScope==adminServerRoleGuestUser", adminServerScope == adminServerRoleGuestUser);

            boolean rsxScope = scope.contains(SCOPE_VALUE_RSX);
            boolean rsxAudience = claimsSet.getAudience().contains(SCOPE_VALUE_RSX);
            Assert.assertEquals("rsxScope==rsxAudience", rsxScope, rsxAudience);
        } else if (tokenClass == TokenClass.REFRESH_TOKEN) {
            // no-op
        } else {
            throw new IllegalArgumentException("unexpected tokenClass: " + tokenClass);
        }
    }

    private static void validateIdToken(
            Flow flow,
            String tokenString,
            String nonce,
            Scope scope,
            String sessionId,
            boolean wSltnAssertion,
            boolean wClientAssertion) throws Exception {
        validateToken(flow, tokenString, nonce, scope, sessionId, wSltnAssertion, wClientAssertion, TokenClass.ID_TOKEN);
    }

    private static void validateAccessToken(
            Flow flow,
            String tokenString,
            String nonce,
            Scope scope,
            boolean wSltnAssertion,
            boolean wClientAssertion) throws Exception {
        validateToken(flow, tokenString, nonce, scope, null /* sessionId */, wSltnAssertion, wClientAssertion, TokenClass.ACCESS_TOKEN);
    }

    private static void validateRefreshToken(
            Flow flow,
            String tokenString,
            Scope scope,
            boolean wSltnAssertion,
            boolean wClientAssertion) throws Exception {
        validateToken(flow, tokenString, null /* nonce */, scope, null /* sessionId */, wSltnAssertion, wClientAssertion, TokenClass.REFRESH_TOKEN);
    }

    private static String extractAuthnResponseTarget(
            Flow flow,
            MockHttpServletResponse response,
            boolean redirectResponseMode,
            boolean ajaxRequest) throws Exception {
        String result;

        if (redirectResponseMode) {
            String redirectUrl = ajaxRequest ? response.getContentAsString() : response.getRedirectedUrl();
            char separator = (flow == Flow.AUTHZ_CODE) ? '?' : '#';
            result = redirectUrl.substring(0, redirectUrl.indexOf(separator));
        } else {
            // response_mode=form_post
            String prefix = "<form method=\"post\" id=\"SamlPostForm\" action=\"";
            result = TestUtil.extractString(response, prefix, "\"");
        }

        return result;
    }

    private static String extractAuthnResponseParameter(
            Flow flow,
            MockHttpServletResponse response,
            String parameterName,
            boolean redirectResponseMode,
            boolean ajaxRequest) throws Exception {
        String result;

        if (redirectResponseMode) {
            String redirectUrl = ajaxRequest ? response.getContentAsString() : response.getRedirectedUrl();
            char separator = (flow == Flow.AUTHZ_CODE) ? '?' : '#';
            String queryString = redirectUrl.substring(redirectUrl.indexOf(separator) + 1);
            Map<String, String> parameters = URLUtils.parseParameters(queryString);
            result = parameters.get(parameterName);
        } else {
            String prefix = String.format("<input type=\"hidden\" name=\"%s\" value=\"", parameterName);
            result = TestUtil.extractString(response, prefix, "\"");
        }

        return result;
    }

    private static Object[] tokenRequestAuthzGrantAndScope(Flow flow, ReadOnlyJWTClaimsSet refreshTokenClaims) throws Exception {
        assert flow.isTokenEndpointFlow();

        AuthorizationGrant authzGrant;
        Scope scope;

        if (flow == Flow.AUTHZ_CODE) {
            authzGrant = new AuthorizationCodeGrant(new AuthorizationCode(AUTHZ_CODE), REDIRECT_URI);
            scope = null;
        } else if (flow == Flow.PASSWORD) {
            authzGrant = new ResourceOwnerPasswordCredentialsGrant(USERNAME, new Secret(PASSWORD));
            scope = new Scope("openid", "offline_access");
        } else if (flow == Flow.CLIENT_CREDS) {
            authzGrant = new ClientCredentialsGrant();
            scope = new Scope("openid");
        } else if (flow == Flow.SOLUTION_USER_CREDS) {
            authzGrant = new SolutionUserCredentialsGrant();
            scope = new Scope("openid");
        } else if (flow == Flow.GSS_TICKET) {
            authzGrant = new GssTicketGrant(GSS_CONTEXT_ID, new byte[10]);
            scope = new Scope("openid", "offline_access");
        } else if (flow == Flow.REFRESH_TOKEN) {
            authzGrant = new RefreshTokenGrant(new RefreshToken(Shared.sign(refreshTokenClaims, TENANT_PRIVATE_KEY).serialize()));
            scope = null;
        } else {
            throw new IllegalArgumentException("unexpected flow: " + flow);
        }

        return new Object[] { authzGrant, scope };
    }
}
