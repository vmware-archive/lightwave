/*
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
 */
package com.vmware.identity.rest.idm.client.test.integration;

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertContainsCertificates;
import static org.junit.Assert.assertFalse;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.test.integration.util.TestClientFactory;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateGranularity;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;

public class CertificateResourceIT extends IntegrationTestBase {

    private static String TEST_CERT_DN = "C=US, ST=WA, L=Bellevue, O=VMware, OU=SSO, CN=junkcert";
    private static CertificateDTO testCert;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);

        testCert = TestGenerator.generateCertificate(TEST_CERT_DN);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testSet() throws GeneralSecurityException, IOException, HttpException, ClientException {
        List<CertificateDTO> certificates = new ArrayList<CertificateDTO>();
        certificates.add(testCert);
        certificates.add(TestGenerator.generateCertificate(TEST_CERT_DN + "1"));

        TenantCredentialsDTO credentials = new TenantCredentialsDTO.Builder()
            .withPrivateKey(new PrivateKeyDTO(TestGenerator.getKeyPair().getPrivate()))
            .withCertificates(certificates)
            .build();

        testAdminClient.certificate().setCredentials(testTenant.getName(), credentials);

        List<CertificateChainDTO> chains = testAdminClient.certificate().get(testTenant.getName(), CertificateScope.TENANT, CertificateGranularity.CHAIN);
        assertFalse(chains.isEmpty());

        List<CertificateDTO> retrievedCerts = chains.get(chains.size() - 1).getCertificates();
        assertContainsCertificates(credentials.getCertificates(), retrievedCerts);

        // TODO If the server used the history of the signing certificates, it could still validate old tokens...
        // Update the client so we have a valid token
        testAdminClient = TestClientFactory.createClient(properties.getHost(),
                testTenant.getName(),
                testTenant.getUsername(),
                testTenant.getPassword());
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<CertificateChainDTO> chains = testAdminClient.certificate().get(testTenant.getName(), CertificateScope.TENANT, CertificateGranularity.CHAIN);

        assertFalse(chains.isEmpty());
        List<CertificateDTO> certs = chains.get(0).getCertificates();

        assertContainsCertificates(testTenant.getCredentials().getCertificates(), certs);
    }

}
