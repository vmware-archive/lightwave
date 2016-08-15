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

import static com.vmware.identity.openidconnect.protocol.TestContext.ERROR_CODE;
import static com.vmware.identity.openidconnect.protocol.TestContext.ERROR_DESCRIPTION;
import static com.vmware.identity.openidconnect.protocol.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.protocol.TestContext.STATE;
import static com.vmware.identity.openidconnect.protocol.TestContext.STATUS_CODE;

import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ResponseMode;

/**
 * @author Yehia Zayour
 */
public class AuthenticationErrorResponseTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testSerialize() {
        AuthenticationErrorResponse authnErrorResponse = new AuthenticationErrorResponse(
                ResponseMode.QUERY,
                REDIRECT_URI,
                STATE,
                false /* isAjaxRequest */,
                new ErrorObject(ERROR_CODE, ERROR_DESCRIPTION, STATUS_CODE));
        HttpResponse httpResponse = authnErrorResponse.toHttpResponse();
        String redirectTargetQuery = httpResponse.getRedirectTarget().getQuery();
        Assert.assertEquals("parameters", parameters(), TestContext.parseParameters(redirectTargetQuery));
    }

    @Test
    public void testParseSuccess() throws ParseException {
        Map<String, String> parameters = parameters();
        HttpRequest httpRequest = HttpRequest.createGetRequest(REDIRECT_URI, parameters);
        AuthenticationErrorResponse authnErrorResponse = AuthenticationErrorResponse.parse(httpRequest);
        Assert.assertEquals("error", ERROR_CODE, authnErrorResponse.getErrorObject().getErrorCode());
        Assert.assertEquals("error_description", ERROR_DESCRIPTION, authnErrorResponse.getErrorObject().getDescription());
    }

    @Test
    public void testParseError() {
        Map<String, String> parameters = parameters();
        parameters.remove("error");
        HttpRequest httpRequest = HttpRequest.createGetRequest(REDIRECT_URI, parameters);
        try {
            AuthenticationErrorResponse.parse(httpRequest);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "missing error parameter", e.getErrorObject().getDescription());
        }
    }

    @Test
    public void testParseErrorInvalidState() {
        Map<String, String> parameters = parameters();
        parameters.put("state", "<>");
        HttpRequest httpRequest = HttpRequest.createGetRequest(REDIRECT_URI, parameters);
        try {
            AuthenticationErrorResponse.parse(httpRequest);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "state parameter must be html friendly", e.getErrorObject().getDescription());
        }
    }

    private static Map<String, String> parameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("state", STATE.getValue());
        parameters.put("error", ERROR_CODE.getValue());
        parameters.put("error_description", ERROR_DESCRIPTION);
        return parameters;
    }
}