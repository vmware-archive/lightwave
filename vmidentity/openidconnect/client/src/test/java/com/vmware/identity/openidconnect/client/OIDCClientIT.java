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

import org.junit.BeforeClass;
import org.junit.Test;

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

    // NonRegNoHOKConfigClient, PasswordCredentialsGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegNoHOKConfigClient,
                passwordCredentialsGrant,
                bearerWithRefreshSpec);
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegNoHOKConfigClient,
                passwordCredentialsGrant,
                bearerWithoutRefreshSpec);
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                passwordCredentialsGrant,
                hokWithRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                passwordCredentialsGrant,
                hokWithoutRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    // NonRegNoHOKConfigClient, SolutionUserCredentialsGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                solutionUserCredentialsGrant,
                bearerWithRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                solutionUserCredentialsGrant,
                bearerWithoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                solutionUserCredentialsGrant,
                hokWithRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                solutionUserCredentialsGrant,
                hokWithoutRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    // NonRegNoHOKConfigClient, ClientCredentialsGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                clientCredentialsGrant,
                bearerWithRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                clientCredentialsGrant,
                bearerWithoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                clientCredentialsGrant,
                hokWithRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                clientCredentialsGrant,
                hokWithoutRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    // NonRegHOKConfigClient, PasswordCredentialsGrant
    @Test
    public void testNonRegHOKConfigClientGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegHOKConfigClient,
                passwordCredentialsGrant,
                bearerWithRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                passwordCredentialsGrant,
                bearerWithoutRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegHOKConfigClient,
                passwordCredentialsGrant,
                hokWithRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                passwordCredentialsGrant,
                hokWithoutRefreshSpec);
    }

    // NonRegHOKConfigClient, SolutionUserCredentialsGrant
    @Test
    public void testNonRegHOKConfigClientGetBearerWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                solutionUserCredentialsGrant,
                bearerWithRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetBearerWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                solutionUserCredentialsGrant,
                bearerWithoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCServerException(
                nonRegHOKConfigClient,
                solutionUserCredentialsGrant,
                hokWithRefreshSpec,
                "Server error response. Error code: invalid_scope; Error description: refresh token (offline_access) is not allowed for this grant_type.");
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                solutionUserCredentialsGrant,
                hokWithoutRefreshSpec);
    }

    // NonRegHOKConfigClient, ClientCredentialsGrant
    @Test
    public void testNonRegHOKConfigClientGetBearerWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                bearerWithRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetBearerWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                bearerWithoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                hokWithRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegHOKConfigClient,
                clientCredentialsGrant,
                hokWithoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    // RegClient, PasswordCredentialsGrant
    @Test
    public void testRegClientGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClient,
                passwordCredentialsGrant,
                bearerWithRefreshSpec);
    }

    @Test
    public void testRegClientGetBearerWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                passwordCredentialsGrant,
                bearerWithoutRefreshSpec);
    }

    @Test
    public void testRegClientGetHOKWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClient,
                passwordCredentialsGrant,
                hokWithRefreshSpec);
    }

    @Test
    public void testRegClientGetHOKWithoutRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                passwordCredentialsGrant,
                hokWithoutRefreshSpec);
    }

    // RegClient, SolutionUserCredentialsGrant
    @Test
    public void testRegClientGetBearerWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                bearerWithRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testRegClientGetBearerWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                bearerWithoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testRegClientGetHOKWithRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                hokWithRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    @Test
    public void testRegClientGetHOKWithoutRefreshBySolutionUserCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                solutionUserCredentialsGrant,
                hokWithoutRefreshSpec,
                "Solution user credentials grant requires an non-null solution assertion.");
    }

    // RegClient, ClientCredentialsGrant
    @Test
    public void testRegClientGetBearerWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                clientCredentialsGrant,
                bearerWithRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testRegClientGetBearerWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                regClient,
                clientCredentialsGrant,
                bearerWithoutRefreshSpec,
                "Client credentials grant requires an non-null client assertion.");
    }

    @Test
    public void testRegClientGetHOKWithRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyOIDCServerException(
                regClient,
                clientCredentialsGrant,
                hokWithRefreshSpec,
                "Server error response. Error code: invalid_scope; Error description: refresh token (offline_access) is not allowed for this grant_type.");
    }

    @Test
    public void testRegClientGetHOKWithoutRefreshByClientCredentialsGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                clientCredentialsGrant,
                hokWithoutRefreshSpec);
    }

    // RegClientWithHA, PasswordCredentialsGrant
    @Test
    public void testRegClientWithHAGetBearerWithRefreshByPasswordCredentialsGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClientWithHA,
                passwordCredentialsGrant,
                bearerWithRefreshSpec);
    }
}
