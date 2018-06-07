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

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertTenantEquals;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;


import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.ForbiddenException;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.client.test.integration.util.TestClientFactory;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.client.test.integration.util.UserGenerator;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;
import com.vmware.identity.rest.idm.data.PasswordPolicyDTO;
import com.vmware.identity.rest.idm.data.OperatorsAccessPolicyDTO;
import com.vmware.identity.rest.idm.data.PrincipalIdentifiersDTO;
import com.vmware.identity.rest.idm.data.SecurityDomainDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;

public class TenantResourceIT extends IntegrationTestBase {

    private static UserDTO testUserWithPriviledge;
    private static UserDTO testUserWithoutPriviledge;
    private static UserDTO testUser;
    private static UserDTO testUserCase;
    private static GroupDTO testGroup;
    private static SolutionUserDTO testSolutionUser;
    private static String systemTenant;
    private static String testTenantName1 = TenantResourceIT.class.getSimpleName() + ".tenant1";
    private static String testTenantName2 = TenantResourceIT.class.getSimpleName() + ".tenant2";

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);
        systemTenant = properties.getSystemTenant();
        testUserWithPriviledge = systemAdminVmdirClient.user().create(systemTenant, TestGenerator.generateVmdirUser("testUser1", systemTenant, "Test user with tenant operator privilege."));
        List<String> members = Arrays.asList(UPNUtil.buildUPN(testUserWithPriviledge.getName() ,testUserWithPriviledge.getDomain()));
        systemAdminVmdirClient.group().addMembers(systemTenant, "TenantOperators", systemTenant, members, MemberType.USER);
        testUserWithoutPriviledge = systemAdminVmdirClient.user().create(systemTenant, TestGenerator.generateVmdirUser("testUser2", systemTenant, "Test user without tenant operator privilege."));
        testUserCase = systemAdminVmdirClient.user().create(systemTenant, TestGenerator.generateVmdirUser("testUser3", systemTenant, "Test user without tenant operator privilege."));

        testUser = systemAdminVmdirClient.user().create(systemTenant, TestGenerator.generateVmdirUser(
                "testUser", systemTenant, "Test user for tenant resource IT."));
        testGroup = systemAdminVmdirClient.group().create(systemTenant, TestGenerator.generateVmdirGroup(
                "testGroup", systemTenant, "Test group for tenant resource IT."));
        testSolutionUser = systemAdminVmdirClient.solutionUser().create(systemTenant,
                TestGenerator.generateVmdirSolutionUser("testSolutionUser", systemTenant,
                        "Test solution user for tenant resource IT.",
                        TestGenerator.generateCertificate("C=US, ST=WA, L=Bellevue, O=VMware, OU=SSO, CN=junkcert")));
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
        systemAdminVmdirClient.user().delete(systemTenant, testUserWithPriviledge.getName(), testUserWithPriviledge.getDomain());
        systemAdminVmdirClient.user().delete(systemTenant, testUserWithoutPriviledge.getName(), testUserWithoutPriviledge.getDomain());
        systemAdminVmdirClient.user().delete(systemTenant, testUserCase.getName(), testUserCase.getDomain());
        systemAdminVmdirClient.user().delete(systemTenant, testUser.getName(), testUser.getDomain());
        systemAdminVmdirClient.group().delete(systemTenant, testGroup.getName(), testGroup.getDomain());
        systemAdminVmdirClient.solutionUser().delete(systemTenant, testSolutionUser.getName());
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        TenantDTO got = testAdminClient.tenant().get(testTenant.getName());
        assertTenantEquals(testTenant, got);
    }

    @Test
    public void testGetConfig() throws ClientProtocolException, HttpException, ClientException, IOException {
        TenantConfigurationDTO config = testAdminClient.tenant().getConfig(testTenant.getName());

        assertNotNull(config);
        assertNotNull(config.getAuthenticationPolicy());
        assertNotNull(config.getBrandPolicy());
        assertNotNull(config.getLockoutPolicy());
        assertNotNull(config.getPasswordPolicy());
        assertNotNull(config.getTokenPolicy());
    }

    @Test
    public void testUpdateConfig() throws ClientProtocolException, HttpException, ClientException, IOException {
        LockoutPolicyDTO lockout = new LockoutPolicyDTO.Builder()
            .withDescription("Updated description")
            .withAutoUnlockIntervalSec(30L)
            .withFailedAttemptIntervalSec(30L)
            .withMaxFailedAttempts(5)
            .build();
        PasswordPolicyDTO password = new PasswordPolicyDTO.Builder()
            .withDescription("Updated description")
            .withMaxIdenticalAdjacentCharacters(5)
            .withMaxLength(30)
            .withMinAlphabeticCount(3)
            .withMinLength(10)
            .withMinLowercaseCount(1)
            .withMinNumericCount(1)
            .withMinSpecialCharCount(1)
            .withMinUppercaseCount(1)
            .withPasswordLifetimeDays(10)
            .withProhibitedPreviousPasswordCount(3)
            .build();

        OperatorsAccessPolicyDTO operatorsDto = new OperatorsAccessPolicyDTO.Builder()
            .withEnabled(true).build();

        TenantConfigurationDTO config = new TenantConfigurationDTO.Builder()
            .withLockoutPolicy(lockout)
            .withPasswordPolicy(password)
            .withOperatorsAccessPolicy(operatorsDto)
            .build();

        // verify default password expirationd days before update
        assertEquals(new Integer(700), testAdminClient.tenant().getConfig(testTenant.getName()).getPasswordPolicy().getPasswordLifetimeDays());
        TenantConfigurationDTO actual = testAdminClient.tenant().updateConfig(testTenant.getName(), config);

        assertEquals(lockout.getDescription(), actual.getLockoutPolicy().getDescription());
        assertEquals(lockout.getAutoUnlockIntervalSec(), actual.getLockoutPolicy().getAutoUnlockIntervalSec());
        assertEquals(lockout.getFailedAttemptIntervalSec(), actual.getLockoutPolicy().getFailedAttemptIntervalSec());
        assertEquals(lockout.getMaxFailedAttempts(), actual.getLockoutPolicy().getMaxFailedAttempts());
        assertEquals(password.getDescription(), actual.getPasswordPolicy().getDescription());
        assertEquals(password.getMaxIdenticalAdjacentCharacters(), actual.getPasswordPolicy().getMaxIdenticalAdjacentCharacters());
        assertEquals(password.getMaxLength(), actual.getPasswordPolicy().getMaxLength());
        assertEquals(password.getMinAlphabeticCount(), actual.getPasswordPolicy().getMinAlphabeticCount());
        assertEquals(password.getMinLength(), actual.getPasswordPolicy().getMinLength());
        assertEquals(password.getMinLowercaseCount(), actual.getPasswordPolicy().getMinLowercaseCount());
        assertEquals(password.getMinNumericCount(), actual.getPasswordPolicy().getMinNumericCount());
        assertEquals(password.getMinSpecialCharCount(), actual.getPasswordPolicy().getMinSpecialCharCount());
        assertEquals(password.getMinUppercaseCount(), actual.getPasswordPolicy().getMinUppercaseCount());
        assertEquals(password.getPasswordLifetimeDays(), actual.getPasswordPolicy().getPasswordLifetimeDays());
        assertEquals(password.getProhibitedPreviousPasswordCount(), actual.getPasswordPolicy().getProhibitedPreviousPasswordCount());
        assertNotNull(actual.getOperatorsAccessPolicy());
        assertEquals(operatorsDto.getEnabled(), actual.getOperatorsAccessPolicy().getEnabled());
    }

    @Test
    public void testCreateAndDelete() throws Exception {
        IdmClient idmClientWithPrviledge = TestClientFactory.createClient(properties.getHost(),
                systemTenant,
                UPNUtil.buildUPN(testUserWithPriviledge.getName() ,testUserWithPriviledge.getDomain()),
                UserGenerator.PASSWORD);
        idmClientWithPrviledge.tenant().create(TestGenerator.generateTenant(testTenantName1));
        Thread.sleep(15 * 1000);

        IdmClient idmClientWithoutPrviledge = TestClientFactory.createClient(properties.getHost(),
                systemTenant,
                UPNUtil.buildUPN(testUserWithoutPriviledge.getName(), testUserWithoutPriviledge.getDomain()),
                UserGenerator.PASSWORD);
        Exception ex = null;
        try {
            idmClientWithoutPrviledge.tenant().create(TestGenerator.generateTenant(testTenantName2));
        } catch (Exception e) {
            ex = e;
        }
        Assert.assertTrue(ex instanceof ForbiddenException);

        ex = null;
        try {
            idmClientWithoutPrviledge.tenant().delete(testTenantName1);
        } catch (Exception e) {
            ex = e;
        }
        Assert.assertTrue(ex instanceof ForbiddenException);

        idmClientWithPrviledge.tenant().delete(testTenantName1);
    }

    @Test
    public void testFindPrincipalIds() throws Exception {
        List<String> pricipalIds = new ArrayList<>();
        pricipalIds.add("testUser"+ '@' + systemTenant);
        pricipalIds.add("testUser3"+ '@' + systemTenant);
        pricipalIds.add("testGroup" + '@' + systemTenant);
        pricipalIds.add("testSolutionUser" + '@' + systemTenant);

        List<String> expectedPrincipalIds = new ArrayList<>();
        expectedPrincipalIds.add(testUser.getName() + "@" + testUser.getDomain());
        expectedPrincipalIds.add(testUserCase.getName() + "@" + testUserCase.getDomain());
        expectedPrincipalIds.add(testGroup.getDomain() + "\\" + testGroup.getName());
        expectedPrincipalIds.add(testSolutionUser.getName() + "@" + testSolutionUser.getDomain());

        PrincipalIdentifiersDTO principalIdsDTO = new PrincipalIdentifiersDTO.Builder().withIds(pricipalIds).build();
        PrincipalIdentifiersDTO normalizedPricipalIdsDTO = systemAdminClient.tenant().findPrincipalIds(systemTenant, principalIdsDTO);

        assertNotNull(normalizedPricipalIdsDTO);
        assertNotNull(normalizedPricipalIdsDTO.getIds());
        assertEquals(normalizedPricipalIdsDTO.getIds().size(), expectedPrincipalIds.size());
        for (String id : expectedPrincipalIds) {
            assertTrue("Expected id: " + id, normalizedPricipalIdsDTO.getIds().contains(id));
        }

        pricipalIds = new ArrayList<>();
        pricipalIds.add("testUser"+ '@' + systemTenant.toUpperCase());
        pricipalIds.add("testUser3"+ '@' + systemTenant.toUpperCase());
        pricipalIds.add("testGroup" + '@' + systemTenant.toUpperCase());
        pricipalIds.add("testSolutionUser" + '@' + systemTenant.toUpperCase());

        principalIdsDTO = new PrincipalIdentifiersDTO.Builder().withIds(pricipalIds).build();
        normalizedPricipalIdsDTO = systemAdminClient.tenant().findPrincipalIds(systemTenant, principalIdsDTO);
        for (String id : expectedPrincipalIds) {
            assertTrue("Expected id: " + id, normalizedPricipalIdsDTO.getIds().contains(id));
        }

        pricipalIds = new ArrayList<>();
        pricipalIds.add("testUser"+ '@' + systemTenant.toLowerCase());
        pricipalIds.add("testUser3"+ '@' + systemTenant.toLowerCase());
        pricipalIds.add("testGroup" + '@' + systemTenant.toLowerCase());
        pricipalIds.add("testSolutionUser" + '@' + systemTenant.toLowerCase());

        principalIdsDTO = new PrincipalIdentifiersDTO.Builder().withIds(pricipalIds).build();
        normalizedPricipalIdsDTO = systemAdminClient.tenant().findPrincipalIds(systemTenant, principalIdsDTO);
        for (String id : expectedPrincipalIds) {
            assertTrue("Expected id: " + id, normalizedPricipalIdsDTO.getIds().contains(id));
        }
    }

    @Test
    public void testGetSecurityDomains() throws Exception {
        List<SecurityDomainDTO> secDomains = testAdminClient.tenant().getSecurityDoimains(testTenant.getName());
        assertTrue(secDomains != null);
        assertTrue(secDomains.size() > 0);

        secDomains = systemAdminClient.tenant().getSecurityDoimains(systemTenantName);
        assertTrue(secDomains != null);
        assertTrue(secDomains.size() > 0);
    }
}
