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

package com.vmware.identity.openidconnect.protocol;

import static com.vmware.identity.openidconnect.protocol.TestContext.AUTHZ_CODE;
import static com.vmware.identity.openidconnect.protocol.TestContext.CERT;
import static com.vmware.identity.openidconnect.protocol.TestContext.CERT_ENCODED;
import static com.vmware.identity.openidconnect.protocol.TestContext.CLIENT_ASSERTION;
import static com.vmware.identity.openidconnect.protocol.TestContext.GSS_CONTEXT_ID;
import static com.vmware.identity.openidconnect.protocol.TestContext.GSS_TICKET;
import static com.vmware.identity.openidconnect.protocol.TestContext.PASSWORD;
import static com.vmware.identity.openidconnect.protocol.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.REFRESH_TOKEN;
import static com.vmware.identity.openidconnect.protocol.TestContext.REQUEST_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.SCOPE;
import static com.vmware.identity.openidconnect.protocol.TestContext.SECUREID_PASSCODE;
import static com.vmware.identity.openidconnect.protocol.TestContext.SECUREID_SESSION_ID;
import static com.vmware.identity.openidconnect.protocol.TestContext.SOLUTION_USER_ASSERTION;
import static com.vmware.identity.openidconnect.protocol.TestContext.USERNAME;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.AuthorizationCodeGrant;
import com.vmware.identity.openidconnect.common.Base64Utils;
import com.vmware.identity.openidconnect.common.ClientCertificateGrant;
import com.vmware.identity.openidconnect.common.ClientCredentialsGrant;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.GSSTicketGrant;
import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.PasswordGrant;
import com.vmware.identity.openidconnect.common.RefreshTokenGrant;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SecureIDGrant;
import com.vmware.identity.openidconnect.common.SolutionUserCredentialsGrant;

/**
 * @author Yehia Zayour
 */
public class TokenRequestTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testPasswordGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.PASSWORD);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new PasswordGrant(USERNAME, PASSWORD),
                Scope.OPENID,
                (SolutionUserAssertion) null,
                (ClientAssertion) null,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testPasswordGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.PASSWORD);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof PasswordGrant);
        PasswordGrant grant = (PasswordGrant) tokenRequest.getAuthorizationGrant();
        Assert.assertEquals("username", grant.getUsername(), USERNAME);
        Assert.assertEquals("password", grant.getPassword(), PASSWORD);
        Assert.assertEquals("scope", tokenRequest.getScope(), SCOPE);
    }

    @Test
    public void testPasswordGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.PASSWORD);
        parameters.remove("password");
        assertParseError(parameters, "missing password parameter");
    }

    @Test
    public void testAuthzCodeGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.AUTHORIZATION_CODE);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new AuthorizationCodeGrant(AUTHZ_CODE, REDIRECT_URI),
                (Scope) null,
                (SolutionUserAssertion) null,
                CLIENT_ASSERTION,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testAuthzCodeGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.AUTHORIZATION_CODE);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof AuthorizationCodeGrant);
        AuthorizationCodeGrant grant = (AuthorizationCodeGrant) tokenRequest.getAuthorizationGrant();
        Assert.assertEquals("code", grant.getAuthorizationCode(), AUTHZ_CODE);
        Assert.assertEquals("redirect_uri", grant.getRedirectURI(), REDIRECT_URI);
        Assert.assertEquals("client_assertion", tokenRequest.getClientAssertion().serialize(), CLIENT_ASSERTION.serialize());
        Assert.assertEquals("scope", tokenRequest.getScope(), null);
    }

    @Test
    public void testAuthzCodeGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.AUTHORIZATION_CODE);
        parameters.remove("code");
        assertParseError(parameters, "missing code parameter");
    }

    @Test
    public void testRefreshTokenGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.REFRESH_TOKEN);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new RefreshTokenGrant(REFRESH_TOKEN.serialize()),
                (Scope) null,
                (SolutionUserAssertion) null,
                (ClientAssertion) null,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testRefreshTokenGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.REFRESH_TOKEN);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof RefreshTokenGrant);
        RefreshTokenGrant grant = (RefreshTokenGrant) tokenRequest.getAuthorizationGrant();
        Assert.assertEquals("refresh_token", grant.getRefreshToken(), REFRESH_TOKEN.serialize());
        Assert.assertEquals("scope", tokenRequest.getScope(), null);
    }

    @Test
    public void testRefreshTokenGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.REFRESH_TOKEN);
        parameters.remove("refresh_token");
        assertParseError(parameters, "missing refresh_token parameter");
    }

    @Test
    public void testClientCredentialsGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.CLIENT_CREDENTIALS);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new ClientCredentialsGrant(),
                SCOPE,
                (SolutionUserAssertion) null,
                CLIENT_ASSERTION,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testClientCredentialsGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.CLIENT_CREDENTIALS);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof ClientCredentialsGrant);
        Assert.assertEquals("client_assertion", tokenRequest.getClientAssertion().serialize(), CLIENT_ASSERTION.serialize());
        Assert.assertEquals("scope", tokenRequest.getScope(), SCOPE);
    }

    @Test
    public void testClientCredentialsGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.CLIENT_CREDENTIALS);
        parameters.remove("client_assertion");
        assertParseError(parameters, "client_assertion parameter is required for client credentials grant");
    }

    @Test
    public void testSolutionUserCredentialsGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.SOLUTION_USER_CREDENTIALS);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new SolutionUserCredentialsGrant(),
                SCOPE,
                SOLUTION_USER_ASSERTION,
                (ClientAssertion) null,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testSolutionUserCredentialsGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.SOLUTION_USER_CREDENTIALS);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof SolutionUserCredentialsGrant);
        Assert.assertEquals("solution_user_assertion", tokenRequest.getSolutionUserAssertion().serialize(), SOLUTION_USER_ASSERTION.serialize());
        Assert.assertEquals("scope", tokenRequest.getScope(), SCOPE);
    }

    @Test
    public void testSolutionUserCredentialsGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.SOLUTION_USER_CREDENTIALS);
        parameters.remove("solution_user_assertion");
        assertParseError(parameters, "solution_user_assertion parameter is required for solution user credentials grant");
    }

    @Test
    public void testGssTicketGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.GSS_TICKET);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new GSSTicketGrant(GSS_CONTEXT_ID, GSS_TICKET),
                SCOPE,
                (SolutionUserAssertion) null,
                (ClientAssertion) null,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testGssTicketGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.GSS_TICKET);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof GSSTicketGrant);
        GSSTicketGrant grant = (GSSTicketGrant) tokenRequest.getAuthorizationGrant();
        Assert.assertEquals("context_id", grant.getContextID(), GSS_CONTEXT_ID);
        Assert.assertEquals("gss_ticket", grant.getGSSTicket().length, GSS_TICKET.length);
        Assert.assertEquals("scope", tokenRequest.getScope(), SCOPE);
    }

    @Test
    public void testGssTicketGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.GSS_TICKET);
        parameters.remove("gss_ticket");
        assertParseError(parameters, "missing gss_ticket parameter");
    }

    @Test
    public void testClientCertGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.CLIENT_CERTIFICATE);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new ClientCertificateGrant(Arrays.asList(CERT)),
                SCOPE,
                (SolutionUserAssertion) null,
                (ClientAssertion) null,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testClientCertGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.CLIENT_CERTIFICATE);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof ClientCertificateGrant);
        ClientCertificateGrant grant = (ClientCertificateGrant) tokenRequest.getAuthorizationGrant();
        Assert.assertEquals("client_certificate_chain", grant.getClientCertificateChain(), Arrays.asList(CERT));
        Assert.assertEquals("scope", tokenRequest.getScope(), SCOPE);
    }

    @Test
    public void testClientCertGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.CLIENT_CERTIFICATE);
        parameters.remove("client_certificate_chain");
        assertParseError(parameters, "missing client_certificate_chain parameter");
    }

    @Test
    public void testSecureIdGrantSerialize() {
        Map<String, String> parameters = parameters(GrantType.SECUREID);
        TokenRequest tokenRequest = new TokenRequest(
                REQUEST_URI,
                new SecureIDGrant(USERNAME, SECUREID_PASSCODE, SECUREID_SESSION_ID),
                SCOPE,
                (SolutionUserAssertion) null,
                (ClientAssertion) null,
                (CorrelationID) null);
        Assert.assertEquals("parameters", parameters, tokenRequest.toHttpRequest().getParameters());
    }

    @Test
    public void testSecureIdGrantParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters(GrantType.SECUREID);
        TokenRequest tokenRequest = TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
        Assert.assertTrue("grant_type", tokenRequest.getAuthorizationGrant() instanceof SecureIDGrant);
        SecureIDGrant grant = (SecureIDGrant) tokenRequest.getAuthorizationGrant();
        Assert.assertEquals("username", grant.getUsername(), USERNAME);
        Assert.assertEquals("passcode", grant.getPasscode(), SECUREID_PASSCODE);
        Assert.assertEquals("session_id", grant.getSessionID(), SECUREID_SESSION_ID);
        Assert.assertEquals("scope", tokenRequest.getScope(), SCOPE);
    }

    @Test
    public void testSecureIdGrantParseError() {
        Map<String, String> parameters = parameters(GrantType.SECUREID);
        parameters.remove("passcode");
        assertParseError(parameters, "missing passcode parameter");
    }

    private static Map<String, String> parameters(GrantType grantType) {
        Map<String, String> parameters = new HashMap<String, String>();

        switch (grantType) {
            case AUTHORIZATION_CODE:
                parameters.put("grant_type", "authorization_code");
                parameters.put("code", AUTHZ_CODE.getValue());
                parameters.put("redirect_uri", REDIRECT_URI.toString());
                parameters.put("client_assertion", CLIENT_ASSERTION.serialize());
                parameters.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
                break;
            case PASSWORD:
                parameters.put("grant_type", "password");
                parameters.put("username", USERNAME);
                parameters.put("password", PASSWORD);
                parameters.put("scope", SCOPE.toString());
                break;
            case REFRESH_TOKEN:
                parameters.put("grant_type", "refresh_token");
                parameters.put("refresh_token", REFRESH_TOKEN.serialize());
                break;
            case CLIENT_CREDENTIALS:
                parameters.put("grant_type", "client_credentials");
                parameters.put("client_assertion", CLIENT_ASSERTION.serialize());
                parameters.put("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
                parameters.put("scope", SCOPE.toString());
                break;
            case SOLUTION_USER_CREDENTIALS:
                parameters.put("grant_type", "urn:vmware:grant_type:solution_user_credentials");
                parameters.put("solution_user_assertion", SOLUTION_USER_ASSERTION.serialize());
                parameters.put("scope", SCOPE.toString());
                break;
            case GSS_TICKET:
                parameters.put("grant_type", "urn:vmware:grant_type:gss_ticket");
                parameters.put("context_id", GSS_CONTEXT_ID);
                parameters.put("gss_ticket", Base64Utils.encodeToString(GSS_TICKET));
                parameters.put("scope", SCOPE.toString());
                break;
            case CLIENT_CERTIFICATE:
                parameters.put("grant_type", "urn:vmware:grant_type:client_certificate");
                parameters.put("client_certificate_chain", CERT_ENCODED);
                parameters.put("scope", SCOPE.toString());
                break;
            case SECUREID:
                parameters.put("grant_type", "urn:vmware:grant_type:secureid");
                parameters.put("username", USERNAME);
                parameters.put("passcode", SECUREID_PASSCODE);
                parameters.put("session_id", Base64Utils.encodeToString(SECUREID_SESSION_ID));
                parameters.put("scope", SCOPE.toString());
                break;
            default:
                throw new IllegalArgumentException("unrecognized: " + grantType);
        }

        return parameters;
    }

    private void assertParseError(Map<String, String> parameters, String expectedErrorDescription) {
        try {
            TokenRequest.parse(HttpRequest.createPostRequest(REQUEST_URI, parameters));
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", expectedErrorDescription, e.getErrorObject().getDescription());
        }
    }
}