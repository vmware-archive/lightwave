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

import static com.vmware.identity.openidconnect.protocol.TestContext.ACCESS_TOKEN;
import static com.vmware.identity.openidconnect.protocol.TestContext.ID_TOKEN;
import net.minidev.json.JSONObject;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public class TokenSuccessResponseTest {
    @BeforeClass
    public static void setup() throws Exception {
        TestContext.initialize();
    }

    @Test
    public void testSerialize() {
        TokenSuccessResponse tokenSuccessResponse = new TokenSuccessResponse(
                ID_TOKEN,
                ACCESS_TOKEN,
                (RefreshToken) null);
        HttpResponse httpResponse = tokenSuccessResponse.toHttpResponse();
        Assert.assertEquals("jsonObject", jsonObject(), httpResponse.getJsonContent());
    }

    @Test
    public void testParseSuccess() throws ParseException {
        JSONObject jsonObject = jsonObject();
        HttpResponse httpResponse = HttpResponse.createJsonResponse(StatusCode.OK, jsonObject);
        TokenSuccessResponse tokenSuccessResponse = TokenSuccessResponse.parse(httpResponse);
        Assert.assertEquals("id_token", ID_TOKEN.serialize(), tokenSuccessResponse.getIDToken().serialize());
        Assert.assertEquals("access_token", ACCESS_TOKEN.serialize(), tokenSuccessResponse.getAccessToken().serialize());
        Assert.assertEquals("token_type", ACCESS_TOKEN.getTokenType().getValue(), tokenSuccessResponse.getAccessToken().getTokenType().getValue());
        Assert.assertEquals("expires_in", ACCESS_TOKEN.getLifetimeSeconds(), tokenSuccessResponse.getAccessToken().getLifetimeSeconds());
    }

    @Test
    public void testParseError() {
        JSONObject jsonObject = jsonObject();
        jsonObject.remove("expires_in");
        HttpResponse httpResponse = HttpResponse.createJsonResponse(StatusCode.OK, jsonObject);
        try {
            TokenSuccessResponse.parse(httpResponse);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("error", "invalid_request", e.getErrorObject().getErrorCode().getValue());
            Assert.assertEquals("error_description", "json is missing expires_in member", e.getErrorObject().getDescription());
        }
    }

    private static JSONObject jsonObject() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("id_token", ID_TOKEN.serialize());
        jsonObject.put("access_token", ACCESS_TOKEN.serialize());
        jsonObject.put("token_type", ACCESS_TOKEN.getTokenType().getValue());
        jsonObject.put("expires_in", ACCESS_TOKEN.getLifetimeSeconds());
        return jsonObject;
    }
}