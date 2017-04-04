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
package com.vmware.identity.rest.idm.client.test.integration.util;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.test.util.CertificateGenerator;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.UserDTO;

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

    public static TenantDTO generateTenant() throws IOException, GeneralSecurityException {
        TenantDTO.Builder builder = new TenantDTO.Builder()
            .withName(TEST_TENANT_NAME)
            .withLongName(TEST_TENANT_LONG_NAME)
            .withKey(TEST_TENANT_KEY)
            .withCredentials(generateTenantCredentials())
            .withUsername(UPNUtil.buildUPN(TEST_TENANT_USERNAME, TEST_TENANT_NAME))
            .withPassword(TEST_TENANT_PASSWORD);

        return builder.build();
    }

    public static ExternalIDPDTO generateExternalIDP(CertificateDTO certificate) throws GeneralSecurityException, IOException {
        return ExternalIdpGenerator.generateExternalIDP(certificate);
    }

    public static CertificateDTO generateCertificate(String dn) throws GeneralSecurityException, IOException {
        return new CertificateDTO(CertificateGenerator.generateSelfSignedCertificate(getKeyPair(), CertificateGenerator.AlgorithmName.SHA256_WITH_RSA, dn));
    }

    public static GroupDTO generateGroup(String name, String domain, String description) {
        return GroupGenerator.generateGroup(name, domain, description);
    }

    public static OIDCClientMetadataDTO generateOIDCClientMetadata() {
        return OidcClientGenerator.generateOIDCClientMetadata();
    }

    public static ResourceServerDTO generateResourceServer() {
        return ResourceServerGenerator.generateResourceServer();
    }

    public static RelyingPartyDTO generateRelyingParty(CertificateDTO certificate) {
        return RelyingPartyGenerator.generateRelyingParty(certificate);
    }

    public static SolutionUserDTO generateSolutionUser(String name, String domain, String description, CertificateDTO certificate) {
        return SolutionUserGenerator.generateSolutionUser(name, domain, description, certificate);
    }

    public static UserDTO generateUser(String name, String domain, String description) {
        return UserGenerator.generateUser(name, domain, description);
    }

    public static PrincipalDTO generatePrincipal(String name, String domain) {
        return new PrincipalDTO(name, domain);
    }

    public static com.vmware.directory.rest.common.data.UserDTO generateVmdirUser(String name, String domain, String description) {
        return UserGenerator.generateVmdirUser(name, domain, description);
    }

    private static TenantCredentialsDTO generateTenantCredentials() throws GeneralSecurityException, IOException {
        return new TenantCredentialsDTO.Builder()
            .withCertificates(getTrustedCertificates())
            .withPrivateKey(new PrivateKeyDTO(getKeyPair().getPrivate()))
            .build();
    }

    private static List<CertificateDTO> getTrustedCertificates() throws GeneralSecurityException, IOException {
        List<CertificateDTO> certs = new ArrayList<CertificateDTO>();
        certs.add(generateCertificate(TEST_CERT_DN_0));
        certs.add(generateCertificate(TEST_CERT_DN_1));

        return certs;
    }

}
