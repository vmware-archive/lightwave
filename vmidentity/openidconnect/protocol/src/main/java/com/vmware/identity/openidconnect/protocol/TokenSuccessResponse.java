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

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class TokenSuccessResponse extends TokenResponse {
    private final IDToken idToken;
    private final AccessToken accessToken;
    private final RefreshToken refreshToken;

    public TokenSuccessResponse(
            IDToken idToken,
            AccessToken accessToken,
            RefreshToken refreshToken) {
        Validate.notNull(idToken, "idToken");
        Validate.notNull(accessToken, "accessToken");
        // nullable refreshToken
        this.idToken = idToken;
        this.accessToken = accessToken;
        this.refreshToken = refreshToken;
    }

    public IDToken getIDToken() {
        return this.idToken;
    }

    public AccessToken getAccessToken() {
        return this.accessToken;
    }

    public RefreshToken getRefreshToken() {
        return this.refreshToken;
    }

    @Override
    public HttpResponse toHttpResponse() {
        return HttpResponse.createJsonResponse(StatusCode.OK, toJSONObject());
    }

    private JSONObject toJSONObject() {
        JSONObject json = new JSONObject();

        json.put("id_token", this.idToken.serialize());
        json.put("access_token", this.accessToken.serialize());
        json.put("token_type", this.accessToken.getTokenType().getValue());
        json.put("expires_in", this.accessToken.getLifetimeSeconds());
        if (this.refreshToken != null) {
            json.put("refresh_token", this.refreshToken.serialize());
        }

        return json;
    }

    public static TokenSuccessResponse parse(HttpResponse httpResponse) throws ParseException {
        Validate.notNull(httpResponse, "httpResponse");

        if (httpResponse.getStatusCode() != StatusCode.OK) {
            throw new ParseException("expecting status code OK, actual: " + httpResponse.getStatusCode());
        }

        JSONObject jsonObject = httpResponse.getJsonContent();
        if (jsonObject == null) {
            throw new ParseException("expecting json http response");
        }

        IDToken idToken = IDToken.parse(jsonObject);
        AccessToken accessToken = AccessToken.parse(jsonObject);
        RefreshToken refreshToken = jsonObject.containsKey("refresh_token") ? RefreshToken.parse(jsonObject) : null;

        return new TokenSuccessResponse(idToken, accessToken, refreshToken);
    }
}