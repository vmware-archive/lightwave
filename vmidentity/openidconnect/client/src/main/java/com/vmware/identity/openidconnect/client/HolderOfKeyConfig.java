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

package com.vmware.identity.openidconnect.client;

import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;

import org.apache.commons.lang3.Validate;

/**
 * Client key configuration
 *
 * @author Jun Sun
 */
public final class HolderOfKeyConfig {
    private final RSAPrivateKey clientPrivateKey;
    private final X509Certificate clientCertificate;

    /**
     * Constructor
     *
     * @param clientPrivateKey          Client private key.
     * @param clientCertificate         Client certificate.
     */
    public HolderOfKeyConfig(RSAPrivateKey clientPrivateKey, X509Certificate clientCertificate) {
        Validate.notNull(clientPrivateKey, "clientPrivateKey");
        Validate.notNull(clientCertificate, "clientCertificate");

        this.clientPrivateKey = clientPrivateKey;
        this.clientCertificate = clientCertificate;
    }

    /**
     * Get client private key
     *
     * @return                          Client private key
     */
    public RSAPrivateKey getClientPrivateKey() {
        return this.clientPrivateKey;
    }

    /**
     * Get client certificate
     *
     * @return                          Client certificate
     */
    public X509Certificate getClientCertificate() {
        return this.clientCertificate;
    }
}
