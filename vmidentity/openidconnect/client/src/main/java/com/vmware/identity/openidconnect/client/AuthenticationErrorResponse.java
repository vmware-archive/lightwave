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

package com.vmware.identity.openidconnect.client;

import org.apache.commons.lang3.Validate;

/**
 * Authentication Error Response
 *
 * @author Jun Sun
 */
public class AuthenticationErrorResponse extends AuthenticationResponse {

    private final State state;
    private final OIDCServerError oidcServerError;
    private final String errorDescription;

    AuthenticationErrorResponse(State state, String errorCode, String errorDescription) {
        Validate.notNull(errorCode, "errorCode");
        Validate.notNull(errorDescription, "errorDescription");

        this.state = state;
        this.oidcServerError = OIDCServerError.getOIDCServerError(errorCode);
        this.errorDescription = errorDescription;
    }

    /**
     * Get state value.
     *
     * @return                  State value in response.
     */
    public State getState() {
        return this.state;
    }

    /**
     * Get OIDC server error
     *
     * @return                  OIDC server error code
     */
    public OIDCServerError getOIDCServerError() {
        return this.oidcServerError;
    }

    /**
     * Get error description
     *
     * @return                  OIDC server error description
     */
    public String getErrorDescription() {
        return this.errorDescription;
    }
}
