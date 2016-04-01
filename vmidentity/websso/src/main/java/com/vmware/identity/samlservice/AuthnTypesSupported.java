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
 * Helper class defining allowed authn type.
 */
public class AuthnTypesSupported {
	private boolean passwordProtectTransport;
	private boolean windowsSession;
	private boolean tlsClientCert;
	private boolean rsaSecureID;

	public AuthnTypesSupported(boolean passwordProtectTransport, boolean windowsSession, boolean tlsClient, boolean rsaSecurID)
	{
		this.passwordProtectTransport = passwordProtectTransport;
		this.windowsSession = windowsSession;
		this.tlsClientCert = tlsClient;
		this.rsaSecureID = rsaSecurID;
	}

	public boolean supportsWindowsSession() {
		return windowsSession;
	}
	public void setWindowsSession(boolean windowsSession) {
		this.windowsSession = windowsSession;
	}
	public boolean supportsTlsClientCert() {
		return tlsClientCert;
	}
	public void setTlsClientCert(boolean tlsClientCert) {
		this.tlsClientCert = tlsClientCert;
	}
	public boolean supportsPasswordProtectTransport() {
		return passwordProtectTransport;
	}
	public void setPasswordProtectTransport(boolean passward) {
		this.passwordProtectTransport = passward;
	}

    public boolean supportsRsaSecureID() {
        return rsaSecureID;
    }

    public void setRsaSecureID(boolean rsaSecureID) {
        this.rsaSecureID = rsaSecureID;
    }
}
