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
package com.vmware.identity.wstrust.client;

import java.security.cert.X509Certificate;

import com.vmware.vim.sso.client.BundleMessageSource.Key;
import com.vmware.vim.sso.client.exception.SsoException;

/**
 * CertificateException that includes useful additional information.
 */
public class CertificateValidationException extends SsoException {
    private static final long serialVersionUID = 7018828306769023783L;

    private final X509Certificate[] certificateChain;
    private final String thumbprint;

    /**
     * Constructor.
     *
     * @param message
     *            The error message.
     * @param certificateChain
     *            The failed certificate chain.
     * @param thumbprint
     *            The thumbprint of the first certificate in the chain.
     */
    public CertificateValidationException(String message, X509Certificate[] certificateChain, String thumbprint) {
        this(message, null, certificateChain, thumbprint);
    }

    public CertificateValidationException(String message, Key messageKey, X509Certificate[] certificateChain,
            String thumbprint) {
        super(message, messageKey, null, thumbprint);
        this.certificateChain = certificateChain;
        this.thumbprint = thumbprint;
    }

    /**
     * Obtains the failed certificate chain.
     *
     * @return the certificate chain.
     */
    public X509Certificate[] getCertificateChain() {
        return certificateChain;
    }

    /**
     * Obtains the thumbprint of the first certificate in the chain.
     *
     * @return the thumbprint.
     */
    public String getThumbprint() {
        return this.thumbprint;
    }

}
