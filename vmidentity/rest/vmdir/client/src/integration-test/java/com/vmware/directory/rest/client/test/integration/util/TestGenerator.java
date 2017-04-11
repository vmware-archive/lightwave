package com.vmware.directory.rest.client.test.integration.util;

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

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.util.ArrayList;
import java.util.List;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.test.util.CertificateGenerator;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;

public class TestGenerator {

    private static final String TEST_TENANT_NAME = "test.integration";
    private static final String TEST_TENANT_LONG_NAME = "test.integration.client";
    private static final String TEST_TENANT_KEY = "(SNP?Vm**[#_ca=";

    private static final String TEST_TENANT_USERNAME = "Administrator";
    private static final String TEST_TENANT_PASSWORD = "Foo!23";

    private static final String TEST_CERT_DN_0 = "C=US, ST=WA, L=Bellevue, O=VMware, OU=SSO, CN=test.integration";
    private static final String TEST_CERT_DN_1 = "C=US, ST=WA, L=Bellevue, O=VMware, OU=SSO, CN=test.integration.client";

    private static KeyPair keyPair;

    public static KeyPair getKeyPair() {
        if (keyPair == null) {
            keyPair = KeyPairUtil.getKeyPair();
        }

        return keyPair;
    }
    public static GroupDTO generateGroup(String name, String domain, String description) {
        return GroupGenerator.generateGroup(name, domain, description);
    }

    public static SolutionUserDTO generateSolutionUser(String name, String domain, String description, CertificateDTO certificate) {
        return SolutionUserGenerator.generateSolutionUser(name, domain, description, certificate);
    }

    public static UserDTO generateUser(String name, String domain, String description) {
        return UserGenerator.generateUser(name, domain, description);
    }

    private static List<CertificateDTO> getTrustedCertificates() throws GeneralSecurityException, IOException {
        List<CertificateDTO> certs = new ArrayList<CertificateDTO>();
        certs.add(generateCertificate(TEST_CERT_DN_0));
        certs.add(generateCertificate(TEST_CERT_DN_1));

        return certs;
    }

    public static CertificateDTO generateCertificate(String dn) throws GeneralSecurityException, IOException {
        return new CertificateDTO(CertificateGenerator.generateSelfSignedCertificate(getKeyPair(), CertificateGenerator.AlgorithmName.SHA256_WITH_RSA, dn));
    }
}

