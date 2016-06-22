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

import static com.vmware.identity.openidconnect.protocol.TestContext.ID_TOKEN;
import static com.vmware.identity.openidconnect.protocol.TestContext.POST_LOGOUT_REDIRECT_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.REQUEST_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.STATE;

import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.protocol.LogoutRequest.ParseException;

/**
 * @author Yehia Zayour
 */
public class LogoutRequestTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testSerialize() {
        LogoutRequest logoutRequest = new LogoutRequest(
                REQUEST_URI,
                ID_TOKEN,
                POST_LOGOUT_REDIRECT_URI,
                STATE,
                (ClientAssertion) null,
                (CorrelationID) null);
        HttpRequest httpRequest = logoutRequest.toHttpRequest();
        Assert.assertEquals("parameters", parameters(), httpRequest.getParameters());
    }

    @Test
    public void testParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters();
        HttpRequest httpRequest = HttpRequest.createGetRequest(REQUEST_URI, parameters);
        LogoutRequest logoutRequest = LogoutRequest.parse(httpRequest);
        Assert.assertEquals("id_token_hint", ID_TOKEN.serialize(), logoutRequest.getIDTokenHint().serialize());
        Assert.assertEquals("post_logout_redirect_uri", POST_LOGOUT_REDIRECT_URI, logoutRequest.getPostLogoutRedirectURI());
        Assert.assertEquals("state", STATE, logoutRequest.getState());
    }

    @Test
    public void testParseError() {
        Map<String, String> parameters = parameters();
        parameters.remove("id_token_hint");
        HttpRequest httpRequest = HttpRequest.createGetRequest(REQUEST_URI, parameters);
        try {
            LogoutRequest.parse(httpRequest);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "missing id_token_hint parameter", e.getErrorObject().getDescription());
        }
    }

    private static Map<String, String> parameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("id_token_hint", ID_TOKEN.serialize());
        parameters.put("post_logout_redirect_uri", POST_LOGOUT_REDIRECT_URI.toString());
        parameters.put("state", STATE.getValue());
        return parameters;
    }
}