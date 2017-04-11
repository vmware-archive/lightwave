/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.idm.server;

import static org.junit.Assert.fail;

import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Hashtable;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.CertRevocationStatusUnknownException;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.server.clientcert.IdmCertificatePathValidator;
import com.vmware.identity.idm.server.clientcert.IdmCrlCache;
import com.vmware.identity.idm.server.clientcert.TenantCrlCache;

public class TenantCrlCacheTest {
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

    @Test
    public void testCacheCreated() {

        Calendar currentDate = new GregorianCalendar();
        if (currentDate.after(testUtil.dodCertExpireDate)) {
            return;
        }
        try {
            System.out.print("running testCacheCreated");
            KeyStore trustStore = testUtil.getTrustStore();
            ClientCertPolicy certPolicy = ClientCertTestUtils.intializeCertPolicy();
            X509Certificate[] certs = testUtil.getDodValidCert1();
            certPolicy.setRevocationCheckEnabled(true);
            certPolicy.setUseCertCRL(true);

            IdmCertificatePathValidator validator = new IdmCertificatePathValidator(
                            trustStore, certPolicy, ClientCertTestUtils.tenant1,null);

            //creating cache
            validator.validate(certs[0], new Hashtable<String, String>());

            //accessing cache
            IdmCrlCache crlCache = TenantCrlCache.get().get(ClientCertTestUtils.tenant1);

            Assert.assertNotNull(crlCache);
            Assert.assertTrue(crlCache.containsKey(testUtil.dodCRLDistributionPointEMAILCA_29));
            return;
        } catch (CertRevocationStatusUnknownException e) {
            fail("revocation check status unkown");
        } catch (Exception e) {
            fail("unexpected error in validating cert");
        }

    }

 }
