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

package com.vmware.identity.idm.server;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Enumeration;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Properties;

import org.apache.commons.lang.Validate;

import com.vmware.identity.idm.ClientCertPolicy;

public class ClientCertTestUtils {

    public static final String tenant1 = "TestTenant1";

    private static final String STS_STORE_PASS = "idm.server.stskey-store.pass";
    private static final String STS_STORE_JKS = "idm.server.stskey-store";
    private static final String CUSTOM_STORE_JKS = "idm.server.custom-store";
    private static final String CUSTOM_STORE_PASS = "idm.server.custom-store.pass";
    private static final String CUSTOM_KEY_PASS = "idm.server.custom-store.key.pass";

    // ocsp responder url
    public final String dodOCSPResponder = "http://ocsp.nsn0.rcvs.nit.disa.mil";

    // trusted CA store.
    // keys in the caStoreName are:
    // private key
    // "ca.pem"
    // "DOD_JITC_EMAIL_CA-29__0x01A5__DOD_JITC_ROOT_CA_2.cer"
    // "DOD_JITC_ROOT_CA_2__0x05__DOD_JITC_ROOT_CA_2.cer"
    // "DOD_JITC_CA-27__0x019F__DOD_JITC_ROOT_CA_2.cer"

    public final String caStoreName = "clientCAstore";
    public final String storePass = "changeme";
    public final String signingCACertAlias = "dod_jitc_email_ca-29__0x01a5__dod_jitc_root_ca_2.cer";

    // Keystore that keeps the test client cert
    public final String clientStoreName = "clientCertStore";

    // Alias of manually created testing certificates, stored in clientCertStore
    public final String validCertAlias = "client.pem";
    public final String expiredCertAlias = "client_expired.pem";

    // local cached CRL files
    public final String dodCRLCacheROOTCA2 = "DODJITCROOTCA2.crl";
    public final String dodCRLCacheEMAILCA_29 = "DODJITCEMAILCA_29.crl";
    public final String dodCRLDistributionPointEMAILCA_29 = "http://crl.nit.disa.mil/crl/DODJITCEMAILCA_29.crl";

    // DOD test client cert stored in clientCertStore
    public final String validDodCertAlias1 = "bill.preston.s.9301000121.email_sig.cer";// expire
                                                                                       // on
                                                                                       // 3/10/2017
    public final String dodValidCert1UPN = "9301000121@mil";
    public final Calendar dodCertExpireDate = new GregorianCalendar(
                    2017, 3, 10);
    // X509 mapp strings for "bill.preston.s.9301000121.email_sig.cer"
    public final String x509_PREFIX = "X509:";
    public final String dodValidCert1_I = "<I>C=US,O=U.S. Government,OU=DoD,OU=PKI,CN=DOD JITC EMAIL CA-29";
    public final String dodValidCert1_S = "<S>C=US,O=U.S. Government,OU=DoD,OU=PKI,OU=DARPA,CN=Bill.Preston.S.9301000121";
    public final String dodValidCert1_SR = "<SR>433816";
    public final String dodValidCert1_SKI = "<SKI>7efdbcb2b04d1a74eb317e104ca36bf65c7f3d0b";
    public final String dodValidCert1_SHA1_PUKEY = "<SHA1-PUKEY>b1beaf825f9b57111dfa313402321355df693d2e";
    public final String dodValidCert1_RFC822 = "<RFC822>bill.s.preston@dod.mil";

    // More DOD test certs store. In P12 format
    //
    private final String dodRevokedStore = "RevokedID.p12";
    private final String dodRevokedCertAlias = "certificate.revoked.9000080969's u.s. government id";
    private final String dodExpiredStore = "ExpiredID.p12";
    private final String dodExpiredCertAlias = "Certificate.Expired.9000080968's U.S. Government ID";
    private final String dodValidStore = "ValidID.p12";
    private final String dodValidCertAlias = "CERTIFICATE.VALID.9000080970's U.S. Government ID";
    private final String dodStorePass = "password";
    private Properties testProps;


    public KeyStore getTrustStore() {

        KeyStore ts = loadKeyStore(caStoreName, storePass);
        return ts;
    }

    // load "JKS" keystore
    private KeyStore loadKeyStore(String keyStoreFile, String pass) {
        return loadKeyStoreWithType(keyStoreFile, pass, "JKS");
    }

    private KeyStore loadKeyStoreWithType(String keyStoreFile, String pass,
                    String storeType) {
        KeyStore ks = null;
        try {
            ks = KeyStore.getInstance(storeType);
            ks.load(getClass().getClassLoader().getResourceAsStream(
                            keyStoreFile), pass.toCharArray());
        } catch (FileNotFoundException fnfe) {
            throw new IllegalArgumentException(String.format(
                            "keystore file [%s] not found", keyStoreFile), fnfe);
        } catch (IOException ioe) {
            String errMsg = ioe.getCause() instanceof UnrecoverableKeyException ? "Wrong keystore password"
                            : "";
            throw new IllegalArgumentException(errMsg, ioe);
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }
        return ks;
    }
    public URL getDODResponderUrl() throws MalformedURLException {

        return new URL(dodOCSPResponder);
    }

    /**
     * @return selfsigned valid cert
     * @throws KeyStoreException
     */
    public X509Certificate[] getValidCert() throws KeyStoreException {
        KeyStore ks = loadKeyStore(clientStoreName, storePass);
        if (!ks.isCertificateEntry(validCertAlias)) {
            throw new KeyStoreException("Cert not in the store");
        }
        X509Certificate leaf = (X509Certificate) ks
                        .getCertificate(validCertAlias);
        X509Certificate[] certs = { leaf };
        return certs;
    }

    /**
     * @return "Certificate.Revoked.9000080969" chain including intermediate CA and root CA
     * @throws KeyStoreException
     */
    public Certificate[] getDoDRevokedCert() throws KeyStoreException {
        KeyStore ks = loadKeyStoreWithType(this.dodRevokedStore,
                        this.dodStorePass, "PKCS12");
        Certificate[] certs = ks.getCertificateChain(this.dodRevokedCertAlias);
        return certs;
    }

    public URL getCRLLocalCacheURL(String cacheName) throws IOException {
        Enumeration<URL> urls = getClass().getClassLoader().getResources(
                        cacheName);
        Validate.notNull(urls, "CRLFile is not null!");
        return urls.nextElement();
    }

    public X509Certificate[] getDodValidCert1() throws KeyStoreException {
        KeyStore ks = loadKeyStore(clientStoreName, storePass);
        if (!ks.isCertificateEntry(validDodCertAlias1)) {
            throw new KeyStoreException("Cert not in the store");
        }
        X509Certificate leaf = (X509Certificate) ks
.getCertificate(validDodCertAlias1);
        X509Certificate[] certs = { leaf };
        return certs;

    }

    /**
     * @return "Certificate.Valid.9000080970" chain including intermediate CA and root CA
     * @throws KeyStoreException
     */
    public Certificate[] getDoDValidCertChain() throws KeyStoreException {
        KeyStore ks = loadKeyStoreWithType(this.dodValidStore,
                        this.dodStorePass, "PKCS12");
        Certificate[] certs = ks.getCertificateChain(this.dodValidCertAlias);
        return certs;
    }

    public static ClientCertPolicy intializeCertPolicy() {
        ClientCertPolicy policy = new ClientCertPolicy
(true, // rev check
                        false, // ocsp
                        true, // failover
                        false, // ocsp nonce
                        null, // ocsp responder url
                        null, // signing cert of ocsp responder,
                        true, // use CRLDP
                        null, // crl url,
                        0, //
                        null // cert policy filters
        );
        return policy;

    }

    public PrivateKey getTenantCredentialPrivateKey(String keyAlias) throws Exception
    {
        Properties props = getTestProperties();

        KeyStore ks = loadKeyStore(props.getProperty(STS_STORE_JKS), props.getProperty(STS_STORE_PASS));
        return (PrivateKey) ks.getKey(keyAlias, props.getProperty(STS_STORE_PASS).toCharArray());
    }

    public Certificate[] getTenantCredentialCert(String certAlias) throws Exception
    {
        Properties props = getTestProperties();

        KeyStore ks = loadKeyStore(props.getProperty(STS_STORE_JKS), props.getProperty(STS_STORE_PASS));
        return ks.getCertificateChain(certAlias);
    }

    public PrivateKey getTenantCredentialCustomPrivateKey(String keyAlias) throws Exception {
        Properties props = getTestProperties();

        KeyStore ks = loadKeyStore(props.getProperty(CUSTOM_STORE_JKS), props.getProperty(CUSTOM_STORE_PASS));
        return (PrivateKey) ks.getKey(keyAlias, props.getProperty(CUSTOM_KEY_PASS).toCharArray());
    }

    public Certificate[] getTenantCredentialCustomCert(String certAlias) throws Exception
    {
        Properties props = getTestProperties();

        KeyStore ks = loadKeyStore(props.getProperty(CUSTOM_STORE_JKS), props.getProperty(CUSTOM_STORE_PASS));
        return ks.getCertificateChain(certAlias);
    }

    private synchronized Properties getTestProperties() throws Exception {
        if (testProps == null) {
           testProps = new Properties();
           testProps.load(getClass().getResourceAsStream("/config.properties"));
        }

        return testProps;
     }

    public List<String> getDodValidCert1X509_IS() throws KeyStoreException {

        String IS = x509_PREFIX + dodValidCert1_I + dodValidCert1_S;
        List<String> result = new ArrayList<String>();
        result.add(IS);
        return result;
    }

    public List<String> getDodValidCert1X509_ISR() throws KeyStoreException {

        String ISR = x509_PREFIX + dodValidCert1_I + dodValidCert1_SR;
        List<String> result = new ArrayList<String>();
        result.add(ISR);
        return result;
    }
}
