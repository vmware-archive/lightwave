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
package com.vmware.identity.rest.core.server.exception.server;

import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import com.vmware.identity.rest.core.data.ErrorInfo;

/**
 * Base class for all exceptions responding with HTTP/1.1 501 Not Implemented
 */
public class NotImplementedError extends WebApplicationException {

    private static final long serialVersionUID = 6594365090680876188L;

    private static final String DEFAULT_ERROR = "not_implemented";

    public NotImplementedError(String message) {
        super(message, Response.status(Response.Status.NOT_IMPLEMENTED)
                .entity(new ErrorInfo(DEFAULT_ERROR, message))
                .type(MediaType.APPLICATION_JSON)
                .build());
    }

}
