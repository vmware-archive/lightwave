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
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.StatusCode;

/**
 * @author Yehia Zayour
 */
public final class LogoutErrorResponse extends LogoutResponse {
    private final ErrorObject errorObject;

    public LogoutErrorResponse(
            URI postLogoutRedirectUri,
            State state,
            ErrorObject errorObject) {
        super(postLogoutRedirectUri, state);

        Validate.notNull(errorObject, "errorObject");
        this.errorObject = errorObject;
    }

    public ErrorObject getErrorObject() {
        return this.errorObject;
    }

    @Override
    public HttpResponse toHttpResponse() {
        URI redirectTarget = URIUtils.appendQueryParameters(super.getPostLogoutRedirectURI(), toParameters());
        return HttpResponse.createRedirectResponse(redirectTarget);
    }

    private Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("state", super.getState().getValue());
        parameters.put("error", this.errorObject.getErrorCode().getValue());
        parameters.put("error_description", this.errorObject.getDescription());
        return parameters;
    }

    public static LogoutErrorResponse parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");
        Map<String, String> parameters = httpRequest.getParameters();

        State state = State.parse(ParameterMapUtils.getString(parameters, "state"));
        ErrorObject errorObject = ErrorObjectMapper.parse(parameters, StatusCode.OK);

        return new LogoutErrorResponse(httpRequest.getURI(), state, errorObject);
    }
}