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
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public abstract class AuthenticationResponse extends ProtocolResponse {
    private final ResponseMode responseMode;
    private final URI redirectUri;
    private final State state;
    private final boolean isAjaxRequest;

    public AuthenticationResponse(
            ResponseMode responseMode,
            URI redirectUri,
            State state,
            boolean isAjaxRequest) {
        Validate.notNull(responseMode, "responseMode");
        Validate.notNull(redirectUri, "redirectUri");
        Validate.notNull(state, "state");

        this.responseMode = responseMode;
        this.redirectUri = redirectUri;
        this.state = state;
        this.isAjaxRequest = isAjaxRequest;
    }

    public URI getRedirectURI() {
        return this.redirectUri;
    }

    public State getState() {
        return this.state;
    }

    protected abstract Map<String, String> toParameters();

    protected abstract String toFormPostResponse();

    @Override
    public HttpResponse toHttpResponse() {
        HttpResponse httpResponse;

        if (this.responseMode == ResponseMode.FORM_POST) {
            String content = toFormPostResponse();
            httpResponse = HttpResponse.createHtmlResponse(StatusCode.OK, content);
        } else {
            Map<String, String> parameters = toParameters();
            URI redirectTarget = (this.responseMode == ResponseMode.QUERY) ?
                    URIUtils.appendQueryParameters(this.redirectUri, parameters) :
                    URIUtils.appendFragmentParameters(this.redirectUri, parameters);
            httpResponse = this.isAjaxRequest ?
                    HttpResponse.createHtmlResponse(StatusCode.OK, redirectTarget.toString()) :
                    HttpResponse.createRedirectResponse(redirectTarget);
        }

        return httpResponse;
    }

    public static AuthenticationResponse parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");
        return httpRequest.getParameters().containsKey("error") ?
                AuthenticationErrorResponse.parse(httpRequest) :
                AuthenticationSuccessResponse.parse(httpRequest);
    }
}