/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client.sample;

/**
 * Some SAML constants defined
 *
 */
public class SAMLNames {

    //Name space name and name space value strings.
    public static final String NS_NAME_SAML_METADATA = "urn:oasis:names:tc:SAML:2.0:metadata";
    public static final String NS_NAME_SAML_DIGTALSIG = "http://www.w3.org/2000/09/xmldsig#";
    public static final String NS_NAME_SAML_SAML = "xmlns:saml";
    public static final String NS_NAME_SAML_DS = "xmlns:ds";
    public static final String NS_VAL_SAML_SAML = "urn:oasis:names:tc:SAML:2.0:assertion";

    //SAML Element name and value strings
    public static final String ENTITIESDESCRIPTOR = "EntitiesDescriptor";
    public static final String ENTDESCRIPTOR = "EntityDescriptor";
    public static final String SPSSODESCRIPTOR = "SPSSODescriptor";
    public static final String IDPSSODESCRIPTOR = "IDPSSODescriptor";
    public static final String ASSERTIONCONSUMERSERVICE = "AssertionConsumerService";
    public static final String SINGLELOGOUTSERVICE = "SingleLogoutService";
    public static final String SINGLESIGNONSERVICE = "SingleSignOnService";
    public static final String KEYDESCRIPTOR = "KeyDescriptor";
    public static final String NAMEIDFORMAT = "NameIDFormat";
    public static final String SSOS = "SingleSignOnService";
    public static final String SLOS = "SingleLogoutService";
    public static final String DS_KEYINFO = "ds:KeyInfo";
    public static final String ENCRIPTIONMETHOD = "EncriptionMethod";
    public static final String DS_X509DATA = "ds:X509Data";
    public static final String DS_X509CERTIFICATE = "ds:X509Certificate";

    public static final String KEYINFO = "KeyInfo";
    public static final String X509DATA = "X509Data";
    public static final String X509CERTIFICATE = "X509Certificate";

    public static final String DS_KEYVALUE = "ds:KeyValue";
    public static final String DS_RSAKEYVALUE = "ds:RSAKeyValue";
    public static final String DS_MODULUS = "ds:Modulus";
    public static final String DS_EXPONENT = "ds:Exponent";
    public static final String IDFORMAT_VAL_EMAILADD =
                        "urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress";
    public static final String IDFORMAT_VAL_PERSIST =
                        "urn:oasis:names:tc:SAML:2.0:nameid-format:persistent";
    public static final String ATTRIBUTE = "saml:Attribute";
    public static final String IDFORMAT_VAL_UPN =
            "http://schemas.xmlsoap.org/claims/UPN";

    // Attribute name and attribute value strings.
    public static final String ENTID = "entityID";
    public static final String VALIDUNTIL = "validUntil";
    public static final String XMLLANG = "xml:lang";
    public static final String ENGLISH = "en";
    public static final String XMLNS = "xmlns";
    public static final String PSE = "protocolSupportEnumeration";
    public static final String REQUIREDPROTOCAL = "urn:oasis:names:tc:SAML:2.0:protocol";
    public static final String FALSE = "false";
    public static final String TRUE = "true";
    public static final String NAME = "Name";
    public static final String NAMEFORMAT = "NameFormat";
    public static final String FRIENDLYNAME = "FriendlyName";
    public static final String USE = "use";
    public static final String SIGNING = "signing";
    public static final String AUTHNREQUESTSIGNED = "AuthnRequestsSigned";
    public static final String BINDING = "Binding";
    public static final String LOCATION = "Location";
    public static final String RESPONSE_LOCATION = "ResponseLocation";
    public static final String INDEX = "index";
    public static final String ISDEFAULT = "isDefault";
    public static final String WANTSIGNED = "WantAuthnRequestsSigned";
    public static final String RESPLOC = "ResponseLocation";

    //Other constants

    public static final String HTTP_REDIRECT_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Redirect";
    public static final String HTTP_POST_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";
    public static final String SOAP_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:SOAP";
    public static final String DATE_FORMAT = "yyyy-MM-dd'T'HH:mm:ss'Z'";
    public static final String UTC = "UTC";
    public static final String X509 = "X.509";

    //This enum allow using saml keywords as enum.
    //The names has to match the keyword strings.
    public static enum SamlKeyword {
        IDPSSODescriptor,
        Organization,
        Extensions,
        SPSSODescriptor,
        AuthnAuthorityDescriptor,
        RoleDescriptor
    };

}
