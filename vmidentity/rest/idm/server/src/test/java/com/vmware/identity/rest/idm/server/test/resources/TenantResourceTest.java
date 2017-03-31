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
package com.vmware.identity.rest.idm.server.test.resources;

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.createControl;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.net.URL;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.GeneralSecurityException;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.DuplicateTenantException;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;
import com.vmware.identity.rest.idm.data.BrandPolicyDTO;
import com.vmware.identity.rest.idm.data.ClientCertificatePolicyDTO;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;
import com.vmware.identity.rest.idm.data.PasswordPolicyDTO;
import com.vmware.identity.rest.idm.data.ProviderPolicyDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.TokenPolicyDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.data.attributes.SearchType;
import com.vmware.identity.rest.idm.data.attributes.TenantConfigType;
import com.vmware.identity.rest.idm.server.mapper.AuthenticationPolicyMapper;
import com.vmware.identity.rest.idm.server.mapper.LockoutPolicyMapper;
import com.vmware.identity.rest.idm.server.mapper.PasswordPolicyMapper;
import com.vmware.identity.rest.idm.server.resources.CertificateResource;
import com.vmware.identity.rest.idm.server.resources.GroupResource;
import com.vmware.identity.rest.idm.server.resources.IdentityProviderResource;
import com.vmware.identity.rest.idm.server.resources.SolutionUserResource;
import com.vmware.identity.rest.idm.server.resources.TenantResource;
import com.vmware.identity.rest.idm.server.resources.UserResource;
import com.vmware.identity.rest.idm.server.test.util.CertificateUtil;
import com.vmware.identity.rest.idm.server.test.util.TestDataGenerator;

/**
 *
 * Unit tests for Tenant Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class TenantResourceTest {

    private static final String TENANT_NAME = "test.vmware";
    private static final String TENANT_LONG_NAME = "test.vmware.long";
    private static final String TENANT_KEY = "tenant.key.random";
    private static final String DOMAIN = "test.vmare";
    private static final String SUBJECT_DN = "CN=test,O=Test Certificate";
    private static final int MAX_USERS_TO_FETCH = 10;

    private static final String ADMIN_USERNAME = "admin";
    private static final String ADMIN_UPN = ADMIN_USERNAME + "@" + DOMAIN;
    private static final String ADMIN_PWD = "testThis!23";

    // Lockout policy related test constants
    private static final String LOCKOUT_DESC = "Lockout policy created for purpose of unit testing";
    private static final long FAILED_ATTEMPT_INTERVAL_SEC = 100;
    private static final int FAILED_ATTEMPTS = 5;
    private static final long AUTO_UNLOCK_INTERVAL_SEC = 5000;

    // Password policy related test constants
    private final String PWD_DESC = "Password policy created for purpose of unit testing";
    private final int MAX_IDENTICAL_ADJACENT_CHARACTERS = 5;
    private final int MAXLENGTH = 100;
    private final int MIN_ALPHABETIC_COUNT = 4;
    private final int MINLENGTH = 10;
    private final int MIN_LOWERCASE_COUNT = 3;
    private final int MIN_NUMERIC_COUNT = 1;
    private final int MIN_SPECIALCHAR_COUNT = 1;
    private final int MIN_UPPERCASE_COUNT = 1;
    private final int PASSWORD_LIFETIME_DAYS = 50;
    private final int PROHIBITED_PREVIOUS_PASSWORD_COUNT = 3;

    // Token policy related test constants
    private final long CLOCKTOLERANCE_MILLIS= 5000;
    private final int DELEGATION_COUNT = 10;
    private final long MAXBEARER_TOKEN_LIFETIME_MILLIS = 10000;
    private final long MAXHOK_TOKEN_LIFETIME_MILLIS = 5000;
    private final long MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS = 100000;
    private final long MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS = 50000;
    private final int RENEW_COUNT = 10;

    // Provider policy related test constants
    private final String DEFAULT_PROVIDER = "test.local";
    private final String DEFAULT_PROVIDER_ALIAS = "test.alias";
    private final boolean DEFAULT_PROVIDER_SELECTION_ENABLED = true;

    // Brand Policy related tests constants
    private final String BRAND_NAME = "REST_IDM_UNIT_TESTS";
    private final String LOGON_BANNER_TITLE = "Welcome to Unit Tests arena of RESTful IDM server !!";
    private final String LOGON_BANNER_CONTENT = "This is a test logon banner content.";

    // Authentication policy related test constants
    private static final String TEST_CERT_LOC = "src/test/resources/test_cert.pem";
    private static final String OCSP_URL = "http://www.ocsp1.com";
    private static final String CRL_URL = "http://www.crl1.com";
    private static final String[] OBJECT_IDS = new String[] {"a", "b", "c"};
    private static final Boolean HINT_ENABLED = true;

    private TenantResource tenantResource;
    private IMocksControl mControl;
    private CasIdmClient mockCasIdmClient;
    private ContainerRequestContext request;
    private IIdentityStoreData mockIdentityStore;
    private IIdentityStoreDataEx mockIIdentityStoreDataEx;
    private Collection<String> defaultIdentityStores;

    @Before
    public void setUp() {
        mControl = createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        mockCasIdmClient = mControl.createMock(CasIdmClient.class);
        mockIdentityStore = mControl.createMock(IIdentityStoreData.class);
        mockIIdentityStoreDataEx = mControl.createMock(IIdentityStoreDataEx.class);
        tenantResource = new TenantResource(request, null);
        tenantResource.setIDMClient(mockCasIdmClient);
        defaultIdentityStores = new ArrayList<String>();
        defaultIdentityStores.add(DEFAULT_PROVIDER);
    }

    @Test
    public void testCreateTenant() throws Exception {
        TenantDTO tenantToCreate = getTestTenantDTO();
        mockCasIdmClient.addTenant(isA(Tenant.class), eq(ADMIN_USERNAME), aryEq(ADMIN_PWD.toCharArray()));
        mockCasIdmClient.setTenantCredentials(eq(TENANT_NAME), isA(Collection.class), isA(PrivateKey.class));
        expect(mockCasIdmClient.getTenant(TENANT_NAME)).andReturn(getTestTenant());
        mControl.replay();

        tenantResource.create(tenantToCreate);

        mControl.verify();
    }

    @Test(expected=BadRequestException.class)
    public void testCreateTenant_WithInvalidTenantCredentials() {
        TenantDTO tenantDTO = TenantDTO.builder()
                .withName(TENANT_NAME)
                .withLongName(TENANT_LONG_NAME)
                .withKey(TENANT_KEY)
                .withCredentials(null)
                .build();

        tenantResource.create(tenantDTO);
    }

    @Test(expected=InternalServerErrorException.class)
    public void testCreateOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIdmClient.addTenant(isA(Tenant.class), eq(ADMIN_USERNAME), aryEq(ADMIN_PWD.toCharArray()));
        expectLastCall().andThrow(new IDMException("unit test duplicate tenant error"));
        mControl.replay();
        tenantResource.create(getTestTenantDTO());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testCreateTenantIfAlreadyExists_ThrowsBadRequestException() throws Exception {
        mockCasIdmClient.addTenant(isA(Tenant.class), eq(ADMIN_USERNAME), aryEq(ADMIN_PWD.toCharArray()));
        expectLastCall().andThrow(new DuplicateTenantException("unit test duplicate tenant error"));
        mControl.replay();
        tenantResource.create(getTestTenantDTO());
    }

    @Test
    public void testGetConfig_LockoutPolicy() throws Exception {
        expect(mockCasIdmClient.getLockoutPolicy(TENANT_NAME)).andReturn(getTestLockoutPolicy());
        mControl.replay();
        TenantConfigurationDTO tenantConfig = tenantResource.getConfig(TENANT_NAME, Arrays.asList(TenantConfigType.LOCKOUT.name()));
        assertLockoutPolicy(tenantConfig.getLockoutPolicy());
        assertNull(tenantConfig.getPasswordPolicy());
        assertNull(tenantConfig.getTokenPolicy());
        assertNull(tenantConfig.getProviderPolicy());
        assertNull(tenantConfig.getBrandPolicy());
    }

    @Test
    public void testGetConfig_PasswordPolicy() throws Exception {
        expect(mockCasIdmClient.getPasswordPolicy(TENANT_NAME)).andReturn(getTestPasswordPolicy());
        mControl.replay();
        TenantConfigurationDTO tenantConfig = tenantResource.getConfig(TENANT_NAME, Arrays.asList(TenantConfigType.PASSWORD.name()));
        assertPasswordPolicy(tenantConfig.getPasswordPolicy());
        assertNull(tenantConfig.getLockoutPolicy());
        assertNull(tenantConfig.getTokenPolicy());
        assertNull(tenantConfig.getProviderPolicy());
        assertNull(tenantConfig.getBrandPolicy());
    }

    @Test
    public void testGetConfig_ProviderPolicy() throws Exception {

        expect(mockCasIdmClient.getDefaultProviders(TENANT_NAME)).andReturn(Arrays.asList(DEFAULT_PROVIDER));
        expect(mockCasIdmClient.isTenantIDPSelectionEnabled(TENANT_NAME)).andReturn(DEFAULT_PROVIDER_SELECTION_ENABLED);
        expect(mockCasIdmClient.getDefaultProviders(TENANT_NAME)).andReturn(defaultIdentityStores);
        expect(mockCasIdmClient.getProvider(TENANT_NAME, DEFAULT_PROVIDER)).andReturn(mockIdentityStore);
        expect(mockIdentityStore.getExtendedIdentityStoreData()).andReturn(mockIIdentityStoreDataEx);
        expectLastCall().times(2);
        expect(mockIIdentityStoreDataEx.getAlias()).andReturn(DEFAULT_PROVIDER_ALIAS);
        mControl.replay();
        TenantConfigurationDTO tenantConfig = tenantResource.getConfig(TENANT_NAME, Arrays.asList(TenantConfigType.PROVIDER.name()));
        assertEquals(DEFAULT_PROVIDER, tenantConfig.getProviderPolicy().getDefaultProvider());
        assertNull(tenantConfig.getPasswordPolicy());
        assertNull(tenantConfig.getLockoutPolicy());
        assertNull(tenantConfig.getTokenPolicy());
        assertNull(tenantConfig.getBrandPolicy());
    }

    @Test
    public void testGetConfig_TokenPolicy() throws Exception {

        expect(mockCasIdmClient.getClockTolerance(TENANT_NAME)).andReturn(CLOCKTOLERANCE_MILLIS);
        expect(mockCasIdmClient.getDelegationCount(TENANT_NAME)).andReturn(DELEGATION_COUNT);
        expect(mockCasIdmClient.getMaximumBearerTokenLifetime(TENANT_NAME)).andReturn(MAXBEARER_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumHoKTokenLifetime(TENANT_NAME)).andReturn(MAXHOK_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumBearerRefreshTokenLifetime(TENANT_NAME)).andReturn(MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumHoKRefreshTokenLifetime(TENANT_NAME)).andReturn(MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getRenewCount(TENANT_NAME)).andReturn(RENEW_COUNT);

        mControl.replay();
        TenantConfigurationDTO tenantConfig = tenantResource.getConfig(TENANT_NAME, Arrays.asList(TenantConfigType.TOKEN.name()));
        assertTokenPolicy(tenantConfig.getTokenPolicy());
        assertNull(tenantConfig.getProviderPolicy());
        assertNull(tenantConfig.getPasswordPolicy());
        assertNull(tenantConfig.getLockoutPolicy());
        assertNull(tenantConfig.getBrandPolicy());
    }

    @Test
    public void testGetConfig_BrandPolicy() throws Exception {
        expect(mockCasIdmClient.getLogonBannerTitle(TENANT_NAME)).andReturn(LOGON_BANNER_TITLE);
        expect(mockCasIdmClient.getLogonBannerContent(TENANT_NAME)).andReturn(LOGON_BANNER_CONTENT);
        expect(mockCasIdmClient.getLogonBannerCheckboxFlag(TENANT_NAME)).andReturn(true);
        expect(mockCasIdmClient.getBrandName(TENANT_NAME)).andReturn(BRAND_NAME);
        mControl.replay();
        TenantConfigurationDTO tenantConfig = tenantResource.getConfig(TENANT_NAME, Arrays.asList(TenantConfigType.BRAND.name()));
        assertBrandPolicy(tenantConfig.getBrandPolicy());
        assertNull(tenantConfig.getTokenPolicy());
        assertNull(tenantConfig.getProviderPolicy());
        assertNull(tenantConfig.getPasswordPolicy());
        assertNull(tenantConfig.getLockoutPolicy());
    }

    @Test
    public void testGetConfig_All() throws Exception {
        expect(mockCasIdmClient.getPasswordPolicy(TENANT_NAME)).andReturn(getTestPasswordPolicy());
        expect(mockCasIdmClient.getDefaultProviders(TENANT_NAME)).andReturn(Arrays.asList(DEFAULT_PROVIDER));
        expect(mockCasIdmClient.getLockoutPolicy(TENANT_NAME)).andReturn(getTestLockoutPolicy());
        expect(mockCasIdmClient.getClockTolerance(TENANT_NAME)).andReturn(CLOCKTOLERANCE_MILLIS);
        expect(mockCasIdmClient.getDelegationCount(TENANT_NAME)).andReturn(DELEGATION_COUNT);
        expect(mockCasIdmClient.getMaximumBearerTokenLifetime(TENANT_NAME)).andReturn(MAXBEARER_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumHoKTokenLifetime(TENANT_NAME)).andReturn(MAXHOK_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumBearerRefreshTokenLifetime(TENANT_NAME)).andReturn(MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumHoKRefreshTokenLifetime(TENANT_NAME)).andReturn(MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getRenewCount(TENANT_NAME)).andReturn(RENEW_COUNT);
        expect(mockCasIdmClient.getLogonBannerTitle(TENANT_NAME)).andReturn(LOGON_BANNER_TITLE);
        expect(mockCasIdmClient.getLogonBannerContent(TENANT_NAME)).andReturn(LOGON_BANNER_CONTENT);
        expect(mockCasIdmClient.getLogonBannerCheckboxFlag(TENANT_NAME)).andReturn(true);
        expect(mockCasIdmClient.getDefaultProviders(TENANT_NAME)).andReturn(defaultIdentityStores);
        expect(mockCasIdmClient.getProvider(TENANT_NAME, DEFAULT_PROVIDER)).andReturn(mockIdentityStore);
        expect(mockIdentityStore.getExtendedIdentityStoreData()).andReturn(mockIIdentityStoreDataEx);
        expectLastCall().times(2);
        expect(mockIIdentityStoreDataEx.getAlias()).andReturn(DEFAULT_PROVIDER_ALIAS);
        expect(mockCasIdmClient.getBrandName(TENANT_NAME)).andReturn(BRAND_NAME);
        expect(mockCasIdmClient.getAuthnPolicy(TENANT_NAME)).andReturn(getTestAuthnPolicy());
        expect(mockCasIdmClient.isTenantIDPSelectionEnabled(TENANT_NAME)).andReturn(DEFAULT_PROVIDER_SELECTION_ENABLED);

        mControl.replay();
        TenantConfigurationDTO tenantConfig = tenantResource.getConfig(TENANT_NAME, Arrays.asList(TenantConfigType.ALL.name()));

        // Assertions
        assertLockoutPolicy(tenantConfig.getLockoutPolicy());
        assertPasswordPolicy(tenantConfig.getPasswordPolicy());
        assertTokenPolicy(tenantConfig.getTokenPolicy());
        assertProviderPolicy(tenantConfig.getProviderPolicy());
        assertBrandPolicy(tenantConfig.getBrandPolicy());
        assertAuthenticationPolicy(tenantConfig.getAuthenticationPolicy());
    }

    @Test
    public void testUpdateConfig() throws Exception {
        TenantConfigurationDTO configToUpdate = TenantConfigurationDTO.builder()
                .withTokenPolicy(getTestTokenPolicyDTO())
                .withProviderPolicy(getProviderPolicyDTO())
                .withBrandPolicy(getTestBrandPolicyDTO())
                .withAuthenticationPolicy(getTestAuthenticationPolicyDTO())
                .build();

        mockCasIdmClient.setLockoutPolicy(eq(TENANT_NAME), isA(LockoutPolicy.class));
        mockCasIdmClient.setPasswordPolicy(eq(TENANT_NAME), isA(PasswordPolicy.class));
        mockCasIdmClient.setDefaultProviders(eq(TENANT_NAME), isA(Collection.class));
        mockCasIdmClient.setClockTolerance(TENANT_NAME, CLOCKTOLERANCE_MILLIS);
        mockCasIdmClient.setDelegationCount(TENANT_NAME, DELEGATION_COUNT);
        mockCasIdmClient.setMaximumBearerTokenLifetime(TENANT_NAME, MAXBEARER_TOKEN_LIFETIME_MILLIS);
        mockCasIdmClient.setMaximumHoKTokenLifetime(TENANT_NAME, MAXHOK_TOKEN_LIFETIME_MILLIS);
        mockCasIdmClient.setMaximumBearerRefreshTokenLifetime(TENANT_NAME, MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS);
        mockCasIdmClient.setMaximumHoKRefreshTokenLifetime(TENANT_NAME, MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS);
        mockCasIdmClient.setRenewCount(TENANT_NAME, RENEW_COUNT);
        mockCasIdmClient.setBrandName(TENANT_NAME, BRAND_NAME);
        mockCasIdmClient.setLogonBannerTitle(TENANT_NAME, LOGON_BANNER_TITLE);
        mockCasIdmClient.setLogonBannerContent(TENANT_NAME, LOGON_BANNER_CONTENT);
        mockCasIdmClient.setLogonBannerCheckboxFlag(TENANT_NAME, true);
        mockCasIdmClient.setAuthnPolicy(eq(TENANT_NAME), isA(AuthnPolicy.class));
        mockCasIdmClient.setLocalIDPAlias(TENANT_NAME, DEFAULT_PROVIDER_ALIAS);
        mockCasIdmClient.setTenantIDPSelectionEnabled(TENANT_NAME, DEFAULT_PROVIDER_SELECTION_ENABLED);
        expect(mockCasIdmClient.getDefaultProviders(TENANT_NAME)).andReturn(defaultIdentityStores);
        expect(mockCasIdmClient.getProvider(TENANT_NAME, DEFAULT_PROVIDER)).andReturn(mockIdentityStore);
        expect(mockIdentityStore.getExtendedIdentityStoreData()).andReturn(mockIIdentityStoreDataEx);
        expectLastCall().times(2);
        expect(mockIIdentityStoreDataEx.getAlias()).andReturn(DEFAULT_PROVIDER_ALIAS);
        expect(mockCasIdmClient.getPasswordPolicy(TENANT_NAME)).andReturn(getTestPasswordPolicy());
        expect(mockCasIdmClient.getDefaultProviders(TENANT_NAME)).andReturn(Arrays.asList(DEFAULT_PROVIDER));
        expect(mockCasIdmClient.getLockoutPolicy(TENANT_NAME)).andReturn(getTestLockoutPolicy());
        expect(mockCasIdmClient.getClockTolerance(TENANT_NAME)).andReturn(CLOCKTOLERANCE_MILLIS);
        expect(mockCasIdmClient.getDelegationCount(TENANT_NAME)).andReturn(DELEGATION_COUNT);
        expect(mockCasIdmClient.getMaximumBearerTokenLifetime(TENANT_NAME)).andReturn(MAXBEARER_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumHoKTokenLifetime(TENANT_NAME)).andReturn(MAXHOK_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumBearerRefreshTokenLifetime(TENANT_NAME)).andReturn(MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getMaximumHoKRefreshTokenLifetime(TENANT_NAME)).andReturn(MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS);
        expect(mockCasIdmClient.getRenewCount(TENANT_NAME)).andReturn(RENEW_COUNT);
        expect(mockCasIdmClient.getBrandName(TENANT_NAME)).andReturn(BRAND_NAME);
        expect(mockCasIdmClient.getLogonBannerTitle(TENANT_NAME)).andReturn(LOGON_BANNER_TITLE);
        expect(mockCasIdmClient.getLogonBannerContent(TENANT_NAME)).andReturn(LOGON_BANNER_CONTENT);
        expect(mockCasIdmClient.getLogonBannerCheckboxFlag(TENANT_NAME)).andReturn(true);
        expect(mockCasIdmClient.getLocalIDPAlias(TENANT_NAME)).andReturn(DEFAULT_PROVIDER_ALIAS);
        expect(mockCasIdmClient.getAuthnPolicy(TENANT_NAME)).andReturn(getTestAuthnPolicy());
        expect(mockCasIdmClient.isTenantIDPSelectionEnabled(TENANT_NAME)).andReturn(DEFAULT_PROVIDER_SELECTION_ENABLED);

        mControl.replay();
        TenantConfigurationDTO updatedConfig = tenantResource.updateConfig(TENANT_NAME, configToUpdate);

        assertTokenPolicy(updatedConfig.getTokenPolicy());
        assertProviderPolicy(updatedConfig.getProviderPolicy());
        assertBrandPolicy(updatedConfig.getBrandPolicy());
        assertAuthenticationPolicy(updatedConfig.getAuthenticationPolicy());
    }

    private void assertProviderPolicy(ProviderPolicyDTO providerPolicy) {
        assertEquals(DEFAULT_PROVIDER, providerPolicy.getDefaultProvider());
        assertEquals(DEFAULT_PROVIDER_ALIAS, providerPolicy.getDefaultProviderAlias());
        assertEquals(DEFAULT_PROVIDER_SELECTION_ENABLED, providerPolicy.isProviderSelectionEnabled());
        assertTrue(providerPolicy.isProviderSelectionEnabled());
    }

    private void assertBrandPolicy(BrandPolicyDTO brandPolicy){
        assertEquals(BRAND_NAME, brandPolicy.getName());
        assertEquals(LOGON_BANNER_TITLE, brandPolicy.getLogonBannerTitle());
        assertEquals(LOGON_BANNER_CONTENT, brandPolicy.getLogonBannerContent());
        assertEquals(Boolean.TRUE, brandPolicy.isLogonBannerCheckboxEnabled());
        assertEquals(Boolean.FALSE, brandPolicy.isLogonBannerDisabled());
    }

    private void assertAuthenticationPolicy(AuthenticationPolicyDTO authenticationPolicy){
        assertTrue(authenticationPolicy.isPasswordBasedAuthenticationEnabled());
        assertTrue(authenticationPolicy.isWindowsBasedAuthenticationEnabled());
        assertTrue(authenticationPolicy.isCertificateBasedAuthenticationEnabled());
        assertClientCertificatePolicy(authenticationPolicy.getClientCertificatePolicy());
    }

    private void assertClientCertificatePolicy(ClientCertificatePolicyDTO clientCertificatePolicy) {
        assertTrue(clientCertificatePolicy.isOcspEnabled());
        assertTrue(clientCertificatePolicy.isFailOverToCrlEnabled());
        assertTrue(clientCertificatePolicy.isCrlDistributionPointUsageEnabled());
        assertTrue(clientCertificatePolicy.isRevocationCheckEnabled());
        assertEquals(OCSP_URL, clientCertificatePolicy.getOcspUrlOverride());
        assertEquals(CRL_URL, clientCertificatePolicy.getCrlDistributionPointOverride());
        assertEquals(3, clientCertificatePolicy.getCertPolicyOIDs().size());
        assertNotNull(clientCertificatePolicy.getTrustedCACertificates());
        assertEquals(1, clientCertificatePolicy.getTrustedCACertificates().size());
        if (HINT_ENABLED) {
            assertTrue(clientCertificatePolicy.isUserNameHintEnabled());
        } else {
            assertTrue(!clientCertificatePolicy.isUserNameHintEnabled());
        }
    }

    @Test
    public void testGetTenant() throws Exception {
        expect(mockCasIdmClient.getTenant(TENANT_NAME)).andReturn(getTestTenant());
        mControl.replay();

        TenantDTO tenant = tenantResource.get(TENANT_NAME);

        assertEquals(TENANT_NAME, tenant.getName());
        assertEquals(TENANT_LONG_NAME, tenant.getLongName());
        assertEquals(TENANT_KEY, tenant.getKey());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIdmClient.getTenant(TENANT_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        tenantResource.get(TENANT_NAME);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIdmClient.getTenant(TENANT_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        tenantResource.get(TENANT_NAME);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIdmClient.getTenant(TENANT_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        tenantResource.get(TENANT_NAME);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetTenantOnNoSuchTenantThrowsNotFoundException() throws Exception {
        expect(mockCasIdmClient.getTenant(TENANT_NAME)).andThrow(new NoSuchTenantException("unit test no such tenant error"));
        mControl.replay();
        tenantResource.get(TENANT_NAME);
    }

    @Test
    public void testDeleteTenant() throws Exception {
        mockCasIdmClient.deleteTenant(TENANT_NAME);
        mControl.replay();

        tenantResource.delete(TENANT_NAME);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIdmClient.deleteTenant(TENANT_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        tenantResource.delete(TENANT_NAME);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIdmClient.deleteTenant(TENANT_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        tenantResource.delete(TENANT_NAME);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIdmClient.deleteTenant(TENANT_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        tenantResource.delete(TENANT_NAME);
        mControl.verify();
    }

    /**
     * Spurious tests to increase test coverage.
     */
    @Test
    public void testUserResource() {
        UserResource userRes = tenantResource.getUserSubResource(TENANT_NAME);
        assertEquals(TENANT_NAME, userRes.getTenant());
    }

    @Test
    public void testGroupResource() {
        GroupResource groupRes = tenantResource.getGroupSubResource(TENANT_NAME);
        assertEquals(TENANT_NAME, groupRes.getTenant());
    }

    @Test
    public void testSolutionUserResource() {
        SolutionUserResource solutionUserRes = tenantResource.getSolutionUserSubResource(TENANT_NAME);
        assertEquals(TENANT_NAME, solutionUserRes.getTenant());
    }

    @Test
    public void testCertificateResource() {
        CertificateResource certRes = tenantResource.getCertificateSubResource(TENANT_NAME);
        assertEquals(TENANT_NAME, certRes.getTenant());
    }

    @Test
    public void testProviderResource() {
        IdentityProviderResource providerRes = tenantResource.getIdentityProviderSubResource(TENANT_NAME);
        assertEquals(TENANT_NAME, providerRes.getTenant());
    }

    @Test
    public void testSearchUsers() throws Exception {
        Set<PersonUser> personUsers = TestDataGenerator.getIdmPersonUsers(1);
        expect(mockCasIdmClient.findPersonUsersByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andReturn(personUsers);
        mControl.replay();
        SearchResultDTO searchResults = tenantResource.searchMembers(TENANT_NAME, MemberType.USER.name(), DOMAIN, 200, "name", "Admin");
        assertEquals(1, searchResults.getUsers().size());
        mControl.verify();
    }


    @Test(expected = NotFoundException.class)
    public void testSearchUsersOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIdmClient.findPersonUsersByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andThrow(new NoSuchTenantException("No such tenant"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.USER.name(), DOMAIN, 200, "name", "Admin");
    }

    @Test(expected = BadRequestException.class)
    public void testSearchUsersOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIdmClient.findPersonUsersByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andThrow(new InvalidArgumentException("Invalid Argument"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.USER.name(), DOMAIN, 200, "name", "Admin");
    }

    @Test(expected = InternalServerErrorException.class)
    public void testSearchUsersOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIdmClient.findPersonUsersByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.USER.name(), DOMAIN, 200, "name", "Admin");
    }

    @Test
    public void testSearchGroupsWithDefaultsInTenant() throws Exception {
        Set<Group> idmGroups = TestDataGenerator.getIdmGroups(1);
        EasyMock.expect(mockCasIdmClient.findGroupsByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andReturn(idmGroups);
        mControl.replay();

        SearchResultDTO searchResults = tenantResource.searchMembers(TENANT_NAME, MemberType.GROUP.name(), DOMAIN, 200, "name", "Admin");
        assertEquals(1, searchResults.getGroups().size());

        mControl.verify();
    }

    @Test
    public void testSearchGroupsInTenantWithRegex() throws Exception {
        Set<Group> adminGroups = TestDataGenerator.getIdmGroups("admin", 5);
        expect(mockCasIdmClient.findGroupsByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andReturn(adminGroups);
        mControl.replay();
        String query = "admin";
        SearchResultDTO searchResults = tenantResource.searchMembers(TENANT_NAME, MemberType.GROUP.name(), DOMAIN, 200, "name", query);
        assertEquals(5, searchResults.getGroups().size());
        mControl.verify();
    }

    @Test(expected=NotFoundException.class)
    public void testSearchGroupsOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIdmClient.findGroupsByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andThrow(new NoSuchTenantException("No such tenant"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.GROUP.name(), DOMAIN, 200, "name", "admin");
        mControl.verify();
    }

    @Test(expected=BadRequestException.class)
    public void testSearchGroupsOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIdmClient.findGroupsByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andThrow(new InvalidArgumentException("Invalid argument"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.GROUP.name(), DOMAIN, 200, "name", "admin");
        mControl.verify();
    }

    @Test(expected=InternalServerErrorException.class)
    public void testSearchGroupsOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIdmClient.findGroupsByName(eq(TENANT_NAME), isA(SearchCriteria.class), eq(200))).andThrow(new IDMException("Invalid argument"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.GROUP.name(), DOMAIN, 200, "name", "admin");
        mControl.verify();
    }

    @Test
    public void testSearchSolutoinUsers_WithDefaults() throws Exception {
        Set<SolutionUser> solutionUsers = TestDataGenerator.getIdmSolutionUsers(1);
        expect(mockCasIdmClient.findSolutionUsers(eq(TENANT_NAME), isA(String.class), eq(MAX_USERS_TO_FETCH))).andReturn(solutionUsers);
        mControl.replay();

        SearchResultDTO searchResults = tenantResource.searchMembers(TENANT_NAME, MemberType.SOLUTIONUSER.name(), DOMAIN, MAX_USERS_TO_FETCH, "name", "admin");
        assertEquals(1, searchResults.getSolutionUsers().size());
        mControl.verify();
    }

    @Test
    public void testSearchSolutionUserByCertDN() throws Exception {
        Set<SolutionUser> solutionUsers = TestDataGenerator.getIdmSolutionUsers(1);
        expect(mockCasIdmClient.findSolutionUserByCertDn(eq(TENANT_NAME), eq(SUBJECT_DN))).andReturn(solutionUsers.iterator().next());
        mControl.replay();

        SearchResultDTO searchResults = tenantResource.searchMembers(TENANT_NAME, MemberType.SOLUTIONUSER.name(), DOMAIN, 200, SearchType.CERT_SUBJECTDN.name(), SUBJECT_DN);
        assertEquals(1, searchResults.getSolutionUsers().size());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testSearchSolutionUser_OnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIdmClient.findSolutionUsers(TENANT_NAME, "admin", 200);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.SOLUTIONUSER.name(), DOMAIN, 200, SearchType.NAME.name(), "admin");
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testSearchSolutionUser_OnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIdmClient.findSolutionUsers(TENANT_NAME, "admin", 200);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.SOLUTIONUSER.name(), DOMAIN, 200, SearchType.NAME.name(), "admin");
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testSearchSolutionUser_OnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIdmClient.findSolutionUsers(TENANT_NAME, "admin", 200);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        tenantResource.searchMembers(TENANT_NAME, MemberType.SOLUTIONUSER.name(), DOMAIN, 200, SearchType.NAME.name(), "admin");
        mControl.verify();
    }

    private Tenant getTestTenant() {
        return new Tenant(TENANT_NAME, TENANT_LONG_NAME, TENANT_KEY);
    }

    private TenantDTO getTestTenantDTO() throws GeneralSecurityException, IOException {
        return TenantDTO.builder()
                .withName(TENANT_NAME)
                .withLongName(TENANT_LONG_NAME)
                .withKey(TENANT_KEY)
                .withCredentials(TestDataGenerator.getTestTenantCredentialsDTO())
                .withUsername(ADMIN_UPN)
                .withPassword(ADMIN_PWD)
                .build();
    }

    private LockoutPolicy getTestLockoutPolicy() {
        return new LockoutPolicy(LOCKOUT_DESC, FAILED_ATTEMPT_INTERVAL_SEC, FAILED_ATTEMPTS, AUTO_UNLOCK_INTERVAL_SEC);
    }

    private AuthnPolicy getTestAuthnPolicy() throws CertificateException, IOException {

        ClientCertPolicy cert = new ClientCertPolicy();
        cert.setRevocationCheckEnabled(true);
        cert.setUseOCSP(true);
        cert.setUseCRLAsFailOver(true);
        cert.setSendOCSPNonce(true);
        cert.setOCSPUrl(new URL(OCSP_URL));
        cert.setUseCertCRL(true);
        cert.setCRLUrl(new URL(CRL_URL));
        cert.setOIDs(OBJECT_IDS);
        cert.setTrustedCAs(new Certificate[] {CertificateUtil.getTestCertificate()});
        cert.setEnableHint(HINT_ENABLED);

        return new AuthnPolicy(true, true, true, cert);
    }

    private static String getTestPEMCert() throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(TEST_CERT_LOC));
        return new String(encoded, Charset.defaultCharset());
    }

    private BrandPolicyDTO getTestBrandPolicyDTO(){
        return BrandPolicyDTO.builder()
                .withName(BRAND_NAME)
                .withLogonBannerTitle(LOGON_BANNER_TITLE)
                .withLogonBannerContent(LOGON_BANNER_CONTENT)
                .withLogonBannerCheckboxEnabled(true)
                .build();
    }

    private AuthenticationPolicyDTO getTestAuthenticationPolicyDTO() throws CertificateException, IOException {
        return AuthenticationPolicyMapper.getAuthenticationPolicyDTO(getTestAuthnPolicy());
    }

    private PasswordPolicy getTestPasswordPolicy() {
        return new PasswordPolicy(PWD_DESC, PROHIBITED_PREVIOUS_PASSWORD_COUNT, MINLENGTH, MAXLENGTH, MIN_ALPHABETIC_COUNT, MIN_UPPERCASE_COUNT, MIN_LOWERCASE_COUNT, MIN_NUMERIC_COUNT, MIN_SPECIALCHAR_COUNT, MAX_IDENTICAL_ADJACENT_CHARACTERS, PASSWORD_LIFETIME_DAYS);
    }

    private ProviderPolicyDTO getProviderPolicyDTO() {
        return ProviderPolicyDTO.builder()
                .withDefaultProvider(DEFAULT_PROVIDER)
                .withDefaultProviderAlias(DEFAULT_PROVIDER_ALIAS)
                .withProviderSelectionEnabled(Boolean.TRUE)
                .build();
    }

    private TokenPolicyDTO getTestTokenPolicyDTO(){
        return TokenPolicyDTO.builder()
                .withClockToleranceMillis(CLOCKTOLERANCE_MILLIS)
                .withDelegationCount(DELEGATION_COUNT)
                .withMaxBearerTokenLifeTimeMillis(MAXBEARER_TOKEN_LIFETIME_MILLIS)
                .withMaxHOKTokenLifeTimeMillis(MAXHOK_TOKEN_LIFETIME_MILLIS)
                .withMaxBearerRefreshTokenLifeTimeMillis(MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS)
                .withMaxHOKRefreshTokenLifeTimeMillis(MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS)
                .withRenewCount(RENEW_COUNT)
                .build();
    }

    private void assertLockoutPolicy(LockoutPolicyDTO lockoutPolicy) {
        assertEquals(LOCKOUT_DESC, lockoutPolicy.getDescription());
        assertEquals(FAILED_ATTEMPT_INTERVAL_SEC, (long) lockoutPolicy.getFailedAttemptIntervalSec());
        assertEquals(FAILED_ATTEMPTS, (int) lockoutPolicy.getMaxFailedAttempts());
        assertEquals(AUTO_UNLOCK_INTERVAL_SEC, (long) lockoutPolicy.getAutoUnlockIntervalSec());
    }

    private void assertPasswordPolicy(PasswordPolicyDTO passwordPolicy) {
        assertEquals(PWD_DESC, passwordPolicy.getDescription());
        assertEquals(MAX_IDENTICAL_ADJACENT_CHARACTERS, (int) passwordPolicy.getMaxIdenticalAdjacentCharacters());
        assertEquals(MAXLENGTH, (int) passwordPolicy.getMaxLength());
        assertEquals(MIN_ALPHABETIC_COUNT, (int) passwordPolicy.getMinAlphabeticCount());
        assertEquals(MINLENGTH, (int) passwordPolicy.getMinLength());
        assertEquals(MIN_LOWERCASE_COUNT, (int) passwordPolicy.getMinLowercaseCount());
        assertEquals(MIN_NUMERIC_COUNT, (int) passwordPolicy.getMinNumericCount());
        assertEquals(MIN_SPECIALCHAR_COUNT, (int) passwordPolicy.getMinSpecialCharCount());
        assertEquals(MIN_UPPERCASE_COUNT, (int) passwordPolicy.getMinUppercaseCount());
        assertEquals(PASSWORD_LIFETIME_DAYS, (int) passwordPolicy.getPasswordLifetimeDays());
        assertEquals(PROHIBITED_PREVIOUS_PASSWORD_COUNT, (int) passwordPolicy.getProhibitedPreviousPasswordCount());
    }

    private void assertTokenPolicy(TokenPolicyDTO tokenPolicy){
        assertEquals(CLOCKTOLERANCE_MILLIS, (long) tokenPolicy.getClockToleranceMillis());
        assertEquals(DELEGATION_COUNT, (int) tokenPolicy.getDelegationCount());
        assertEquals(MAXBEARER_TOKEN_LIFETIME_MILLIS, (long) tokenPolicy.getMaxBearerTokenLifeTimeMillis());
        assertEquals(MAXHOK_TOKEN_LIFETIME_MILLIS, (long) tokenPolicy.getMaxHOKTokenLifeTimeMillis());
        assertEquals(MAXBEARER_REFRESH_TOKEN_LIFETIME_MILLIS, (long) tokenPolicy.getMaxBearerRefreshTokenLifeTimeMillis());
        assertEquals(MAXHOK_REFRESH_TOKEN_LIFETIME_MILLIS, (long) tokenPolicy.getMaxHOKRefreshTokenLifeTimeMillis());
        assertEquals(RENEW_COUNT, (int) tokenPolicy.getRenewCount());
    }

}
