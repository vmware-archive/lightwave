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

import static com.vmware.identity.openidconnect.protocol.TestContext.POST_LOGOUT_REDIRECT_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.STATE;

import java.net.URI;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.SessionID;

/**
 * @author Yehia Zayour
 */
public class LogoutSuccessResponseTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testSerialize() {
        LogoutSuccessResponse logoutSuccessResponse = new LogoutSuccessResponse(
                POST_LOGOUT_REDIRECT_URI,
                STATE,
                (SessionID) null,
                Collections.<URI>emptySet());
        HttpResponse httpResponse = logoutSuccessResponse.toHttpResponse();
        String html = httpResponse.getHtmlContent();
        Assert.assertTrue("post_logout_redirect_uri", html.contains(POST_LOGOUT_REDIRECT_URI.toString()));
        Assert.assertTrue("state", html.contains(STATE.getValue()));
    }

    @Test
    public void testParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters();
        HttpRequest httpRequest = HttpRequest.createGetRequest(POST_LOGOUT_REDIRECT_URI, parameters);
        LogoutSuccessResponse logoutSuccessResponse = LogoutSuccessResponse.parse(httpRequest);
        Assert.assertEquals("state", STATE, logoutSuccessResponse.getState());
    }

    @Test
    public void testParseError() {
        Map<String, String> parameters = parameters();
        parameters.remove("state");
        HttpRequest httpRequest = HttpRequest.createGetRequest(POST_LOGOUT_REDIRECT_URI, parameters);
        try {
            LogoutSuccessResponse.parse(httpRequest);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "missing state parameter", e.getErrorObject().getDescription());
        }
    }

    @Test
    public void testParseErrorInvalidState() {
        Map<String, String> parameters = parameters();
        parameters.put("state", "\"");
        HttpRequest httpRequest = HttpRequest.createGetRequest(POST_LOGOUT_REDIRECT_URI, parameters);
        try {
            LogoutSuccessResponse.parse(httpRequest);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "state parameter must be html friendly", e.getErrorObject().getDescription());
        }
    }

    private static Map<String, String> parameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("state", STATE.getValue());
        return parameters;
    }
}