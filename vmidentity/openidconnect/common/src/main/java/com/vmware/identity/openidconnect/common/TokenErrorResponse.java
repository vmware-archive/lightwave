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

package com.vmware.identity.openidconnect.common;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;

/**
 * @author Yehia Zayour
 */
public class TokenErrorResponse extends com.nimbusds.oauth2.sdk.TokenErrorResponse {
    public TokenErrorResponse(ErrorObject error) {
        super(error);
    }

    public static TokenErrorResponse parse(HTTPResponse httpResponse) throws ParseException {
        Validate.notNull(httpResponse, "httpResponse");

        httpResponse.ensureStatusCodeNotOK();
        JSONObject jsonObject = httpResponse.getContentAsJSONObject();
        return parse(jsonObject);
    }

    public static TokenErrorResponse parse(JSONObject jsonObject) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");
        com.nimbusds.oauth2.sdk.TokenErrorResponse nimbusResponse = com.nimbusds.oauth2.sdk.TokenErrorResponse.parse(jsonObject);
        return new TokenErrorResponse(nimbusResponse.getErrorObject());
    }
}
