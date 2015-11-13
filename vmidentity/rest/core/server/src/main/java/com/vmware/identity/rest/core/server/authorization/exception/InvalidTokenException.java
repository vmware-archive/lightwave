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
package com.vmware.identity.rest.core.server.authorization.exception;

import com.vmware.identity.rest.core.server.exception.client.NotAuthorizedException;

/**
 * Exception class for invalid token exceptions.
 *
 * <p>Note that if there is an exception that is causing this to be thrown
 * that it should be logged separately from this exception. This exception
 * is intentionally opaque.
 */
public class InvalidTokenException extends NotAuthorizedException {

    private static final long serialVersionUID = 6704640833856606543L;

    private static final String DEFAULT_ERROR = "invalid_token";

    public InvalidTokenException(String message) {
        super(DEFAULT_ERROR, message);
    }

}
