/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
/**
 * @author schai
 * Some SAML keywords are defined here
 */
package com.vmware.identity.idm.client;

import java.net.URI;
import java.util.Arrays;
import java.util.List;

public class SAMLNames
{
    //Name space name and name space value strings.
    public static final String XMLNS = "xmlns";
    public static final String NS_NAME_SAML_METADATA =
            "urn:oasis:names:tc:SAML:2.0:metadata";

    public static final String NS_NAME_SAML_SAML = "xmlns:saml";
    public static final String NS_VAL_SAML_SAML =
            "urn:oasis:names:tc:SAML:2.0:assertion";

    public static final String NS_NAME_SAML_VMWARE_ES = "xmlns:vmes";
    public static final String NS_VAL_SAML_VMWARE_ES =
            "http://vmware.com/schemas/attr-names/2012/04/Extensions";

    public static final String NS_NAME_DS_PREFIX = "ds";

    public static final String NS_NAME_SAML_DS = "xmlns:" + NS_NAME_DS_PREFIX;
    public static final String NS_NAME_SAML_DIGTALSIG =
            "http://www.w3.org/2000/09/xmldsig#";

    //SAML Element name and value strings
    public static final String ENTITIESDESCRIPTOR = "EntitiesDescriptor";
    public static final String ENTDESCRIPTOR = "EntityDescriptor";
    public static final String SPSSODESCRIPTOR = "SPSSODescriptor";
    public static final String IDPSSODESCRIPTOR = "IDPSSODescriptor";
    public static final String ENTITY_ID_PLACEHOLDER = "/SAML2/Metadata/";
    public static final String ASSERTIONCONSUMERSERVICE =
            "AssertionConsumerService";
    public static final String SP_ASSERTIONCONSUMERSERVICE_PLACEHOLDER = "/SAML2/SP/ACS/";
    public static final String SP_SINGLELOGOUTSERVICE_PLACEHOLDER = "/SAML2/SP/SLO/";
    public static final String SINGLELOGOUTSERVICE = "SingleLogoutService";
    public static final String SINGLESIGNONSERVICE = "SingleSignOnService";
    public static final String EXTENSIONS = "Extensions";
    public static final String ORGANIZATION = "Organization";
    public static final String ORGANIZATIONNAME = "OrganizationName";
    public static final String ORGANIZATIONDISPLAYNAME =
            "OrganizationDisplayName";
    public static final String ORGANIZATIONURL = "OrganizationURL";
    public static final String EXPORTEDBY = "vmes:ExportedBy";
    public static final String EXPORTEDBY_VAL =
            "Exported by VMware Identity Server (c) 2012";
    public static final String EXPORTEDON = "vmes:ExportedOn";
    public static final String KEYDESCRIPTOR = "KeyDescriptor";
    public static final String NAMEIDFORMAT = "NameIDFormat";
    public static final String SSOS = "SingleSignOnService";
    public static final String SLOS = "SingleLogoutService";
    public static final String ENCRIPTIONMETHOD = "EncriptionMethod";

    public static final String KEYINFO = "KeyInfo";
    public static final String X509DATA = "X509Data";
    public static final String X509CERTIFICATE = "X509Certificate";
    public static final String ASSERTION = "Assertion";

    public static final String DS_KEYINFO = NS_NAME_DS_PREFIX + ":" + KEYINFO;
    public static final String DS_X509DATA = NS_NAME_DS_PREFIX + ":" + X509DATA;
    public static final String DS_X509CERTIFICATE = NS_NAME_DS_PREFIX + ":" + X509CERTIFICATE;

    public static final String DS_KEYVALUE = NS_NAME_DS_PREFIX + ":KeyValue";
    public static final String DS_RSAKEYVALUE = NS_NAME_DS_PREFIX + ":RSAKeyValue";
    public static final String DS_MODULUS = NS_NAME_DS_PREFIX + ":Modulus";
    public static final String DS_EXPONENT = NS_NAME_DS_PREFIX + ":Exponent";

    public static final URI IDFORMAT_VAL_EMAILADD =
            URI.create("urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress");
    public static final URI IDFORMAT_VAL_PERSIST =
            URI.create("urn:oasis:names:tc:SAML:2.0:nameid-format:persistent");
    public static final URI IDFORMAT_VAL_UPN =
            URI.create("http://schemas.xmlsoap.org/claims/UPN");

    public static final List<URI> SUPPORTEDNAMEIDFORMATS = Arrays.asList(
            IDFORMAT_VAL_EMAILADD, IDFORMAT_VAL_PERSIST, IDFORMAT_VAL_UPN);
    public static final String ATTRIBUTE = "saml:Attribute";
    //extension elements
    public static final String LOCALOSIDSTORE = "vmes:LocalOsIdentityStore";
    public static final String ACTIVEDIRECTORYIDSTORE =
            "vmes:ActiveDirectoryIdentityStore";
    public static final String KDC = "vmes:KDC";
    public static final String ATTRIBUTEMAP = "vmes:AttributeMap";


    // Attribute name and attribute value strings.
    public static final String ENTID = "entityID";
    public static final String VALIDUNTIL = "validUntil";
    public static final String XMLLANG = "xml:lang";
    public static final String ENGLISH = "en";
    public static final String PSE = "protocolSupportEnumeration";
    public static final String REQUIREDPROTOCAL =
            "urn:oasis:names:tc:SAML:2.0:protocol";
    public static final String FALSE = "false";
    public static final String TRUE = "true";
    public static final String NAME = "Name";
    public static final String NAMEFORMAT = "NameFormat";
    public static final String FRIENDLYNAME = "FriendlyName";
    public static final String USE = "use";
    public static final String SIGNING = "signing";
    public static final String AUTHNREQUESTSIGNED = "AuthnRequestsSigned";
    public static final String WANTASSERTIONSSIGNED = "WantAssertionsSigned";
    public static final String BINDING = "Binding";
    public static final String LOCATION = "Location";
    public static final String RESPONSE_LOCATION ="ResponseLocation";
    public static final String INDEX = "index";
    public static final String ISDEFAULT = "isDefault";
    public static final String WANTSIGNED = "WantAuthnRequestsSigned";
    public static final String RESPLOC = "ResponseLocation";


    //extension attributes in vmres name space
    public static final String VMES_ISDEFAULT = "vmes:isDefault";
    public static final String CLOCKTOLERANCE = "vmes:clockTolerance";
    public static final String DELEGATIONCNT = "vmes:delegationCount";
    public static final String RENEWCNT = "vmes:renewCount";
    public static final String MAXHOKTKNLIFETIME =
            "vmes:maximumHokTokenLifeTime";
    public static final String MAXBEARERTKNLIFETIME =
            "vmes:maximumBearerTokenLifeTime";
    public static final String ISSUERNAME = "vmes:issuerName";
    public static final String IDVALUE = "vmes:value";
    public static final String IDNAME = "vmes:name";
    public static final String IDUSERNAME = "vmes:userName";
    public static final String IDUSEMACHACCT = "vmes:useMachineAccount";
    public static final String IDSPN = "vmes:spn";
    public static final String IDUSERPWD = "vmes:userPassword";
    public static final String IDSEARCHBASEDN = "vmes:searchBaseDn";
    public static final String IDSEARCHTIMEOUTSCNDS =
            "vmes:searchTimeoutSeconds";
    public static final String IDFRIENDLYNAME = "vmes:friendlyName";


    //Other constants

    public static final String HTTP_REDIRECT_BINDING =
            "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Redirect";
    public static final String HTTP_POST_BINDING =
            "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";
    public static final String SOAP_BINDING =
            "urn:oasis:names:tc:SAML:2.0:bindings:SOAP";
    public static final String DATE_FORMAT = "yyyy-MM-dd'T'HH:mm:ss'Z'";
    public static final String UTC = "UTC";
    public static final String X509 = "X.509";


    //This enum allow using saml keywords as enum.
    //The names has to match the keyword strings.
    public static enum SamlKeyword
    {
        IDPSSODescriptor, Organization, Extensions, SPSSODescriptor, AuthnAuthorityDescriptor, RoleDescriptor
    }

    public static boolean isSupportedNameIdFormat(URI nameIDFormat)
    {
        return SUPPORTEDNAMEIDFORMATS.contains(nameIDFormat);
    }
}
