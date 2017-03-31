/*
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
 */
package com.vmware.identity.websso.client;

/**
 * Error constants for websso client library TODO item.(part of ) replace all
 * hard-coded error message in the lib It is part of task# 904516
 */
public class Error {

    /*
     * Error Status
     */
    public static final String NO_ERROR = "Success";
    public static final String BAD_REQUEST = "BadRequest";
    public static final String BAD_RESPONSE = "BadResponse";
    public static final String FORBIDDEN = "Forbidden";

    /*
     * Minor status. Detailed error message
     */
    public static final String IN_RESPONSE_TO = "No authentication request found matching InResponseTo attribute of SAML Response from IDP.";
    public static final String ISSUER = "Issuer not trusted: ";
    public static final String CONDITION = "Invalid condition value in token. If this is issued by legitimate IDP, it is possibly caused by clock skew or slow response due to network issues.";
    public static final String SIGNATURE = "Invalid signature.";
    public static final String NOT_SIGNED = "Assertion not signed.";
    public static final String ACS_DESTINATION = "The destination does not match ACS.";
    public static final String SLO_DESTINATION = "The destination does not match "
            + "service provider's SLO service location.";
    public static final String SAML_DESTINATION = "The SAML message was not intended to this destination.";
    public static final String EXT_TOKEN_NOT_VALIDATABLE = "Failed in validating the subject in external token.";
    public static final String IDP_SESSION_EXPIRED = "Failed to validate the session expiry date.";

    /*
     * Internal error messages
     */
    public static final String INTERNAL_ERROR_TYPE = "WEBSSO client library internal error. ";
    public static final String IDPList = "Unable to choose external IDP due to missing IDPList and multiple trusted IDP registered.";
}
