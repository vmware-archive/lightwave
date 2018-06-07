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

import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.TokenType;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;

/**
 * OIDC Client Integration Test
 *
 * @author Jun Sun
 */
public class OIDCClientIT extends OIDCClientITBase {

    @BeforeClass
    public static void setUp() throws Exception {
        setUp("config.properties");
    }

    @AfterClass
    public static void cleanup() throws Exception {
        tearDown();
    }

    // NonRegNoHOKConfigClient, PasswordGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegNoHOKConfigClient,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegNoHOKConfigClient,
                passwordGrantForSystemTenant,
                withoutRefreshSpec);
    }

    // NonRegNoHOKConfigClient, SolutionUserCredentialsGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                solutionUserCredentialsGrant,
                withRefreshSpec,
                "Solution user credentials grant requires an non-null solution user assertion.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                solutionUserCredentialsGrant,
                withoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution user assertion.");
    }

    // NonRegNoHOKConfigClient, ClientCredentialsGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                clientCredentialsGrant,
                withRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                clientCredentialsGrant,
                withoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    // NonRegHOKConfigClient, PasswordGrant
    @Test
    public void testNonRegHOKConfigClientGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegHOKConfigClient,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                passwordGrantForSystemTenant,
                withoutRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegHOKConfigClient,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                passwordGrantForSystemTenant,
                withoutRefreshSpec);
    }

    // NonRegHOKConfigClient, SolutionUserCredentialsGrant
    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                solutionUserCredentialsGrant,
                withoutRefreshSpec);
    }

    // NonRegHOKConfigClient, ClientCredentialsGrant
    @Test
    public void testNonRegHOKConfigClientGetBearerWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                withRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetBearerWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                withoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                withRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                withoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    // RegClient, PasswordGrant
    @Test
    public void testRegClientGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClient,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    @Test
    public void testRegClientGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                passwordGrantForSystemTenant,
                withoutRefreshSpec);
    }

    @Test
    public void testRegClientGetHOKWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClient,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    @Test
    public void testRegClientGetHOKWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                passwordGrantForSystemTenant,
                withoutRefreshSpec);
    }

    // RegClient, SolutionUserCredentialsGrant
    @Test
    public void testRegClientGetBearerWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                withRefreshSpec,
                "Solution user credentials grant requires an non-null solution user assertion.");
    }

    @Test
    public void testRegClientGetBearerWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                withoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution user assertion.");
    }

    @Test
    public void testRegClientGetHOKWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                withRefreshSpec,
                "Solution user credentials grant requires an non-null solution user assertion.");
    }

    @Test
    public void testRegClientGetHOKWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                withoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution user assertion.");
    }

    // RegClient, ClientCredentialsGrant
    @Test
    public void testRegClientGetHOKWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                clientCredentialsGrant,
                withoutRefreshSpec);
    }

    // RegClientWithHA, PasswordGrant
    @Test
    public void testRegClientWithHAGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClientWithHA,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    // RegClientWithoutAuthn
    @Test
    public void testRegClientWithoutAuthnGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClientWithoutAuthn,
                passwordGrantForSystemTenant,
                withRefreshSpec);
    }

    @Test
    public void testRegClientWithoutAuthnGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClientWithoutAuthn,
                passwordGrantForSystemTenant,
                withoutRefreshSpec);
    }

    @Test
    public void testRegClientWithoutAuthnGetHOKWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClientWithoutAuthn,
                solutionUserCredentialsGrant,
                withoutRefreshSpec);
    }

    @Test
    public void testRegClientWithoutAuthnGetHOKWithouRefreshByMultiTenantSolutionUser() throws Exception {
        TestUtils.verifyTokens(
                regClientWithoutAuthnForRegularTenant,
                solutionUserCredentialsGrant,
                withoutRefreshSpec);
    }

    @Test
    public void testRegClientWithoutAuthnGetHOKWithouRefreshByMultiTenantOidcClient() throws Exception {
        TestUtils.verifyTokens(
                regClientWithoutAuthnForRegularTenantWithMultiTenant,
                passwordGrantForRegularTenant,
                withoutRefreshSpec);
    }

    @Test
    public void testGetMultiTenantOIDCClientWithRegularTenantUserCredentials() throws Exception {
        OIDCClientDTO oidcClient = idmClientForRegularTenant.oidcClient().get(regularTenant, clientIdWithMultiTenant.getValue());

        verifyOidcUri(oidcClient.getOIDCClientMetadataDTO().getLogoutUri(), regularTenant);

        for (String uriString : oidcClient.getOIDCClientMetadataDTO().getPostLogoutRedirectUris()) {
            verifyOidcUri(uriString, regularTenant);
        }

        for (String uriString : oidcClient.getOIDCClientMetadataDTO().getRedirectUris()) {
            verifyOidcUri(uriString, regularTenant);
        }
    }

    private void verifyOidcUri(String uriString, String tenant) throws URISyntaxException {
        Assert.assertTrue(uriString.endsWith(tenant));
    }

    @Test
    public void testGroupFiltering() throws Exception {
        OIDCTokens tokens = nonRegNoHOKConfigClient.acquireTokens(passwordGrantForSystemTenant, groupFilteringSpec);
        ResourceServerAccessToken accessToken = ResourceServerAccessToken.build(
                tokens.getAccessToken().getValue(),
                connectionConfigForSystemTenant.getProviderPublicKey(),
                connectionConfigForSystemTenant.getIssuer(),
                RESOURCE_SERVER_NAME,
                5 * 60L /* clockToleranceInSeconds */);
        Collection<String> actualGroups = accessToken.getGroups();
        List<String> expectedGroups = Arrays.asList(systemTenant + "\\administrators");
        Assert.assertEquals("groups", expectedGroups, actualGroups);
    }

    @Test
    public void testAdminServerResourceRequestUsingHOKToken() throws Exception {
        OIDCTokens tokens = nonRegHOKConfigClient.acquireTokensBySolutionUserCredentials(withoutRefreshSpec);
        Assert.assertEquals("token type", TokenType.HOK, tokens.getIDToken().getTokenType());
        IdmClient idmClient = TestUtils.createIdmClient(
                tokens.getAccessToken(),
                domainControllerFQDN,
                domainControllerPort,
                ks,
                clientPrivateKey1);
        String[] parts = systemTenantAdminUsername.split("@");
        idmClient.user().get(systemTenant, parts[0], parts[1]);
    }
}
