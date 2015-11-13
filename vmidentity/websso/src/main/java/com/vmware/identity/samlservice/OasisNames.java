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
 * Some OASIS constants are defined here.
 */
public final class OasisNames {
    public final static String SAML2_PREFIX = "urn:oasis:names:tc:SAML:2.0:";
    public final static String STATUS_PREFIX = SAML2_PREFIX + "status:";

    //The permissible top level status codes
    public final static String REQUESTER = STATUS_PREFIX + "Requester";
    public final static String RESPONDER = STATUS_PREFIX + "Responder";
    public final static String SUCCESS = STATUS_PREFIX + "Success";
    public final static String VERSION_MISMATCH = STATUS_PREFIX + "VersionMismatch";

    //The permissible second level status codes
    public final static String PARTIAL_LOGOUT = STATUS_PREFIX + "PartialLogout";
    public static final String NO_SUPPORTED_IDP = STATUS_PREFIX + "NoSupportedIDP";
    public final static String REQUEST_DENIED = STATUS_PREFIX + "RequestDenied";
    public final static String REQUEST_UNSUPPORTED = STATUS_PREFIX + "RequestUnsupported";
    public final static String REQUEST_VERSION_TOO_HIGH = STATUS_PREFIX + "RequestVersionTooHigh";
    public final static String REQUEST_VERSION_TOO_LOW = STATUS_PREFIX + "RequestVersionTooLow";
    public final static String INVALID_NAMEID_POLICY = STATUS_PREFIX + "InvalidNameIDPolicy";

    public final static String ASSERTION = SAML2_PREFIX + "assertion";
    public final static String PROTOCOL = SAML2_PREFIX + "protocol";

    public final static String ENTITY = SAML2_PREFIX + "nameid-format:entity";
    public final static String PERSISTENT = SAML2_PREFIX + "nameid-format:persistent";
    public final static String X509SUBJECTNAME = "urn:oasis:names:tc:SAML:1.1:nameid-format:X509SubjectName";
    public final static String UNSPECIFIED = "urn:oasis:names:tc:SAML:1.1:nameid-format:unspecified";

    public final static String HTTP_POST = SAML2_PREFIX + "bindings:HTTP-POST";
    public final static String HTTP_REDIRECT = SAML2_PREFIX + "bindings:HTTP-Redirect";


    //Authn context class identifiers
    public final static String PASSWORD_PROTECTED_TRANSPORT = SAML2_PREFIX + "ac:classes:PasswordProtectedTransport";
    public final static String INTEGRATED_WINDOWS = "urn:federation:authentication:windows";

    public final static String EMAIL_ADDRESS = "urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress";

    public final static String SCHEMAS = "http://schemas.xmlsoap.org";
    public final static String IDENTITY_FORMAT_UPN = SCHEMAS + "/claims/UPN";
    public final static String IDENTITY_FORMAT_EMAIL_ADDRESS = SCHEMAS + "/ws/2005/05/identity/claims/emailaddress";
}
