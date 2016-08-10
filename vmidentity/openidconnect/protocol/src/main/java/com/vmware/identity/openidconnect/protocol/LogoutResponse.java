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

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.State;

/**
 * @author Yehia Zayour
 */
public abstract class LogoutResponse extends ProtocolResponse {
    private final URI postLogoutRedirectUri;
    private final State state;

    public LogoutResponse(URI postLogoutRedirectUri, State state) {
        Validate.notNull(postLogoutRedirectUri, "postLogoutRedirectUri");
        Validate.notNull(state, "state");
        this.postLogoutRedirectUri = postLogoutRedirectUri;
        this.state = state;
    }

    public URI getPostLogoutRedirectURI() {
        return this.postLogoutRedirectUri;
    }

    public State getState() {
        return this.state;
    }

    public static LogoutResponse parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");
        return httpRequest.getParameters().containsKey("error") ?
                LogoutErrorResponse.parse(httpRequest) :
                LogoutSuccessResponse.parse(httpRequest);
    }
}