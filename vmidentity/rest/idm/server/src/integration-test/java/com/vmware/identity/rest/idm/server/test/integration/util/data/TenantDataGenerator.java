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
package com.vmware.identity.rest.idm.server.test.integration.util.data;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;

/**
 * Data provider for tenant resource integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class TenantDataGenerator {

    public static TenantDTO createTestTenantDTO(String tenantName, String longName, String key, String adminUsername, String adminPwd) throws IOException, GeneralSecurityException {
        return TenantDTO.builder()
                        .withName(tenantName)
                        .withLongName(longName)
                        .withKey(key)
                        .withCredentials(getTestTenantCredentialsDTO())
                        .withUsername(adminUsername)
                        .withPassword(adminPwd)
                        .build();
    }

    public static TenantCredentialsDTO getTestTenantCredentialsDTO() throws IOException, GeneralSecurityException {
        List<CertificateDTO> trustedCerts = new ArrayList<CertificateDTO>();
        trustedCerts.add(CertificateDTO.builder().withEncoded(CertificateDataGenerator.getDefaultTestPEMCert()).build());
        trustedCerts.add(CertificateDTO.builder().withEncoded(CertificateDataGenerator.getTestPEMCert("src/integration-test/resources/crish_cert.pem")).build());
        return TenantCredentialsDTO.builder().withCertificates(trustedCerts).withPrivateKey(getTestPrivateKeyDTO()).build();
    }

    public static PrivateKeyDTO getTestPrivateKeyDTO() throws GeneralSecurityException {
        String algorithm = "RSA";
        // Generate a 1024-bit Digital Signature Algorithm (DSA) key pair
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance(algorithm);
        keyGen.initialize(1024);
        KeyPair keypair = keyGen.genKeyPair();
        PrivateKey privateKey = keypair.getPrivate();

        return new PrivateKeyDTO(privateKey);
    }

}
