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

import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.oauth2.sdk.token.AccessToken;
import com.nimbusds.oauth2.sdk.token.BearerAccessToken;
import com.nimbusds.oauth2.sdk.token.RefreshToken;
import com.nimbusds.oauth2.sdk.util.JSONObjectUtils;
import com.nimbusds.openid.connect.sdk.OIDCAccessTokenResponse;

/**
 * @author Yehia Zayour
 */
public class TokenSuccessResponse extends OIDCAccessTokenResponse {
    public TokenSuccessResponse(
            IDToken idToken,
            AccessToken accessToken,
            RefreshToken refreshToken) {
        super(accessToken, refreshToken, idToken);
        Validate.notNull(idToken, "idToken");
    }

    @Override
    public IDToken getIDToken() {
        return (IDToken) super.getIDToken();
    }

    public static TokenSuccessResponse parse(HTTPResponse httpResponse) throws ParseException {
        Validate.notNull(httpResponse, "httpResponse");

        httpResponse.ensureStatusCode(HTTPResponse.SC_OK);
        JSONObject jsonObject = httpResponse.getContentAsJSONObject();
        return parse(jsonObject);
    }

    public static TokenSuccessResponse parse(JSONObject jsonObject) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");

        SignedJWT idTokenJwt;
        try {
            idTokenJwt = SignedJWT.parse(JSONObjectUtils.getString(jsonObject, "id_token"));
        } catch (java.text.ParseException e) {
            throw new ParseException("failed to parse id_token", e);
        }
        IDToken idToken = new IDToken(idTokenJwt);

        SignedJWT accessTokenJwt;
        try {
            accessTokenJwt = SignedJWT.parse(JSONObjectUtils.getString(jsonObject, "access_token"));
        } catch (java.text.ParseException e) {
            throw new ParseException("failed to parse access_token", e);
        }

        long expiresIn = JSONObjectUtils.getLong(jsonObject, "expires_in");
        String tokenType = JSONObjectUtils.getString(jsonObject, "token_type");

        AccessToken accessToken;
        if (tokenType.equals(TokenType.BEARER.getName())) {
            accessToken = new BearerAccessToken(accessTokenJwt.serialize(), expiresIn, (Scope) null);
        } else if (tokenType.equals(TokenType.HOK.getName())) {
            accessToken = new HolderOfKeyAccessToken(accessTokenJwt.serialize(), expiresIn);
        } else {
            throw new ParseException("invalid token_type value: " + tokenType);
        }

        RefreshToken refreshToken = RefreshToken.parse(jsonObject);

        return new TokenSuccessResponse(idToken, accessToken, refreshToken);
    }
}
