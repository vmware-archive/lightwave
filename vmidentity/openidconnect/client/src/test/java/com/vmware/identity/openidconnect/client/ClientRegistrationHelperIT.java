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
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Properties;
import java.util.Set;
import java.util.UUID;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * Client Registration Integration Test
 *
 * @author Jun Sun
 */
public class ClientRegistrationHelperIT {

    private static Properties properties;
    private static String username;
    private static String password;
    private static String tenant;
    private static ClientID clientId, clientId2;

    private static RSAPublicKey providerPublicKey;
    private static X509Certificate clientX509Certificate;

    private static ConnectionConfig connectionConfig;

    private static String solutionUser;

    private static KeyStore ks;

    private static ClientRegistrationHelper clientRegistrationHelper;
    private static AccessToken accessToken;

    private static OIDCClient nonRegisteredClient;

    @BeforeClass
    public static void setUp() throws Exception {
        properties = new Properties();
        properties.load(OIDCClientIT.class.getClassLoader().getResourceAsStream("config.properties"));
        username = properties.getProperty("admin.user");
        password = properties.getProperty("admin.password");
        tenant = properties.getProperty("tenant");
        String domainControllerFQDN = properties.getProperty("oidc.op.FQDN");
        int domainControllerPort = Integer.parseInt(properties.getProperty("oidc.op.port"));

        // create admin client with STS token
        ks = KeyStore.getInstance("JKS");
        ks.load(null, null);

        // all REST calls should be replaced with REST Admin client library later.
        AuthenticationFrameworkHelper authenticationFrameworkHelper = new AuthenticationFrameworkHelper(
                domainControllerFQDN,
                domainControllerPort);
        authenticationFrameworkHelper.populateSSLCertificates(ks);

        // create key pair and client private key, certificate
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair clientKeyPair = keyGen.generateKeyPair();
        solutionUser = properties.getProperty("oidc.rp.prefix") + UUID.randomUUID().toString();
        clientX509Certificate = TestUtils.generateCertificate(clientKeyPair, solutionUser);

        // Retrieve metadata
        MetadataHelper metadataHelper = new MetadataHelper.Builder(domainControllerFQDN)
        .domainControllerPort(domainControllerPort)
        .tenant(tenant)
        .keyStore(ks).build();
        ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
        providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

        // create client config
        connectionConfig = new ConnectionConfig(providerMetadata, providerPublicKey, ks);

        // create client registration helper
        clientRegistrationHelper = new ClientRegistrationHelper.Builder(domainControllerFQDN)
        .domainControllerPort(domainControllerPort)
        .tenant(tenant)
        .keyStore(ks).build();

        ClientConfig clientConfig = new ClientConfig(connectionConfig, null, null);
        nonRegisteredClient = new OIDCClient(clientConfig);

        PasswordCredentialsGrant passwordGrant = new PasswordCredentialsGrant(username, password);
        TokenSpec tokenSpec = new TokenSpec.Builder(TokenType.BEARER).
                refreshToken(true).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList("rs_admin_server")).build();

        OIDCTokens oidcTokens = nonRegisteredClient.acquireTokens(passwordGrant, tokenSpec);
        accessToken = oidcTokens.getAccessToken();
    }

    @Test
    public void testRegisterClient() throws Exception {
        try{
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();
            Assert.assertNotNull(clientId.getValue());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testGetAllClients() throws Exception {
        try {
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();

            // register another client
            redirectURI = new URI("https://test.com:7444/openidconnect/redirect2");
            postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout2");
            logoutURI = new URI("https://test.com:7444/openidconnect/logout2");
            redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation2 = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId2 = clientInformation2.getClientId();

            Collection<ClientInformation> clientInformations = clientRegistrationHelper.getAllClients(accessToken, TokenType.BEARER);
            clientInformations.contains(clientInformation);
            clientInformations.contains(clientInformation2);
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
            if (clientId2 != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId2);
            }
        }
    }

    @Test
    public void testGetClient() throws Exception {
        try {
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();

            ClientInformation clientInformationGet = clientRegistrationHelper.getClient(accessToken, TokenType.BEARER, clientId);
            Assert.assertEquals(clientInformation.getClientId().getValue(), clientInformationGet.getClientId().getValue());
            Assert.assertEquals(clientInformation.getRedirectUris(), clientInformationGet.getRedirectUris());
            Assert.assertEquals(clientInformation.getTokenEndpointAuthMethod(), clientInformationGet.getTokenEndpointAuthMethod());
            Assert.assertEquals(clientInformation.getPostLogoutRedirectUris(), clientInformationGet.getPostLogoutRedirectUris());
            Assert.assertEquals(clientInformation.getLogoutUri(), clientInformationGet.getLogoutUri());
            Assert.assertEquals(clientInformation.getCertSubjectDN(), clientInformationGet.getCertSubjectDN());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testDeleteClient() throws Exception {
        URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
        URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
        URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
        Set<URI> redirectURIs = new HashSet<URI>();
        redirectURIs.add(redirectURI);
        Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
        postLogoutRedirectURIs.add(postLogoutRedirectURI);

        ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                accessToken,
                TokenType.BEARER,
                redirectURIs,
                logoutURI,
                postLogoutRedirectURIs,
                ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                clientX509Certificate.getSubjectDN().getName());
        clientId = clientInformation.getClientId();

        clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);

        // expect the server to throw a not_found exception
        try {
            clientRegistrationHelper.getClient(accessToken, TokenType.BEARER, clientId);
        } catch (AdminServerException e) {
            Assert.assertEquals(404, e.getHttpStatusCode());
        }
    }

    @Test
    public void testUpdateClient() throws Exception {
        try {
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();

            // update client
            redirectURI = new URI("https://test.com:7444/openidconnect/redirect2");
            postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout2");
            logoutURI = new URI("https://test.com:7444/openidconnect/logout2");
            redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            clientInformation = clientRegistrationHelper.updateClient(
                    accessToken,
                    TokenType.BEARER,
                    clientId,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());

            // client id should be the same
            Assert.assertEquals(clientId.getValue(), clientInformation.getClientId().getValue());

            ClientInformation clientInformationGet = clientRegistrationHelper.getClient(accessToken, TokenType.BEARER, clientId);
            Assert.assertEquals(clientInformation.getClientId().getValue(), clientInformationGet.getClientId().getValue());
            Assert.assertEquals(clientInformation.getRedirectUris(), clientInformationGet.getRedirectUris());
            Assert.assertEquals(clientInformation.getTokenEndpointAuthMethod(), clientInformationGet.getTokenEndpointAuthMethod());
            Assert.assertEquals(clientInformation.getPostLogoutRedirectUris(), clientInformationGet.getPostLogoutRedirectUris());
            Assert.assertEquals(clientInformation.getLogoutUri(), clientInformationGet.getLogoutUri());
            Assert.assertEquals(clientInformation.getCertSubjectDN(), clientInformationGet.getCertSubjectDN());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testRegisterClientWithDuplicatedRedirectURIs() throws Exception {
        try{
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();
            Assert.assertNotNull(clientId.getValue());

            // register another client with same redirect URIs, expect the server to throw a bad request exception
            try {
                clientRegistrationHelper.registerClient(
                        accessToken,
                        TokenType.BEARER,
                        redirectURIs,
                        logoutURI,
                        postLogoutRedirectURIs,
                        ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                        clientX509Certificate.getSubjectDN().getName());
            } catch (AdminServerException e) {
                Assert.assertEquals(400, e.getHttpStatusCode());
                Assert.assertTrue(e.getCauseMessage().matches("At least one of the OIDC client redirect URI:(.*)is already registered."));
            }
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testRegisterClientWithNullLogoutURI() throws Exception {
        try{
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI postLogoutRedirectURI = new URI("https://test.com:7444/openidconnect/postlogout");
            URI logoutURI = null;
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();
            postLogoutRedirectURIs.add(postLogoutRedirectURI);

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();
            Assert.assertNotNull(clientId.getValue());
            Assert.assertNull(clientInformation.getLogoutUri());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testRegisterClientWithNullPostLogoutRedirectURI() throws Exception {
        try{
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = null;

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();
            Assert.assertNotNull(clientId.getValue());
            Assert.assertNull(clientInformation.getPostLogoutRedirectUris());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testRegisterClientWithEmptyPostLogoutRedirectURI() throws Exception {
        try{
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = new HashSet<URI>();

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.PRIVATE_KEY_JWT,
                    clientX509Certificate.getSubjectDN().getName());
            clientId = clientInformation.getClientId();
            Assert.assertNotNull(clientId.getValue());
            Assert.assertNull(clientInformation.getPostLogoutRedirectUris());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }

    @Test
    public void testRegisterClientWithNullCertSunjectDN() throws Exception {
        try{
            URI redirectURI = new URI("https://test.com:7444/openidconnect/redirect");
            URI logoutURI = new URI("https://test.com:7444/openidconnect/logout");
            Set<URI> redirectURIs = new HashSet<URI>();
            redirectURIs.add(redirectURI);
            Set<URI> postLogoutRedirectURIs = null;

            ClientInformation clientInformation = clientRegistrationHelper.registerClient(
                    accessToken,
                    TokenType.BEARER,
                    redirectURIs,
                    logoutURI,
                    postLogoutRedirectURIs,
                    ClientAuthenticationMethod.NONE,
                    null);
            clientId = clientInformation.getClientId();
            Assert.assertNotNull(clientId.getValue());
            Assert.assertNull(clientInformation.getCertSubjectDN());
        } finally {
            if (clientId != null) {
                clientRegistrationHelper.deleteClient(accessToken, TokenType.BEARER, clientId);
            }
        }
    }
}
