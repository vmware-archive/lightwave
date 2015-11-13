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
package com.vmware.identity.rest.core.client.exceptions;

import org.apache.http.HttpException;

/**
 * The {@code GeneralRequestException} exception class represents a generic set
 * of HTTP exceptions, similar to those of the {@link WebApplicationException}.
 * However, this class is used when the exception did not originate from the REST
 * web application (e.g. a 404 error from accessing a server not running the REST
 * web application).
 */
public class GeneralRequestException extends HttpException {

    private static final long serialVersionUID = -1772702113396511240L;

    private final int statusCode;
    private final String serverResponse;

    /**
     * Construct a {@code GeneralRequestException} with a status code, an error message, the
     * full response from the web server, and the cause.
     *
     * @param statusCode the status code of the exception.
     * @param error the exception detail message.
     * @param serverResponse the full contents of the response from the web server.
     * @param e the {@code Throwable} that caused this exception, or {@code null}
     *  if the cause is unavailable, unknown, or not a {@code Throwable}.
     */
    public GeneralRequestException(int statusCode, String error, String serverResponse, Throwable e) {
        super(error, e);
        this.statusCode = statusCode;
        this.serverResponse = serverResponse;
    }

    /**
     * Get the status code associated with this exception.
     *
     * @return the status code.
     */
    public int getStatusCode() {
        return statusCode;
    }

    /**
     * Get the contents of the server's response.
     *
     * @return the contents of the server's response.
     */
    public String getServerResponse() {
        return serverResponse;
    }

}
