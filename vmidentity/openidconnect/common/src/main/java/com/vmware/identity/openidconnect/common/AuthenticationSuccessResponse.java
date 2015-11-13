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

import java.net.URI;

import javax.mail.internet.ContentType;
import javax.mail.internet.ParseException;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.AuthorizationCode;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.oauth2.sdk.id.State;
import com.nimbusds.oauth2.sdk.token.AccessToken;
import com.nimbusds.openid.connect.sdk.ResponseMode;

/**
 * @author Yehia Zayour
 */
public class AuthenticationSuccessResponse extends com.nimbusds.openid.connect.sdk.AuthenticationSuccessResponse {
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

    private final ResponseMode responseMode;
    private final boolean isAjaxRequest;

    public AuthenticationSuccessResponse(
            URI redirectUri,
            AuthorizationCode code,
            IDToken idToken,
            AccessToken accessToken,
            State state,
            ResponseMode responseMode,
            boolean isAjaxRequest) {
        super(redirectUri, code, idToken, accessToken, state);

        Validate.notNull(responseMode, "responseMode");
        this.responseMode = responseMode;
        this.isAjaxRequest = isAjaxRequest;
    }

    @Override
    public HTTPResponse toHTTPResponse() throws SerializeException {
        HTTPResponse httpResponse;

        if (this.responseMode.equals(ResponseMode.FORM_POST)) {
            httpResponse = formPostResponse();
        } else {
            // query or fragment response mode
            httpResponse = super.toHTTPResponse();
            if (this.isAjaxRequest) {
                httpResponse = ajaxRedirectResponse(httpResponse.getLocation());
            }
        }

        return httpResponse;
    }

    private HTTPResponse formPostResponse() throws SerializeException {
        HTTPResponse httpResponse = new HTTPResponse(HTTPResponse.SC_OK);

        try {
            httpResponse.setContentType(new ContentType("text/html;charset=UTF-8"));
        } catch (ParseException e) {
            throw new SerializeException("could not set response type header", e);
        }

        httpResponse.setCacheControl("no-cache, no-store");
        httpResponse.setPragma("no-cache");

        String form;
        if (super.getAuthorizationCode() != null) {
            form = String.format(
                    FORM_AUTHZ_CODE,
                    super.getRedirectionURI().toString(),
                    super.getState().getValue(),
                    super.getAuthorizationCode().getValue());
        } else if (super.getIDToken() != null && super.getAccessToken() == null) {
            form = String.format(
                    FORM_ID_TOKEN_ONLY,
                    super.getRedirectionURI().toString(),
                    super.getState().getValue(),
                    super.getIDToken().serialize());
        } else if (super.getIDToken() != null && super.getAccessToken() != null) {
            form = String.format(
                    FORM_ID_TOKEN_ACCESS_TOKEN,
                    super.getRedirectionURI().toString(),
                    super.getState().getValue(),
                    super.getIDToken().serialize(),
                    super.getAccessToken().toString(),
                    super.getAccessToken().getType().getValue(),
                    super.getAccessToken().getLifetime());
        } else {
            throw new IllegalArgumentException("unexpected authn success response");
        }

        httpResponse.setContent(String.format(HTML_RESPONSE, form));

        return httpResponse;
    }

    private static HTTPResponse ajaxRedirectResponse(URI redirectLocation) throws SerializeException {
        HTTPResponse httpResponse = new HTTPResponse(HTTPResponse.SC_OK);

        try {
            httpResponse.setContentType(new ContentType("text/html;charset=UTF-8"));
        } catch (ParseException e) {
            throw new SerializeException("could not set response type header", e);
        }

        httpResponse.setCacheControl("no-cache, no-store");
        httpResponse.setPragma("no-cache");
        httpResponse.setContent(redirectLocation.toString());

        return httpResponse;
    }
}
