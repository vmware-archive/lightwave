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

/**
 *
 */
package com.vmware.identity;

/**
 * Some commonly used constants
 *
 */
public final class TestConstants {
	public static final String SIGNATURE_ALGORITHM = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
	public static final String AUTHORIZATION = "bogus";
	public static final String BAD_NAMEID = "bogus";
	public static final String USER = "user"; // default user name to return from authenticate()
	public static final String DOMAIN = "example.com"; // default domain to return from authenticate()
	public static final String RELAY_STATE = "Some interesting relay state"; // relay state parameter to use in tests
	public static final String BAD_PROVIDER = "http://badprovider"; // non-existent provider URL
	public static final String EMAIL_ADDRESS_VALUE = "someuser@somedomain.com"; // email address we expect to see in the token
	public static final String NAME_ID_FORMAT = "<saml2:NameID Format=\"%s\">";
    public static final String RELYING_PARTY = "http://www.example.com/SAML2/SSO"; // sample relying party URL
}
