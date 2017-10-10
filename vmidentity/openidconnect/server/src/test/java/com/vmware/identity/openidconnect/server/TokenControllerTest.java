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
import static com.vmware.identity.openidconnect.server.TestContext.AUTHZ_CODE;
import static com.vmware.identity.openidconnect.server.TestContext.AUTHZ_ENDPOINT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_CERT_SUBJECT_DN;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_ID;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_PRIVATE_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.GROUP_FILTER_RS_X;
import static com.vmware.identity.openidconnect.server.TestContext.GROUP_MEMBERSHIP;
import static com.vmware.identity.openidconnect.server.TestContext.GROUP_MEMBERSHIP_FILTERED;
import static com.vmware.identity.openidconnect.server.TestContext.GSS_CONTEXT_ID;
import static com.vmware.identity.openidconnect.server.TestContext.ISSUER;
import static com.vmware.identity.openidconnect.server.TestContext.NONCE;
import static com.vmware.identity.openidconnect.server.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.SCOPE_VALUE_RSX;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_ID;
import static com.vmware.identity.openidconnect.server.TestContext.STATE;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_CERT;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_PRIVATE_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_PUBLIC_KEY;
import static com.vmware.identity.openidconnect.server.TestContext.USERNAME;
import static com.vmware.identity.openidconnect.server.TestContext.clientAssertionClaims;
import static com.vmware.identity.openidconnect.server.TestContext.idmClientBuilder;
import static com.vmware.identity.openidconnect.server.TestContext.initialize;
import static com.vmware.identity.openidconnect.server.TestContext.personUserAssertionClaims;
import static com.vmware.identity.openidconnect.server.TestContext.qualifyGroupMembership;
import static com.vmware.identity.openidconnect.server.TestContext.refreshTokenClaims;
import static com.vmware.identity.openidconnect.server.TestContext.refreshTokenClaimsClient;
import static com.vmware.identity.openidconnect.server.TestContext.refreshTokenClaimsSltn;
import static com.vmware.identity.openidconnect.server.TestContext.solutionUserAssertionClaims;
import static com.vmware.identity.openidconnect.server.TestContext.tokenController;
import static com.vmware.identity.openidconnect.server.TestContext.tokenRequestParameters;
import static com.vmware.identity.openidconnect.server.TestContext.tokenRequestParametersClient;
import static com.vmware.identity.openidconnect.server.TestContext.tokenRequestParametersClientId;
import static com.vmware.identity.openidconnect.server.TestContext.tokenRequestParametersSltn;
import static com.vmware.identity.openidconnect.server.TestContext.validateTokenErrorResponse;
import static com.vmware.identity.openidconnect.server.TestContext.validateTokenSuccessResponse;

import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.interfaces.RSAPrivateKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.JWSSigner;
import com.nimbusds.jose.crypto.MACSigner;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.PlainJWT;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.ClientAssertion;

/**
 * @author Yehia Zayour
 */
public class TokenControllerTest {
    @BeforeClass
    public static void setup() throws Exception {
        initialize();
    }

    @Test
    public void testAuthzCodeFlow() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for authz code grant");
    }

    @Test
    public void testAuthzCodeFlowSltn() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        params.put("solution_user_assertion", TestUtil.sign(solutionUserAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for authz code grant");
    }

    @Test
    public void testAuthzCodeFlowClient() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowClientId() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        params.put("client_id", CLIENT_ID);
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for authz code grant");
    }

    @Test
    public void testAuthzCodeFlowMissingAuthzCode() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("code", null);
        assertErrorResponse(flow, params, "invalid_request", "missing code parameter");
    }

    @Test
    public void testAuthzCodeFlowInvalidAuthzCode() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("code", params.get("code") + "non_matching");
        assertErrorResponse(flow, params, "invalid_grant", "invalid authorization code");
    }

    @Test
    public void testAuthzCodeFlowMissingRedirectUri() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("redirect_uri", null);
        assertErrorResponse(flow, params, "invalid_request", "missing redirect_uri parameter");
    }

    @Test
    public void testAuthzCodeFlowInvalidRedirectUri() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("redirect_uri", "this is not a valid url");
        assertErrorResponse(flow, params, "invalid_request", "invalid redirect_uri parameter");
    }

    @Test
    public void testAuthzCodeFlowNonMatchingClientId() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        ClientID clientId = new ClientID("__new_client_id__");
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.issuer(clientId.getValue());
        claimsBuilder = claimsBuilder.subject(clientId.getValue());
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        CasIdmClient idmClient = idmClientBuilder().clientId(clientId.getValue()).build();
        TokenController controller = tokenController(idmClient);
        String expectedErrorMessage = "client_id does not match that of the original authn request";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testAuthzCodeFlowNonMatchingRedirectUri() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("redirect_uri", params.get("redirect_uri") + "non_matching");
        String expectedErrorMessage = "redirect_uri does not match that of the original authn request";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage);
    }

    @Test
    public void testAuthzCodeFlowNonMatchingTenant() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);

        String tenant = TENANT_NAME + "non_matching";
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
                (ClientAssertion) null,
                new CorrelationID());
        authzCodeManager.add(
                new AuthorizationCode(AUTHZ_CODE),
                new PersonUser(new PrincipalId(USERNAME, tenant), tenant),
                new SessionID(SESSION_ID),
                originalAuthnRequest);
        TokenController controller = tokenController();
        controller.setAuthorizationCodeManager(authzCodeManager);

        String expectedErrorMessage = "tenant does not match that of the original authn request";
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testAuthzCodeFlowScopeParameter() throws Exception {
        // in authz code flow, we ignore scope token request parameter (because it was already supplied in the authn request)
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("scope", "openid");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testAuthzCodeFlowDisabledPersonUser() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().personUserEnabled(false).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "user has been disabled or deleted", controller);
    }

    @Test
    public void testAuthzCodeFlowDisabledSolutionUser() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().solutionUserEnabled(false).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "solution user has been disabled or deleted", controller);
    }

    @Test
    public void testAuthzCodeFlowSolutionUserNotMemberOfActAsGroup() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().systemGroupMembership(Collections.<String>emptySet()).build();
        TokenController controller = tokenController(idmClient);
        String expectedErrorMessage = "solution user acting as a person user must be a member of ActAsUsers group";
        assertErrorResponse(flow, params, "access_denied", expectedErrorMessage, controller);
    }

    @Test
    public void testClientCredentialsFlow() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for client credentials grant");
    }

    @Test
    public void testClientCredentialsFlowSltn() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        params.put("solution_user_assertion", TestUtil.sign(solutionUserAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for client credentials grant");
    }

    @Test
    public void testClientCredentialsFlowClient() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testClientCredentialsFlowClientId() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.remove("client_assertion");
        params.put("client_id", CLIENT_ID);
        assertErrorResponse(flow, params, "invalid_request", "client_assertion parameter is required for client credentials grant");
    }

    @Test
    public void testClientCredentialsFlowRefreshTokenDisallowed() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("scope", "openid offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "refresh token (offline_access) is not allowed for client credentials grant");
    }

    @Test
    public void testClientCredentialsFlowIncorrectTokenClass() throws Exception {
        Flow flow = Flow.CLIENT_CREDS;
        Map<String, String> params = tokenRequestParametersClient(flow);
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "solution_user_assertion");
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion has incorrect token_class claim");
    }

    @Test
    public void testSolutionUserCredentialsFlow() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        params.remove("solution_user_assertion");
        assertErrorResponse(flow, params, "invalid_request", "solution_user_assertion parameter is required for solution user credentials grant");
    }

    @Test
    public void testSolutionUserCredentialsFlowSltn() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testSolutionUserCredentialsFlowClient() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        params.remove("solution_user_assertion");
        params.put("client_assertion", TestUtil.sign(clientAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        assertErrorResponse(flow, params, "invalid_request", "solution_user_assertion parameter is required for solution user credentials grant");
    }

    @Test
    public void testSolutionUserCredentialsFlowClientId() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        params.put("client_id", CLIENT_ID);
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).solutionUserCertSubjectDN(CLIENT_CERT_SUBJECT_DN).build();
        TokenController controller = tokenController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testSolutionUserCredentialsFlowRefreshTokenDisallowed() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        params.put("scope", "openid offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "refresh token (offline_access) is not allowed for solution user credentials grant");
    }

    @Test
    public void testSolutionUserCredentialsFlowIncorrectTokenClass() throws Exception {
        Flow flow = Flow.SOLUTION_USER_CREDS;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        JWTClaimsSet.Builder claimsBuilder = solutionUserAssertionClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "client_assertion");
        params.put("solution_user_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "solution_user_assertion has incorrect token_class claim");
    }

    @Test
    public void testPasswordFlow() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowSltn() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowClient() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowClientId() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParametersClientId(flow);
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testPasswordFlowMissingCorrelationId() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("correlation_id");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPasswordFlowIncorrectCredentials() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("password", params.get("password") + "non_matching");
        assertErrorResponse(flow, params, "invalid_grant", "incorrect username or password");
    }

    @Test
    public void testPersonUserCertFlow() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPersonUserCertFlowSltn() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPersonUserCertFlowClient() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testPersonUserCertFlowClientId() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParametersClientId(flow);
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testPersonUserCertFlowMissingCert() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("person_user_certificate");
        assertErrorResponse(flow, params, "invalid_request", "missing person_user_certificate parameter");
    }

    @Test
    public void testPersonUserCertFlowInvalidCert() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("person_user_certificate", "not_a_cert");
        assertErrorResponse(flow, params, "invalid_request", "failed to parse person_user_certificate parameter");
    }

    @Test
    public void testPersonUserCertFlowIncorrectCert() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParameters(flow);
        CasIdmClient idmClient = idmClientBuilder().clientCertificate(TENANT_CERT).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "invalid_grant", "invalid person user cert", controller);
    }

    @Test
    public void testPersonUserCertFlowIncorrectTokenClass() throws Exception {
        Flow flow = Flow.PERSON_USER_CERT;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = personUserAssertionClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "solution_user_assertion");
        params.put("person_user_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "person_user_assertion has incorrect token_class claim");
    }

    @Test
    public void testGssTicketFlow() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testGssTicketFlow2Legged() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParameters(flow);
        byte[] gssServerLeg = new byte[1];
        CasIdmClient idmClient = idmClientBuilder().gssServerLeg(gssServerLeg).build();
        TokenController controller = tokenController(idmClient);
        String serverLegBase64 = Base64Utils.encodeToString(gssServerLeg);
        String expectedErrorMessage = String.format("gss_continue_needed:%s:%s", GSS_CONTEXT_ID, serverLegBase64);
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testGssTicketFlowSltn() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testGssTicketFlowClient() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testGssTicketFlowClientId() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParametersClientId(flow);
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testGssTicketFlowMissingContextId() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("context_id");
        assertErrorResponse(flow, params, "invalid_request", "missing context_id parameter");
    }

    @Test
    public void testGssTicketFlowMissingGssTicket() throws Exception {
        Flow flow = Flow.GSS_TICKET;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("gss_ticket");
        assertErrorResponse(flow, params, "invalid_request", "missing gss_ticket parameter");
    }

    @Test
    public void testSecurIdFlow() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testSecurIdFlow2Legged() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        String sessionId = "_session_id_xyz_";
        CasIdmClient idmClient = idmClientBuilder().securIdSessionId(sessionId).build();
        TokenController controller = tokenController(idmClient);
        String sessionIdBase64 = Base64Utils.encodeToString(sessionId);
        String expectedErrorMessage = String.format("securid_next_code_required:%s", sessionIdBase64);
        assertErrorResponse(flow, params, "invalid_grant", expectedErrorMessage, controller);
    }

    @Test
    public void testSecurIdFlowSltnn() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testSecurIdFlowClient() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testSecurIdFlowClientId() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParametersClientId(flow);
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testSecurIdFlowWithoutSessionId() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("session_id");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testSecurIdFlowMissingUsername() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("username");
        assertErrorResponse(flow, params, "invalid_request", "missing username parameter");
    }

    @Test
    public void testSecurIdFlowMissingPasscode() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("passcode");
        assertErrorResponse(flow, params, "invalid_request", "missing passcode parameter");
    }

    @Test
    public void testSecurIdFlowIncorrectPasscode() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("passcode", params.get("passcode") + "non_matching");
        assertErrorResponse(flow, params, "invalid_grant", "incorrect securid username or passcode");
    }

    @Test
    public void testSecurIdFlowNewPinRequired() throws Exception {
        Flow flow = Flow.SECURID;
        Map<String, String> params = tokenRequestParameters(flow);
        CasIdmClient idmClient = idmClientBuilder().securIdNewPinRequired(true).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "invalid_grant", "new securid pin required", controller);
    }

    @Test
    public void testRefreshTokenFlow() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testRefreshTokenFlowSltn() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParametersSltn(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testRefreshTokenFlowClient() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParametersClient(flow);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testRefreshTokenFlowClientId() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParametersClientId(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.claim("client_id", CLIENT_ID);
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        assertSuccessResponse(flow, params, controller);
    }

    @Test
    public void testRefreshTokenFlowInvalidRequest() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("refresh_token");
        assertErrorResponse(flow, params, "invalid_request", "missing refresh_token parameter");
    }

    @Test
    public void testRefreshTokenFlowInvalidJwt() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("refresh_token", "this is not a valid jwt");
        String expectedErrorMessage = "failed to parse refresh_token parameter";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testRefreshTokenFlowIncorrectSolutionUser() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaimsSltn();
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this solution user");
    }

    @Test
    public void testRefreshTokenFlowIncorrectClient() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaimsClient();
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this client");
    }

    @Test
    public void testRefreshTokenFlowIncorrectClientId() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.claim("client_id", CLIENT_ID);
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this client", controller);
    }

    @Test
    public void testRefreshTokenFlowIncorrectTenant() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.claim("tenant", TENANT_NAME + "non_matching");
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token was not issued to this tenant");
    }

    @Test
    public void testRefreshTokenFlowDisabledPersonUser() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().personUserEnabled(false).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "user has been disabled or deleted", controller);
    }

    @Test
    public void testRefreshTokenFlowDisabledSolutionUser() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().solutionUserEnabled(false).build();
        TokenController controller = tokenController(idmClient);
        assertErrorResponse(flow, params, "access_denied", "solution user has been disabled or deleted", controller);
    }

    @Test
    public void testRefreshTokenFlowJwtUnsigned() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        params.put("refresh_token", (new PlainJWT(claimsBuilder.build())).serialize());
        String expectedErrorMessage = "failed to parse refresh_token parameter";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidSignatureKey() throws Exception {
        // jwt should be signed using server private key
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token has an invalid signature");
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidSignatureAlgorithm() throws Exception {
        // make sure we do not have the security vulnerability described in the following article:
        // https://auth0.com/blog/2015/03/31/critical-vulnerabilities-in-json-web-token-libraries/
        // an attacker creates a jwt signed with the server's public key but using a symmetric algorithm
        // server might accept the token if the implementation does not check that the algorithm is asymmetric
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        SignedJWT signedJWT = new SignedJWT(new JWSHeader(JWSAlgorithm.HS256), claimsBuilder.build());
        JWSSigner signer = new MACSigner(TENANT_PUBLIC_KEY.getEncoded());
        signedJWT.sign(signer);
        params.put("refresh_token", signedJWT.serialize());
        assertErrorResponse(flow, params, "server_error", "error while verifying refresh_token signature");
    }

    @Test
    public void testRefreshTokenFlowJwtIncorrectTokenClass() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.claim("token_class", "id_token");
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "refresh_token has incorrect token_class claim");
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidSubject() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.subject("username_at_domain");
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "failed to parse subject into a PersonUser");
    }

    @Test
    public void testRefreshTokenFlowJwtEmptyAudience() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.audience(new ArrayList<String>());
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "refresh_token is missing aud claim");
    }

    @Test
    public void testRefreshTokenFlowJwtMissingScope() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.claim("scope", null);
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "refresh_token is missing scope claim");
    }

    @Test
    public void testRefreshTokenFlowJwtInvalidScope() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        claimsBuilder = claimsBuilder.claim("scope", "openid abc");
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "refresh_token has invalid scope claim");
    }

    @Test
    public void testRefreshTokenFlowJwtNotYetValid() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        Date now = new Date();
        claimsBuilder = claimsBuilder.issueTime(new Date(now.getTime() + 8*1000L)); // issued 8 seconds in the future
        claimsBuilder = claimsBuilder.expirationTime(new Date(now.getTime() + 9*1000L)); // expires 9 seconds in the future
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token is not yet valid");
    }

    @Test
    public void testRefreshTokenFlowJwtExpired() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        Date now = new Date();
        claimsBuilder = claimsBuilder.issueTime(new Date(now.getTime() - 9*1000L)); // issued 9 seconds ago
        claimsBuilder = claimsBuilder.expirationTime(new Date(now.getTime() - 8*1000L)); // expired 8 seconds ago
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_grant", "refresh_token has expired");
    }

    @Test
    public void testRefreshTokenFlowJwtIssuedAfterExpired() throws Exception {
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        JWTClaimsSet.Builder claimsBuilder = refreshTokenClaims();
        Date now = new Date();
        claimsBuilder = claimsBuilder.issueTime(new Date(now.getTime() + 2*1000L));
        claimsBuilder = claimsBuilder.expirationTime(now);
        params.put("refresh_token", TestUtil.sign(claimsBuilder.build(), TENANT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "refresh_token iat must be before exp");
    }

    @Test
    public void testRefreshTokenFlowScopeParameter() throws Exception {
        // in refresh token flow, we ignore scope token request parameter (because it was already supplied in previous request)
        Flow flow = Flow.REFRESH_TOKEN;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueIdTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueIdTokenGroupsAccessTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups at_groups rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, tokenController(), expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsAccessTokenGroupsFiltered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups at_groups_filtered rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP_FILTERED;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, tokenController(), expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsFiltered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP_FILTERED;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, tokenController(), expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsFilteredOneNotRegistered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered rs_x rs_y");
        Map<String, ResourceServer> resourceServerMap = new HashMap<String, ResourceServer>();
        resourceServerMap.put("rs_x", new ResourceServer.Builder("rs_x").groupFilter(GROUP_FILTER_RS_X).build());
        resourceServerMap.put("rs_y", new ResourceServer.Builder("rs_y").groupFilter(null).build());
        CasIdmClient idmClient = idmClientBuilder().resourceServerMap(resourceServerMap).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP; // one of the resource servers has no filter, so no filtering can be done
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsFilteredBothNotRegistered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered rs_x rs_y");
        CasIdmClient idmClient = idmClientBuilder().resourceServerMap(Collections.<String, ResourceServer>emptyMap()).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP; // neither one of the resource servers has a filter, so no filtering can be done
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsFilteredAccessTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered at_groups rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP_FILTERED;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, tokenController(), expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsFilteredAccessTokenGroupsFiltered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered at_groups_filtered rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP_FILTERED;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP_FILTERED;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, tokenController(), expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueIdTokenGroupsFilteredNoResourceServer() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered");
        assertErrorResponse(flow, params, "invalid_scope", "id_groups_filtered requested but no resource server requested");
    }

    @Test
    public void testScopeValueIdTokenGroupsFilteredWithIdTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid id_groups_filtered id_groups");
        assertErrorResponse(flow, params, "invalid_scope", "id_groups together with id_groups_filtered is not allowed");
    }

    @Test
    public void testScopeValueAccessTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid at_groups rs_x rs_y");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueAccessTokenGroupsFiltered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid at_groups_filtered rs_x rs_y");
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP_FILTERED;
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, tokenController(), expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueAccessTokenGroupsFilteredOneNotRegistered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid at_groups_filtered rs_x rs_y");
        Map<String, ResourceServer> resourceServerMap = new HashMap<String, ResourceServer>();
        resourceServerMap.put("rs_x", new ResourceServer.Builder("rs_x").groupFilter(GROUP_FILTER_RS_X).build());
        resourceServerMap.put("rs_y", new ResourceServer.Builder("rs_y").build());
        CasIdmClient idmClient = idmClientBuilder().resourceServerMap(resourceServerMap).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP; // one of the resource servers has no filter, so no filtering can be done
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueAccessTokenGroupsFilteredBothNotRegistered() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid at_groups_filtered rs_x rs_y");
        CasIdmClient idmClient = idmClientBuilder().resourceServerMap(Collections.<String, ResourceServer>emptyMap()).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP; // neither one of the resource servers has a filter, so no filtering can be done
        String      expectedAdminServerRole   = ADMIN_SERVER_ROLE;
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueAccessTokenGroupsFilteredNoResourceServer() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid at_groups_filtered");
        assertErrorResponse(flow, params, "invalid_scope", "at_groups_filtered requested but no resource server requested");
    }

    @Test
    public void testScopeValueAccessTokenGroupsFilteredWithAccessTokenGroups() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid at_groups_filtered at_groups");
        assertErrorResponse(flow, params, "invalid_scope", "at_groups together with at_groups_filtered is not allowed");
    }

    @Test
    public void testScopeValueAdminServer() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testScopeValueAdminServerUserIsAdministrator() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        Set<String> groupMembership = qualifyGroupMembership(Arrays.asList("x", "users", "systemconfiguration.administrators", "administrators"));
        CasIdmClient idmClient = idmClientBuilder().groupMembership(groupMembership).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = "Administrator";
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueAdminServerUserIsConfigurationUser() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        Set<String> groupMembership = qualifyGroupMembership(Arrays.asList("x", "users", "systemconfiguration.administrators"));
        CasIdmClient idmClient = idmClientBuilder().groupMembership(groupMembership).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = "ConfigurationUser";
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueAdminServerUserIsRegularUser() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        Set<String> groupMembership = qualifyGroupMembership(Arrays.asList("x", "users"));
        CasIdmClient idmClient = idmClientBuilder().groupMembership(groupMembership).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = "RegularUser";
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueAdminServerUserIsGuestUser() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid rs_admin_server");
        Set<String> groupMembership = qualifyGroupMembership(Arrays.asList("x"));
        CasIdmClient idmClient = idmClientBuilder().groupMembership(groupMembership).build();
        TokenController controller = tokenController(idmClient);
        Set<String> expectedIdTokenGroups     = GROUP_MEMBERSHIP;
        Set<String> expectedAccessTokenGroups = GROUP_MEMBERSHIP;
        String      expectedAdminServerRole   = "GuestUser";
        assertSuccessResponse(flow, params, controller, expectedIdTokenGroups, expectedAccessTokenGroups, expectedAdminServerRole);
    }

    @Test
    public void testScopeValueResourceServerX() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid " + SCOPE_VALUE_RSX);
        assertSuccessResponse(flow, params);
    }

    @Test
    public void testUnregisteredClient() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().clientId(null).build();
        TokenController controller = tokenController(idmClient);
        String expectedErrorMessage = "unregistered client";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage, controller);
    }

    @Test
    public void testNonExistentTenant() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        TokenController controller = tokenController();
        controller.acquireTokens(request, response, "non_matching_tenant");
        String expectedErrorMessage = "non-existent tenant";
        validateTokenErrorResponse(response, flow, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testQueryParametersDisallowed() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        request.setQueryString("p1=v1");
        MockHttpServletResponse response = new MockHttpServletResponse();
        TokenController controller = tokenController();
        controller.acquireTokens(request, response, TENANT_NAME);
        String expectedErrorMessage = "query parameters are not allowed at token endpoint";
        validateTokenErrorResponse(response, flow, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testUnsupportedGrantType() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = new HashMap<String, String>();
        params.put("grant_type", "urn:ietf:params:oauth:grant-type:jwt-bearer");
        assertErrorResponse(flow, params, "unsupported_grant_type", "unsupported grant_type parameter");
    }

    @Test
    public void testEmptyGrantType() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("grant_type", "");
        assertErrorResponse(flow, params, "invalid_request", "missing grant_type parameter");
    }

    @Test
    public void testMissingGrantType() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("grant_type");
        assertErrorResponse(flow, params, "invalid_request", "missing grant_type parameter");
    }

    @Test
    public void testMissingScope() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.remove("scope");
        assertErrorResponse(flow, params, "invalid_request", "missing scope parameter");
    }

    @Test
    public void testMissingOpenIdScopeValue() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "missing openid scope value");
    }

    @Test
    public void testUnrecognizedScopeValue() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid profile");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: profile");
    }

    @Test
    public void testMultipleSpacesScopeValue() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid   offline_access");
        assertErrorResponse(flow, params, "invalid_scope", "scope must be a sequence of single-space-delimited values");
    }

    @Test
    public void testInvalidResourceServerNameScopeValue() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("scope", "openid rs_");
        assertErrorResponse(flow, params, "invalid_scope", "unrecognized scope value: rs_");
    }

    @Test
    public void testBothAssertionsDisallowed() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("solution_user_assertion", TestUtil.sign(solutionUserAssertionClaims().build(), CLIENT_PRIVATE_KEY).serialize());
        assertErrorResponse(flow, params, "invalid_request", "client_assertion and solution_user_assertion in the same request is not allowed");
    }

    @Test
    public void testInvalidSolutionUserAssertion() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("solution_user_assertion", "this is an invalid JWT");
        assertErrorResponse(flow, params, "invalid_request", "failed to parse solution_user_assertion parameter");
    }

    @Test
    public void testUnsupportedClientAuthnMethodIsIgnored() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParameters(flow);
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        request.addHeader("Authorization", "Basic czZCaGRSa3F0Mzo3RmpmcDBaQnIxS3REUmJuZlZkbUl3");
        MockHttpServletResponse response = new MockHttpServletResponse();
        TokenController controller = tokenController();
        controller.acquireTokens(request, response);
        validateTokenSuccessResponse(response, flow, Scope.parse(params.get("scope")), false /* wSltnAssertion */, false /* wClientAssertion */, NONCE);
    }

    @Test
    public void testSolutionUserAssertionIssuerAndSubjectMismatch() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet.Builder claimsBuilder = solutionUserAssertionClaims();
        claimsBuilder = claimsBuilder.issuer(ISSUER + "non_matching");
        Map<String, String> params = tokenRequestParametersSltn(flow, claimsBuilder.build());
        String expectedErrorMessage = "solution_user_assertion issuer and subject must be the same";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testClientAssertionMissingSubject() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.subject(null);
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        String expectedErrorMessage = "client_assertion is missing sub claim";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testClientAssertionIssuerAndSubjectMismatch() throws Exception {
        Flow flow = Flow.PASSWORD;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.issuer(ISSUER + "non_matching");
        Map<String, String> params = tokenRequestParameters(flow);
        params.put("client_assertion", TestUtil.sign(claimsBuilder.build(), CLIENT_PRIVATE_KEY).serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
        String expectedErrorMessage = "client_assertion issuer and subject must be the same";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testClientAssertionIssuerAndClientIdMismatch() throws Exception {
        Flow flow = Flow.PASSWORD;
        Map<String, String> params = tokenRequestParametersClient(flow);
        params.put("client_id", CLIENT_ID + "non_matching");
        String expectedErrorMessage = "client_assertion issuer must match client_id";
        assertErrorResponse(flow, params, "invalid_request", expectedErrorMessage);
    }

    @Test
    public void testAssertionIncorrectAudience() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.audience("https://sso.com/invalid_token_endpoint");
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        String expectedErrorMessage = "client_assertion audience does not match request URI";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionMultiValuedAudience() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.audience(Arrays.asList("v1", "v2"));
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        String expectedErrorMessage = "client_assertion audience should be single-valued";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionInvalidAudienceURI() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        claimsBuilder = claimsBuilder.audience("this_is_not_a_valid_uri");
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        String expectedErrorMessage = "client_assertion audience should be a valid URI";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionIssuedAtTooFarInThePast() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        Date now = new Date();
        Date issuedAt = new Date(now.getTime() - 5 * 60 * 1000L); // issued 5 mins ago
        claimsBuilder = claimsBuilder.issueTime(issuedAt);
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        String expectedErrorMessage = "client_assertion has expired";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionIssuedAtTooFarInTheFuture() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        Date now = new Date();
        Date issuedAt = new Date(now.getTime() + 5 * 60 * 1000L); // issued 5 mins in the future
        claimsBuilder = claimsBuilder.issueTime(issuedAt);
        Map<String, String> params = tokenRequestParametersClient(flow, claimsBuilder.build());
        String expectedErrorMessage = "client_assertion is not yet valid";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    @Test
    public void testAssertionNoRegisteredCert() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);
        CasIdmClient idmClient = idmClientBuilder().tokenEndpointAuthMethod("none").clientCertSubjectDN(null).build();
        TokenController controller = tokenController(idmClient);
        String expectedErrorMessage = "client_assertion parameter is not allowed since client did not register a cert";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage, controller);
    }

    @Test
    public void testAssertionInvalidSignature() throws Exception {
        Flow flow = Flow.AUTHZ_CODE;
        Map<String, String> params = tokenRequestParametersClient(flow);

        KeyPairGenerator keyGenerator = KeyPairGenerator.getInstance("RSA");
        keyGenerator.initialize(1024);
        KeyPair kp = keyGenerator.genKeyPair();
        RSAPrivateKey privateKey = (RSAPrivateKey) kp.getPrivate();

        JWTClaimsSet.Builder claimsBuilder = clientAssertionClaims();
        SignedJWT privateKeyJwt = TestUtil.sign(claimsBuilder.build(), privateKey);
        params.put("client_assertion", privateKeyJwt.serialize());
        params.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");

        String expectedErrorMessage = "client_assertion has an invalid signature";
        assertErrorResponse(flow, params, "invalid_client", expectedErrorMessage);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params) throws Exception {
        assertSuccessResponse(flow, params, tokenController(), GROUP_MEMBERSHIP, GROUP_MEMBERSHIP, ADMIN_SERVER_ROLE);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            TokenController controller) throws Exception {
        assertSuccessResponse(flow, params, controller, GROUP_MEMBERSHIP, GROUP_MEMBERSHIP, ADMIN_SERVER_ROLE);
    }

    private static void assertSuccessResponse(
            Flow flow,
            Map<String, String> params,
            TokenController controller,
            Set<String> expectedIdTokenGroups,
            Set<String> expectedAccessTokenGroups,
            String expectedAdminServerRole) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller);
        String scopeString = params.get("scope");
        Scope scope = (scopeString == null) ? Scope.OPENID : Scope.parse(scopeString);
        boolean wSltnAssertion = params.containsKey("solution_user_assertion");
        boolean wClientAssertion = params.containsKey("client_assertion");
        validateTokenSuccessResponse(
                response,
                flow,
                scope,
                wSltnAssertion,
                wClientAssertion,
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
        assertErrorResponse(flow, params, expectedError, expectedErrorDescription, tokenController());
    }

    private static void assertErrorResponse(
            Flow flow,
            Map<String, String> params,
            String expectedError,
            String expectedErrorDescription,
            TokenController controller) throws Exception {
        MockHttpServletResponse response = doRequest(params, controller);
        validateTokenErrorResponse(response, flow, expectedError, expectedErrorDescription);
    }

    private static MockHttpServletResponse doRequest(
            Map<String, String> params,
            TokenController controller) throws Exception {
        MockHttpServletRequest request = TestUtil.createPostRequest(params);
        MockHttpServletResponse response = new MockHttpServletResponse();
        controller.acquireTokens(request, response);
        return response;
    }
}