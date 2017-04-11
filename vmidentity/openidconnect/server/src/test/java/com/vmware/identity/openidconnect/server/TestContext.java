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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.net.URI;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Security;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.servlet.http.Cookie;

import net.minidev.json.JSONObject;
import net.minidev.json.parser.JSONParser;

import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jose.JWSVerifier;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.ScopeValue;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.JWTUtils;

/**
 * @author Yehia Zayour
 */
public class TestContext {
    public static final String SERVER_NAME = "psc.vmware.com";
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
    public static final String SECURID_PASSCODE = "_securid_passcode_xyz_";
    public static final String SESSION_ID = "_session_id_xyz_";
    public static final String SOLUTION_USERNAME = "_solution_username_xyz_";
    public static final String CLIENT_CERT_SUBJECT_DN = "OU=abc,C=US,DC=local,DC=vsphere,CN=_solution_username_xyz_";
    public static final String GSS_CONTEXT_ID = "_context_id_xyz_";
    public static final String ADMIN_SERVER_ROLE = "GuestUser";
    public static final PersonUser PERSON_USER = new PersonUser(new PrincipalId(USERNAME, TENANT_NAME), TENANT_NAME);
    public static final Set<String> GROUP_FILTER_RS_X           = new HashSet<String>(Arrays.asList(                "o\\c", "o\\d"));
    public static final Set<String> GROUP_FILTER_RS_Y           = new HashSet<String>(Arrays.asList(        "o\\b", "o\\c"        ));
    public static final Set<String> GROUP_MEMBERSHIP            = new HashSet<String>(Arrays.asList("o\\A", "o\\B", "o\\C"        ));
    public static final Set<String> GROUP_MEMBERSHIP_FILTERED   = new HashSet<String>(Arrays.asList(        "o\\b", "o\\c"        ));
    public static final URI AUTHZ_ENDPOINT_URI          = URI.create("https://psc.vmware.com/openidconnect/oidc/authorize/" + TENANT_NAME);
    public static final URI TOKEN_ENDPOINT_URI          = URI.create("https://psc.vmware.com/openidconnect/token/" + TENANT_NAME);
    public static final URI LOGOUT_ENDPOINT_URI         = URI.create("https://psc.vmware.com/openidconnect/logout/" + TENANT_NAME);
    public static final URI JWKS_ENDPOINT_URI           = URI.create("https://psc.vmware.com/openidconnect/jwks/" + TENANT_NAME);
    public static final URI REDIRECT_URI                = URI.create("https://vcenter-server.com/relying-party/redirect");
    public static final URI POST_LOGOUT_REDIRECT_URI    = URI.create("https://vcenter-server.com/relying-party/post-logout-redirect");
    public static final URI LOGOUT_URI                  = URI.create("https://vcenter-server.com/relying-party/logout");
    public static SolutionUser SOLUTION_USER;
    public static String SESSION_COOKIE_NAME;
    public static RSAPrivateKey TENANT_PRIVATE_KEY;
    public static RSAPublicKey TENANT_PUBLIC_KEY;
    public static X509Certificate TENANT_CERT;
    public static RSAPrivateKey CLIENT_PRIVATE_KEY;
    public static RSAPublicKey CLIENT_PUBLIC_KEY;
    public static X509Certificate CLIENT_CERT;

    public static void initialize() throws Exception {
        SESSION_COOKIE_NAME = SessionManager.getSessionCookieName(TENANT_NAME);

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

    public static AuthenticationController authnController() {
        return authnController(idmClient());
    }

    public static AuthenticationController authnController(CasIdmClient idmClient) {
        return new AuthenticationController(idmClient, authzCodeManager(), sessionManager(), messageSource());
    }

    public static TokenController tokenController() {
        return tokenController(idmClient());
    }

    public static TokenController tokenController(CasIdmClient idmClient) {
        return new TokenController(idmClient, authzCodeManager());
    }

    public static LogoutController logoutController() {
        return logoutController(idmClient());
    }

    public static LogoutController logoutController(CasIdmClient idmClient) {
        return new LogoutController(idmClient, sessionManager());
    }

    public static MockIdmClient idmClient() {
        return idmClientBuilder().build();
    }

    public static MockIdmClient.Builder idmClientBuilder() {
        long tokenBearerLifetimeMs        = 1000L * 60 * 5;
        long tokenHokLifetimeMs           = 1000L * 60 * 60 * 2;
        long refreshTokenBearerLifetimeMs = 1000L * 60 * 60 * 6;
        long refreshTokenHokLifetimeMs    = 1000L * 60 * 60 * 24 * 30;
        long clockToleranceMs             = 0L;

        boolean allowPasswordAuthn = true;
        boolean allowWindowsSessionAuthn = true;
        boolean allowSmartCardAuthn = false;

        Map<String, ResourceServer> resourceServerMap = new HashMap<String, ResourceServer>();
        resourceServerMap.put("rs_x", new ResourceServer.Builder("rs_x").groupFilter(GROUP_FILTER_RS_X).build());
        resourceServerMap.put("rs_y", new ResourceServer.Builder("rs_y").groupFilter(GROUP_FILTER_RS_Y).build());

        return new MockIdmClient.Builder().
                tenantName(TENANT_NAME).
                tenantPrivateKey(TENANT_PRIVATE_KEY).
                tenantCertificate(TENANT_CERT).
                authnPolicy(new AuthnPolicy(allowPasswordAuthn, allowWindowsSessionAuthn, allowSmartCardAuthn, null /* ClientCertPolicy */)).
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
                securIdPasscode(SECURID_PASSCODE).
                gssContextId(GSS_CONTEXT_ID).
                personUserEnabled(true).

                solutionUsername(SOLUTION_USERNAME).
                solutionUserEnabled(true).

                maxBearerTokenLifetime(tokenBearerLifetimeMs).
                maxHoKTokenLifetime(tokenHokLifetimeMs).
                maxBearerRefreshTokenLifetime(refreshTokenBearerLifetimeMs).
                maxHoKRefreshTokenLifetime(refreshTokenHokLifetimeMs).
                clockTolerance(clockToleranceMs).

                systemGroupMembership(Collections.singleton("ActAsUsers")).
                groupMembership(GROUP_MEMBERSHIP).
                resourceServerMap(resourceServerMap);
    }

    public static AuthorizationCodeManager authzCodeManager() {
        AuthorizationCodeManager authzCodeManager = new AuthorizationCodeManager();

        AuthenticationRequest originalAuthnRequest = new AuthenticationRequest(
                AUTHZ_ENDPOINT_URI,
                ResponseType.authorizationCode(),
                ResponseMode.FORM_POST,
                new ClientID(CLIENT_ID),
                REDIRECT_URI,
                Scope.OPENID,
                new State(STATE),
                new Nonce(NONCE),
                null /* clientAssertion */,
                null /* correlationId */);

        authzCodeManager.add(
                new AuthorizationCode(AUTHZ_CODE),
                PERSON_USER,
                new SessionID(SESSION_ID),
                originalAuthnRequest);

        return authzCodeManager;
    }

    public static SessionManager sessionManager() {
        SessionManager sessionManager = new SessionManager();

        ClientInfo clientInfo = new ClientInfo(
                new ClientID(CLIENT_ID),
                Collections.singleton(REDIRECT_URI),
                Collections.singleton(POST_LOGOUT_REDIRECT_URI),
                LOGOUT_URI,
                CLIENT_CERT_SUBJECT_DN,
                0L /* authnRequestClientAssertionLifetimeMs */);

        sessionManager.add(new SessionID(SESSION_ID), PERSON_USER, LoginMethod.PASSWORD, clientInfo);
        return sessionManager;
    }

    public static ResourceBundleMessageSource messageSource() {
        ResourceBundleMessageSource messageSource = new ResourceBundleMessageSource();
        messageSource.setBasename("messages");
        return messageSource;
    }

    public static Set<String> qualifyGroupMembership(List<String> groups) {
        Set<String> result = new HashSet<String>();
        for (String group : groups) {
            result.add(String.format("%s\\%s", TENANT_NAME, group));
        }
        return result;
    }

    public static JWTClaimsSet.Builder idTokenClaims() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "id_token");
        claimsBuilder = claimsBuilder.claim("token_type", "Bearer");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(ISSUER);
        claimsBuilder = claimsBuilder.subject(PERSON_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.audience(CLIENT_ID);
        claimsBuilder = claimsBuilder.issueTime(now);
        claimsBuilder = claimsBuilder.expirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsBuilder = claimsBuilder.claim("tenant", TENANT_NAME);
        claimsBuilder = claimsBuilder.claim("scope", "openid");
        return claimsBuilder;
    }

    public static JWTClaimsSet.Builder refreshTokenClaims() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "refresh_token");
        claimsBuilder = claimsBuilder.claim("token_type", "Bearer");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(ISSUER);
        claimsBuilder = claimsBuilder.subject(PERSON_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.audience(PERSON_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.issueTime(now);
        claimsBuilder = claimsBuilder.expirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsBuilder = claimsBuilder.claim("tenant", TENANT_NAME);
        claimsBuilder = claimsBuilder.claim("scope", "openid");
        claimsBuilder = claimsBuilder.claim("sid", SESSION_ID);
        return claimsBuilder;
    }

    public static JWTClaimsSet.Builder refreshTokenClaimsSltn() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "refresh_token");
        claimsBuilder = claimsBuilder.claim("token_type", "hotk-pk");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(ISSUER);
        claimsBuilder = claimsBuilder.subject(PERSON_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.audience(SOLUTION_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.issueTime(now);
        claimsBuilder = claimsBuilder.expirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsBuilder = claimsBuilder.claim("tenant", TENANT_NAME);
        claimsBuilder = claimsBuilder.claim("scope", "openid");
        claimsBuilder = claimsBuilder.claim("sid", SESSION_ID);
        claimsBuilder = claimsBuilder.claim("act_as", SOLUTION_USER.getSubject().getValue());
        return claimsBuilder;
    }

    public static JWTClaimsSet.Builder refreshTokenClaimsClient() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "refresh_token");
        claimsBuilder = claimsBuilder.claim("token_type", "hotk-pk");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(ISSUER);
        claimsBuilder = claimsBuilder.subject(PERSON_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.audience(CLIENT_ID);
        claimsBuilder = claimsBuilder.issueTime(now);
        claimsBuilder = claimsBuilder.expirationTime(new Date(now.getTime() + 2 * 60 * 1000L));
        claimsBuilder = claimsBuilder.claim("tenant", TENANT_NAME);
        claimsBuilder = claimsBuilder.claim("scope", "openid");
        claimsBuilder = claimsBuilder.claim("sid", SESSION_ID);
        claimsBuilder = claimsBuilder.claim("act_as", SOLUTION_USER.getSubject().getValue());
        claimsBuilder = claimsBuilder.claim("client_id", CLIENT_ID);
        return claimsBuilder;
    }

    public static JWTClaimsSet.Builder personUserAssertionClaims() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "person_user_assertion");
        claimsBuilder = claimsBuilder.claim("token_type", "Bearer");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(CLIENT_CERT_SUBJECT_DN);
        claimsBuilder = claimsBuilder.subject(CLIENT_CERT_SUBJECT_DN);
        claimsBuilder = claimsBuilder.audience("https://" + SERVER_NAME);
        claimsBuilder = claimsBuilder.issueTime(now);
        return claimsBuilder;
    }

    public static JWTClaimsSet.Builder solutionUserAssertionClaims() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "solution_user_assertion");
        claimsBuilder = claimsBuilder.claim("token_type", "Bearer");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(CLIENT_CERT_SUBJECT_DN);
        claimsBuilder = claimsBuilder.subject(CLIENT_CERT_SUBJECT_DN);
        claimsBuilder = claimsBuilder.audience("https://" + SERVER_NAME);
        claimsBuilder = claimsBuilder.issueTime(now);
        return claimsBuilder;
    }

    public static JWTClaimsSet.Builder clientAssertionClaims() {
        Date now = new Date();

        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.claim("token_class", "client_assertion");
        claimsBuilder = claimsBuilder.claim("token_type", "Bearer");
        claimsBuilder = claimsBuilder.jwtID((new JWTID()).getValue());
        claimsBuilder = claimsBuilder.issuer(CLIENT_ID);
        claimsBuilder = claimsBuilder.subject(CLIENT_ID);
        claimsBuilder = claimsBuilder.audience("https://" + SERVER_NAME);
        claimsBuilder = claimsBuilder.issueTime(now);
        return claimsBuilder;
    }

    public static String passwordLoginString() {
        return passwordLoginString(USERNAME, PASSWORD);
    }

    public static String passwordLoginString(String username, String password) {
        String unp = username + ":" + password;
        String unp64 = Base64Utils.encodeToString(unp);
        return "Basic " + unp64;
    }

    public static String gssCIPLoginString() {
        return gssCIPLoginString(GSS_CONTEXT_ID);
    }

    public static String gssCIPLoginString(String contextId) {
        return String.format("Negotiate %s _gss_ticket__xyz_", contextId);
    }

    public static String gssBrowserLoginString() {
        return String.format("Negotiate %s", GSS_CONTEXT_ID);
    }

    public static String securIdLoginString() {
        return securIdLoginString(USERNAME, SECURID_PASSCODE, null);
    }

    public static String securIdLoginString(String username, String passcode, String sessionId) {
        String unp = username + ":" + passcode;
        String unp64 = Base64Utils.encodeToString(unp);
        String sessionId64 = (sessionId == null) ? null : Base64Utils.encodeToString(sessionId);
        return String.format("RSAAM %s %s", sessionId64, unp64);
    }

    public static Map<String, String> authnRequestParameters(Flow flow) throws Exception {
        return authnRequestParameters(flow, "form_post");
    }

    public static Map<String, String> authnRequestParameters(Flow flow, String responseMode) throws Exception {
        assert flow.isAuthzEndpointFlow();

        String responseType;
        switch (flow) {
            case AUTHZ_CODE:
                responseType = "code";
                break;
            case IMPLICIT:
                responseType = "id_token token";
                break;
            case IMPLICIT_ID_TOKEN_ONLY:
                responseType = "id_token";
                break;
            default:
                throw new IllegalArgumentException("unrecognized flow value " + flow.toString());
        }

        Map<String, String> params = new HashMap<String, String>();
        params.put("response_type", responseType);
        params.put("response_mode", responseMode);
        params.put("client_id", CLIENT_ID);
        params.put("redirect_uri", REDIRECT_URI.toString());
        params.put("scope", "openid");
        params.put("state", STATE);
        params.put("nonce", NONCE);
        params.put("client_assertion", TestUtil.sign(clientAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
        return params;
    }

    public static Map<String, String> tokenRequestParameters(Flow flow) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.AUTHZ_CODE && flow != Flow.CLIENT_CREDS && flow != Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParameters(flow, refreshTokenClaims().build());
        return params;
    }

    public static Map<String, String> tokenRequestParametersSltn(Flow flow) throws Exception {
        return tokenRequestParametersSltn(flow, solutionUserAssertionClaims().build());
    }

    public static Map<String, String> tokenRequestParametersSltn(Flow flow, JWTClaimsSet solutionUserAssertionClaims) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.AUTHZ_CODE && flow != Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParameters(flow, refreshTokenClaimsSltn().build());
        params.put("solution_user_assertion", TestUtil.sign(solutionUserAssertionClaims, CLIENT_PRIVATE_KEY).serialize());
        return params;
    }

    public static Map<String, String> tokenRequestParametersClient(Flow flow) throws Exception {
        return tokenRequestParametersClient(flow, clientAssertionClaims().build());
    }

    public static Map<String, String> tokenRequestParametersClient(Flow flow, JWTClaimsSet clientAssertionClaims) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParameters(flow, refreshTokenClaimsClient().build());
        params.put("client_assertion", TestUtil.sign(clientAssertionClaims, CLIENT_PRIVATE_KEY).serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        return params;
    }

    public static Map<String, String> tokenRequestParametersClientId(Flow flow) throws Exception {
        assert flow.isTokenEndpointFlow();
        assert flow != Flow.AUTHZ_CODE && flow != Flow.CLIENT_CREDS && flow != Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParameters(flow, refreshTokenClaims().build());
        params.put("client_id", CLIENT_ID);
        return params;
    }

    public static Map<String, String> logoutRequestParameters() throws Exception {
        Map<String, String> params = new HashMap<String, String>();
        params.put("id_token_hint", TestUtil.sign(idTokenClaims().build(), TENANT_PRIVATE_KEY).serialize());
        params.put("post_logout_redirect_uri", POST_LOGOUT_REDIRECT_URI.toString());
        params.put("state", LOGOUT_STATE);
        params.put("client_assertion", TestUtil.sign(clientAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
        return params;
    }

    public static AuthnResponse validateAuthnSuccessResponse(
            MockHttpServletResponse response,
            Flow        flow,
            Scope       scope,
            boolean     redirectResponseMode,
            boolean     ajaxRequest,
            String      expectedState,
            String      expectedNonce) throws Exception {
        return validateAuthnSuccessResponse(
                response,
                flow,
                scope,
                redirectResponseMode,
                ajaxRequest,
                expectedState,
                expectedNonce,
                GROUP_MEMBERSHIP,
                GROUP_MEMBERSHIP,
                ADMIN_SERVER_ROLE);
    }

    public static AuthnResponse validateAuthnSuccessResponse(
            MockHttpServletResponse response,
            Flow        flow,
            Scope       scope,
            boolean     redirectResponseMode,
            boolean     ajaxRequest,
            String      expectedState,
            String      expectedNonce,
            Set<String> expectedIdTokenGroups,
            Set<String> expectedAccessTokenGroups,
            String      expectedAdminServerRole) throws Exception {
        assert flow.isAuthzEndpointFlow();

        assertEquals("status", (redirectResponseMode && !ajaxRequest) ? 302 : 200, response.getStatus());
        assertEquals("redirectTarget", REDIRECT_URI.toString(), extractAuthnResponseTarget(response, flow, redirectResponseMode, ajaxRequest));
        assertNull("error", extractAuthnResponseParameter(response, flow, "error", redirectResponseMode, ajaxRequest));
        assertEquals("state", expectedState, extractAuthnResponseParameter(response, flow, "state", redirectResponseMode, ajaxRequest));
        assertNull("refresh_token", extractAuthnResponseParameter(response, flow, "refresh_token", redirectResponseMode, ajaxRequest));

        String authzCode = extractAuthnResponseParameter(response, flow, "code", redirectResponseMode, ajaxRequest);
        String idToken = null;
        String accessToken = null;
        if (flow == Flow.AUTHZ_CODE) {
            assertNotNull("authzCode", authzCode);
        } else if (flow.isImplicit()) {
            assertNull("authzCode", authzCode);
            Cookie sessionCookie = response.getCookie(SessionManager.getSessionCookieName(TENANT_NAME));
            String expectedSessionId = (sessionCookie != null) ? sessionCookie.getValue() : SESSION_ID;
            idToken = extractAuthnResponseParameter(response, flow, "id_token", redirectResponseMode, ajaxRequest);
            validateToken(
                    "id_token",
                    idToken,
                    flow,
                    scope,
                    false /* wSltnAssertion */,
                    false /* wClientAssertion */,
                    expectedNonce,
                    expectedSessionId,
                    expectedIdTokenGroups,
                    expectedAccessTokenGroups,
                    expectedAdminServerRole);
            if (flow == Flow.IMPLICIT) {
                assertEquals("token_type==Bearer", "Bearer", extractAuthnResponseParameter(response, flow, "token_type", redirectResponseMode, ajaxRequest));
                assertEquals("expires_in==300", "300", extractAuthnResponseParameter(response, flow, "expires_in", redirectResponseMode, ajaxRequest));
                accessToken = extractAuthnResponseParameter(response, flow, "access_token", redirectResponseMode, ajaxRequest);
                validateToken(
                        "access_token",
                        accessToken,
                        flow,
                        scope,
                        false /* wSltnAssertion */,
                        false /* wClientAssertion */,
                        expectedNonce,
                        expectedSessionId,
                        expectedIdTokenGroups,
                        expectedAccessTokenGroups,
                        expectedAdminServerRole);
            }
            if (flow == Flow.IMPLICIT_ID_TOKEN_ONLY) {
                assertNull("access_token", extractAuthnResponseParameter(response, flow, "access_token", redirectResponseMode, ajaxRequest));
            }
        }

        Cookie sessionCookie = response.getCookie(SESSION_COOKIE_NAME);
        assertNotNull("sessionCookie", sessionCookie);

        return new AuthnResponse(idToken, accessToken, authzCode);
    }

    public static void validateAuthnErrorResponse(
            MockHttpServletResponse response,
            Flow        flow,
            boolean     redirectResponseMode,
            boolean     ajaxRequest,
            String      expectedError,
            String      expectedErrorDescription) throws Exception {
        assert flow.isAuthzEndpointFlow();

        assertEquals("status", (redirectResponseMode && !ajaxRequest) ? 302 : 200, response.getStatus());
        assertEquals("redirectTarget", REDIRECT_URI.toString(), extractAuthnResponseTarget(response, flow, redirectResponseMode, ajaxRequest));
        assertEquals("state", STATE, extractAuthnResponseParameter(response, flow, "state", redirectResponseMode, ajaxRequest));
        assertEquals("error", expectedError, extractAuthnResponseParameter(response, flow, "error", redirectResponseMode, ajaxRequest));
        assertEquals(
                "error_description",
                expectedErrorDescription,
                extractAuthnResponseParameter(response, flow, "error_description", redirectResponseMode, ajaxRequest));
        assertNull("sessionCookie", response.getCookie(SESSION_COOKIE_NAME));
    }

    public static TokenResponse validateTokenSuccessResponse(
            MockHttpServletResponse response,
            Flow        flow,
            Scope       scope,
            boolean     wSltnAssertion,
            boolean     wClientAssertion,
            String      expectedNonce) throws Exception {
        return validateTokenSuccessResponse(
                response,
                flow,
                scope,
                wSltnAssertion,
                wClientAssertion,
                expectedNonce,
                GROUP_MEMBERSHIP,
                GROUP_MEMBERSHIP,
                ADMIN_SERVER_ROLE);
    }

    public static TokenResponse validateTokenSuccessResponse(
            MockHttpServletResponse response,
            Flow        flow,
            Scope       scope,
            boolean     wSltnAssertion,
            boolean     wClientAssertion,
            String      expectedNonce,
            Set<String> expectedIdTokenGroups,
            Set<String> expectedAccessTokenGroups,
            String      expectedAdminServerRole) throws Exception {
        assert flow.isTokenEndpointFlow();

        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(response.getContentAsString());
        String idToken      = (String)  jsonObject.get("id_token");
        String accessToken  = (String)  jsonObject.get("access_token");
        String refreshToken = (String)  jsonObject.get("refresh_token");
        String tokenType    = (String)  jsonObject.get("token_type");
        Integer expiresIn   = (Integer) jsonObject.get("expires_in");

        assertEquals("token_type", (wSltnAssertion || wClientAssertion) ? "hotk-pk" : "Bearer", tokenType);
        assertEquals("expires_in", (wSltnAssertion || wClientAssertion) ? 7200 : 300, expiresIn.intValue());

        validateToken(
                "id_token",
                idToken,
                flow,
                scope,
                wSltnAssertion,
                wClientAssertion,
                expectedNonce,
                null /* expectedSessionId */,
                expectedIdTokenGroups,
                expectedAccessTokenGroups,
                expectedAdminServerRole);

        validateToken(
                "access_token",
                accessToken,
                flow,
                scope,
                wSltnAssertion,
                wClientAssertion,
                expectedNonce,
                null /* expectedSessionId */,
                expectedIdTokenGroups,
                expectedAccessTokenGroups,
                expectedAdminServerRole);

        boolean refreshTokenShouldExist =
                scope.contains(ScopeValue.OFFLINE_ACCESS) &&
                (flow == Flow.AUTHZ_CODE || flow == Flow.PASSWORD || flow == Flow.GSS_TICKET || flow == Flow.PERSON_USER_CERT || flow == Flow.SECURID);
        assertEquals("refreshTokenShouldExist", refreshTokenShouldExist, refreshToken != null);
        if (refreshTokenShouldExist) {
            validateToken(
                    "refresh_token",
                    refreshToken,
                    flow,
                    scope,
                    wSltnAssertion,
                    wClientAssertion,
                    expectedNonce,
                    null /* expectedSessionId */,
                    expectedIdTokenGroups,
                    expectedAccessTokenGroups,
                    expectedAdminServerRole);
        }

        return new TokenResponse(idToken, accessToken, refreshToken);
    }

    public static void validateTokenErrorResponse(
            MockHttpServletResponse response,
            Flow        flow,
            String      expectedError,
            String      expectedErrorDescription) throws Exception {
        assert flow.isTokenEndpointFlow();

        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(response.getContentAsString());
        String error = (String) jsonObject.get("error");
        String errorDescription = (String) jsonObject.get("error_description");

        assertEquals("error", expectedError, error);
        assertEquals("error_description", expectedErrorDescription, errorDescription);
    }

    public static void validateLogoutSuccessResponse(
            MockHttpServletResponse response,
            boolean     expectingLogoutUriLinks) throws Exception {
        validateLogoutSuccessResponse(
                response,
                expectingLogoutUriLinks,
                SESSION_ID,
                new URI[0]);
    }

    public static void validateLogoutSuccessResponse(
            MockHttpServletResponse response,
            boolean     expectingLogoutUriLinks,
            String      expectedSessionId,
            URI[]       expectedLogoutUris) throws Exception {
        assertNull("response.getErrorMessage", response.getErrorMessage());
        assertEquals("response.getStatus", 200, response.getStatus());

        String expectedRedirectTarget = String.format("%s?state=%s", POST_LOGOUT_REDIRECT_URI.toString(), LOGOUT_STATE);
        String redirectTarget = TestUtil.extractString(response, "var postLogoutRedirectUriWithState = \"", "\"");
        assertEquals("postLogoutRedirectUriWithState", expectedRedirectTarget, redirectTarget);

        String logoutUriLinks = TestUtil.extractString(response, "<!-- logoutUriLinks --> ", " <!-- logoutUriLinks -->");
        if (expectingLogoutUriLinks) {
            int expectedLength = 0;
            for (URI expectedLogoutUri : expectedLogoutUris) {
                String expectedLogoutUriWithSid = String.format("%s?sid=%s", expectedLogoutUri, expectedSessionId);
                String expectedLogoutUriLink = String.format("<iframe src=\"%s\">", expectedLogoutUriWithSid);
                assertTrue("logoutUriLinks.contains(expectedLogoutUriLink)", logoutUriLinks.contains(expectedLogoutUriLink));
                expectedLength += expectedLogoutUriLink.length();
            }
            assertEquals("logoutUriLinks.length()", expectedLength, logoutUriLinks.length());
        } else {
            assertEquals("logoutUriLinks", "", logoutUriLinks);
        }

        Cookie sessionCookie = response.getCookie(SessionManager.getSessionCookieName(TENANT_NAME));
        assertNotNull("sessionCookie", sessionCookie);
        assertEquals("sessionCookie value is empty", "", sessionCookie.getValue());
    }

    public static void validateLogoutErrorResponse(
            MockHttpServletResponse response,
            String      expectedError,
            String      expectedErrorDescription) throws Exception {
        String redirectUrl = response.getRedirectedUrl();
        String queryString = redirectUrl.substring(redirectUrl.indexOf('?') + 1);
        Map<String, String> params = TestUtil.parseParameters(queryString);
        assertEquals("state", LOGOUT_STATE, params.get("state"));
        assertEquals("error", expectedError, params.get("error"));
        assertEquals("error_description", expectedErrorDescription, params.get("error_description"));
    }

    private static void validateToken(
            String      tokenClass,
            String      tokenString,
            Flow        flow,
            Scope       scope,
            boolean     wSltnAssertion,
            boolean     wClientAssertion,
            String      expectedNonce,
            String      expectedSessionId,
            Set<String> expectedIdTokenGroups,
            Set<String> expectedAccessTokenGroups,
            String      expectedAdminServerRole) throws Exception {
        assertTrue("tokenString not null or empty", tokenString != null && !tokenString.isEmpty());
        SignedJWT token = SignedJWT.parse(tokenString);
        assertNotNull("token", token);

        // verify signature
        JWSVerifier verifier = new RSASSAVerifier(TENANT_PUBLIC_KEY);
        assertTrue(token.verify(verifier));

        Date now = new Date();
        JWTClaimsSet claims = token.getJWTClaimsSet();

        assertEquals("token_class", tokenClass, claims.getStringClaim("token_class"));
        assertEquals("scope", scope.getScopeValues(), Scope.parse(claims.getStringClaim("scope")).getScopeValues());
        assertEquals("tenant", TENANT_NAME, claims.getStringClaim("tenant"));
        assertEquals("issuer", ISSUER, claims.getIssuer());

        String expectedSubject = (flow == Flow.SOLUTION_USER_CREDS || flow == Flow.CLIENT_CREDS) ?
                SOLUTION_USER.getSubject().getValue() :
                PERSON_USER.getSubject().getValue();
        assertEquals("subject", expectedSubject, claims.getSubject());

        assertTrue("issued at", claims.getIssueTime().before(now));
        assertTrue("expiration", claims.getExpirationTime().after(now));
        assertNotNull("jwt_id", claims.getJWTID());

        if (flow.isImplicit() || flow == Flow.AUTHZ_CODE) {
            assertNotNull("nonce", claims.getStringClaim("nonce"));
            assertEquals("nonce", expectedNonce, claims.getStringClaim("nonce"));
        } else if (flow == Flow.REFRESH_TOKEN) {
            // nonce can be null or not (depends on how we acquired the refresh_token)
        } else {
            assertNull("nonce", claims.getStringClaim("nonce"));
        }

        if (flow.isImplicit() || flow == Flow.AUTHZ_CODE) {
            assertNotNull("sid", claims.getStringClaim("sid"));
        }
        if (flow.isImplicit()) {
            assertEquals("sid", expectedSessionId, claims.getStringClaim("sid"));
        }

        assertEquals("token_type", (wSltnAssertion || wClientAssertion) ? "hotk-pk" : "Bearer", claims.getStringClaim("token_type"));

        if (wSltnAssertion || wClientAssertion) {
            JSONObject hotk = (JSONObject) claims.getClaim("hotk");
            assertNotNull("hotk", hotk);
            JWKSet jwkSet = JWKSet.parse(hotk);
            assertNotNull("jwkSet", jwkSet);
            RSAPublicKey publicKey = JWTUtils.getPublicKey(jwkSet);
            assertEquals("access_token hotk claim contains CLIENT_PUBLIC_KEY", CLIENT_PUBLIC_KEY, publicKey);

            if (flow != Flow.SOLUTION_USER_CREDS && flow != Flow.CLIENT_CREDS) {
                assertEquals("act_as", SOLUTION_USER.getSubject().getValue(), claims.getStringClaim("act_as"));
            }
        }

        if (tokenClass.equals("id_token")) {
            boolean idGroupsScope = scope.contains(ScopeValue.ID_TOKEN_GROUPS) || scope.contains(ScopeValue.ID_TOKEN_GROUPS_FILTERED);
            boolean idGroupsExist = claims.getClaim("groups") != null;
            assertEquals("idGroupsScope==idGroupsExist", idGroupsScope, idGroupsExist);
            if (idGroupsScope) {
                Set<String> idTokenGroups = new HashSet<String>(Arrays.asList(claims.getStringArrayClaim("groups")));
                assertEquals("idTokenGroups", expectedIdTokenGroups, idTokenGroups);
            }
        } else if (tokenClass.equals("access_token")) {
            boolean atGroupsScope = scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS) || scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED);
            boolean atGroupsExist = claims.getClaim("groups") != null;
            assertEquals("atGroupsScope==atGroupsExist", atGroupsScope, atGroupsExist);
            if (atGroupsExist) {
                Set<String> accessTokenGroups = new HashSet<String>(Arrays.asList(claims.getStringArrayClaim("groups")));
                assertEquals("accessTokenGroups", expectedAccessTokenGroups, accessTokenGroups);
            }

            boolean adminServerScope = scope.contains(ScopeValue.RESOURCE_SERVER_ADMIN_SERVER);
            boolean adminServerAudience = claims.getAudience().contains("rs_admin_server");
            boolean adminServerRoleExists = claims.getStringClaim("admin_server_role") != null;
            assertEquals("adminServerScope==adminServerAudience", adminServerScope, adminServerAudience);
            assertEquals("adminServerScope==adminServerRoleExists", adminServerScope, adminServerRoleExists);
            if (adminServerScope) {
                assertEquals("admin_server_role", expectedAdminServerRole, claims.getStringClaim("admin_server_role"));
            }

            boolean rsxScope = scope.contains(ScopeValue.parse(SCOPE_VALUE_RSX));
            boolean rsxAudience = claims.getAudience().contains(SCOPE_VALUE_RSX);
            assertEquals("rsxScope==rsxAudience", rsxScope, rsxAudience);
        } else if (tokenClass.equals("refresh_token")) {
            // no-op
        } else {
            throw new IllegalArgumentException("unexpected tokenClass: " + tokenClass);
        }
    }

    private static String extractAuthnResponseTarget(
            MockHttpServletResponse response,
            Flow flow,
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
            MockHttpServletResponse response,
            Flow flow,
            String parameterName,
            boolean redirectResponseMode,
            boolean ajaxRequest) throws Exception {
        String result;

        if (redirectResponseMode) {
            String redirectUrl = ajaxRequest ? response.getContentAsString() : response.getRedirectedUrl();
            char separator = (flow == Flow.AUTHZ_CODE) ? '?' : '#';
            String queryString = redirectUrl.substring(redirectUrl.indexOf(separator) + 1);
            Map<String, String> params = TestUtil.parseParameters(queryString);
            result = params.get(parameterName);
        } else {
            String prefix = String.format("<input type=\"hidden\" name=\"%s\" value=\"", parameterName);
            result = TestUtil.extractString(response, prefix, "\"");
        }

        return result;
    }

    private static Map<String, String> tokenRequestParameters(
            Flow flow,
            JWTClaimsSet refreshTokenClaims) throws Exception {
        assert flow.isTokenEndpointFlow();

        Map<String, String> params = new HashMap<String, String>();
        switch (flow) {
            case AUTHZ_CODE:
                params.put("grant_type", "authorization_code");
                params.put("code", AUTHZ_CODE);
                params.put("redirect_uri", REDIRECT_URI.toString());
                break;
            case PASSWORD:
                params.put("grant_type", "password");
                params.put("username", USERNAME);
                params.put("password", PASSWORD);
                params.put("scope", "openid offline_access");
                break;
            case CLIENT_CREDS:
                params.put("grant_type", "client_credentials");
                params.put("scope", "openid");
                break;
            case PERSON_USER_CERT:
                params.put("grant_type", "urn:vmware:grant_type:person_user_certificate");
                params.put("person_user_certificate", Base64Utils.encodeToString(CLIENT_CERT.getEncoded()));
                params.put("person_user_assertion", TestUtil.sign(personUserAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
                params.put("scope", "openid offline_access");
                break;
            case SOLUTION_USER_CREDS:
                params.put("grant_type", "urn:vmware:grant_type:solution_user_credentials");
                params.put("scope", "openid");
                break;
            case GSS_TICKET:
                params.put("grant_type", "urn:vmware:grant_type:gss_ticket");
                params.put("context_id", GSS_CONTEXT_ID);
                params.put("gss_ticket", "===");
                params.put("scope", "openid offline_access");
                break;
            case SECURID:
                params.put("grant_type", "urn:vmware:grant_type:securid");
                params.put("username", USERNAME);
                params.put("passcode", SECURID_PASSCODE);
                params.put("session_id", "_session_id_xyz_");
                params.put("scope", "openid offline_access");
                break;
            case REFRESH_TOKEN:
                params.put("grant_type", "refresh_token");
                params.put("refresh_token", TestUtil.sign(refreshTokenClaims, TENANT_PRIVATE_KEY).serialize());
                break;
            default:
                throw new IllegalArgumentException("unexpected flow: " + flow);
        }
        return params;
    }

    public static class AuthnResponse {
        private final SignedJWT idToken;
        private final SignedJWT accessToken;
        private final String authzCode;

        private AuthnResponse(String idToken, String accessToken, String authzCode) throws Exception {
            this.idToken = (idToken == null) ? null : SignedJWT.parse(idToken);
            this.accessToken = (accessToken == null) ? null : SignedJWT.parse(accessToken);
            this.authzCode = authzCode;
        }

        public SignedJWT getIDToken() {
            return this.idToken;
        }

        public SignedJWT getAccessToken() {
            return this.accessToken;
        }

        public String getAuthzCode() {
            return this.authzCode;
        }
    }

    public static class TokenResponse {
        private final SignedJWT idToken;
        private final SignedJWT accessToken;
        private final SignedJWT refreshToken;

        private TokenResponse(String idToken, String accessToken, String refreshToken) throws Exception {
            this.idToken = SignedJWT.parse(idToken);
            this.accessToken = SignedJWT.parse(accessToken);
            this.refreshToken = (refreshToken == null) ? null : SignedJWT.parse(refreshToken);
        }

        public SignedJWT getIdToken() {
            return this.idToken;
        }

        public SignedJWT getAccessToken() {
            return this.accessToken;
        }

        public SignedJWT getRefreshToken() {
            return this.refreshToken;
        }
    }
}