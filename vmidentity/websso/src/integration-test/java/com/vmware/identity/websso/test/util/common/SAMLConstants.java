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

package com.vmware.identity.websso.test.util.common;

public interface SAMLConstants {

	public static final String SSOSTS_Path = "/websso/SAML2/SSO";

	public static String ENTITY_DESCRIPTOR = "EntityDescriptor";
	public static String ENTITY_ID = "entityID";

	public static final String CastleSessionCookieName = "CastleSession%s";
	public static final String CookieHeaderName = "Set-Cookie";
	public static final String ChromeUserAgent = "ApacheClient Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.118 Safari/537.36";
}
