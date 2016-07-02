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

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class AuthenticationErrorResponse extends AuthenticationResponse {
    private static final String HTML_RESPONSE =
            "<html>" +
            "    <head>" +
            "        <script language=\"JavaScript\" type=\"text/javascript\">" +
            "            function load(){ document.getElementById('SamlPostForm').submit(); }" +
            "        </script>" +
            "    </head>" +
            "    <body onload=\"load()\">" +
            "        <form method=\"post\" id=\"SamlPostForm\" action=\"%s\">" +
            "            <input type=\"hidden\" name=\"state\" value=\"%s\" />" +
            "            <input type=\"hidden\" name=\"error\" value=\"%s\" />" +
            "            <input type=\"hidden\" name=\"error_description\" value=\"%s\" />" +
            "            <input type=\"submit\" value=\"Submit\" style=\"position:absolute; left:-9999px; width:1px; height:1px;\" />" +
            "        </form>" +
            "    </body>" +
            "</html>";

    private final ErrorObject errorObject;

    public AuthenticationErrorResponse(
            ResponseMode responseMode,
            URI redirectUri,
            State state,
            boolean isAjaxRequest,
            ErrorObject errorObject) {
        super(responseMode, redirectUri, state, isAjaxRequest);
        Validate.notNull(errorObject, "errorObject");
        this.errorObject = errorObject;
    }

    public ErrorObject getErrorObject() {
        return this.errorObject;
    }

    @Override
    protected Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("state", super.getState().getValue());
        parameters.put("error", this.errorObject.getErrorCode().getValue());
        parameters.put("error_description", this.errorObject.getDescription());
        return parameters;
    }

    @Override
    protected String toFormPostResponse() {
        return String.format(
                HTML_RESPONSE,
                super.getRedirectURI().toString(),
                super.getState().getValue(),
                this.errorObject.getErrorCode().getValue(),
                this.errorObject.getDescription());
    }

    public static AuthenticationErrorResponse parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");
        Map<String, String> parameters = httpRequest.getParameters();

        State state = State.parse(ParameterMapUtils.getString(parameters, "state"));
        ErrorObject errorObject = ErrorObjectMapper.parse(parameters, StatusCode.OK /* we do not know and it doesn't matter */);

        return new AuthenticationErrorResponse(
                ResponseMode.FORM_POST, // we don't really know but it doesn't matter
                httpRequest.getURI(),
                state,
                false /* isAjaxRequest, we don't really know but it doesn't matter */,
                errorObject);
    }
}