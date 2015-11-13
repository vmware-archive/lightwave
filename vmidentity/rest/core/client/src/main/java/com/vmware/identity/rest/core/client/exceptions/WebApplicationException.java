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

import com.vmware.identity.rest.core.client.exceptions.client.BadRequestException;
import com.vmware.identity.rest.core.client.exceptions.client.ForbiddenException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;
import com.vmware.identity.rest.core.client.exceptions.client.UnauthorizedException;
import com.vmware.identity.rest.core.client.exceptions.server.InternalServerErrorException;
import com.vmware.identity.rest.core.client.exceptions.server.NotImplementedException;
import com.vmware.identity.rest.core.data.ErrorInfo;

/**
 * The {@code WebApplicationException} exception class represents the various HTTP
 * errors issued by the REST application server. This is the highest level exception
 * and it can be further broken down by the error code:
 * <p>
 * <table align="center" summary="HTTP error codes and their associated exception class">
 *  <tr><td><b><u>HTTP Error Code</b></u></td>  <td><b><u>Exception Class</b></u></td></tr>
 *  <tr><td>{@code 400 Bad Request}</td>        <td>{@link BadRequestException}</td></tr>
 *  <tr><td>{@code 401 Unauthorized}</td>       <td>{@link UnauthorizedException}</td></tr>
 *  <tr><td>{@code 403 Forbidden}</td>          <td>{@link ForbiddenException}</td></tr>
 *  <tr><td>{@code 404 Not Found}</td>          <td>{@link NotFoundException}</td></tr>
 *  <tr><td>{@code 500 Server Error}</td>       <td>{@link InternalServerErrorException}</td></tr>
 *  <tr><td>{@code 501 Not Implemented}</td>    <td>{@link NotImplementedException}</td></tr>
 * </table>
 */
public class WebApplicationException extends HttpException {

    private static final long serialVersionUID = 2366734519034487711L;

    private String serverCause;

    /**
     * Construct a {@code WebApplicationException} from an {@link ErrorInfo} object
     * retrieved from the server
     *
     * @param error the error info retrieved from the server
     */
    protected WebApplicationException(ErrorInfo error) {
        super(error.getDetails());
        this.serverCause = error.getCause();
    }

    /**
     * Retrieve the cause of the exception on the server
     *
     * @return the cause of the exception on the server
     */
    public String getServerCause() {
        return serverCause;
    }
}
