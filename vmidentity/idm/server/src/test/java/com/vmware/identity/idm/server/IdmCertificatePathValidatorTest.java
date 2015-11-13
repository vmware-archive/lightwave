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

import static org.junit.Assert.*;

import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Arrays;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import com.vmware.identity.idm.CertRevocationStatusUnknownException;
import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.IdmCertificateRevokedException;
import com.vmware.identity.idm.server.clientcert.IdmCertificatePathValidator;

public class IdmCertificatePathValidatorTest {

    private static ClientCertTestUtils testUtil = new ClientCertTestUtils();

    /**
     * Initializing for test
     */
    @Before
    public void setUp() throws Exception {
        System.setProperties(new ThreadLocalProperties(System.getProperties()));
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     *
     * @throws CertificateRevocationCheckException
     * @throws KeyStoreException
     */
    @Test
    public final void testEnableRevCheckOff()
                    throws CertificateRevocationCheckException,
                    KeyStoreException {

        KeyStore trustStore = testUtil.getTrustStore();
        ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
        certPolicy.setRevocationCheckEnabled(false);

        X509Certificate[] certs = testUtil.getValidCert();
        IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                        trustStore, certPolicy);

        // rev check off
        try {
            validator.validate(certs[0]);
        }
        catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

    /**
     * CRL test with wrong CRL cheche. Using selfsigned cert with DOD CRL cache.
     *
     */
    @Test
    public void testCRLOverrideWithWrongCRLFile() {

        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getValidCert();

            // rev check on
            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseCertCRL(true);
            certPolicy.setCRLUrl(testUtil
                            .getCRLLocalCacheURL(testUtil.dodCRLCacheROOTCA2));

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(certs[0]);
            fail("CRL check should not succeed with wrong CRL file");
        } catch (CertRevocationStatusUnknownException e) {
            return;
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }


    /**
     * CRL test with wrong CRL cheche. Using selfsigned cert with DOD CRL cache.
     * Uses override only without in-cert CRLDP.
     *
     */
    @Test
    public void testCRLOverrideWithWrongCRLFile2() {

        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getValidCert();

            // rev check on
            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseCertCRL(false);
            certPolicy.setCRLUrl(testUtil
                            .getCRLLocalCacheURL(testUtil.dodCRLCacheROOTCA2));

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(certs[0]);
            fail("CRL check should not succeed with wrong CRL file");
        } catch (CertRevocationStatusUnknownException e) {
            return;
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

    /**
     * CRLDP test with DOD CRLDP. Using dod soft cert Note: turn this off as
     * CRLDP is unpredicatable
     *
     */
    @Test
    public void testDodChainRevocationOff() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(false);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(trustStore,
                            certPolicy);

            validator.validate(certs[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            // currently it could throw to here
            fail("unexpected error in validating cert");
        }

    }

    /**
     * CRLDP test with DOD CRLDP. Using dod soft cert Note: turn this off as
     * CRLDP is unpredicatable
     *
     */
    @Test
    public void testDodCRLDP() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseCertCRL(true);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(certs[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            // currently it could throw to here
            fail("unexpected error in validating cert");
        }

    }

    /**
     * CRLDP test with local DOD CRL cache. Using dod soft cert:
     *  note: we have to turn on in-cert CRLDP as the cache is only for one end-cert.
     */
    @Test
    public void testDodCRLCache() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseCertCRL(true);
            certPolicy.setCRLUrl(testUtil.getCRLLocalCacheURL(testUtil.dodCRLCacheEMAILCA_29));

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(certs[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

    /**
     * OCSP test using dod soft cert:
     *
     */
    @Test
    public void testDodOCSP() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseOCSP(true);
            certPolicy.setUseCertCRL(false);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(certs[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

    /**
     * Certificate policy filter test
     *
     */
    @Test
    public void testPolicyFilterSucceed() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils
                    .intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseOCSP(true);
            certPolicy.setUseCertCRL(false);
            String[] allowedPolicies = { "2.16.840.1.101.2.1.11.9",
                    "2.16.840.1.101.2.1.11.19" };
            certPolicy.setOIDs(allowedPolicies);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                    trustStore, certPolicy);

            validator.validate(certs[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

    /**
     * Certificate policy filter test
     *
     */
    @Test
    public void testPolicyFilterFail() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils
                    .intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(false);
            certPolicy.setUseOCSP(false);
            certPolicy.setUseCertCRL(true);
            String[] allowedPolicies = { "2.16.840.1.101.2.1.11.90",
                    "2.16.840.1.101.2.1.11.29" };
            certPolicy.setOIDs(allowedPolicies);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                    trustStore, certPolicy);
            validator.validate(certs[0]);
            fail("unexpected success in validating cert");
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            return;
        }

    }

    /**
     * OCSP test using explicit responder URL:
     *
     */
    @Test
    public void testDodOCSPWithResponder() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();

            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseOCSP(true);
            certPolicy.setUseCRLAsFailOver(true);
            certPolicy.setOCSPUrl(testUtil.getDODResponderUrl());

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(certs[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }
    }

    /**
     * OCSP test using explicit responder URL:
     */
    @Test
    public void testDodRevoked() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            Certificate[] certs = testUtil.getDoDRevokedCert();

            X509Certificate[] x509Certificates = Arrays.copyOf(certs,
                            certs.length, X509Certificate[].class);
            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseCertCRL(true);
            certPolicy.setUseOCSP(true);
            certPolicy.setOCSPUrl(testUtil.getDODResponderUrl());

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(x509Certificates[0]);
            fail("unexpected success in validating cert");
        } catch (IdmCertificateRevokedException e) {
            return;
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

    /**
     * DOD valid soft cert chain testing:
     */
    @Test
    public void testDodValidCertChain() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            Certificate[] certs = testUtil.getDoDValidCertChain();

            X509Certificate[] x509Certificates = Arrays.copyOf(certs,
                            certs.length, X509Certificate[].class);
            certPolicy.setRevocationCheckEnabled(true);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy);

            validator.validate(x509Certificates[0]);
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

}
