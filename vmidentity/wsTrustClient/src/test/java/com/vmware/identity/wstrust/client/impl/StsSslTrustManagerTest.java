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

import java.net.URL;
import java.security.KeyStore;
import java.security.cert.CertStore;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.CollectionCertStoreParameters;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.ConnectionConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.SSLTrustedManagerConfig;

public class StsSslTrustManagerTest {
	// Ignoring as of now. Need to check if we really need this JKS keystore test since we use VKS now.
    @Ignore
    @Test
    public void testFileBasedCrl() throws Exception {
        KeyStore serverCertKeyStore = KeyStore.getInstance("JKS");
        serverCertKeyStore.load(getClass().getClassLoader().getResourceAsStream("server-cert-keystore.jks"),
                "password".toCharArray());
        KeyStore.TrustedCertificateEntry serverCertKeyEntry = (KeyStore.TrustedCertificateEntry) serverCertKeyStore
                .getEntry("server_cert", null);
        X509Certificate serverCert = (X509Certificate) serverCertKeyEntry.getTrustedCertificate();

        KeyStore caCertKeyStore = KeyStore.getInstance("JKS");
        caCertKeyStore.load(getClass().getClassLoader().getResourceAsStream("ca-cert-keystore.jks"),
                "password".toCharArray());
        KeyStore.TrustedCertificateEntry caKeyEntry = (KeyStore.TrustedCertificateEntry) caCertKeyStore.getEntry(
                "ca_cert", null);
        X509Certificate caCert = (X509Certificate) caKeyEntry.getTrustedCertificate();

        Collection<Object> certStoreContent = new ArrayList<Object>();
        CertStore crlCertStore = CertStore.getInstance("Collection",
                new CollectionCertStoreParameters(certStoreContent));
        SSLTrustedManagerConfig sslTrustedManagerConfig = new SSLTrustedManagerConfig(caCertKeyStore, crlCertStore);

        ConnectionConfig config = new ConnectionConfig(new URL("http://sts.vmware.com"), sslTrustedManagerConfig);
        StsSslTrustManager trustManager = new StsSslTrustManager(config.getSSLTrustedManagerConfig());

        // should succeed as serverCert is not yet in the CRL
        trustManager.checkServerTrusted(new X509Certificate[] { serverCert }, "RSA");

        // load the CRL and validate that serverCert is rejected
        CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
        certStoreContent.add((X509CRL) certFactory.generateCRL(getClass().getClassLoader().getResourceAsStream(
                "crl.pem")));
        certStoreContent.add((X509CRL) certFactory.generateCRL(getClass().getClassLoader().getResourceAsStream(
                "crl2.pem")));

        try {
            trustManager.checkServerTrusted(new X509Certificate[] { serverCert }, "RSA");
            Assert.fail("Expected CertificateException not thrown");
        } catch (UntrustedSslCertificateException e) {
        }

        // should succeed since caCert is not in the CRL (only serverCert is)
        trustManager.checkServerTrusted(new X509Certificate[] { caCert }, "RSA");
    }
}
