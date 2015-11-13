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
import java.util.Set;

import javax.mail.internet.ContentType;
import javax.mail.internet.ParseException;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.Response;
import com.nimbusds.oauth2.sdk.SerializeException;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.oauth2.sdk.id.State;

/**
 * @author Yehia Zayour
 */
public class LogoutSuccessResponse implements Response {
    private static final String HTML_RESPONSE =
            "<html>" +
            "    <head>" +
            "        <script type=\"text/javascript\">" +
            "            var postLogoutRedirectUriWithState = \"%s\";" +
            "            if (postLogoutRedirectUriWithState != \"\") {" +
            "                window.onload = function() {" +
            "                    document.location = postLogoutRedirectUriWithState;" +
            "                }" +
            "            }" +
            "        </script>" +
            "    </head>" +
            "    <body>" +
            "        <!-- logoutUriImageLinks --> %s <!-- logoutUriImageLinks -->" +
            "    </body>" +
            "</html>";

    private final URI postLogoutRedirectUri;
    private final State state;
    private final SessionID sessionId;
    private final Set<URI> logoutUris;

    public LogoutSuccessResponse(
            URI postLogoutRedirectUri,
            State state,
            SessionID sessionId,
            Set<URI> logoutUris){
        Validate.notNull(logoutUris); // pass in empty set instead
        if (!logoutUris.isEmpty() && sessionId == null) {
            throw new IllegalArgumentException("sessionId should not be null when logoutUris is non-empty");
        }

        this.postLogoutRedirectUri = postLogoutRedirectUri;
        this.state = state;
        this.sessionId = sessionId;
        this.logoutUris = logoutUris;
    }

    @Override
    public boolean indicatesSuccess() {
        return true;
    }

    @Override
    public HTTPResponse toHTTPResponse() throws SerializeException {
        HTTPResponse httpResponse = new HTTPResponse(HTTPResponse.SC_OK);

        try {
            httpResponse.setContentType(new ContentType("text/html;charset=UTF-8"));
        } catch (ParseException e) {
            throw new SerializeException("could not set response type header", e);
        }

        httpResponse.setCacheControl("no-cache, no-store");
        httpResponse.setPragma("no-cache");

        String postLogoutRedirectUriWithState = "";
        if (this.postLogoutRedirectUri != null) {
            postLogoutRedirectUriWithState = (this.state == null) ?
                    this.postLogoutRedirectUri.toString() :
                    CommonUtils.appendQueryParameter(this.postLogoutRedirectUri, "state", this.state.getValue());
        }

        StringBuilder logoutUriImageLinks = new StringBuilder();
        for (URI logoutUri : this.logoutUris) {
            String logoutUriWithSid = CommonUtils.appendQueryParameter(logoutUri, "sid", this.sessionId.getValue());
            logoutUriImageLinks.append(String.format("<img src=\"%s\">", logoutUriWithSid));
        }

        httpResponse.setContent(String.format(HTML_RESPONSE, postLogoutRedirectUriWithState, logoutUriImageLinks.toString()));

        return httpResponse;
    }
}
