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
package com.vmware.identity.rest.core.client.exceptions.client;

import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.data.ErrorInfo;

/**
 * The {@code UnauthorizedException} exception class represents the content of
 * a 401 (Unauthorized) error issued by the REST web application.
 */
public class UnauthorizedException extends WebApplicationException {

    private static final long serialVersionUID = 9126305553484711698L;

    /**
     * Construct an {@code UnauthorizedException} from an {@link ErrorInfo} object
     * retrieved from the server.
     *
     * @param error the error info retrieved from the server.
     */
    public UnauthorizedException(ErrorInfo error) {
        super(error);
    }

}
