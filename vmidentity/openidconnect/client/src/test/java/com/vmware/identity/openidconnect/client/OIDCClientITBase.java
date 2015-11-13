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
import java.util.Properties;
import java.util.Set;
import java.util.UUID;

import org.junit.AfterClass;

/**
 * Base Class for OIDC Client Integration Test
 *
 * @author Jun Sun
 */
public class OIDCClientITBase {

    static ClientID clientId;
    static ClientRegistrationHelper clientRegistrationByAdminHelper;
    static AccessToken accessToken;
    static OIDCClient nonRegNoHOKConfigClient, nonRegHOKConfigClient, regClient, regClientWithHA;
    static TokenSpec bearerWithRefreshSpec, bearerWithoutRefreshSpec, hokWithRefreshSpec, hokWithoutRefreshSpec;
    static PasswordCredentialsGrant passwordCredentialsGrant;
    static SolutionUserCredentialsGrant solutionUserCredentialsGrant;
    static ClientCredentialsGrant clientCredentialsGrant;

    public static void setUp(String config) throws Exception {
        Properties properties = new Properties();
        properties.load(OIDCClientIT.class.getClassLoader().getResourceAsStream(config));
        String username = properties.getProperty("admin.user");
        String password = properties.getProperty("admin.password");
        String tenant = properties.getProperty("tenant");
        String domainControllerFQDN = properties.getProperty("oidc.op.FQDN");
        int domainControllerPort = Integer.parseInt(properties.getProperty("oidc.op.port"));

        // create admin client with STS token
        KeyStore ks = KeyStore.getInstance("JKS");
        ks.load(null, null);

        // all REST calls should be replaced with REST Admin client library later.
        AuthenticationFrameworkHelper authenticationFrameworkHelper = new AuthenticationFrameworkHelper(
                domainControllerFQDN,
                domainControllerPort);
        authenticationFrameworkHelper.populateSSLCertificates(ks);

        AdminServerHelper adminServerHelper = new AdminServerHelper.Builder(domainControllerFQDN)
        .domainControllerPort(domainControllerPort)
        .tenant(tenant)
        .keyStore(ks).build();

        // retrieve OIDC meta data
        MetadataHelper metadataHelper = new MetadataHelper.Builder(domainControllerFQDN)
        .domainControllerPort(domainControllerPort)
        .tenant(tenant)
        .keyStore(ks).build();

        ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
        RSAPublicKey providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

        // create a non-registered OIDC client and get bearer tokens by admin user name/password
        ConnectionConfig connectionConfig = new ConnectionConfig(providerMetadata, providerPublicKey, ks);
        ClientConfig clientConfig = new ClientConfig(connectionConfig, null, null);
        nonRegNoHOKConfigClient = new OIDCClient(clientConfig);
        passwordCredentialsGrant = new PasswordCredentialsGrant(
                username,
                password);
        TokenSpec tokenSpec = new TokenSpec.Builder(TokenType.BEARER).resouceServers(Arrays.asList("rs_admin_server")).build();
        OIDCTokens oidcTokens = nonRegNoHOKConfigClient.acquireTokens(passwordCredentialsGrant, tokenSpec);
        accessToken = oidcTokens.getAccessToken();

        // create key pair and client private key, certificate
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair clientKeyPair = keyGen.generateKeyPair();
        RSAPrivateKey clientPrivateKey = (RSAPrivateKey) clientKeyPair.getPrivate();
        String solutionUserName = properties.getProperty("oidc.rp.prefix") + UUID.randomUUID().toString();
        X509Certificate clientCertificate = TestUtils.generateCertificate(clientKeyPair, solutionUserName);

        // call REST admin server to create a solution user
        adminServerHelper.createSolutionUser(
                accessToken,
                TokenType.BEARER,
                solutionUserName,
                clientCertificate);

        // call REST admin server to add the solution user to ActAs group
        String solutionUserUPN = solutionUserName + "@" + tenant;
        adminServerHelper.addSolutionUserToActAsUsersGroup(
                accessToken,
                TokenType.BEARER,
                solutionUserUPN);

        // create client config
        connectionConfig = new ConnectionConfig(providerMetadata, providerPublicKey, ks);
        HolderOfKeyConfig holderOfKeyConfig = new HolderOfKeyConfig(clientPrivateKey, clientCertificate);

        // create client registration helper
        clientRegistrationByAdminHelper = new ClientRegistrationHelper.Builder(domainControllerFQDN)
        .domainControllerPort(domainControllerPort)
        .tenant(tenant)
        .keyStore(ks).build();

        URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
        URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
        URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
        Set<URI> redirectURIs = new HashSet<URI>();
        redirectURIs.add(redirectURI);
        Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
        postLogoutRedirectURIs.add(postLogoutRedirectURI);

        ClientInformation clientInformation = clientRegistrationByAdminHelper.registerClient(
                accessToken,
                TokenType.BEARER,
                redirectURIs,
                logoutURI,
                postLogoutRedirectURIs,
                ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                clientCertificate.getSubjectDN().getName());
        clientId = clientInformation.getClientId();

        // create clients
        clientConfig = new ClientConfig(connectionConfig, null, null);
        nonRegNoHOKConfigClient = new OIDCClient(clientConfig);
        clientConfig = new ClientConfig(connectionConfig, null, holderOfKeyConfig);
        nonRegHOKConfigClient = new OIDCClient(clientConfig);
        clientConfig = new ClientConfig(connectionConfig, clientId, holderOfKeyConfig);
        regClient = new OIDCClient(clientConfig);

        // create registered HA client
        String domainName = null;
        String availableDomainController = connectionConfig.getIssuer().getURI().getHost();
        ClientDCCacheFactory factory = new MockClientDCCacheFactory(domainName, availableDomainController);
        HighAvailabilityConfig haConfig = new HighAvailabilityConfig(domainName, factory);
        clientConfig = new ClientConfig(connectionConfig, clientId, holderOfKeyConfig, haConfig);
        regClientWithHA = new OIDCClient(clientConfig);

        // create token specs
        bearerWithRefreshSpec = new TokenSpec.Builder(TokenType.BEARER).
                refreshToken(true).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList("rs_admin_server")).build();
        bearerWithoutRefreshSpec = new TokenSpec.Builder(TokenType.BEARER).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList("rs_admin_server")).build();
        hokWithRefreshSpec = new TokenSpec.Builder(TokenType.HOK).
                refreshToken(true).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList("rs_admin_server")).build();
        hokWithoutRefreshSpec = new TokenSpec.Builder(TokenType.HOK).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList("rs_admin_server")).build();

        // create grants
        passwordCredentialsGrant = new PasswordCredentialsGrant(username, password);
        solutionUserCredentialsGrant = new SolutionUserCredentialsGrant();
        clientCredentialsGrant = new ClientCredentialsGrant();
    }

    @AfterClass
    public static void tearDown() throws Exception {
        // TODO: delete solution user by calling REST admin client library

        // delete OIDC client here.
        clientRegistrationByAdminHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
    }
}
