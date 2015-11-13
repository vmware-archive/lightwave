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

import org.apache.commons.lang3.SystemUtils;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * OIDC Client GSS Grant Integration Test
 *
 * @author Jun Sun
 */
public class OIDCClientGssIT extends OIDCClientITBase {

    @BeforeClass
    public static void setUp() throws Exception {
        setUp("resources/config.properties");
    }

    // NonRegNoHOKConfigClient, GssGrant
    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegNoHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                bearerWithRefreshSpec);
    }

    @Test
    public void testNonRegNoHOKConfigClientGetBearerWithoutRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegNoHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                bearerWithoutRefreshSpec);
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithRefreshByGssGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                hokWithRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    @Test
    public void testNonRegNoHOKConfigClientGetHOKWithoutRefreshByGssGrant() throws Exception {
        TestUtils.verifyOIDCClientException(
                nonRegNoHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                hokWithoutRefreshSpec,
                "Holder of key configuation can not be null if HOK token is requested.");
    }

    // NonRegHOKConfigClient, GssGrant
    @Test
    public void testNonRegHOKConfigClientGetBearerWithRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                bearerWithRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetBearerWithoutRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                bearerWithoutRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                nonRegHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                hokWithRefreshSpec);
    }

    @Test
    public void testNonRegHOKConfigClientGetHOKWithoutRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokens(
                nonRegHOKConfigClient,
                new GssGrant(getNegotiationHandler()),
                hokWithoutRefreshSpec);
    }

    // RegClient, GssGrant
    @Test
    public void testRegClientGetBearerWithRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClient,
                new GssGrant(getNegotiationHandler()),
                bearerWithRefreshSpec);
    }

    @Test
    public void testRegClientGetBearerWithoutRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                new GssGrant(getNegotiationHandler()),
                bearerWithoutRefreshSpec);
    }

    @Test
    public void testRegClientGetHOKWithRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokensWithRefresh(
                regClient,
                new GssGrant(getNegotiationHandler()),
                hokWithRefreshSpec);
    }

    @Test
    public void testRegClientGetHOKWithoutRefreshByGssGrant() throws Exception {
        TestUtils.verifyTokens(
                regClient,
                new GssGrant(getNegotiationHandler()),
                hokWithoutRefreshSpec);
    }

    private NegotiationHandler getNegotiationHandler() throws Exception {
        if (SystemUtils.IS_OS_LINUX) {
            return GSSTestUtils.getKerberosNegotiationHandler();
        } else {
            throw new UnsupportedOperationException("OS must be Linux.");
        }
    }
}
