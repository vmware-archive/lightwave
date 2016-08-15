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

/**
 * @author Yehia Zayour
 */
public enum ErrorMessage {
    BAD_REQUEST("BadRequest"),
    RESPONDER("Responder"),
    UNAUTHORIZED("Unauthorized"),
    SECURID_NEW_PIN_REQUIRED("Unauthorized.SecureIDNewPinRequired"),
    SECURID_NEXT_CODE("Unauthorized.SecureIDNextCode"),
    NO_CLIENT_CERT("Unauthorized.NoClientCert"),
    LOGGED_OUT_TLS_SESSION("Unauthorized.LoggedOutTLSSession"),
    INVALID_CREDENTIAL("Unauthorized.InvalidCredential");

    private final String value;

    private ErrorMessage(String value) {
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }
}