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

import java.security.KeyStoreException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IdmClientCertificateParsingException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.server.clientcert.IdmClientCertificateValidator;

public class IdmClientCertificateValidatorTest {

    static ClientCertTestUtils testUtil = new ClientCertTestUtils();

    @Test
    public void testUPNExtraction() throws KeyStoreException, IdmClientCertificateParsingException,
                    InvalidPrincipalException, IDMException {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        X509Certificate[] certs = testUtil.getDodValidCert1();
        ClientCertPolicy certPolicy = new ClientCertPolicy();
        certPolicy.setTrustedCAs(certs);

        String upn = IdmClientCertificateValidator.extractUPN(certs[0]);
        assertTrue(upn.equals(testUtil.dodValidCert1UPN));
    }

    @Test
    public void testValidateAltSec_IS() throws KeyStoreException {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        X509Certificate[] certs = testUtil.getDodValidCert1();
        List<String> altSecIdentities = testUtil.getDodValidCert1X509_IS();

        try {
            IdmClientCertificateValidator.ValidateAltSec(certs[0], altSecIdentities);
        }
        catch (Exception e) {
            fail();
        }
    }

    @Test
    public void testValidateAltSec_ISR() throws KeyStoreException {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        X509Certificate[] certs = testUtil.getDodValidCert1();
        List<String> altSecIdentities = testUtil.getDodValidCert1X509_ISR();

        try {
            IdmClientCertificateValidator.ValidateAltSec(certs[0], altSecIdentities);
        }
        catch (Exception e) {
            fail();
        }
    }

    @Test
    public void testValidateAltSec_SKI() throws KeyStoreException {
        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        X509Certificate[] certs = testUtil.getDodValidCert1();
        List<String> altSecIdentities = new ArrayList<String>();
        altSecIdentities.add(testUtil.x509_PREFIX + testUtil.dodValidCert1_SKI);

        try {
            IdmClientCertificateValidator.ValidateAltSec(certs[0], altSecIdentities);
        }
        catch (Exception e) {
            fail();
        }

    }

    @Test
    public void testValidateAltSec_S() throws KeyStoreException {
        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        X509Certificate[] certs = testUtil.getDodValidCert1();
        List<String> altSecIdentities = new ArrayList<String>();
        altSecIdentities.add(testUtil.x509_PREFIX + testUtil.dodValidCert1_S);

        try {
            IdmClientCertificateValidator.ValidateAltSec(certs[0], altSecIdentities);
        }
        catch (Exception e) {
            fail();
        }
    }

    @Ignore
    @Test
    public void testValidateAltSec_SHA1_PUKEY() throws KeyStoreException {
        // TODO PR1744196 unclear how to output pubkey from cert and hash it.
        // This is failing for some reason.
        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }

        X509Certificate[] certs = testUtil.getDodValidCert1();
        List<String> altSecIdentities = new ArrayList<String>();
        altSecIdentities.add(testUtil.x509_PREFIX + testUtil.dodValidCert1_SHA1_PUKEY);

        try {
            IdmClientCertificateValidator.ValidateAltSec(certs[0], altSecIdentities);
        }
        catch (Exception e) {
            fail();
        }
    }

    @Test
    public void testValidateAltSec_RFC822() throws KeyStoreException {
        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }

        X509Certificate[] certs = testUtil.getDodValidCert1();
        List<String> altSecIdentities = new ArrayList<String>();
        altSecIdentities.add(testUtil.x509_PREFIX + testUtil.dodValidCert1_RFC822);

        try {
            IdmClientCertificateValidator.ValidateAltSec(certs[0], altSecIdentities);
        }
        catch (Exception e) {
            fail();
        }
    }

}
