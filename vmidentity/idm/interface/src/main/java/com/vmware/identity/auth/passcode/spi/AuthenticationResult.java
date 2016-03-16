/*
 *
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
 *
 */
package com.vmware.identity.auth.passcode.spi;

/**
 * This class contains the status of the authentication.
 *
 * @author aantochi
 *
 */
public class AuthenticationResult {

    private int statusCode;

    public static final int ACCESS_OK = 0;
    public static final int ACCESS_DENIED = 1;
    public static final int NEXT_CODE_REQUIRED = 2;
    public static final int NEXT_CODE_BAD = 4;
    public static final int NEW_PIN_REQUIRED = 5;

    public AuthenticationResult(int status) {
        this.statusCode = status;
    }

    public int getStatusCode() {
        return statusCode;
    }
}