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

package com.vmware.identity.openidconnect.client;

import java.io.IOException;
import java.net.URI;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;
import java.util.UUID;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.openidconnect.protocol.ClientCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.PasswordGrant;
import com.vmware.identity.openidconnect.protocol.SolutionUserCredentialsGrant;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;

/**
 * Base Class for OIDC Client Integration Test
 *
 * @author Jun Sun
 */
public class OIDCClientITBase {

    static final String RESOURCE_SERVER_NAME = "rs_oidc_client_integration_tests";
    static final long CLOCK_TOLERANCE_IN_SECONDS = 5 * 60L; // 5 mins

    static ClientID clientId;
    static ClientID clientIdWithoutAuthn;
    static ClientID clientIdWithoutAuthnForRegularTenant;
    static ClientID clientIdWithMultiTenant;
    static AccessToken accessTokenSystemTenant;
    static AccessToken accessTokenRegularTenant;
    static ConnectionConfig connectionConfigForSystemTenant;
    static ConnectionConfig connectionConfigForRegularTenant;
    static ClientConfig clientConfigForSystemTenant;
    static ClientConfig clientConfigForRegularTenant;
    static ClientConfig clientConfigForRegularTenantWithMultiTenant;
    static OIDCClient nonRegNoHOKConfigClient, nonRegHOKConfigClient, nonRegNoHOKConfigClientForRegularTenant,
                      regClient, regClientWithHA, regClientWithoutAuthn, regClientWithoutAuthnForRegularTenant, regClientWithoutAuthnForRegularTenantWithMultiTenant;
    static TokenSpec withRefreshSpec, withoutRefreshSpec, groupFilteringSpec;
    static PasswordGrant passwordGrantForSystemTenant;
    static PasswordGrant passwordGrantForRegularTenant;
    static SolutionUserCredentialsGrant solutionUserCredentialsGrant;
    static ClientCredentialsGrant clientCredentialsGrant;

    static VmdirClient vmdirClientForSystemTenant;
    static IdmClient idmClientForSystemTenant;
    static IdmClient idmClientForRegularTenant;
    static String systemTenant;
    static String regularTenant;
    static String systemTenantAdminUsername;
    static String regularTenantAdminUsername;
    static String solutionUserName;
    static String multiTenantSolutionUserName;
    static String domainControllerFQDN;
    static int domainControllerPort;
    static KeyStore ks;
    static RSAPrivateKey clientPrivateKey1;
    static RSAPrivateKey clientPrivateKey2;
    static X509Certificate clientCert1;
    static X509Certificate clientCert2;

    public static void setUp(String config) throws Exception {
        Properties properties = new Properties();
        properties.load(OIDCClientIT.class.getClassLoader().getResourceAsStream(config));
        systemTenantAdminUsername = properties.getProperty("admin.user");
        String systemTenantAdminPassword = properties.getProperty("admin.password");
        regularTenantAdminUsername = properties.getProperty("tenant1.admin.user");
        String regularTenantAdminPassword = properties.getProperty("tenant1.admin.password");
        systemTenant = properties.getProperty("tenant");
        regularTenant= properties.getProperty("tenant1");
        domainControllerPort = Integer.parseInt(properties.getProperty("oidc.op.port"));
        domainControllerFQDN = System.getProperty("host");
        if (domainControllerFQDN == null || domainControllerFQDN.length() == 0) {
            throw new IllegalStateException("missing host argument, invoke with mvn verify -DskipIntegrationTests=false -Dhost=<host>");
        }

        // create admin client with STS token
        ks = KeyStore.getInstance("JKS");
        ks.load(null, null);

        // create REST afd client to populate SSL certificates
        TestUtils.populateSSLCertificates(
                domainControllerFQDN,
                domainControllerPort,
                ks);

        // retrieve OIDC meta data from system tenant
        MetadataHelper metadataHelper = new MetadataHelper.Builder(domainControllerFQDN)
                .domainControllerPort(domainControllerPort)
                .tenant(systemTenant)
                .keyStore(ks).build();

        ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
        RSAPublicKey providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

        connectionConfigForSystemTenant = new ConnectionConfig(providerMetadata, providerPublicKey, ks);
        clientConfigForSystemTenant = new ClientConfig(connectionConfigForSystemTenant, null, null, CLOCK_TOLERANCE_IN_SECONDS);
        accessTokenSystemTenant = TestUtils.getOidcBearerTokenWithUsernamePassword(clientConfigForSystemTenant, systemTenantAdminUsername, systemTenantAdminPassword);

        // create REST idm client for system tenant
        idmClientForSystemTenant = TestUtils.createIdmClient(
                accessTokenSystemTenant,
                domainControllerFQDN,
                domainControllerPort,
                ks,
                null);


        // Create REST Vmdir client for system tenant
        vmdirClientForSystemTenant = TestUtils.createVMdirClient(
                accessTokenSystemTenant,
                domainControllerFQDN,
                domainControllerPort,
                ks);

        // create a non-system tenant
        createTenant(regularTenant, regularTenantAdminUsername, regularTenantAdminPassword, properties.getProperty("tenant1.issuer"), idmClientForSystemTenant);
        Thread.sleep(15 * 1000); // wait for tenant creation to finish

        // retrieve OIDC meta data from regular tenant
        metadataHelper = new MetadataHelper.Builder(domainControllerFQDN)
                .domainControllerPort(domainControllerPort)
                .tenant(regularTenant)
                .keyStore(ks).build();

        providerMetadata = metadataHelper.getProviderMetadata();
        providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

        connectionConfigForRegularTenant = new ConnectionConfig(providerMetadata, providerPublicKey, ks);
        clientConfigForRegularTenant = new ClientConfig(connectionConfigForRegularTenant, null, null, CLOCK_TOLERANCE_IN_SECONDS);
        accessTokenRegularTenant =  TestUtils.getOidcBearerTokenWithUsernamePassword(clientConfigForRegularTenant, regularTenantAdminUsername, regularTenantAdminPassword);

        // create REST idm client for non-system tenant
        idmClientForRegularTenant = TestUtils.createIdmClient(
                accessTokenRegularTenant,
                domainControllerFQDN,
                domainControllerPort,
                ks,
                null);

        // create key pair and client private key, certificate
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(2048, new SecureRandom());
        KeyPair clientKeyPair = keyGen.generateKeyPair();

        // create a regular solution user
        solutionUserName = properties.getProperty("oidc.rp.prefix") + UUID.randomUUID().toString();
        clientPrivateKey1 = (RSAPrivateKey) clientKeyPair.getPrivate();
        clientCert1 = TestUtils.generateCertificate(clientKeyPair, solutionUserName, null);
        createSolutionUser(solutionUserName, systemTenant, false, vmdirClientForSystemTenant, clientCert1);

        // create a multi tenant solution user
        multiTenantSolutionUserName = properties.getProperty("oidc.rp.prefix") + UUID.randomUUID().toString();
        clientPrivateKey2 = (RSAPrivateKey) clientKeyPair.getPrivate();
        clientCert2 = TestUtils.generateCertificate(clientKeyPair, multiTenantSolutionUserName, null);
        createSolutionUser(multiTenantSolutionUserName, systemTenant, true, vmdirClientForSystemTenant, clientCert2);

        clientId = registerOidcClient(systemTenant, false, clientCert1.getSubjectDN().getName(), "private_key_jwt", idmClientForSystemTenant);
        clientIdWithoutAuthn = registerOidcClient(systemTenant, false, null, "none", idmClientForSystemTenant);
        clientIdWithoutAuthnForRegularTenant = registerOidcClient(regularTenant, false, null, "none", idmClientForRegularTenant);
        clientIdWithMultiTenant = registerOidcClient(systemTenant, true, null, "none", idmClientForSystemTenant);

        // register resource server for group filtering tests
        Set<String> groupFilter = new HashSet<String>(Arrays.asList("dummyDomain\\dummyUser", systemTenant + "\\Administrators"));
        ResourceServerDTO resourceServer = new ResourceServerDTO.Builder().withName(RESOURCE_SERVER_NAME).withGroupFilter(groupFilter).build();
        idmClientForSystemTenant.resourceServer().register(systemTenant, resourceServer);

        // create client config
        HolderOfKeyConfig holderOfKeyConfig1 = new HolderOfKeyConfig(clientPrivateKey1, clientCert1);
        HolderOfKeyConfig holderOfKeyConfig2 = new HolderOfKeyConfig(clientPrivateKey2, clientCert2);

        // create clients
        clientConfigForSystemTenant = new ClientConfig(connectionConfigForSystemTenant, null, null, CLOCK_TOLERANCE_IN_SECONDS);
        nonRegNoHOKConfigClient = new OIDCClient(clientConfigForSystemTenant);
        clientConfigForSystemTenant = new ClientConfig(connectionConfigForSystemTenant, null, holderOfKeyConfig1, CLOCK_TOLERANCE_IN_SECONDS);
        nonRegHOKConfigClient = new OIDCClient(clientConfigForSystemTenant);
        clientConfigForSystemTenant = new ClientConfig(connectionConfigForSystemTenant, clientId, holderOfKeyConfig1, null, CLOCK_TOLERANCE_IN_SECONDS, ClientAuthenticationMethod.PRIVATE_KEY_JWT);
        regClient = new OIDCClient(clientConfigForSystemTenant);
        clientConfigForSystemTenant = new ClientConfig(connectionConfigForSystemTenant, clientIdWithoutAuthn, holderOfKeyConfig1, CLOCK_TOLERANCE_IN_SECONDS);
        regClientWithoutAuthn = new OIDCClient(clientConfigForSystemTenant);
        clientConfigForRegularTenant = new ClientConfig(connectionConfigForRegularTenant, clientIdWithoutAuthnForRegularTenant, holderOfKeyConfig2, CLOCK_TOLERANCE_IN_SECONDS);
        regClientWithoutAuthnForRegularTenant = new OIDCClient(clientConfigForRegularTenant);
        clientConfigForRegularTenantWithMultiTenant = new ClientConfig(connectionConfigForRegularTenant, clientIdWithMultiTenant, holderOfKeyConfig2, null, CLOCK_TOLERANCE_IN_SECONDS, null);
        regClientWithoutAuthnForRegularTenantWithMultiTenant = new OIDCClient(clientConfigForRegularTenantWithMultiTenant);

        // create registered HA client
        String domainName = null;
        URI issuerUri = URI.create(connectionConfigForSystemTenant.getIssuer().getValue());
        String availableDomainController = issuerUri.getHost();
        ClientDCCacheFactory factory = new MockClientDCCacheFactory(domainName, availableDomainController);
        HighAvailabilityConfig haConfig = new HighAvailabilityConfig(domainName, factory);
        clientConfigForSystemTenant = new ClientConfig(connectionConfigForSystemTenant, clientId, holderOfKeyConfig1, haConfig, CLOCK_TOLERANCE_IN_SECONDS, ClientAuthenticationMethod.PRIVATE_KEY_JWT);
        regClientWithHA = new OIDCClient(clientConfigForSystemTenant);

        withRefreshSpec = new TokenSpec.Builder().
                refreshToken(true).
                idTokenGroups(GroupMembershipType.FULL).
                accessTokenGroups(GroupMembershipType.FULL).
                resourceServers(Arrays.asList("rs_admin_server")).build();
        withoutRefreshSpec = new TokenSpec.Builder().
                idTokenGroups(GroupMembershipType.FULL).
                accessTokenGroups(GroupMembershipType.FULL).
                resourceServers(Arrays.asList("rs_admin_server")).build();
        groupFilteringSpec = new TokenSpec.Builder().
                idTokenGroups(GroupMembershipType.NONE).
                accessTokenGroups(GroupMembershipType.FILTERED).
                resourceServers(Arrays.asList(RESOURCE_SERVER_NAME)).build();

        // create grants
        passwordGrantForSystemTenant = new PasswordGrant(systemTenantAdminUsername, systemTenantAdminPassword);
        passwordGrantForRegularTenant = new PasswordGrant(regularTenantAdminUsername, regularTenantAdminPassword);
        solutionUserCredentialsGrant = new SolutionUserCredentialsGrant();
        clientCredentialsGrant = new ClientCredentialsGrant();
    }

    private static TenantDTO createTenant(String tenant, String adminUsername, String adminPassword,
            String issuer, IdmClient idmClient) throws Exception {
        TenantDTO tenantDTO = new TenantDTO.Builder()
            .withName(tenant)
            .withUsername(adminUsername)
            .withPassword(adminPassword)
            .withIssuer(issuer)
            .build();
        return idmClient.tenant().create(tenantDTO);
    }

    private static void createSolutionUser(String username, String tenant, boolean multiTenant,
            VmdirClient vmdirClient, X509Certificate cert) throws Exception {
        CertificateDTO certificateDTO = new CertificateDTO.Builder()
                .withEncoded(TestUtils.convertToBase64PEMString(cert))
                .build();

        SolutionUserDTO solutionUserDTO = new SolutionUserDTO.Builder()
                .withName(username)
                .withDomain(tenant)
                .withCertificate(certificateDTO)
                .withMultiTenant(multiTenant)
                .build();
        vmdirClient.solutionUser().create(tenant, solutionUserDTO);

        // add the solution user to ActAs group and Users group
        List<String> members = Arrays.asList(username + "@" + tenant);
        vmdirClient.group().addMembers(tenant, "ActAsUsers", tenant, members, MemberType.USER);
        vmdirClient.group().addMembers(tenant, "Users", tenant, members, MemberType.USER);
    }

    private static ClientID registerOidcClient(String tenant, Boolean multiTenant, String subjectDN, String authnMethod, IdmClient idmClient)
            throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException {
        String tenantPlaceholder = "/{tenant}";
        String redirecURI = "https://test.com:7444/openidconnect/redirect";
        if (multiTenant == Boolean.TRUE) {
            redirecURI += tenantPlaceholder;
        }
        List<String> redirectURIs = Arrays.asList(redirecURI);

        String postLogoutRedirectURI = "https://test.com:7444/openidconnect/postlogout";
        if (multiTenant == Boolean.TRUE) {
            postLogoutRedirectURI += tenantPlaceholder;
        }
        List<String> postLogoutRedirectURIs = Arrays.asList(postLogoutRedirectURI);

        String logoutURI = "https://test.com:7444/openidconnect/logout";
        if (multiTenant == Boolean.TRUE) {
            logoutURI += tenantPlaceholder;
        }

        OIDCClientMetadataDTO.Builder oidcMetadataBuilder = new OIDCClientMetadataDTO.Builder()
                .withTokenEndpointAuthMethod(authnMethod)
                .withCertSubjectDN(subjectDN)
                .withMultiTenant(multiTenant);

        if (multiTenant == Boolean.TRUE) {
            oidcMetadataBuilder.withLogoutUriTemplate(logoutURI)
                    .withPostLogoutRedirectUriTemplates(postLogoutRedirectURIs)
                    .withRedirectUriTemplates(redirectURIs);
        } else {
            oidcMetadataBuilder.withRedirectUris(redirectURIs)
                   .withPostLogoutRedirectUris(postLogoutRedirectURIs)
                   .withLogoutUri(logoutURI);
        }
        OIDCClientDTO oidcClientDTO = idmClient.oidcClient().register(tenant, oidcMetadataBuilder.build());
        return new ClientID(oidcClientDTO.getClientId());
    }

    public static void tearDown() throws Exception {
        idmClientForSystemTenant.tenant().delete(regularTenant);
        vmdirClientForSystemTenant.user().delete(systemTenant, solutionUserName, systemTenant);
        vmdirClientForSystemTenant.user().delete(systemTenant, multiTenantSolutionUserName, systemTenant);
        idmClientForSystemTenant.oidcClient().delete(systemTenant, clientId.getValue());
        idmClientForSystemTenant.oidcClient().delete(systemTenant, clientIdWithoutAuthn.getValue());
        idmClientForSystemTenant.oidcClient().delete(systemTenant, clientIdWithMultiTenant.getValue());
        idmClientForSystemTenant.resourceServer().delete(systemTenant, RESOURCE_SERVER_NAME);
    }
}
