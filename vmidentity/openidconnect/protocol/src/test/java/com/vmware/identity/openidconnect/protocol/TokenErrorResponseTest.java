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
import static com.vmware.identity.openidconnect.protocol.TestContext.STATUS_CODE;
import net.minidev.json.JSONObject;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public class TokenErrorResponseTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testSerialize() throws ParseException {
        TokenErrorResponse tokenErrorResponse = new TokenErrorResponse(new ErrorObject(ERROR_CODE, ERROR_DESCRIPTION, STATUS_CODE));
        HttpResponse httpResponse = tokenErrorResponse.toHttpResponse();
        Assert.assertEquals("jsonObject", jsonObject(), httpResponse.getJsonContent());
    }

    @Test
    public void testParseSuccess() throws ParseException {
        JSONObject jsonObject = jsonObject();
        HttpResponse httpResponse = HttpResponse.createJsonResponse(STATUS_CODE, jsonObject);
        TokenErrorResponse tokenErrorResponse = TokenErrorResponse.parse(httpResponse);
        Assert.assertEquals("status_code", STATUS_CODE, tokenErrorResponse.getErrorObject().getStatusCode());
        Assert.assertEquals("error", ERROR_CODE, tokenErrorResponse.getErrorObject().getErrorCode());
        Assert.assertEquals("error_description", ERROR_DESCRIPTION, tokenErrorResponse.getErrorObject().getDescription());
    }

    @Test
    public void testParseError() {
        JSONObject jsonObject = jsonObject();
        jsonObject.remove("error");
        HttpResponse httpResponse = HttpResponse.createJsonResponse(STATUS_CODE, jsonObject);
        try {
            TokenErrorResponse.parse(httpResponse);
            Assert.fail("expected ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "json is missing error member", e.getErrorObject().getDescription());
        }
    }

    private static JSONObject jsonObject() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("error", ERROR_CODE.getValue());
        jsonObject.put("error_description", ERROR_DESCRIPTION);
        return jsonObject;
    }
}