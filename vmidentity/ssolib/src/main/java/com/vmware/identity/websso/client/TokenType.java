/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 * ************************************************************************/
package com.vmware.identity.websso.client;

/**
 * Enum identifying SAML token type.
 *	Note: we always use bearer in this version of Websso.
 */
public enum TokenType {
	BEARER,
	HOLDER_OF_KEY
}
