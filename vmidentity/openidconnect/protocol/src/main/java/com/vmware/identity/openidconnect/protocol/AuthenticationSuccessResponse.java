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

import java.net.URI;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.State;

/**
 * @author Yehia Zayour
 */
public final class AuthenticationSuccessResponse extends AuthenticationResponse {
    private static final String HTML_RESPONSE =
            "<html>" +
            "    <head>" +
            "        <script language=\"JavaScript\" type=\"text/javascript\">" +
            "            function load(){ document.getElementById('SamlPostForm').submit(); }" +
            "        </script>" +
            "    </head>" +
            "    <body onload=\"load()\">" +
            "        %s " +
            "    </body>" +
            "</html>";

    private static final String FORM_AUTHZ_CODE =
            "<form method=\"post\" id=\"SamlPostForm\" action=\"%s\">" +
            "    <input type=\"hidden\" name=\"state\" value=\"%s\" />" +
            "    <input type=\"hidden\" name=\"code\" value=\"%s\" />" +
            "    <input type=\"submit\" value=\"Submit\" style=\"position:absolute; left:-9999px; width:1px; height:1px;\" />" +
            "</form>";

    private static final String FORM_ID_TOKEN_ONLY =
            "<form method=\"post\" id=\"SamlPostForm\" action=\"%s\">" +
            "    <input type=\"hidden\" name=\"state\" value=\"%s\" />" +
            "    <input type=\"hidden\" name=\"id_token\" value=\"%s\" />" +
            "    <input type=\"submit\" value=\"Submit\" style=\"position:absolute; left:-9999px; width:1px; height:1px;\" />" +
            "</form>";

    private static final String FORM_ID_TOKEN_ACCESS_TOKEN =
            "<form method=\"post\" id=\"SamlPostForm\" action=\"%s\">" +
            "    <input type=\"hidden\" name=\"state\" value=\"%s\" />" +
            "    <input type=\"hidden\" name=\"id_token\" value=\"%s\" />" +
            "    <input type=\"hidden\" name=\"access_token\" value=\"%s\" />" +
            "    <input type=\"hidden\" name=\"token_type\" value=\"%s\" />" +
            "    <input type=\"hidden\" name=\"expires_in\" value=\"%s\" />" +
            "    <input type=\"submit\" value=\"Submit\" style=\"position:absolute; left:-9999px; width:1px; height:1px;\" />" +
            "</form>";

    private final AuthorizationCode code;
    private final IDToken idToken;
    private final AccessToken accessToken;

    public AuthenticationSuccessResponse(
            ResponseMode responseMode,
            URI redirectUri,
            State state,
            boolean isAjaxRequest,
            AuthorizationCode code,
            IDToken idToken,
            AccessToken accessToken) {
        super(responseMode, redirectUri, state, isAjaxRequest);
        // nullable code
        // nullable idToken
        // nullable accessToken
        this.code = code;
        this.idToken = idToken;
        this.accessToken = accessToken;
    }

    public AuthorizationCode getAuthorizationCode() {
        return this.code;
    }

    public IDToken getIDToken() {
        return this.idToken;
    }

    public AccessToken getAccessToken() {
        return this.accessToken;
    }

    @Override
    protected Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("state", super.getState().getValue());
        if (this.code != null) {
            parameters.put("code", this.code.getValue());
        }
        if (this.idToken != null) {
            parameters.put("id_token", this.idToken.serialize());
        }
        if (this.accessToken != null) {
            parameters.put("access_token", this.accessToken.serialize());
            parameters.put("token_type", this.accessToken.getTokenType().getValue());
            parameters.put("expires_in", Long.toString(this.accessToken.getLifetimeSeconds()));
        }
        return parameters;
    }

    @Override
    protected String toFormPostResponse() {
        String form;
        if (this.code != null) {
            form = String.format(
                    FORM_AUTHZ_CODE,
                    super.getRedirectURI().toString(),
                    super.getState().getValue(),
                    this.code.getValue());
        } else if (this.idToken != null && this.accessToken == null) {
            form = String.format(
                    FORM_ID_TOKEN_ONLY,
                    super.getRedirectURI().toString(),
                    super.getState().getValue(),
                    this.idToken.serialize());
        } else if (this.idToken != null && this.accessToken != null) {
            form = String.format(
                    FORM_ID_TOKEN_ACCESS_TOKEN,
                    super.getRedirectURI().toString(),
                    super.getState().getValue(),
                    this.idToken.serialize(),
                    this.accessToken.serialize(),
                    this.accessToken.getTokenType().getValue(),
                    this.accessToken.getLifetimeSeconds());
        } else {
            throw new IllegalArgumentException("unexpected authn success response");
        }
        return String.format(HTML_RESPONSE, form);
    }

    public static AuthenticationSuccessResponse parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");
        Map<String, String> parameters = httpRequest.getParameters();

        State state = State.parse(ParameterMapUtils.getString(parameters, "state"));

        AuthorizationCode code = null;
        if (parameters.containsKey("code")) {
            code = new AuthorizationCode(ParameterMapUtils.getString(parameters, "code"));
        }

        IDToken idToken = null;
        if (parameters.containsKey("id_token")) {
            idToken = IDToken.parse(parameters);
        }

        AccessToken accessToken = null;
        if (parameters.containsKey("access_token")) {
            accessToken = AccessToken.parse(parameters);
        }

        return new AuthenticationSuccessResponse(
                ResponseMode.FORM_POST, // we don't really know but it doesn't matter
                httpRequest.getURI(),
                state,
                false /* isAjaxRequest, we don't really know but it doesn't matter */,
                code,
                idToken,
                accessToken);
    }
}