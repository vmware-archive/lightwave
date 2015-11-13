/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

/**
 * Thrown upon creating SSL connection to indicate untrusted SSL certificate of
 * the other peer.
 */
public class UntrustedSslCertificateException extends CertificateException {

    private static final long serialVersionUID = 5984391942127159179L;

    private final X509Certificate[] chain;
    private final String thumbprint;

    /**
     * Constructor
     *
     * @param msg
     *            the detail message
     * @param chain
     *            certificate chain for STS server identity
     * @param thumbprint
     *            thumbprint of the leaf certificate from STS server identity
     */
    UntrustedSslCertificateException(String msg, X509Certificate[] chain, String thumbprint) {

        super(msg);

        assert chain != null;
        assert !thumbprint.isEmpty();

        this.chain = chain;
        this.thumbprint = thumbprint;
    }

    /**
     * @return the certificate chain of the STS service
     */
    public X509Certificate[] getServerCertificateChain() {
        return chain;
    }

    /**
     * @return the thumbprint of the STS service's certificate
     */
    public String getServerCertificateThumbprint() {
        return thumbprint;
    }

}
