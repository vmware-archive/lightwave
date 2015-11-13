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


/**
 * REST Admin server side exception.
 *
 * @author Jun Sun
 */
public class AdminServerException extends Exception {

    private static final long serialVersionUID = 1L;

    private final int httpStatusCode;
    private final String cause;

    AdminServerException(int httpStatusCode, String error, String details, String cause) {
        super(String.format(
                "Admin server error response. Error: %s; Details: %s; Cause: %s.",
                error,
                details,
                cause));

        this.httpStatusCode = httpStatusCode;
        this.cause = cause;
    }

    /**
     * Get Http status code
     *
     * @return                          REST Admin server error object
     */
    public int getHttpStatusCode() {
        return this.httpStatusCode;
    }


    /**
     * Get cause of the exception
     *
     * @return                          Cause of the exception
     */
    public String getCauseMessage() {
        return this.cause;
    }
}
