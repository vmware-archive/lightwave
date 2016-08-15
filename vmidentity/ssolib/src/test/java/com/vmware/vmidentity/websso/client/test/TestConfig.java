/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import com.vmware.identity.websso.client.SamlNames;

/**
 * @author root
 * 
 */
public class TestConfig {

    public final static String tenantName = "exampleSP";
    // IDP config
    public final static String localHost = "localhost";
    public final static String idpEntityID = "https://prmh-nimbus-vm-185-060.eng.vmware.com:7444/websso/SAML2/Metadata/csp";

    public final static String SsoService_loc = idpEntityID;
    public final static String SsoService_binding_0 = SamlNames.HTTP_REDIRECT;

    // public final static String IdpSloService_loc =
    public final static String IdpSloService_loc = idpEntityID.replaceFirst("SSO", "SLO");
    public final static String IdpSloService_binding_0 = SamlNames.HTTP_REDIRECT;

    // SP Config
    public final static String spEntityID = "http://" + localHost + ":8080/exampleSP/SAML2";
    public final static boolean authnSigned = false;
    public final static String SpSign_algorithm_URI = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
    public final static String ACS0_binding = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";
    public final static String ACS0_endpoint = "http://example.com:8080/exampleSP/SAML2/SSO";
    public final static String SpSloService_loc = "http://example.com:8080/exampleSP/SAML2/SLO";
    public final static String SpSloService_binding_0 = SamlNames.HTTP_REDIRECT;

}
