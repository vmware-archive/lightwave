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

import static com.vmware.identity.openidconnect.protocol.TestContext.CLIENT_ID;
import static com.vmware.identity.openidconnect.protocol.TestContext.NONCE;
import static com.vmware.identity.openidconnect.protocol.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.REQUEST_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.SCOPE;
import static com.vmware.identity.openidconnect.protocol.TestContext.STATE;

import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest.ParseException;

/**
 * @author Yehia Zayour
 */
public class AuthenticationRequestTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testSerialize() {
        AuthenticationRequest authnRequest = new AuthenticationRequest(
                REQUEST_URI,
                ResponseType.authorizationCode(),
                ResponseMode.QUERY,
                CLIENT_ID,
                REDIRECT_URI,
                SCOPE,
                STATE,
                NONCE,
                (ClientAssertion) null,
                (CorrelationID) null);
        HttpRequest httpRequest = authnRequest.toHttpRequest();
        Assert.assertEquals("parameters", parameters(), httpRequest.getParameters());
    }

    @Test
    public void testParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters();
        HttpRequest httpRequest = HttpRequest.createGetRequest(REQUEST_URI, parameters);
        AuthenticationRequest authnRequest = AuthenticationRequest.parse(httpRequest);
        Assert.assertEquals("redirect_uri", authnRequest.getRedirectURI(), REDIRECT_URI);
        Assert.assertEquals("client_id", authnRequest.getClientID(), CLIENT_ID);
        Assert.assertEquals("scope", authnRequest.getScope(), SCOPE);
        Assert.assertEquals("state", authnRequest.getState(), STATE);
        Assert.assertEquals("nonce", authnRequest.getNonce(), NONCE);
    }

    @Test
    public void testParseError() {
        Map<String, String> parameters = parameters();
        parameters.remove("response_type");
        HttpRequest httpRequest = HttpRequest.createGetRequest(REQUEST_URI, parameters);
        try {
            AuthenticationRequest.parse(httpRequest);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "missing response_type parameter", e.getErrorObject().getDescription());
        }
    }

    private static Map<String, String> parameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("response_type", "code");
        parameters.put("response_mode", "query");
        parameters.put("client_id", CLIENT_ID.getValue());
        parameters.put("redirect_uri", REDIRECT_URI.toString());
        parameters.put("scope", SCOPE.toString());
        parameters.put("state", STATE.getValue());
        parameters.put("nonce", NONCE.getValue());
        return parameters;
    }
}