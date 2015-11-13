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

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import javax.servlet.http.Cookie;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.ui.ExtendedModelMap;
import org.springframework.web.servlet.ModelAndView;

import com.nimbusds.jose.util.Base64;
import com.nimbusds.oauth2.sdk.Scope;

/**
 * @author Yehia Zayour
 */
public class LoginTest {
    private static final Flow FLOW = Flow.AUTHZ_CODE;
    private static final Scope SCOPE = new Scope("openid");

    @BeforeClass
    public static void initialize() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testPasswordLogin() throws Exception {
        String loginString = TestContext.passwordLoginString();
        assertSuccessResponse(loginString);
    }

    @Test
    public void testPasswordLoginIncorrectCredentials() throws Exception {
        String loginString = TestContext.passwordLoginString(TestContext.USERNAME + "_non_matching", TestContext.PASSWORD + "_non_matching");
        assertErrorResponse(loginString, 401, "Unauthorized: Incorrect username/password", null);
    }

    @Test
    public void testPasswordLoginInvalidLoginString() throws Exception {
        String loginString = TestContext.passwordLoginString() + " extra";
        assertErrorResponse(loginString, 400, "invalid_request: malformed password login string", null);
    }

    @Test
    public void testPasswordLoginInvalidUsernamePassword() throws Exception {
        String unp = "usernamepassword"; // should be username:password
        String unp64 = Base64.encode(unp).toString();
        String loginString = "Basic " + unp64;
        assertErrorResponse(loginString, 400, "invalid_request: malformed username:password in login string", null);
    }

    @Test
    public void testGssLoginOneLegged() throws Exception {
        String loginString = TestContext.gssLoginString();
        assertSuccessResponse(loginString);
    }

    @Test
    public void testGssLoginTwoLegged() throws Exception {
        String contextId = TestContext.GSS_CONTEXT_ID;
        String loginString = TestContext.gssLoginString(contextId);
        IdmClient idmClient = TestContext.idmClientBuilder().gssServerLeg(new byte[1]).build();
        assertErrorResponse(loginString, 401, "Unauthorized: continue Negotiate required", "Negotiate " + contextId, idmClient);
    }

    @Test
    public void testGssLoginInvalidTicket() throws Exception {
        String contextId = TestContext.GSS_CONTEXT_ID + "non_matching";
        String loginString = TestContext.gssLoginString(contextId);
        assertErrorResponse(loginString, 401, "Unauthorized: invalid gss token", null);
    }

    @Test
    public void testGssLoginInvalidLoginString() throws Exception {
        String loginString = TestContext.gssLoginString() + " extra";
        assertErrorResponse(loginString, 400, "invalid_request: malformed gss login string", null);
    }

    @Test
    public void testInvalidLoginMethod() throws Exception {
        String loginString = "invalid_method";
        assertErrorResponse(loginString, 400, "invalid_request: invalid login method", null);
    }

    @Test
    public void testSessionLogin() throws Exception {
        Cookie sessionCookie = new Cookie(TestContext.SESSION_COOKIE_NAME, TestContext.SESSION_ID);
        Object[] result = doRequest(null /* loginString */, sessionCookie);
        ModelAndView modelView = (ModelAndView) result[0];
        MockHttpServletResponse response = (MockHttpServletResponse) result[1];
        Assert.assertNull("modelView", modelView);
        Assert.assertNull("sessionCookie", response.getCookie(TestContext.SESSION_COOKIE_NAME));
        TestContext.validateAuthnSuccessResponse(FLOW, response, SCOPE);
    }

    @Test
    public void testMissingLogin() throws Exception {
        Object[] result = doRequest(null /* loginString */, null /* sessionCookie */);
        ModelAndView modelView = (ModelAndView) result[0];
        MockHttpServletResponse response = (MockHttpServletResponse) result[1];
        Assert.assertNotNull("modelView", modelView); // logon form should be served
        Assert.assertNull("sessionCookie", response.getCookie(TestContext.SESSION_COOKIE_NAME));
        Assert.assertEquals("status", 200, response.getStatus());
    }

    private static void assertSuccessResponse(String loginString) throws Exception {
        Object[] result = doRequest(loginString, null /* sessionCookie */);
        ModelAndView modelView = (ModelAndView) result[0];
        MockHttpServletResponse response = (MockHttpServletResponse) result[1];
        Assert.assertNull("modelView", modelView);
        Assert.assertNotNull("sessionCookie", response.getCookie(TestContext.SESSION_COOKIE_NAME));
        TestContext.validateAuthnSuccessResponse(FLOW, response, SCOPE, TestContext.STATE, TestContext.NONCE, false, true);
    }

    private static void assertErrorResponse(
            String loginString,
            int expectedStatusCode,
            String expectedError,
            String expectedAuthzResponseHeaderPrefix) throws Exception {
        assertErrorResponse(loginString, expectedStatusCode, expectedError, expectedAuthzResponseHeaderPrefix, TestContext.idmClient());
    }

    private static void assertErrorResponse(
            String loginString,
            int expectedStatusCode,
            String expectedError,
            String expectedAuthzResponseHeaderPrefix,
            IdmClient idmClient) throws Exception {
        Object[] result = doRequest(loginString, null /* sessionCookie */, idmClient);
        ModelAndView modelView = (ModelAndView) result[0];
        MockHttpServletResponse response = (MockHttpServletResponse) result[1];
        Assert.assertNull("modelView", modelView);
        Assert.assertNull("sessionCookie", response.getCookie(TestContext.SESSION_COOKIE_NAME));
        Assert.assertEquals("status", expectedStatusCode, response.getStatus());
        Object errorResponseHeader = response.getHeader("CastleError");
        Assert.assertNotNull("errorResponseHeader", errorResponseHeader);
        Assert.assertEquals("errorMessage", expectedError, response.getErrorMessage());

        if (expectedAuthzResponseHeaderPrefix != null) {
            Object authzResponseHeader = response.getHeader("CastleAuthorization");
            Assert.assertNotNull("authzResponseHeader", authzResponseHeader);
            Assert.assertTrue(
                    "expectedAuthzResponseHeaderPrefix",
                    authzResponseHeader.toString().startsWith(expectedAuthzResponseHeaderPrefix));
        }
    }

    private static Object[] doRequest(
            String loginString,
            Cookie sessionCookie) throws Exception {
        return doRequest(loginString, sessionCookie, TestContext.idmClient());
    }

    private static Object[] doRequest(
            String loginString,
            Cookie sessionCookie,
            IdmClient idmClient) throws Exception {
        Map<String, String> queryParams = TestContext.authnRequestParameters(FLOW);

        MockHttpServletRequest request;
        if (loginString != null) {
            Map<String, String> formParams = new HashMap<String, String>();
            formParams.put("CastleAuthorization", loginString);
            request = TestUtil.createPostRequestWithQueryString(formParams, queryParams);
        } else {
            request = TestUtil.createGetRequest(queryParams);
            if (sessionCookie != null) {
                request.setCookies(sessionCookie);
            }
        }

        MockHttpServletResponse response = new MockHttpServletResponse();
        AuthorizationController authzEndpoint = TestContext.authzController(idmClient);
        ModelAndView modelView = authzEndpoint.authorize(new ExtendedModelMap(), Locale.ENGLISH, request, response);
        return new Object[] { modelView, response };
    }
}
