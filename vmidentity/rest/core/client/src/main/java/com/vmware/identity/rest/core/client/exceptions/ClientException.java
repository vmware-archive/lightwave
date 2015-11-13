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

/**
 * The {@code ClientException} exception class represents any exception that
 * occurred on the side of the client library. These exceptions are distinct
 * from those that may occur as the result of a 4xx exception returned from
 * the REST server.
 */
public class ClientException extends Exception {

    private static final long serialVersionUID = 5558764351676089129L;

    /**
     * @see Exception#Exception(String)
     */
    public ClientException(String message) {
        super(message);
    }

    /**
     * @see Exception#Exception(String, Throwable)
     */
    public ClientException(String message, Throwable t) {
        super(message, t);
    }

}
