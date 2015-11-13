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

import java.io.File;
import java.security.cert.CRL;
import java.security.cert.CertStore;
import java.security.cert.CertStoreException;
import java.security.cert.CertificateException;
import java.security.cert.X509CRLSelector;
import java.security.cert.X509Certificate;

import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.SSLTrustedManagerConfig;

/**
 * X.509 certificate trust
 */
public class StsSslTrustManager implements X509TrustManager {
    private static final String DEFAULT_TRUST_MANAGER_ALGO = "PKIX";

    private static final Logger log = LoggerFactory.getLogger(StsSslTrustManager.class);

    private final X509TrustManager certTrustManager;
    private final SSLTrustedManagerConfig sslConfig;

    /**
     * Constructor
     *
     * @param sslTrustManagerConfig
     *            STS SSL configuration. Cannot be {@code null}.
     */
    public StsSslTrustManager(SSLTrustedManagerConfig sslTrustManagerConfig) {
        assert sslTrustManagerConfig != null;

        this.sslConfig = sslTrustManagerConfig;
        this.certTrustManager = getDefaultTrustManager();
    }

    /**
     * @throws UntrustedSslCertificateException
     */
    @Override
    public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {

        CertStore crlCertStore = getCRLCertStore();
        // Check for revoked certificates in the chain
        if (crlCertStore != null) {
            try {
                for (X509Certificate cert : chain) {
                    X509CRLSelector selector = new X509CRLSelector();
                    selector.setCertificateChecking(cert);
                    for (CRL crl : crlCertStore.getCRLs(selector)) {
                        if (crl.isRevoked(cert)) {
                            throw new CertificateException("Server certificate revoked by the CRL");
                        }
                    }
                }
            } catch (CertStoreException e) {
                throw new CertificateException("Failed to validate certificate against CertStore CRL", e);
            }
        }

        if (certTrustManager != null) {
            try {

                certTrustManager.checkServerTrusted(chain, authType);
                if (log.isDebugEnabled()) {
                    log.debug("The SSL certificate of STS service was successfully"
                            + " verified against the list of client-trusted certificates");
                }

            } catch (CertificateException e) {
                if (log.isDebugEnabled()) {
                    log.debug("The SSL certificate of STS service cannot be verified"
                            + " against the list of client-trusted certificates", e);
                }
                
                throw new UntrustedSslCertificateException(
                        "The SSL certificate of STS service cannot be verified", chain, null);
            }
        }
        else
        {
            throw new UntrustedSslCertificateException(
                    "The SSL certificate of STS service cannot be verified", chain, null);
        }
    }

    @Override
    public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {

        if (certTrustManager != null) {
            certTrustManager.checkClientTrusted(chain, authType);
        }
    }

    @Override
    public X509Certificate[] getAcceptedIssuers() {
        return certTrustManager != null ? certTrustManager.getAcceptedIssuers() : new X509Certificate[0];
    }

    private X509TrustManager getDefaultTrustManager() {

        TrustManagerFactory factory;
        try {

            factory = TrustManagerFactory.getInstance(DEFAULT_TRUST_MANAGER_ALGO);
            factory.init(sslConfig.getKeyStore());

        } catch (Exception e) {
            String errMsg = "Unable to create trust manager factory";
            log.error(errMsg, e);
            throw new IllegalStateException(errMsg, e);
        }

        for (TrustManager trustManager : factory.getTrustManagers()) {
            if (trustManager instanceof X509TrustManager) {
                return (X509TrustManager) trustManager;
            }
        }

        throw new IllegalStateException("Unable to find default trust manager");
    }

    private CertStore getCRLCertStore() {
        if (skipServerCertCrlChecking())
            return null;
        return this.sslConfig.getCrlCertStore();
    }

    /**
     * as a safety precaution, allow turning off CRL checking for server certs
     * by placing the following file in the current directory of the executing
     * JVM
     */
    private static boolean skipServerCertCrlChecking() {

        boolean result = false;

        try {
            File file = new File("WstClientSkipServerCertCrlChecking");
            if (file.exists() && file.isFile()) {
                result = true;
                log.warn(String.format("skipping CRL check for server cert because of existence of file: %s",
                        file.getAbsolutePath()));
            }
        } catch (Throwable e) {
        }

        return result;
    }
}
