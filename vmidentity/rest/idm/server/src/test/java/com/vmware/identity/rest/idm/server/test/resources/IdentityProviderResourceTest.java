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

import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;

import java.net.MalformedURLException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidProviderException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.attributes.IdentityProviderType;
import com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper;
import com.vmware.identity.rest.idm.server.resources.IdentityProviderResource;

/**
 *
 * Unit tests for IdentityProvider Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class IdentityProviderResourceTest {

    // Constants - Common
    private static final String TENANT = "test.tenant";
    private static final String USER_NAME = "testUser@domain";
    private static final String PASSWORD = "UnitTest123!";
    // AD -related constants
    private static final String AD_PROVIDER_NAME = "ad.ssolabs.eng.wmare.com";
    private static final String AD_SPN = "spn.ad.ssolabs.eng.vmware.com";

    // Open LDAP related constants
    private static final String OPEN_LDAP_PROVIDER_NAME = "openladap.ssolabs.eng.vmware.com";
    private static final String OPEN_LDAP_ALIAS = "openldap.alias";
    private static final String OPEN_LDAP_FRIENDLY_NAME = "openladap.friendly.name";
    private static final String OPEN_LDAP_USER_BASE_DN = "dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com";
    private static final String OPEN_LDAP_GROUP_BASE_DN = "dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com";
    private static final String OPEN_LDAP_USER = "cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com";

    // Probe related constants
    private static final String LDAP_URL_TO_PROBE = "ldap://openladap.ssolabs.eng.vmware.com";

    private static final Map<String, String> attributesMap = new HashMap<String, String>() {
        {
            put("http://vmware.com/schemas/attr-names/2011/07/isSolution", "subjectType");
            put("http://schemas.xmlsoap.org/claims/UPN", "userPrincipalName");
            put("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname", "sn");
        }
    };

    private static final Collection<String> connectionStrings = new HashSet<String>() {
        {
            add("test.sso.labs.1");
            add("test.sso.labs.2");
        }
    };

    private static Set<String> upnSuffixes = new HashSet<String>() {
        {
            add("upn.suffix.sso.labs.1");
            add("upn.suffix.sso.labs.2");
        }
    };

    private static Collection<X509Certificate> idsCertificates = new ArrayList<X509Certificate>();

    private IMocksControl mControl;
    private IdentityProviderResource identitySourceResource;
    private CasIdmClient mockCasIDMClient;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        mockCasIDMClient = mControl.createMock(CasIdmClient.class);
        identitySourceResource = new IdentityProviderResource(TENANT, request, null);
        identitySourceResource.setIDMClient(mockCasIDMClient);
    }

    @Test
    public void testGetProvider() throws Exception {
        expect(mockCasIDMClient.getProvider(TENANT, AD_PROVIDER_NAME)).andReturn(getTestADIdentityProvider());
        mControl.replay();

        IdentityProviderDTO provider = identitySourceResource.get(AD_PROVIDER_NAME);
        assertEquals("Failed asserting identity provider name", AD_PROVIDER_NAME, provider.getName());
        assertEquals(3, attributesMap.size());
        assertEquals("Failed asserting service principal name", AD_SPN, provider.getServicePrincipalName());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetProviderOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.getProvider(TENANT, AD_PROVIDER_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        identitySourceResource.get(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetProviderOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.getProvider(TENANT, AD_PROVIDER_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        identitySourceResource.get(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetProviderOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.getProvider(TENANT, AD_PROVIDER_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        identitySourceResource.get(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test
    public void testGetOpenLDAPIdentityProviderHappyCase() throws Exception {
        expect(mockCasIDMClient.getProvider(TENANT, OPEN_LDAP_PROVIDER_NAME)).andReturn(getTestOpenLDAPIdentityProvider());
        mControl.replay();

        IdentityProviderDTO provider = identitySourceResource.get(OPEN_LDAP_PROVIDER_NAME);

        assertEquals("Failed asserting identity provider name", OPEN_LDAP_PROVIDER_NAME, provider.getName());
        assertEquals(3, attributesMap.size());
        assertEquals("Failed asserting ldap friendly name", OPEN_LDAP_FRIENDLY_NAME, provider.getFriendlyName());
        assertEquals("Failed asserting base dn for users ", OPEN_LDAP_USER_BASE_DN, provider.getUserBaseDN());
        assertEquals("Failed asserting base dn for groups", OPEN_LDAP_GROUP_BASE_DN, provider.getGroupBaseDN());
        assertEquals("Failed asserting authentication type", AuthenticationType.PASSWORD.name(), provider.getAuthenticationType());
        assertEquals("Failed asserting number of connection strings", 2, provider.getConnectionStrings().size());
        assertEquals("Failed asserting number of UPN suffixes", 2, provider.getUpnSuffixes().size());
        assertEquals("Failed asserting searchTimeOut ", new Long(0), provider.getSearchTimeOutInSeconds());
        assertEquals("Failed asserting idenitty store type", IdentityProviderType.IDENTITY_STORE_TYPE_LDAP.name(), provider.getType());

        mControl.verify();
    }

    @Test
    public void testUpdateProvider() throws Exception {
        mockCasIDMClient.setProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expect(mockCasIDMClient.getProvider(TENANT, AD_PROVIDER_NAME)).andReturn(getTestADIdentityProvider());
        mControl.replay();
        IdentityProviderDTO identityProvider = identitySourceResource.update(AD_PROVIDER_NAME, getTestADIdentityProviderDTO());
        assertEquals(DomainType.EXTERNAL_DOMAIN.name(), identityProvider.getDomainType());
        assertEquals(AD_PROVIDER_NAME, identityProvider.getName());
        assertEquals(3, identityProvider.getAttributesMap().size());
        assertEquals(AD_SPN, identityProvider.getServicePrincipalName());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateProviderOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.setProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        identitySourceResource.update(AD_PROVIDER_NAME, getTestADIdentityProviderDTO());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateProviderOnIDMLoginIssue_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.setProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new IDMLoginException("invalid login cred"));
        mControl.replay();
        identitySourceResource.update(AD_PROVIDER_NAME, getTestADIdentityProviderDTO());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateProviderOnInvalidProvider_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.setProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new InvalidProviderException("invalid login cred", null, null));
        mControl.replay();
        identitySourceResource.update(AD_PROVIDER_NAME, getTestADIdentityProviderDTO());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testUpdateProviderOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.setProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        IdentityProviderDTO testProviderInput = IdentityProviderMapper.getIdentityProviderDTO(getTestADIdentityProvider());
        identitySourceResource.update(AD_PROVIDER_NAME, testProviderInput);
        mControl.verify();
    }

    @Test
    public void testDeleteProvider() throws Exception {
        mockCasIDMClient.deleteProvider(TENANT, AD_PROVIDER_NAME);
        mControl.replay();
        identitySourceResource.delete(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteProviderOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.deleteProvider(TENANT, AD_PROVIDER_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        identitySourceResource.delete(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteProviderOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.deleteProvider(TENANT, AD_PROVIDER_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        identitySourceResource.delete(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteProviderOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.deleteProvider(TENANT, AD_PROVIDER_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        identitySourceResource.delete(AD_PROVIDER_NAME);
        mControl.verify();
    }

    @Test
    public void testAddADIdentityProvider() throws Exception {
        mockCasIDMClient.addProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expect(mockCasIDMClient.getProvider(TENANT, AD_PROVIDER_NAME)).andReturn(getTestADIdentityProvider());

        mControl.replay();

        IdentityProviderDTO identityProvider = identitySourceResource.create(getTestADIdentityProviderDTO(), false);

        assertEquals(DomainType.EXTERNAL_DOMAIN.name(), identityProvider.getDomainType());
        assertEquals(AD_PROVIDER_NAME, identityProvider.getName());
        assertEquals(3, identityProvider.getAttributesMap().size());
        assertEquals(AD_SPN, identityProvider.getServicePrincipalName());
        mControl.verify();
    }

    @Test
    public void testAddLocalOsProvider() throws Exception {
        String LOCALOS_NAME = "localOS";
        String LOCALOS_ALIAS = "localOS-Alias";
        mockCasIDMClient.addProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expect(mockCasIDMClient.getProvider(TENANT, LOCALOS_NAME)).andReturn(IdentityStoreData.CreateLocalOSIdentityStoreData(LOCALOS_NAME, LOCALOS_ALIAS));

        mControl.replay();

        IdentityProviderDTO localOsProvider = IdentityProviderDTO.builder().withName(LOCALOS_NAME).withAlias(LOCALOS_ALIAS).withType(IdentityProviderType.IDENTITY_STORE_TYPE_LOCAL_OS.name()).build();
        assertEquals(LOCALOS_NAME, localOsProvider.getName());
        assertEquals(LOCALOS_ALIAS, localOsProvider.getAlias());
    }

    @Test(expected = NotFoundException.class)
    public void testAddProviderOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.addProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        identitySourceResource.create(getTestADIdentityProviderDTO(), false);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testAddProviderOnMalformedURL_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.addProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new MalformedURLException("invalid argument"));
        mControl.replay();
        identitySourceResource.create(getTestADIdentityProviderDTO(), false);
        mControl.verify();

    }

    @Test(expected = InternalServerErrorException.class)
    public void testAddProviderOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.addProvider(eq(TENANT), isA(IIdentityStoreData.class));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        IdentityProviderDTO testADToAdd = IdentityProviderMapper.getIdentityProviderDTO(getTestADIdentityProvider());
        identitySourceResource.create(testADToAdd, false);
        mControl.verify();
    }

    @Test
    public void testGetAll() throws Exception {
        Collection<IIdentityStoreData> providers = new HashSet<IIdentityStoreData>();
        providers.add(getTestADIdentityProvider());
        providers.add(getTestOpenLDAPIdentityProvider());
        expect(mockCasIDMClient.getProviders(TENANT)).andReturn(providers);
        mControl.replay();

        Collection<IdentityProviderDTO> resultProviders = identitySourceResource.getAll();
        assertEquals(2, resultProviders.size());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetAllProvidersOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.getProviders(TENANT);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        identitySourceResource.getAll();
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetAllProvidersOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.getProviders(TENANT);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        identitySourceResource.getAll();
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetAllProvidersOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.getProviders(TENANT);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        identitySourceResource.getAll();
        mControl.verify();
    }


    @Test
    public void testProbeProvider() throws Exception {
        mockCasIDMClient.probeProviderConnectivity(isA(String.class), isA(IdentityStoreData.class));
        mControl.replay();
        identitySourceResource.create(getTestOpenLDAPIdentityProviderDTO(), true);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testProbeProviderOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.probeProviderConnectivity(isA(String.class), isA(IdentityStoreData.class));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        identitySourceResource.create(getTestOpenLDAPIdentityProviderDTO(), true);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testProbeProviderOnIDMLoginException_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.probeProviderConnectivity(isA(String.class), isA(IdentityStoreData.class));
        expectLastCall().andThrow(new IDMLoginException("login failed"));
        mControl.replay();
        identitySourceResource.create(getTestOpenLDAPIdentityProviderDTO(), true);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testProbeProviderOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.probeProviderConnectivity(isA(String.class), isA(IdentityStoreData.class));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        identitySourceResource.create(IdentityProviderMapper.getIdentityProviderDTO(getTestOpenLDAPIdentityProvider()), true);
        mControl.verify();
    }

    public static IdentityStoreData getTestADIdentityProvider() {
        return IdentityStoreData.createActiveDirectoryIdentityStoreData(AD_PROVIDER_NAME, USER_NAME, false, AD_SPN, PASSWORD, attributesMap, null, null);
    }

    public static IdentityStoreData getTestOpenLDAPIdentityProvider() {
        return IdentityStoreData.CreateExternalIdentityStoreData(OPEN_LDAP_PROVIDER_NAME,
                                                                 OPEN_LDAP_ALIAS,
                                                                 IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
                                                                 AuthenticationType.PASSWORD,
                                                                 OPEN_LDAP_FRIENDLY_NAME,
                                                                 0,
                                                                 OPEN_LDAP_USER,
                                                                 PASSWORD,
                                                                 OPEN_LDAP_USER_BASE_DN,
                                                                 OPEN_LDAP_GROUP_BASE_DN,
                                                                 connectionStrings,
                                                                 attributesMap,
                                                                 null,
                                                                 upnSuffixes);
    }

    private static IdentityProviderDTO getTestOpenLDAPIdentityProviderDTO(){
        IdentityStoreData identityStore = getTestOpenLDAPIdentityProvider();
        return getTestExternalIdentityProviderDTO(identityStore);
    }

    private static IdentityProviderDTO getTestExternalIdentityProviderDTO(IdentityStoreData identityStore){
        IdentityProviderDTO.Builder providerBuilder = IdentityProviderDTO.builder();
        IIdentityStoreDataEx extendedIdentityStoreData = identityStore.getExtendedIdentityStoreData();
        DomainType domain = identityStore.getDomainType();
        String domainName = identityStore.getName();
        providerBuilder.withDomainType(domain.name())
                       .withName(domainName)
                       .withAlias(extendedIdentityStoreData.getAlias())
                       .withType(extendedIdentityStoreData.getProviderType().name())
                       .withAuthenticationType(extendedIdentityStoreData.getAuthenticationType().name())
                       .withFriendlyName(extendedIdentityStoreData.getFriendlyName())
                       .withUsername(extendedIdentityStoreData.getUserName())
                       .withPassword(extendedIdentityStoreData.getPassword())
                       .withSearchTimeOutInSeconds((long) extendedIdentityStoreData.getSearchTimeoutSeconds())
                       .withMachineAccount(extendedIdentityStoreData.useMachineAccount())
                       .withServicePrincipalName(extendedIdentityStoreData.getServicePrincipalName())
                       .withUserBaseDN(extendedIdentityStoreData.getUserBaseDn())
                       .withGroupBaseDN(extendedIdentityStoreData.getGroupBaseDn())
                       .withConnectionStrings(extendedIdentityStoreData.getConnectionStrings())
                       .withAttributesMap(extendedIdentityStoreData.getAttributeMap())
                       .withUPNSuffixes(extendedIdentityStoreData.getUpnSuffixes())
                       .withMatchingRuleInChainEnabled(true)
                       .withBaseDnForNestedGroupsEnabled(true)
                       .withDirectGroupsSearchEnabled(true)
                       .withSiteAffinityEnabled(true);
        return providerBuilder.build();
    }

    private static IdentityProviderDTO getTestADIdentityProviderDTO() {
        IdentityStoreData identityStore = getTestADIdentityProvider();
        return getTestExternalIdentityProviderDTO(identityStore);
    }
}
