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

package com.vmware.identity.openidconnect.server;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ErrorObject;

/**
 * @author Yehia Zayour
 */
public class ServerException extends Exception {
    private static final long serialVersionUID = 1L;

    private final ErrorObject errorObject;

    public ServerException(ErrorObject errorObject) {
        Validate.notNull(errorObject, "errorObject");
        this.errorObject = errorObject;
    }

    public ServerException(ErrorObject errorObject, Throwable cause) {
        super(cause);
        Validate.notNull(errorObject, "errorObject");
        Validate.notNull(cause, "cause");
        this.errorObject = errorObject;
    }

    public ErrorObject getErrorObject() {
        return this.errorObject;
    }
}