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

import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.interfaces.RSAPrivateKey;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jose.util.Base64;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.PlainJWT;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.AuthorizationCode;
import com.nimbusds.oauth2.sdk.ResponseType;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.oauth2.sdk.id.State;
import com.nimbusds.openid.connect.sdk.Nonce;
import com.nimbusds.openid.connect.sdk.OIDCScopeValue;
import com.nimbusds.openid.connect.sdk.ResponseMode;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.openidconnect.common.AuthenticationRequest;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class TokenControllerTest {
    @BeforeClass
    public static void initialize() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testAuthzCodeFlow() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for authz code flow");
    }

    @Test
    public void testAuthzCodeFlowSltn() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        params.put("solution_assertion", Shared.sign(TestContext.sltnAssertionClaims(), TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for authz code flow");
    }

    @Test
    public void testAuthzCodeFlowClient() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowInvalidAuthzCode() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.put("code", params.get("code") + "non_matching");
        assertErrorResponse(flow, params, "invalid_grant", "invalid authorization code");
    }

    @Test
    public void testAuthzCodeFlowNonMatchingClientId() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        ClientID clientId = new ClientID();
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setIssuer(clientId.getValue());
        claimsSet.setSubject(clientId.getValue());
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow, claimsSet);
        IdmClient idmClient = TestContext.idmClientBuilder().clientId(clientId.toString()).build();
        TokenController controller = TestContext.tokenController(idmClient);
        String expectedErrorMessage = "client_id does not match that of the original authn request";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testAuthzCodeFlowNonMatchingRedirectUri() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.put("redirect_uri", params.get("redirect_uri") + "non_matching");
        String expectedErrorMessage = "redirect_uri does not match that of the original authn request";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage);
    }

    @Test
    public void testAuthzCodeFlowNonMatchingTenant() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);

        String tenant = TestContext.TENANT_NAME + "non_matching";
        AuthorizationCodeManager authzCodeManager = new AuthorizationCodeManager();
        AuthenticationRequest originalAuthnRequest = new AuthenticationRequest(
                TestContext.AUTHZ_ENDPOINT_URI,
                new ResponseType(ResponseType.Value.CODE),
                ResponseMode.FORM_POST,
                new ClientID(TestContext.CLIENT_ID),
                TestContext.REDIRECT_URI,
                new Scope(OIDCScopeValue.OPENID),
                new State(TestContext.STATE),
                new Nonce(TestContext.NONCE),
                (SignedJWT) null,
                new CorrelationID());
        authzCodeManager.add(
                new AuthorizationCode(TestContext.AUTHZ_CODE),
                new PersonUser(new PrincipalId(TestContext.USERNAME, tenant), tenant),
                new SessionID(TestContext.SESSION_ID),
                originalAuthnRequest);
        TokenController controller = TestContext.tokenController();
        controller.setAuthorizationCodeManager(authzCodeManager);

        String expectedErrorMessage = "tenant does not match that of the original authn request";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testAuthzCodeFlowTokenScopeParameter() throws Exception {
        // in authz code flow, we do not allow scope token request parameter (because it was already supplied in the authn request)
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.put("scope", "openid");
        assertErrorResponse(flow, params, "invalid_request", "scope parameter is not allowed in token request for authz code flow");
    }

    @Test
    public void testAuthzCodeFlowDisabledPersonUser() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().personUserEnabled(false).build();
        TokenController controller = TestContext.tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "user has been disabled or deleted", controller);
    }

    @Test
    public void testAuthzCodeFlowDisabledSolutionUser() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().solutionUserEnabled(false).build();
        TokenController controller = TestContext.tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "solution user has been disabled or deleted", controller);
    }

    @Test
    public void testAuthzCodeFlowSolutionUserNotMemberOfActAsGroup() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().isMemberOfSystemGroup(false).build();
        TokenController controller = TestContext.tokenController(idmClient);
        String expectedErrorMessage = "solution user acting as a person user must be a member of ActAsUsers group";
        assertErrorResponse(flow, params, "access_denied", expectedErrorMessage, controller);
    }

    @Test
    public void testClientCredentialsFlow() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for client credentials flow");
    }

    @Test
    public void testClientCredentialsFlowSltn() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        params.put("solution_assertion", Shared.sign(TestContext.sltnAssertionClaims(), TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for client credentials flow");
    }

    @Test
    public void testClientCredentialsFlowClient() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testClientCredentialsFlowRefreshTokenDisallowed() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.put("scope", "openid offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "refresh token (offline_access) is not allowed for this grant_type");
    }

    @Test
    public void testSolutionUserCredentials() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        params.remove("solution_assertion");
        assertErrorResponse(flow, params, "invalid_request", "solution_assertion parameter is required for solution user credentials flow");
    }

    @Test
    public void testSolutionUserCredentialsSltn() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testSolutionUserCredentialsClient() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        params.remove("solution_assertion");
        params.put("client_assertion", Shared.sign(TestContext.clientAssertionClaims(), TestContext.CLIENT_PRIVATE_KEY).serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        assertErrorResponse(flow, params, "invalid_request", "solution_assertion parameter is required for solution user credentials flow");
    }

    @Test
    public void testSolutionUserCredentialsFlowRefreshTokenDisallowed() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        params.put("scope", "openid offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "refresh token (offline_access) is not allowed for this grant_type");
    }

    @Test
    public void testPasswordFlow() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowSltn() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowClient() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowMissingCorrelationId() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.remove("correlation_id");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowIncorrectCredentials() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("password", params.get("password") + "non_matching");
        assertErrorResponse(flow, params, "invalid_grant", "incorrect username or password");
    }

    @Test
    public void testGssTicketFlow() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testGssTicketFlow2Legged() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        byte[] gssServerLeg = new byte[1];
        IdmClient idmClient = TestContext.idmClientBuilder().gssServerLeg(gssServerLeg).build();
        TokenController controller = TestContext.tokenController(idmClient);
        String serverLegBase64 = Base64.encode(gssServerLeg).toString();
        String expectedErrorMessage = String.format("gss_continue_needed:%s:%s", TestContext.GSS_CONTEXT_ID, serverLegBase64);
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testGssTicketFlowSltn() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testGssTicketFlowClient() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testGssTicketFlowMissingContextId() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.remove("context_id");
        assertErrorResponse(flow, params, "invalid_request", "missing context_id parameter");
    }

    @Test
    public void testGssTicketFlowMissingGssTicket() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.remove("gss_ticket");
        assertErrorResponse(flow, params, "invalid_request", "missing gss_ticket parameter");
    }

    @Test
    public void testRefreshTokenFlow() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testRefreshTokenFlowSltn() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testRefreshTokenFlowClient() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testRefreshTokenFlowInvalidRequest() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.remove("refresh_token");
        assertErrorResponse(flow, params, "invalid_request", "Missing or empty \"refresh_token\" parameter");
    }

    @Test
    public void testRefreshTokenFlowInvalidJwt() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("refresh_token", "this is not a valid jwt");
        String expectedErrorMessage =
                "failed to parse SignedJWT out of refresh_token: " +
                "Invalid serialized plain/JWS/JWE object: Missing part delimiters";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage);
    }

    @Test
    public void testRefreshTokenFlowIncorrectSolutionUser() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaimsSltn();
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this solution user");
    }

    @Test
    public void testRefreshTokenFlowIncorrectClient() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaimsClient();
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this client");
    }

    @Test
    public void testRefreshTokenFlowIncorrectTenant() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setClaim("tenant", claimsSet.getStringClaim("tenant") + "non_matching");
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this tenant");
    }

    @Test
    public void testRefreshTokenFlowDisabledPersonUser() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().personUserEnabled(false).build();
        TokenController controller = TestContext.tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "user has been disabled or deleted", controller);
    }

    @Test
    public void testRefreshTokenFlowDisabledSolutionUser() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().solutionUserEnabled(false).build();
        TokenController controller = TestContext.tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "solution user has been disabled or deleted", controller);
    }

    @Test
    public void testRefreshTokenFlowJwtUnsigned() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        params.put("refresh_token", (new PlainJWT(claimsSet)).serialize());
        String expectedErrorMessage =
                "failed to parse SignedJWT out of refresh_token: " +
                "Invalid JWS header: The algorithm \"alg\" header parameter must be for signatures";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage);
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidSignature() throws Exception {
        // jwt should be signed using server private key
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "jwt has an invalid signature");
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidClaims() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setClaim("token_class", true);
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "failed to parse claims out of jwt");
    }

    @Test
    public void testRefreshTokenFlowJwtIncorrectTokenClass() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setClaim("token_class", "id_token");
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "jwt is missing a token_class=refresh_token claim");
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidSubject() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setSubject("username_at_domain");
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "failed to parse subject into a PersonUser");
    }

    @Test
    public void testRefreshTokenFlowJwtMissingSubject() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setSubject(null);
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "jwt is missing sub (subject) claim");
    }

    @Test
    public void testRefreshTokenFlowJwtMissingScope() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setClaim("scope", null);
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "jwt is missing scope claim");
    }

    @Test
    public void testRefreshTokenFlowJwtMissingExpiration() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setExpirationTime(null);
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "jwt is missing exp (expiration) claim");
    }

    @Test
    public void testRefreshTokenFlowJwtExpired() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        JWTClaimsSet claimsSet = TestContext.refreshTokenClaims();
        claimsSet.setExpirationTime(claimsSet.getIssueTime());
        params.put("refresh_token", Shared.sign(claimsSet, TestContext.TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "jwt has expired");
    }

    @Test
    public void testRefreshTokenFlowTokenScopeParameter() throws Exception {
        // in refresh token flow, we do not allow scope token request parameter (because it was already supplied in previous request)
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "openid");
        assertErrorResponse(flow, params, "invalid_request", "scope parameter is not allowed in token request for refresh token flow");
    }

    @Test
    public void testScopeValueIdTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "openid id_groups");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueAccessTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "openid at_groups");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueAdminServer() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueResourceServerX() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "openid " + TestContext.SCOPE_VALUE_RSX);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testUnregisteredClient() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().clientId(null).build();
        TokenController controller = TestContext.tokenController(idmClient);
        String expectedErrorMessage = "unregistered client";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage, controller);
    }

    @Test
    public void testNonExistentTenant() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        TokenController tokenEndpoint = TestContext.tokenController();
        tokenEndpoint.token(request, response, "non_matching_tenant");
        String expectedErrorMessage = "non-existent tenant";
        TestContext.validateTokenErrorResponse(flow, response, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testUnsupportedGrantType() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = new HashMap<String, String>();
        params.put("grant_type", "urn:ietf:params:oauth:grant-type:jwt-bearer");
        params.put("assertion", (Shared.sign(TestContext.clientAssertionClaims(), TestContext.CLIENT_PRIVATE_KEY)).serialize());
        params.put("correlation_id", "_cid_xyz_");
        assertErrorResponse(flow, params, "unsupported_grant_type", "Unsupported grant type");
    }

    @Test
    public void testInvalidRequest() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.put("grant_type", "invalid_grant_type");
        assertErrorResponse(flow, params, "invalid_request", "Invalid or unsupported grant type: invalid_grant_type");
    }

    @Test
    public void testMissingGrantType() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.remove("grant_type");
        assertErrorResponse(flow, params, "invalid_request", "missing grant_type parameter");
    }

    @Test
    public void testMissingOpenIdScope() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "profile");
        assertErrorResponse(flow, params, "invalid_request", "missing scope=openid parameter");
    }

    @Test
    public void testUnrecognizedScopeValue() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile");
    }

    @Test
    public void testClientIdDisallowed() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("client_id", TestContext.CLIENT_ID);
        assertErrorResponse(flow, params, "invalid_request", "client_id parameter is not allowed, send client_assertion instead");
    }

    @Test
    public void testBothAssertionsDisallowed() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        params.put("solution_assertion", Shared.sign(TestContext.sltnAssertionClaims(), TestContext.CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion and solution_assertion in the same request is not allowed");
    }

    @Test
    public void testInvalidSolutionAssertion() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("solution_assertion", "this is an invalid JWT");
        assertErrorResponse(flow, params, "invalid_request", "failed to parse solution_assertion parameter");
    }

    @Test
    public void testUnsupportedClientAuthnMethodIsIgnored() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        request.addHeader("Authorization", "Basic czZCaGRSa3F0Mzo3RmpmcDBaQnIxS3REUmJuZlZkbUl3");
        MockHttpServletResponse response = new MockHttpServletResponse();
        TokenController tokenEndpoint = TestContext.tokenController();
        tokenEndpoint.token(request, response);
        TestContext.validateTokenSuccessResponse(flow, response, Scope.parse(params.get("scope")), false /* wSltnAssertion */, false /* wClientAssertion */);
    }

    @Test
    public void testAssertionIssuerMissing() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet claimsSet = TestContext.sltnAssertionClaims();
        claimsSet.setIssuer(null);
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow, claimsSet);
        String expectedErrorMessage = "jwt is missing iss (issuer) claim";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionSubjectMissing() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet claimsSet = TestContext.sltnAssertionClaims();
        claimsSet.setSubject(null);
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow, claimsSet);
        String expectedErrorMessage = "jwt is missing sub (subject) claim";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionIssuerAndSubjectMismatch() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet claimsSet = TestContext.sltnAssertionClaims();
        claimsSet.setIssuer(claimsSet.getIssuer() + "non_matching");
        Map<String, String> params = TestContext.tokenRequestParametersSltn(flow, claimsSet);
        String expectedErrorMessage = "assertion issuer and subject must be the same";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionInvalidIssuer() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setIssuer(claimsSet.getIssuer() + "non_matching");
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("client_assertion", Shared.sign(claimsSet, TestContext.CLIENT_PRIVATE_KEY).serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        String expectedErrorMessage = "Issuer and subject in client JWT assertion must designate the same client identifier";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testAssertionInvalidSubject() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setSubject(claimsSet.getSubject() + "non_matching");
        Map<String, String> params = TestContext.tokenRequestParameters(flow);
        params.put("client_assertion", Shared.sign(claimsSet, TestContext.CLIENT_PRIVATE_KEY).serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        String expectedErrorMessage = "Issuer and subject in client JWT assertion must designate the same client identifier";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testAssertionInvalidAudience() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setAudience("http://invalid_token_endpoint");
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow, claimsSet);
        String expectedErrorMessage = "jwt audience does not match request URL";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionInvalidTokenClass() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setClaim("token_class", "solution_assertion");
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow, claimsSet);
        String expectedErrorMessage = "jwt is missing a token_class=client_assertion claim";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionExpired() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setExpirationTime(claimsSet.getIssueTime());
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow, claimsSet);
        String expectedErrorMessage = "jwt has expired";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionIssuedAt() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setIssueTime(new Date(claimsSet.getIssueTime().getTime() - 5 * 60 * 1000L)); // issued 5 mins ago
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow, claimsSet);
        String expectedErrorMessage = "jwt must be issued recently";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionIssuedAtMissing() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        claimsSet.setIssueTime(null);
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow, claimsSet);
        String expectedErrorMessage = "jwt is missing an iat (issued at) claim";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionNoRegisteredCert() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);
        IdmClient idmClient = TestContext.idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = TestContext.tokenController(idmClient);
        String expectedErrorMessage = "client authn failed because client did not register a cert";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage, controller);
    }

    @Test
    public void testAssertionInvalidSignature() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = TestContext.tokenRequestParametersClient(flow);

        KeyPairGenerator keyGenerator = KeyPairGenerator.getInstance("RSA");
        keyGenerator.initialize(1024);
        KeyPair kp = keyGenerator.genKeyPair();
        RSAPrivateKey privateKey = (RSAPrivateKey) kp.getPrivate();

        JWTClaimsSet claimsSet = TestContext.clientAssertionClaims();
        SignedJWT privateKeyJwt = Shared.sign(claimsSet, privateKey);
        params.put("client_assertion", privateKeyJwt.serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");

        String expectedErrorMessage = "jwt has an invalid signature";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params) throws Exception {
        assertSuccessResponse(flow, params, TestContext.tokenController());
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            TokenController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller);
        String scopeString = params.get("scope");
        Scope scope = (scopeString == null) ? new Scope("openid") : Scope.parse(scopeString);
        boolean wSltnAssertion = params.containsKey("solution_assertion");
        boolean wClientAssertion = params.containsKey("client_assertion");
        TestContext.validateTokenSuccessResponse(flow, response, scope, wSltnAssertion, wClientAssertion);
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription) throws Exception {
        assertErrorResponse(flow, params, expectedError, expectedErrorDescription, TestContext.tokenController());
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription,
            TokenController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller);
        TestContext.validateTokenErrorResponse(flow, response, expectedError, expectedErrorDescription);
    }

    private static MockHttpServletResponse doRequest(
            Map<String, String> params,
            TokenController controller) throws Exception {
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        controller.token(request, response);
        return response;
    }
}
