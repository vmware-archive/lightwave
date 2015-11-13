/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.ldap;

import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;



public class SslX509EqualityMatchVerificationCallback implements
        ISslX509VerificationCallback {

    private Set<X509Certificate> trustedCertificates;
    //private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SslX509EqualityMatchVerificationCallback.class);
    private static final Log log = LogFactory
            .getLog(SslX509EqualityMatchVerificationCallback.class);

    public SslX509EqualityMatchVerificationCallback(
            Set<X509Certificate> certificates) {
        this.trustedCertificates = certificates;
    }

    @Override
    public boolean isTrustedCertificate(X509Certificate cert) {
        boolean certValidationResult = false;
        String fingerprint = "";
        try {
            fingerprint = SslUtil.computeHexCertificateThumbprint(cert);
        } catch (Exception e) {
            log.info("Can not calculate thumbprint");
        }

        try {
            if (this.trustedCertificates == null
                    || trustedCertificates.size() == 0) {
                log.error(String
                        .format("Server SSL certificate verification failed for [Subject: %s] [SHA1 Fingerprint: %s]. Trusted certificates store is empty.",
                                cert.getSubjectX500Principal().getName(), fingerprint));
            } else {
                cert.checkValidity();

                if (trustedCertificates.contains(cert)) {
                    certValidationResult = true;
                } else {
                    log.error(String
                            .format("Server SSL certificate verification failed for [Subject: %s] [SHA1 Fingerprint: %s].: No match found in the trusted certificates store.",
                                    cert.getSubjectX500Principal().getName(), fingerprint));
                }
            }
        } catch (CertificateExpiredException | CertificateNotYetValidException e) {
            log.error(String.format(
                    "Server SSL certificate verification failed for  [Subject: %s] [SHA1 Fingerprint: %s].", cert
                            .getSubjectX500Principal().getName(), fingerprint), e);
        }
        return certValidationResult;
    }

}
