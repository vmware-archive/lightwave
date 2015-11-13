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

import java.net.URL;
import java.security.Key;
import java.security.KeyStore;
import java.security.Provider;
import java.security.cert.CertStore;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * This class represents all the configuration needed for SSO client library to
 * function properly
 */
public class SecurityTokenServiceConfig {

    private static final int REQUEST_VALIDITY_IN_SECONDS = 600; // 10 minutes

    private final ConnectionConfig connConfig;
    private X509Certificate[] trustedIssuerCertificates;
    private final ExecutorService executorService;
    private final HolderOfKeyConfig holderOfKeyConfig;

    /**
     * Creates STS configuration object suitable for bearer confirmation type.<br>
     * If the executor service is {@code null} a default single threaded
     * executor will be created. Otherwise, it is the caller's responsibility to
     * shutdown the executor when it is no longer needed.
     *
     * @param connConfig
     *            used to configure the connection with STS web service; Cannot
     *            be <code>null</code>
     * @param trustedRootCertificates
     *            the X.509 certificates of one or more authorities the client
     *            trusts when assertions will be validated; Cannot be
     *            <code>null</code>
     * @param executorService
     *            the ExecutorService used for asynchronous calls to the STS;
     *            Can be {@code null}
     */
    public SecurityTokenServiceConfig(ConnectionConfig connConfig, X509Certificate[] trustedIssuerCertificates,
            ExecutorService executorService) {

        this(connConfig, trustedIssuerCertificates, executorService, null);
    }

    /**
     * Creates STS configuration object suitable for holder of key confirmation
     * type.<br>
     * If the executor service is null a default single threaded executor will
     * be created.
     *
     * @param connConfig
     *            used to configure the connection with STS web service; Cannot
     *            be <code>null</code>
     * @param trustedRootCertificates
     *            The X.509 certificates of one or more authorities the client
     *            trusts. Cannot be <code>null</code>.
     * @param executorService
     *            The ExecutorService used for asynchronous calls to the STS.
     *            Can be <code>null</code>
     * @param holderOfKeyConfig
     *            The holder of key configuration properties. Can be
     *            <code>null</code>
     */
    public SecurityTokenServiceConfig(ConnectionConfig connConfig, X509Certificate[] trustedIssuerCertificates,
            ExecutorService executorService, HolderOfKeyConfig holderOfKeyConfig) {

        ValidateUtil.validateNotNull(connConfig, "STS connection configuration");

        this.connConfig = connConfig;
        this.trustedIssuerCertificates = trustedIssuerCertificates;
        this.executorService = (executorService != null) ? executorService : Executors.newSingleThreadExecutor();

        this.holderOfKeyConfig = holderOfKeyConfig;
    }

    /**
     * @return the X.509 certificates the caller trusts to issue tokens or to
     *         sign the tokens issuer's certificate.
     */
    public X509Certificate[] getTrustedRootCertificates() {
        return this.trustedIssuerCertificates;
    }

    /**
     * @return the ExecutorService used for asynchronous calls to the STS
     */
    public ExecutorService getExecutorService() {
        return executorService;
    }

    /**
     * Returns holder of key configuration properties
     *
     * @return holder of key configuration properties. Might be
     *         <code>null</code> .
     */
    public HolderOfKeyConfig getHolderOfKeyConfig() {
        return holderOfKeyConfig;
    }

    /**
     * Returns STS SSL configuration properties
     *
     * @return STS SSL configuration properties. Might be <code>null</code>.
     */
    public ConnectionConfig getConnectionConfig() {
        return connConfig;
    }

    /**
     * Returns the amount of time in seconds that requests created are supposed
     * to be valid
     *
     * @return a positive number of seconds
     */
    public int getRequestValidityInSeconds() {
        return REQUEST_VALIDITY_IN_SECONDS;
    }

    /**
     * Class containing holder of key configuration properties
     */
    public static class HolderOfKeyConfig {

        private static final String INVALID_ARGUMENTS_ERR_MGS = "Input parameters cannot be null";

        private final Key privateKey;
        private final X509Certificate certificate;
        private final Provider securityProvider;
        private final Logger log = LoggerFactory.getLogger(HolderOfKeyConfig.class);

        /**
         * Creates new holder of key configuration
         *
         * @param privateKey
         *            The private key used to sign messages to the security
         *            token service. This key should pair with the certificate
         *            parameter. Cannot be null.
         * @param certificate
         *            The certificate that will be embedded into the holder of
         *            key token. This certificate should match the private key.
         *            If this parameter is null then the subject to which the
         *            token is issued should have certificate mapped to it.
         * @param securityProvider
         *            The security provider used for signing the messages to the
         *            security token service. If this parameter is null the
         *            default provider will be used.
         */
        public HolderOfKeyConfig(Key privateKey, X509Certificate certificate, Provider securityProvider) {
            if (privateKey == null) {
                log.error(INVALID_ARGUMENTS_ERR_MGS);
                throw new IllegalArgumentException(INVALID_ARGUMENTS_ERR_MGS);
            }

            if (certificate.getPublicKey() instanceof RSAPublicKey) {
                if (!(privateKey instanceof RSAPrivateKey)
                        || !((RSAPrivateKey) privateKey).getModulus().equals(
                                ((RSAPublicKey) certificate.getPublicKey()).getModulus()))
                    throw new IllegalArgumentException("Certificate doesn't match with private key");
            }

            this.privateKey = privateKey;
            this.certificate = certificate;
            this.securityProvider = securityProvider;
        }

        /**
         * Returns the private key used to sign the messages to STS
         *
         * @return private key
         */
        public Key getPrivateKey() {
            return privateKey;
        }

        /**
         * Returns the certificate which should be embedded into the holder of
         * key token.
         *
         * @return certificate
         */
        public X509Certificate getCertificate() {
            return certificate;
        }

        /**
         * Returns the security provider used for signing the messages to the
         * security token service.
         *
         * @return security provider
         */
        public Provider getSecurityProvider() {
            return securityProvider;
        }
    }

    /**
     * Configuration needed for establishing connection with the STS service. It
     * is highly recommended to use URL with HTTPS protocol, in which case
     * trusted root certificates and/or thumbprint should be specified.
     * Non-HTTPS URL is also supported for development purposes and in this case
     * no certificates or thumbprints are required.
     * <p>
     * Establishing a trusted connection with STS server would require any of
     * the following:
     * <ul>
     * <li>a certificate path to exist from any of the trusted root certificates
     * to the certificate ( or certificate path ) provided by the STS service</li>
     * <li>the thumbprint of the STS service's leaf certificate to be specified
     * as a trusted thumbprint</li>
     * </ul>
     */
    public static class ConnectionConfig {
        private final Logger log = LoggerFactory.getLogger(ConnectionConfig.class);

        private static final int DEFAULT_MAX_RETRY_ATTEMPS = 5;

        private String domain;
        private String tenant;
        private URL url;
        private int maxRetryAttempts;
        private SSLTrustedManagerConfig sslConfig;

        /**
         * Constructor.
         *
         * @param url
         *            The URL of STS web service; HTTPS is the recommended
         *            protocol to be used. HTTP is supported for development
         *            purposes. Cannot be {@code null}.
         * @param SSLTrustedManagerConfig
         *            configuration that contains the SSL certificates of the
         *            STS web service; Can be {@code null}
         */
        public ConnectionConfig(URL url, SSLTrustedManagerConfig sslConfig) {
            ValidateUtil.validateNotNull(url, "STS service URL");

            this.url = url;

            if (!isHttpsProtocol(url)) {
                log.warn("This configuration will establish untrusted connection with the STS server."
                        + "It is acceptable for developing purposes only!");
            }

            this.sslConfig = sslConfig;
            this.maxRetryAttempts = 0;
        }

        /**
         * Constructor.
         *
         * @param SSLTrustedManagerConfig
         *            configuration that contains the SSL certificates of the
         *            STS web service; Can be {@code null}
         * @param domain
         *            The domain of STS service; Cannot be {@code null}.
         * @param tenant
         *            The tenant of STS service; Cannot be {@code null}.
         */
        public ConnectionConfig(String domain, String tenant, SSLTrustedManagerConfig sslConfig) {
            this(domain, tenant, sslConfig, DEFAULT_MAX_RETRY_ATTEMPS);
        }

        /**
         * Constructor.
         *
         * @param domain
         *            The domain of STS service; Cannot be {@code null}.
         * @param tenant
         *            The tenant of STS service; Cannot be {@code null}.
         * @param SSLTrustedManagerConfig
         *            configuration that contains the SSL certificates of the
         *            STS web service; Can be {@code null}
         * @param maxRetryAttempts
         *            maximum number of retry in case of failure
         */
        public ConnectionConfig(String domain, String tenant, SSLTrustedManagerConfig sslConfig, int maxRetryAttempts) {
            ValidateUtil.validateNotNull(domain, "STS domain name");
            ValidateUtil.validateNotNull(tenant, "STS tenant name");

            this.domain = domain;
            this.tenant = tenant;
            this.sslConfig = sslConfig;

            if (sslConfig == null) {
                log.warn("This configuration will establish untrusted connection with the STS server."
                        + "It is acceptable for developing purposes only!");
            }

            this.maxRetryAttempts = maxRetryAttempts;
        }

        /**
         * @return the STS server URL. Can be {@code null}.
         */
        public URL getUrl() {
            return this.url;
        }

        /**
         * @return the STS server domain name. Can be {@code null}.
         */
        public String getDomain() {
            return this.domain;
        }

        /**
         * @return the tenant name. Can be {@code null}.
         */
        public String getTenant() {
            return this.tenant;
        }

        /**
         * @return the configuration that contains the SSL certificates of the
         *         STS web service;. Cannot be {@code null}.
         */
        public SSLTrustedManagerConfig getSSLTrustedManagerConfig() {
            return this.sslConfig;
        }

        /**
         * @return the max retry attempts number that contains the SSL
         *         certificates of the STS web service;. Cannot be {@code null}.
         */
        public int getMaxRetryAttempts() {
            return this.maxRetryAttempts;
        }

        private static boolean isHttpsProtocol(URL url) {
            return "https".equals(url.getProtocol());
        }
    }

    public static class SSLTrustedManagerConfig {
        private final CertStore crlCertStore;
        private final KeyStore keyStore;

        public SSLTrustedManagerConfig(KeyStore keyStore) {
            this(keyStore, null);
        }

        public SSLTrustedManagerConfig(KeyStore keyStore, CertStore crlCertStore) {
            this.keyStore = keyStore;
            this.crlCertStore = crlCertStore;
        }

        /**
         * @return the CRL cert store. Might be {@code null}.
         */
        public CertStore getCrlCertStore() {
            return crlCertStore;
        }

        /**
         * @return the keystore that has the SSL certificates of the STS web
         *         service. Might be {@code null}.
         */
        public KeyStore getKeyStore() {
            return keyStore;
        }
    }
}
