/*
 *
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
 *
 */
package com.vmware.identity.idm;

import java.security.cert.X509Certificate;

/**
 * This exception is thrown when a tenant with the given name does not exist. */
public class CertificateInUseException extends IDMException {
	/**
     * Serial version id
     */
	private static final long serialVersionUID = -4284171716790355184L;

	private final X509Certificate certificate;

	public CertificateInUseException(String message, X509Certificate certificate) {
        super(message);
        assert certificate != null;
        this.certificate = certificate;
    }

    public CertificateInUseException(X509Certificate certificate, Throwable ex) {
        super(ex);
        assert certificate != null;
        this.certificate = certificate;
    }

    public CertificateInUseException(String message, X509Certificate certificate, Throwable ex) {
        super(message, ex);
        assert certificate != null;
        this.certificate = certificate;
    }

    public X509Certificate getCertificate() {
       return certificate;
    }
}
