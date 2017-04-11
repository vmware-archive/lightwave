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
package com.vmware.identity.rest.idm.server.test.integration.resources;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;
import com.vmware.identity.rest.idm.data.BrandPolicyDTO;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;
import com.vmware.identity.rest.idm.data.PasswordPolicyDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.TokenPolicyDTO;
import com.vmware.identity.rest.idm.data.ProviderPolicyDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.data.attributes.SearchType;
import com.vmware.identity.rest.idm.data.attributes.TenantConfigType;
import com.vmware.identity.rest.idm.server.resources.TenantResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.data.TenantDataGenerator;

/**
 * Integration tests for Tenant Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
public class TenantResourceIT extends TestBase {
    private static final String SSO_TENANT_NAME = "sso.local";
    private static final String SSO_TENANT_LONGNAME = "test.sso.local";
    private static final String SSO_TENANT_KEY = "(SNP?Vm**[#_ca=";

    private static final String ADMIN_SSO_TENANT_UPN = "admin"+"@"+SSO_TENANT_NAME;
    private static final String ADMIN_SSO_TENANT_PWD = "foo!23";

    private static final String BRAND_NAME = "BRAND_NAME_INTEG_TESTS";
    private final String LOGON_BANNER_TITLE = "Test Logon Banner Title";
    private final String LOGON_BANNER_CONTENT = "Welcome to Unit Tests arena of RESTful IDM server !!";

    private static final String DEFAULT_PROVIDER = "localos";

    private TenantResource tenantResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        tenantResource = new TenantResource(request, null);
        tenantResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetTenantInfo() throws Exception {

        try {
            // Test setup [Create a new tenant]
            tenantHelper.createTenant(SSO_TENANT_NAME, SSO_TENANT_LONGNAME, SSO_TENANT_KEY);

            TenantDTO tenantDTO = tenantResource.get(SSO_TENANT_NAME);

            assertEquals(SSO_TENANT_NAME, tenantDTO.getName());
            assertEquals(SSO_TENANT_LONGNAME, tenantDTO.getLongName());
        } finally {
            tenantHelper.deleteTenant(SSO_TENANT_NAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetTenantInfo_WithNonExistentTenant_ThrowsNotFoundEx() {
        tenantResource.get("unknown.local");
    }

    @Ignore
    @Test
    public void testCreateTenant() throws Exception {
        try {
            TenantDTO tenantToCreate = TenantDataGenerator.createTestTenantDTO(SSO_TENANT_NAME, SSO_TENANT_LONGNAME, SSO_TENANT_KEY, ADMIN_SSO_TENANT_UPN, ADMIN_SSO_TENANT_PWD);
            TenantDTO tenant = tenantResource.create(tenantToCreate);
            assertEquals(SSO_TENANT_NAME, tenant.getName());
            assertEquals(SSO_TENANT_LONGNAME, tenant.getLongName());
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            tenantHelper.deleteTenant(SSO_TENANT_NAME);
        }
    }

    @Test(expected = NoSuchTenantException.class)
    public void testDeleteTenant() throws Exception {
        // Test setup [Create a new tenant]
        tenantHelper.createTenant(SSO_TENANT_NAME, SSO_TENANT_LONGNAME, SSO_TENANT_KEY);

        tenantResource.delete(SSO_TENANT_NAME);

        tenantHelper.getTenant(SSO_TENANT_NAME);
    }

    @Test
    public void testGetConfig() {
        TenantConfigurationDTO config = tenantResource.getConfig(DEFAULT_TENANT, Arrays.asList(TenantConfigType.ALL.name()));

        // Assert lockout policy related configs
        assertEquals(Long.valueOf(180), config.getLockoutPolicy().getFailedAttemptIntervalSec());
        assertEquals(Integer.valueOf(5), config.getLockoutPolicy().getMaxFailedAttempts());
        assertEquals(Long.valueOf(300), config.getLockoutPolicy().getAutoUnlockIntervalSec());

        // Assert password policy related configs
        assertEquals(Integer.valueOf(3), config.getPasswordPolicy().getMaxIdenticalAdjacentCharacters());
        assertEquals(Integer.valueOf(20), config.getPasswordPolicy().getMaxLength());
        assertEquals(Integer.valueOf(2), config.getPasswordPolicy().getMinAlphabeticCount());
        assertEquals(Integer.valueOf(8), config.getPasswordPolicy().getMinLength());
        assertEquals(Integer.valueOf(1), config.getPasswordPolicy().getMinLowercaseCount());
        assertEquals(Integer.valueOf(1), config.getPasswordPolicy().getMinNumericCount());
        assertEquals(Integer.valueOf(1), config.getPasswordPolicy().getMinSpecialCharCount());
        assertEquals(Integer.valueOf(1), config.getPasswordPolicy().getMinUppercaseCount());
        assertEquals(Integer.valueOf(90), config.getPasswordPolicy().getPasswordLifetimeDays());
        assertEquals(Integer.valueOf(5), config.getPasswordPolicy().getProhibitedPreviousPasswordCount());

        assertEquals(DEFAULT_PROVIDER, config.getProviderPolicy().getDefaultProvider());
        assertNull(config.getProviderPolicy().getDefaultProviderAlias());
        assertTrue(config.getProviderPolicy().isProviderSelectionEnabled());

        // assert token policy related configs
        assertEquals(Long.valueOf(600000), config.getTokenPolicy().getClockToleranceMillis());
        assertEquals(Long.valueOf(300000), config.getTokenPolicy().getMaxBearerTokenLifeTimeMillis());
        assertEquals(Long.valueOf(21600000), config.getTokenPolicy().getMaxBearerRefreshTokenLifeTimeMillis());
        assertEquals(Integer.valueOf(10), config.getTokenPolicy().getDelegationCount());
        assertEquals(Integer.valueOf(10), config.getTokenPolicy().getRenewCount());

        // assert brand policy related configs. By default branding information is not set
        assertNull(config.getBrandPolicy().getLogonBannerTitle());
        assertNull(config.getBrandPolicy().getLogonBannerContent());
        assertTrue(config.getBrandPolicy().isLogonBannerCheckboxEnabled());
        assertTrue(config.getBrandPolicy().isLogonBannerDisabled());
        assertEquals(BRAND_NAME,config.getBrandPolicy().getName());
    }

    @Test
    public void testUpdateConfig() {

        TenantConfigurationDTO beforeUpdate = tenantResource.getConfig(DEFAULT_TENANT, Arrays.asList(TenantConfigType.ALL.name()));

        try {
            LockoutPolicyDTO lockoutBeforeUpdate = beforeUpdate.getLockoutPolicy();
            PasswordPolicyDTO passwordBeforeUpdate = beforeUpdate.getPasswordPolicy();
            TokenPolicyDTO tokenBeforeUpdate = beforeUpdate.getTokenPolicy();

            LockoutPolicyDTO lockoutPolicyToUpdate =
                    LockoutPolicyDTO.builder().withDescription("updated lockout policy").withAutoUnlockIntervalSec(lockoutBeforeUpdate.getAutoUnlockIntervalSec() + 1)
                    .withFailedAttemptIntervalSec(lockoutBeforeUpdate.getFailedAttemptIntervalSec() + 1).withMaxFailedAttempts(lockoutBeforeUpdate.getMaxFailedAttempts() + 1).build();

            PasswordPolicyDTO pwdPolicyToUpdate =
                    PasswordPolicyDTO.builder().withDescription("updated password policy").withMaxIdenticalAdjacentCharacters(passwordBeforeUpdate.getMaxIdenticalAdjacentCharacters() + 1)
                    .withMaxLength(passwordBeforeUpdate.getMaxLength() + 1).withMinAlphabeticCount(passwordBeforeUpdate.getMinAlphabeticCount()).withMinLength(passwordBeforeUpdate.getMinLength())
                    .withMinLowercaseCount(passwordBeforeUpdate.getMinLowercaseCount()).withMinNumericCount(passwordBeforeUpdate.getMinNumericCount()).withMinSpecialCharCount(passwordBeforeUpdate.getMinSpecialCharCount())
                    .withMinUppercaseCount(passwordBeforeUpdate.getMinUppercaseCount()).withPasswordLifetimeDays(passwordBeforeUpdate.getPasswordLifetimeDays())
                    .withProhibitedPreviousPasswordCount(passwordBeforeUpdate.getProhibitedPreviousPasswordCount()).build();

            TokenPolicyDTO tokenPolicyToUpdate =
                    TokenPolicyDTO.builder().withClockToleranceMillis(tokenBeforeUpdate.getClockToleranceMillis() + 1).withDelegationCount(tokenBeforeUpdate.getDelegationCount() + 1).withRenewCount(tokenBeforeUpdate.getRenewCount() + 1)
                    .withMaxBearerTokenLifeTimeMillis(tokenBeforeUpdate.getMaxBearerTokenLifeTimeMillis() + 1)
                    .withMaxBearerRefreshTokenLifeTimeMillis(tokenBeforeUpdate.getMaxBearerRefreshTokenLifeTimeMillis() + 1).build();

            ProviderPolicyDTO providerPolicyToUpdate = ProviderPolicyDTO.builder()
                    .withDefaultProvider(DEFAULT_PROVIDER)
                    .withDefaultProviderAlias(VSPHERE_LOCAL)
                    .withProviderSelectionEnabled(Boolean.TRUE)
                    .build();

            BrandPolicyDTO brandPolicyToUpdate = BrandPolicyDTO.builder()
                    .withLogonBannerDisabled(true)
                    .withLogonBannerTitle(LOGON_BANNER_TITLE)
                    .withLogonBannerContent(LOGON_BANNER_CONTENT)
                    .withLogonBannerCheckboxEnabled(true)
                    .withName(BRAND_NAME)
                    .build();

            AuthenticationPolicyDTO authenticationPolicyToUpdate = AuthenticationPolicyDTO.builder()
                    .withPasswordBasedAuthenticationEnabled(true)
                    .withWindowsBasedAuthenticationEnabled(true)
                    .withCertificateBasedAuthenticationEnabled(true)
                    .build();

            TenantConfigurationDTO tenantConfigToUpdate = TenantConfigurationDTO.builder()
                    .withLockoutPolicy(lockoutPolicyToUpdate)
                    .withPasswordPolicy(pwdPolicyToUpdate)
                    .withTokenPolicy(tokenPolicyToUpdate)
                    .withProviderPolicy(providerPolicyToUpdate)
                    .withBrandPolicy(brandPolicyToUpdate)
                    .withAuthenticationPolicy(authenticationPolicyToUpdate)
                    .build();

            TenantConfigurationDTO afterUpdate = tenantResource.updateConfig(DEFAULT_TENANT, tenantConfigToUpdate);

            assertEquals("updated lockout policy", afterUpdate.getLockoutPolicy().getDescription());
            assertEquals(lockoutBeforeUpdate.getAutoUnlockIntervalSec() + 1, (long) afterUpdate.getLockoutPolicy().getAutoUnlockIntervalSec());
            assertEquals(lockoutBeforeUpdate.getFailedAttemptIntervalSec() + 1, (long) afterUpdate.getLockoutPolicy().getFailedAttemptIntervalSec());
            assertEquals(lockoutBeforeUpdate.getMaxFailedAttempts() + 1, (int) afterUpdate.getLockoutPolicy().getMaxFailedAttempts());
            assertEquals(beforeUpdate.getPasswordPolicy().getMaxIdenticalAdjacentCharacters() + 1, (int) afterUpdate.getPasswordPolicy().getMaxIdenticalAdjacentCharacters());

            assertEquals("updated password policy", afterUpdate.getPasswordPolicy().getDescription());
            assertEquals(tokenBeforeUpdate.getClockToleranceMillis() + 1, (long) afterUpdate.getTokenPolicy().getClockToleranceMillis());
            assertEquals(tokenBeforeUpdate.getDelegationCount() + 1, (int) afterUpdate.getTokenPolicy().getDelegationCount());
            assertEquals(tokenBeforeUpdate.getMaxBearerTokenLifeTimeMillis() + 1, (long) afterUpdate.getTokenPolicy().getMaxBearerTokenLifeTimeMillis());
            assertEquals(tokenBeforeUpdate.getMaxBearerRefreshTokenLifeTimeMillis() + 1, (long) afterUpdate.getTokenPolicy().getMaxBearerRefreshTokenLifeTimeMillis());
            assertEquals(tokenBeforeUpdate.getRenewCount() + 1, (int) afterUpdate.getTokenPolicy().getRenewCount());

            assertTrue(afterUpdate.getAuthenticationPolicy().isPasswordBasedAuthenticationEnabled());
            assertTrue(afterUpdate.getAuthenticationPolicy().isWindowsBasedAuthenticationEnabled());
            assertTrue(afterUpdate.getAuthenticationPolicy().isCertificateBasedAuthenticationEnabled());

            assertEquals(BRAND_NAME, afterUpdate.getBrandPolicy().getName());
            assertNull(afterUpdate.getBrandPolicy().getLogonBannerTitle());
            assertNull(afterUpdate.getBrandPolicy().getLogonBannerContent());
            assertTrue(afterUpdate.getBrandPolicy().isLogonBannerCheckboxEnabled());
            assertTrue(afterUpdate.getBrandPolicy().isLogonBannerDisabled());

            assertNull(afterUpdate.getProviderPolicy().getDefaultProviderAlias());
            assertTrue(afterUpdate.getProviderPolicy().isProviderSelectionEnabled());
        } finally {
            tenantResource.updateConfig(DEFAULT_TENANT, beforeUpdate);
        }
    }

    @Test
    public void testSearchUser() {
        SearchResultDTO searchResults = tenantResource.searchMembers(DEFAULT_TENANT, MemberType.USER.name(), DEFAULT_TENANT, 200, SearchType.NAME.name(), "admin");
        assertEquals(1, searchResults.getUsers().size());
    }

    @Test
    public void testSearchGroup() throws Exception {
        String testGroupName = "testGroup";
        try {
            // Prepare test set up [create group]
            groupHelper.createGroup(DEFAULT_TENANT, "testGroup");
            SearchResultDTO searchResults = tenantResource.searchMembers(DEFAULT_TENANT, MemberType.GROUP.name(), DEFAULT_TENANT, 200, SearchType.NAME.name(), "test");
            assertEquals(1, searchResults.getGroups().size());
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, testGroupName);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testSearchGroup_WithNonExistentTenant_ThrowsNotFoundEx() {
        tenantResource.searchMembers("unknown.local", MemberType.GROUP.name(), DEFAULT_TENANT, 200, SearchType.NAME.name(), "test");
    }

    @Test
    public void testSearchSolutionUser() throws Exception {
        String testSolutionUserName = "testSolutionUser";
        try {
            // Test setup [Create solution user]
            solutionUserHelper.createSolutionUser(DEFAULT_SYSTEM_DOMAIN, testSolutionUserName);
            SearchResultDTO searchResults = tenantResource.searchMembers(DEFAULT_TENANT, MemberType.SOLUTIONUSER.name(), DEFAULT_TENANT, 200, SearchType.NAME.name(), "test");
            assertEquals(1, searchResults.getSolutionUsers().size());
        } finally {
            solutionUserHelper.deleteSolutionUser(DEFAULT_SYSTEM_DOMAIN, testSolutionUserName);
        }
    }

    @Test
    public void testSearchSolutionUser_WithUnknownUsers() {
        SearchResultDTO searchResults = tenantResource.searchMembers(DEFAULT_TENANT, MemberType.SOLUTIONUSER.name(), DEFAULT_TENANT, 200, SearchType.NAME.name(), "unknown");
        assertEquals(0, searchResults.getSolutionUsers().size());
    }

    @Test
    public void testSearchSolutionUser_CertSubjectDN() throws Exception{
        String SOLUTION_USERNAME = "testSolutionUser";

        try{
            // Create a solution user
            solutionUserHelper.createSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);
            SolutionUser solutionUser = solutionUserHelper.findSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);
            tenantResource.searchMembers(DEFAULT_TENANT, MemberType.SOLUTIONUSER.name(), DEFAULT_SYSTEM_DOMAIN, 10, SearchType.CERT_SUBJECTDN.name(), solutionUser.getCert().getSubjectDN().getName());
        } finally {
            solutionUserHelper.deleteSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);
        }

    }

    @Test(expected = NotFoundException.class)
    public void testSearchSolutionUser_WithNonExistentTenant_ThrowsNotFoundEx() {
        tenantResource.searchMembers("unknown.local", MemberType.SOLUTIONUSER.name(), "unknown.local", 200, SearchType.NAME.name(), "unknown");

    }
}
