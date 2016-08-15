/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.identity.websso.client;

/**
 * @author root
 *
 */
public class SamlNames {
    public final static String SAML2_PREFIX = "urn:oasis:names:tc:SAML:2.0:";
    public final static String STATUS_PREFIX = SAML2_PREFIX + "status:";

    public final static String REQUESTER = STATUS_PREFIX + "Requester";
    public final static String RESPONDER = STATUS_PREFIX + "Responder";
    public final static String SUCCESS = STATUS_PREFIX + "Success";
    public final static String VERSION_MISMATCH = STATUS_PREFIX + "VersionMismatch";

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

    //Binding type identifyer
    public final static String HTTP_POST = SAML2_PREFIX + "bindings:HTTP-POST";
    public final static String HTTP_REDIRECT = SAML2_PREFIX + "bindings:HTTP-Redirect";

    //default authncontexttype identifier
    public final static String PASSWORD_PROTECTED_TRANSPORT = SAML2_PREFIX + "ac:classes:PasswordProtectedTransport";
    public final static String INTEGRATED_WINDOWS = "urn:federation:authentication:windows";

    //NameIDFormatType identifier
    public final static String EMAIL_ADDRESS = "urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress";

    public final static String SCHEMAS = "http://schemas.xmlsoap.org";
    public final static String IDENTITY_FORMAT_UPN = SCHEMAS + "/claims/UPN";
    public final static String IDENTITY_FORMAT_EMAIL_ADDRESS = SCHEMAS + "/ws/2005/05/identity/claims/emailaddress";

    //token type
    public final static String URI_BEARER = SAML2_PREFIX + "cm:bearer";
}
