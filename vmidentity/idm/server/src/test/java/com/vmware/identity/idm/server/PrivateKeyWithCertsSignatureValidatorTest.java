/*
 *
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
 *
 */
package com.vmware.identity.idm.server;

import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;

import junit.framework.Assert;

import org.junit.Test;

public class PrivateKeyWithCertsSignatureValidatorTest {


    private static final String customKeyAlias = "le-v2webserver-a732d6b5-db80-435a-b3d2-e360a0392976";
    private static final String keyAlias = "stskey";

    @Test
    public void test_ValidatePrivateKeyWithCert_VMCACerts() {
        ClientCertTestUtils utils = new ClientCertTestUtils();
        PrivateKey key;
        try {
            key = utils.getTenantCredentialPrivateKey(keyAlias);
            Certificate[] cert = utils.getTenantCredentialCert(keyAlias);
            new PrivateKeyWithCertificateSignatureValidator().validate(key, cert[0]);
        } catch (Exception e) {
            Assert.fail(e.getMessage());
        }
    }

    @Test
    public void test_ValidatePrivateKeyWithCert_CustomCerts() {
        ClientCertTestUtils utils = new ClientCertTestUtils();
        PrivateKey key;
        try {
            key = utils.getTenantCredentialCustomPrivateKey(customKeyAlias);
            Certificate[] cert = utils.getTenantCredentialCustomCert(customKeyAlias);
            new PrivateKeyWithCertificateSignatureValidator().validate(key, cert[0]);
        } catch (Exception e) {
            Assert.fail(e.getMessage());
        }
    }

    @Test(expected = IllegalArgumentException.class)
    public void test_ValidatePrivateKeyWithCert_NoMatch() throws Exception {
        ClientCertTestUtils utils = new ClientCertTestUtils();
        PrivateKey key;
        try {
            key = utils.getTenantCredentialPrivateKey(keyAlias);
            Certificate[] cert = utils.getTenantCredentialCustomCert(customKeyAlias);
            new PrivateKeyWithCertificateSignatureValidator().validate(key, cert[0]);
        } catch (UnrecoverableKeyException | KeyStoreException | NoSuchAlgorithmException e) {
            Assert.fail(e.getMessage());
        }
    }
}
