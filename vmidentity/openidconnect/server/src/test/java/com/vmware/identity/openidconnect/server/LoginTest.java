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

import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_CERT;
import static com.vmware.identity.openidconnect.server.TestContext.GSS_CONTEXT_ID;
import static com.vmware.identity.openidconnect.server.TestContext.NONCE;
import static com.vmware.identity.openidconnect.server.TestContext.PASSWORD;
import static com.vmware.identity.openidconnect.server.TestContext.SECURID_PASSCODE;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_COOKIE_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.SESSION_ID;
import static com.vmware.identity.openidconnect.server.TestContext.STATE;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_CERT;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.USERNAME;
import static com.vmware.identity.openidconnect.server.TestContext.authnController;
import static com.vmware.identity.openidconnect.server.TestContext.authnRequestParameters;
import static com.vmware.identity.openidconnect.server.TestContext.gssBrowserLoginString;
import static com.vmware.identity.openidconnect.server.TestContext.gssCIPLoginString;
import static com.vmware.identity.openidconnect.server.TestContext.idmClient;
import static com.vmware.identity.openidconnect.server.TestContext.idmClientBuilder;
import static com.vmware.identity.openidconnect.server.TestContext.initialize;
import static com.vmware.identity.openidconnect.server.TestContext.passwordLoginString;
import static com.vmware.identity.openidconnect.server.TestContext.securIdLoginString;
import static com.vmware.identity.openidconnect.server.TestContext.validateAuthnSuccessResponse;

import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import javax.servlet.http.Cookie;

import org.apache.commons.lang3.tuple.Pair;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.ui.ExtendedModelMap;
import org.springframework.web.servlet.ModelAndView;

import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.protocol.Base64Utils;

/**
 * @author Yehia Zayour
 */
public class LoginTest {
    @BeforeClass
    public static void setup() throws Exception {
        initialize();
    }

    @Test
    public void testPasswordLogin() throws Exception {
        String loginString = passwordLoginString();
        assertSuccessResponse(loginString);
    }

    @Test
    public void testPasswordLoginIncorrectCredentials() throws Exception {
        String loginString = passwordLoginString(USERNAME + "_non_matching", PASSWORD + "_non_matching");
        assertErrorResponse(loginString, "access_denied: incorrect username or password", null);
    }

    @Test
    public void testPasswordLoginInvalidLoginString() throws Exception {
        String loginString = passwordLoginString() + " extra";
        assertErrorResponse(loginString, "access_denied: malformed password login string", null);
    }

    @Test
    public void testPasswordLoginInvalidUsernamePassword() throws Exception {
        String unp = "usernamepassword"; // should be username:password
        String unp64 = Base64Utils.encodeToString(unp);
        String loginString = "Basic " + unp64;
        assertErrorResponse(loginString, "access_denied: malformed username:password in login string", null);
    }

    @Test
    public void testPersonUserCertLoginAgainstReverseProxy() throws Exception {
        String certString64 = Base64Utils.encodeToString(CLIENT_CERT.getEncoded());
        assertSuccessResponseUsingPersonUserCert(certString64, null);
    }

    @Test
    public void testPersonUserCertLoginAgainstTomcat() throws Exception {
        assertSuccessResponseUsingPersonUserCert(null, new X509Certificate[] { CLIENT_CERT });
    }

    @Test
    public void testPersonUserCertLoginInvalidCert() throws Exception {
        assertErrorResponseUsingPersonUserCert("ThisIsNotACert", null, (Cookie) null, "access_denied: failed to parse person user cert");
    }

    @Test
    public void testPersonUserCertLoginIncorrectCert() throws Exception {
        X509Certificate cert = TENANT_CERT;
        byte[] certBytes = cert.getEncoded();
        String certString64 = Base64Utils.encodeToString(certBytes);
        assertErrorResponseUsingPersonUserCert(certString64, null, (Cookie) null, "access_denied: invalid person user cert");
    }

    @Test
    public void testPersonUserCertLoginMissingCert() throws Exception {
        assertErrorResponseUsingPersonUserCert(null, null, (Cookie) null, "access_denied: missing person user cert");
    }

    @Test
    public void testPersonUserCertLoginReopenBrowserWindow() throws Exception {
        Cookie cookie = new Cookie(SessionManager.getPersonUserCertificateLoggedOutCookieName(TENANT_NAME), "");
        assertErrorResponseUsingPersonUserCert(null, null, cookie, "access_denied: already logged in once on this browser session");
    }

    @Test
    public void testGssLoginBrowserOneLegged() throws Exception {
        String loginString = gssBrowserLoginString();
        String authzHeader = "Negotiate _gss_ticket_";
        assertSuccessResponse(loginString, authzHeader);
    }

    @Test
    public void testGssLoginBrowserTwoLegged() throws Exception {
        String loginString = gssBrowserLoginString();
        String authzHeader = "Negotiate _gss_ticket_";
        byte[] gssServerLeg = new byte[1];
        String gssServerLeg64 = Base64Utils.encodeToString(gssServerLeg);
        CasIdmClient idmClient = idmClientBuilder().gssServerLeg(gssServerLeg).build();
        assertErrorResponse(loginString, authzHeader, "access_denied: gss_continue_needed", null, "Negotiate " + gssServerLeg64, idmClient);
    }

    @Test
    public void testGssLoginBrowserPrompt() throws Exception {
        String loginString = gssBrowserLoginString();
        assertErrorResponse(loginString, null, "access_denied: authorization_header_needed", null, "Negotiate", idmClient());
    }

    @Test
    public void testGssLoginCIPOneLegged() throws Exception {
        String loginString = gssCIPLoginString();
        assertSuccessResponse(loginString);
    }

    @Test
    public void testGssLoginCIPTwoLegged() throws Exception {
        String contextId = GSS_CONTEXT_ID;
        String loginString = gssCIPLoginString(contextId);
        byte[] gssServerLeg = new byte[1];
        String gssServerLeg64 = Base64Utils.encodeToString(gssServerLeg);
        CasIdmClient idmClient = idmClientBuilder().gssServerLeg(gssServerLeg).build();
        assertErrorResponse(loginString, "access_denied: gss_continue_needed", String.format("Negotiate %s %s", contextId, gssServerLeg64), idmClient);
    }

    @Test
    public void testGssLoginCIPInvalidTicket() throws Exception {
        String contextId = GSS_CONTEXT_ID + "non_matching";
        String loginString = gssCIPLoginString(contextId);
        assertErrorResponse(loginString, "access_denied: invalid gss ticket", null);
    }

    @Test
    public void testGssLoginCIPInvalidLoginString() throws Exception {
        String loginString = gssCIPLoginString() + " extra";
        assertErrorResponse(loginString, "access_denied: malformed gss login string", null);
    }

    @Test
    public void testSecurIdLoginOneLeggedWithSessionId() throws Exception {
        String loginString = securIdLoginString(USERNAME, SECURID_PASSCODE, "securid_session_id");
        assertSuccessResponse(loginString);
    }

    @Test
    public void testSecurIdLoginOneLeggedWithoutSessionId() throws Exception {
        String loginString = securIdLoginString(USERNAME, SECURID_PASSCODE, null);
        assertSuccessResponse(loginString);
    }

    @Test
    public void testSecurIdLoginTwoLeggedWithSessionId() throws Exception {
        String sessionId = "securid_session_id";
        String sessionId64 = Base64Utils.encodeToString(sessionId);
        String loginString = securIdLoginString(USERNAME, SECURID_PASSCODE, sessionId);
        CasIdmClient idmClient = idmClientBuilder().securIdSessionId(sessionId).build();
        assertErrorResponse(loginString, "access_denied: securid_next_code_required", "RSAAM " + sessionId64, idmClient);
    }

    @Test
    public void testSecurIdLoginTwoLeggedWithoutSessionId() throws Exception {
        String sessionId = "securid_session_id";
        String sessionId64 = Base64Utils.encodeToString(sessionId);
        String loginString = securIdLoginString(USERNAME, SECURID_PASSCODE, null);
        CasIdmClient idmClient = idmClientBuilder().securIdSessionId(sessionId).build();
        assertErrorResponse(loginString, "access_denied: securid_next_code_required", "RSAAM " + sessionId64, idmClient);
    }

    @Test
    public void testSecurIdLoginIncorrectCredentials() throws Exception {
        String loginString = securIdLoginString(USERNAME + "_non_matching", SECURID_PASSCODE + "_non_matching", null);
        assertErrorResponse(loginString, "access_denied: incorrect securid username or passcode", null);
    }

    @Test
    public void testSecurIdLoginInvalidLoginString() throws Exception {
        String loginString = securIdLoginString() + " extra";
        assertErrorResponse(loginString, "access_denied: malformed securid login string", null);
    }

    @Test
    public void testSecurIdLoginInvalidUsernamePasscode() throws Exception {
        String unp = "usernamepasscode"; // should be username:passcode
        String unp64 = Base64Utils.encodeToString(unp);
        String loginString = "RSAAM " + unp64;
        assertErrorResponse(loginString, "access_denied: malformed username:passcode in securid login string", null);
    }

    @Test
    public void testSecurIdLoginNewPinRequired() throws Exception {
        String loginString = securIdLoginString();
        CasIdmClient idmClient = idmClientBuilder().securIdNewPinRequired(true).build();
        assertErrorResponse(loginString, "access_denied: new securid pin required", null, idmClient);
    }

    @Test
    public void testInvalidLoginMethod() throws Exception {
        String loginString = "invalid_method";
        assertErrorResponse(loginString, "access_denied: invalid login method", null);
    }

    @Test
    public void testSessionLogin() throws Exception {
        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, SESSION_ID);
        Pair<ModelAndView, MockHttpServletResponse> result = doRequest(null /* loginString */, sessionCookie);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();
        Assert.assertNull("modelView", modelView);
        validateAuthnSuccessResponse(response, Flow.AUTHZ_CODE, Scope.OPENID, false, false, STATE, NONCE);
    }

    @Test
    public void testLoginStringWithSessionCookieMatching() throws Exception {
        // if request has both a loginString and session cookie, then if the session cookie matches, use it and ignore the loginString
        String loginString = passwordLoginString();
        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, SESSION_ID);
        Pair<ModelAndView, MockHttpServletResponse> result = doRequest(loginString, sessionCookie);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();
        Assert.assertNull("modelView", modelView);
        boolean ajaxRequest = false; // it is actually an ajax request but then TestContext would expect a session cookie to be returned
        validateAuthnSuccessResponse(response, Flow.AUTHZ_CODE, Scope.OPENID, false, ajaxRequest, STATE, NONCE);
    }

    @Test
    public void testLoginStringWithSessionCookieNonMatching() throws Exception {
        // if request has both a loginString and session cookie, then if the session cookie does not match, process the loginString
        String loginString = passwordLoginString();
        Cookie nonMatchingsessionCookie = new Cookie(SESSION_COOKIE_NAME, SESSION_ID + "_nonmatching");
        Pair<ModelAndView, MockHttpServletResponse> result = doRequest(loginString, nonMatchingsessionCookie);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();
        Assert.assertNull("modelView", modelView);
        validateAuthnSuccessResponse(response, Flow.AUTHZ_CODE, Scope.OPENID, false, true, STATE, NONCE);
    }

    @Test
    public void testMissingLogin() throws Exception {
        Pair<ModelAndView, MockHttpServletResponse> result = doRequest(null /* loginString */, null /* sessionCookie */);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();
        Assert.assertNotNull("modelView", modelView); // logon form should be served
        Assert.assertNull("sessionCookie", response.getCookie(SESSION_COOKIE_NAME));
        Assert.assertEquals("status", 200, response.getStatus());
    }

    private static void assertSuccessResponse(String loginString) throws Exception {
        assertSuccessResponse(loginString, null /* authzHeader */);
    }

    private static void assertSuccessResponse(String loginString, String authzHeader) throws Exception {
        Pair<ModelAndView, MockHttpServletResponse> result = doRequest(loginString, authzHeader, null /* sessionCookie */, idmClient());
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();
        Assert.assertNull("modelView", modelView);
        validateAuthnSuccessResponse(response, Flow.AUTHZ_CODE, Scope.OPENID, false, true, STATE, NONCE);
    }

    private static void assertSuccessResponseUsingPersonUserCert(
            String certHeader,
            Object certAttribute) throws Exception {
        Pair<ModelAndView, MockHttpServletResponse> result = doRequestUsingPersonUserCert(certHeader, certAttribute, (Cookie) null);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();

        Assert.assertNull("modelView", modelView);
        validateAuthnSuccessResponse(response, Flow.AUTHZ_CODE, Scope.OPENID, false /* redirectResponseMode */, true /* ajaxRequest */, STATE, NONCE);
    }

    private static void assertErrorResponse(
            String loginString,
            String expectedError,
            String expectedAuthzResponseHeader) throws Exception {
        assertErrorResponse(loginString, null, expectedError, expectedAuthzResponseHeader, null, idmClient());
    }

    private static void assertErrorResponse(
            String loginString,
            String expectedError,
            String expectedAuthzResponseHeader,
            CasIdmClient idmClient) throws Exception {
        assertErrorResponse(loginString, null, expectedError, expectedAuthzResponseHeader, null, idmClient);
    }

    private static void assertErrorResponse(
            String loginString,
            String authzHeader,
            String expectedError,
            String expectedAuthzResponseHeader,
            String expectedAuthenticateHeader,
            CasIdmClient idmClient) throws Exception {
        Pair<ModelAndView, MockHttpServletResponse> result = doRequest(loginString, authzHeader, null /* sessionCookie */, idmClient);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();
        Assert.assertNull("modelView", modelView);
        Assert.assertNull("sessionCookie", response.getCookie(SESSION_COOKIE_NAME));
        Assert.assertEquals("status", 401, response.getStatus());
        Object errorResponseHeader = response.getHeader("CastleError");
        Assert.assertNotNull("errorResponseHeader", errorResponseHeader);
        Assert.assertEquals("errorMessage", expectedError, response.getErrorMessage());

        if (expectedAuthzResponseHeader != null) {
            Object authzResponseHeader = response.getHeader("CastleAuthorization");
            Assert.assertNotNull("authzResponseHeader", authzResponseHeader);
            Assert.assertEquals("expectedAuthzResponseHeader", expectedAuthzResponseHeader, authzResponseHeader.toString());
        }

        if (expectedAuthenticateHeader != null) {
            Object wwwAuthenticateHeader = response.getHeader("WWW-Authenticate");
            Assert.assertNotNull("wwwAuthenticateHeader", wwwAuthenticateHeader);
            Assert.assertEquals("expectedAuthenticateHeader", expectedAuthenticateHeader, wwwAuthenticateHeader.toString());
        }
    }

    private static void assertErrorResponseUsingPersonUserCert(
            String certHeader,
            Object certAttribute,
            Cookie cookie,
            String expectedError) throws Exception {
        Pair<ModelAndView, MockHttpServletResponse> result = doRequestUsingPersonUserCert(certHeader, certAttribute, cookie);
        ModelAndView modelView = result.getLeft();
        MockHttpServletResponse response = result.getRight();

        Assert.assertNull("modelView", modelView);
        Assert.assertNull("sessionCookie", response.getCookie(SESSION_COOKIE_NAME));
        Assert.assertEquals("status", 401, response.getStatus());
        Assert.assertNotNull("errorResponseHeader", response.getHeader("CastleError"));
        Assert.assertEquals("errorMessage", expectedError, response.getErrorMessage());
    }

    private static Pair<ModelAndView, MockHttpServletResponse> doRequest(
            String loginString,
            Cookie sessionCookie) throws Exception {
        return doRequest(loginString, null /* authzHeader */, sessionCookie, idmClient());
    }

    private static Pair<ModelAndView, MockHttpServletResponse> doRequest(
            String loginString,
            String authzHeader,
            Cookie sessionCookie,
            CasIdmClient idmClient) throws Exception {
        Map<String, String> queryParams = authnRequestParameters(Flow.AUTHZ_CODE);

        MockHttpServletRequest request;
        if (loginString != null) {
            Map<String, String> formParams = new HashMap<String, String>();
            formParams.put("CastleAuthorization", loginString);
            request = TestUtil.createPostRequestWithQueryString(formParams, queryParams);
        } else {
            request = TestUtil.createGetRequest(queryParams);
        }
        if (authzHeader != null) {
            request.addHeader("Authorization", authzHeader);
        }
        if (sessionCookie != null) {
            request.setCookies(sessionCookie);
        }

        MockHttpServletResponse response = new MockHttpServletResponse();
        AuthenticationController controller = authnController(idmClient);
        ModelAndView modelView = controller.authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        return Pair.of(modelView, response);
    }

    private static Pair<ModelAndView, MockHttpServletResponse> doRequestUsingPersonUserCert(
            String certHeader,
            Object certAttribute,
            Cookie cookie) throws Exception {
        Map<String, String> queryParams = authnRequestParameters(Flow.AUTHZ_CODE);
        Map<String, String> formParams = new HashMap<String, String>();
        formParams.put("CastleAuthorization", "TLSClient ");
        MockHttpServletRequest request = TestUtil.createPostRequestWithQueryString(formParams, queryParams);
        if (certHeader != null) {
            request.addHeader("X-SSL-Client-Certificate", certHeader);
        }
        if (certAttribute != null) {
            request.setAttribute("javax.servlet.request.X509Certificate", certAttribute);
        }
        if (cookie != null) {
            request.setCookies(cookie);
        }

        MockHttpServletResponse response = new MockHttpServletResponse();
        ModelAndView modelView = authnController().authenticate(new ExtendedModelMap(), Locale.ENGLISH, request, response, TENANT_NAME);
        return Pair.of(modelView, response);
    }
}