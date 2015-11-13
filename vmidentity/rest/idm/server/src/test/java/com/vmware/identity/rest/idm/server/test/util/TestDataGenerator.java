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
package com.vmware.identity.rest.idm.server.test.util;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;

/**
 * Generates test data. The sole purpose of this class is only for unit testing
 *
 * @version REST-IDM 1.0
 * @created Jan 7, 2015
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class TestDataGenerator {

    // PrincipalId related test constants
    private static final String TEST_PRINCIPAL_NAME = "testPrincipalName";
    private static final String TEST_PRINICPAL_DOMAIN = "test.vmware";

    // PersonUser related test constants
    private static final boolean TEST_DEFAULT_DISABLED = false;
    private static final boolean TEST_DEFAULT_LOCKED = false;

    // Group related test constants
    private static final String TEST_GROUP_DESC = "Group created for purpose of unit testing";
    private static final String TEST_GROUP_ALIAS_DOMAIN = "testAlias.local";
    private static final String TEST_GROUP_ALIAS_NAME = "testAliasGroup";
    private static final String TEST_GROUP_NAME = "testGroup";

    // Certificate related test constants
    private static final String TEST_CERT_LOC = "src/test/resources/test_cert.pem";

    /**
     * Generate IDM groups for testing
     *
     * @param numOfGroups Total number of idm groups to be generated
     * @return
     */
    public static Set<Group> getIdmGroups(int numOfGroups) {
        Set<Group> groups = new HashSet<Group>();
        for (int i = 0; i < numOfGroups; i++) {
            PrincipalId principal = new PrincipalId(TEST_PRINCIPAL_NAME + i, TEST_PRINICPAL_DOMAIN + i);
            GroupDetail groupDetail = new GroupDetail(TEST_GROUP_DESC);
            Group group = new Group(principal, groupDetail);
            groups.add(group);
        }
        return groups;
    }

    public static Set<Group> getIdmGroups(String principalName, int numOfGroups) {
        Set<Group> groups = new HashSet<Group>();
        for (int i = 0; i < numOfGroups; i++) {
            PrincipalId principal = new PrincipalId(principalName + i, TEST_PRINICPAL_DOMAIN + i);
            GroupDetail groupDetail = new GroupDetail(TEST_GROUP_DESC);
            Group group = new Group(principal, groupDetail);
            groups.add(group);
        }
        return groups;
    }

    /**
     * Generate IDM person users for testing
     *
     * @param numOfUsers Total number of person users to be generated.
     * @return
     */
    public static Set<PersonUser> getIdmPersonUsers(int numOfUsers) {
        Set<PersonUser> users = new HashSet<PersonUser>();
        for (int i = 0; i < numOfUsers; i++) {
            PersonDetail personDetail = new PersonDetail.Builder().build();
            PrincipalId principal = new PrincipalId(TEST_PRINCIPAL_NAME + i, TEST_PRINICPAL_DOMAIN);
            PersonUser person = new PersonUser(principal, personDetail, TEST_DEFAULT_DISABLED, TEST_DEFAULT_LOCKED);
            users.add(person);
        }
        return users;
    }

    /**
     * Generate solution users for testing
     *
     * @param numOfUsrs Total number of solution users to be generated.
     * @return
     */
    public static Set<SolutionUser> getIdmSolutionUsers(int numOfUsrs) {
        Set<SolutionUser> users = new HashSet<SolutionUser>();
        for (int i = 0; i < numOfUsrs; i++) {
            X509Certificate certificate = null; // TODO : Add test certificate later if needed.
            PrincipalId principal = new PrincipalId(TEST_PRINCIPAL_NAME + i, TEST_PRINICPAL_DOMAIN);
            SolutionDetail detail = new SolutionDetail(certificate);
            SolutionUser solutionUser = new SolutionUser(principal, detail, TEST_DEFAULT_DISABLED);
            users.add(solutionUser);
        }
        return users;
    }


    public static Collection<IIdentityStoreData> getTestSystemIDP(String systemTenantName) {
        Collection<IIdentityStoreData> idps = new ArrayList<IIdentityStoreData>();
        idps.add(IdentityStoreData.CreateSystemIdentityStoreData(systemTenantName));
        return idps;
    }

    public static TenantCredentialsDTO getTestTenantCredentialsDTO() throws IOException, GeneralSecurityException {
        List<CertificateDTO> trustedCerts = new ArrayList<CertificateDTO>();
        trustedCerts.add(CertificateDTO.builder().withEncoded(getTestPEMCert(TEST_CERT_LOC)).build());

        return TenantCredentialsDTO.builder().withCertificates(trustedCerts).withPrivateKey(getTestPrivateKeyDTO()).build();
    }

    public static String getTestPEMCert(String certLocation) throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(certLocation));
        return new String(encoded, Charset.defaultCharset());
    }

    public static PrivateKeyDTO getTestPrivateKeyDTO() throws GeneralSecurityException {
        String algorithm = "RSA";
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance(algorithm);
        keyGen.initialize(1024);
        KeyPair keypair = keyGen.genKeyPair();
        PrivateKey privateKey = keypair.getPrivate();
        return new PrivateKeyDTO(privateKey);
    }
}
