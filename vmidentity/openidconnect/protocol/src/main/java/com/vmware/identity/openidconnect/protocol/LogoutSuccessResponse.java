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
import java.util.Collections;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class LogoutSuccessResponse extends LogoutResponse {
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
            "        <!-- logoutUriLinks --> %s <!-- logoutUriLinks -->" +
            "    </body>" +
            "</html>";

    private final SessionID sessionId;
    private final Set<URI> logoutUris;

    public LogoutSuccessResponse(
            URI postLogoutRedirectUri,
            State state,
            SessionID sessionId,
            Set<URI> logoutUris){
        super(postLogoutRedirectUri, state);

        Validate.notNull(logoutUris); // pass in empty set instead
        if (!logoutUris.isEmpty() && sessionId == null) {
            throw new IllegalArgumentException("sessionId should not be null when logoutUris is non-empty");
        }

        this.sessionId = sessionId;
        this.logoutUris = logoutUris;
    }

    @Override
    public HttpResponse toHttpResponse() {
        URI postLogoutRedirectUriWithState = URIUtils.appendQueryParameter(super.getPostLogoutRedirectURI(), "state", super.getState().getValue());

        StringBuilder logoutUriLinks = new StringBuilder();
        for (URI logoutUri : this.logoutUris) {
            URI logoutUriWithSid = URIUtils.appendQueryParameter(logoutUri, "sid", this.sessionId.getValue());
            logoutUriLinks.append(String.format("<iframe src=\"%s\">", logoutUriWithSid.toString()));
        }

        String content = String.format(HTML_RESPONSE, postLogoutRedirectUriWithState, logoutUriLinks.toString());
        return HttpResponse.createHtmlResponse(StatusCode.OK, content);
    }

    public static LogoutSuccessResponse parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");
        Map<String, String> parameters = httpRequest.getParameters();

        State state = State.parse(ParameterMapUtils.getString(parameters, "state"));

        return new LogoutSuccessResponse(httpRequest.getURI(), state, (SessionID) null, Collections.<URI>emptySet());
    }
}