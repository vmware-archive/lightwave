package com.vmware.vmidentity.websso.client.test;

/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/

/**
 * Some commonly used constants in unit test
 * 
 */
public final class TestConstants {
    // public static final String SIGNATURE_ALGORITHM =
    // "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
    public static final String BAD_NAMEID = "bogus";
    public static final String USER = "j.Doe"; // default user name to return
                                               // from authenticate()
    public static final String DOMAIN = "example.com"; // default domain to
                                                       // return from
                                                       // authenticate()
    public static final String RELAY_STATE = "Some interesting relay state"; // relay
                                                                             // state
                                                                             // parameter
                                                                             // to
                                                                             // use
                                                                             // in
                                                                             // tests
    public static final String BAD_PROVIDER = "http://badprovider"; // non-existent
                                                                    // provider
                                                                    // URL
    public static final String EMAIL_ADDRESS_VALUE = "someuser@somedomain.com"; // email
                                                                                // address
                                                                                // we
                                                                                // expect
                                                                                // to
                                                                                // see
                                                                                // in
                                                                                // the
                                                                                // token
    public static final String NAME_ID_FORMAT = "<saml2:NameID Format=\"%s\">";
    public static final String RELYING_PARTY = "http://www.example.com/SAML2/SSO"; // sample
                                                                                   // relying
                                                                                   // party
                                                                                   // URL
}
