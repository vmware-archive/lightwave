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

package com.vmware.identity.samlservice;

/**
 * Error constants for websso TODO replace all hard-coded error message websso
 * validationresult ctr
 */
public class WebSSOError {

    /*
     * Error Status that is not reserved SAML status code These are keyed to
     * localized message table. The localized status message is send to browser
     */
    public static final String UNAUTHORIZED = "Unauthorized";
    public static final String BAD_REQUEST = "BadRequest";
    /*
     * Minor (secondary) status. Detailed error message
     */
    public static final String NO_CLIENT_CERT = "NoClientCert";
    public static final String INVALID_CREDENTIAL = "InvalidCredential";
    public static final String LOGGED_OUT_TLS_SESSION = "LoggedOutTLSSession";
    public static final String SECUREID_NEWPIN_REQUIRED = "SecureIDNewPinRequired";
    public static final String SECUREID_NEXTCODE_MODE = "SecureIDNextCode";
    /*
     * Internal error messages
     */

}