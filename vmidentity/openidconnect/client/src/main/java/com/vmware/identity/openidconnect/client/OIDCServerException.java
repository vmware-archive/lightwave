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

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.State;

/**
 * OIDC server side exception.
 *
 * @author Jun Sun
 */
public final class OIDCServerException extends Exception {

    private static final long serialVersionUID = 2L;

    private final ErrorObject errorObject;
    private final State state;

    OIDCServerException(ErrorObject errorObject) {
        super(String.format(
                "Server error response. Error code: %s; Error description: %s.",
                errorObject.getErrorCode().getValue(),
                errorObject.getDescription()));

        this.errorObject = errorObject;
        this.state = null;
    }

    OIDCServerException(ErrorObject errorObject, State state) {
        super(String.format(
                "Server error response. Error code: %s; Error description: %s.",
                errorObject.getErrorCode().getValue(),
                errorObject.getDescription()));

        this.errorObject = errorObject;
        this.state = state;
    }

    /**
     * Get error object
     *
     * @return                          error object
     */
    public ErrorObject getErrorObject() {
        return this.errorObject;
    }

    public State getState() {
        return this.state;
    }
}
