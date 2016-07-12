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

package com.vmware.identity.openidconnect.common;

/**
 * @author Yehia Zayour
 */
public final class ParseException extends Exception {
    private static final long serialVersionUID = 1L;

    private final ErrorObject errorObject;

    public ParseException(String message) {
        super(message);
        this.errorObject = ErrorObject.invalidRequest(message);
    }

    public ParseException(String message, Throwable cause) {
        super(message, cause);
        this.errorObject = ErrorObject.invalidRequest(message);
    }

    public ParseException(ErrorObject errorObject) {
        super(errorObject.getDescription());
        this.errorObject = errorObject;
    }

    public ParseException(ErrorObject errorObject, Throwable cause) {
        super(errorObject.getDescription(), cause);
        this.errorObject = errorObject;
    }

    public ErrorObject getErrorObject() {
        return this.errorObject;
    }
}