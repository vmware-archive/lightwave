/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.openidconnect.server;

import static com.vmware.identity.openidconnect.server.TestContext.*;
import static org.junit.Assert.fail;

import java.util.UUID;

import org.junit.Test;

import com.vmware.identity.idm.client.CasIdmClient;

import org.junit.Assert;
import org.junit.BeforeClass;

public class FederatedIdentityProcessorTest {

    @BeforeClass
    public static void setup() throws Exception {
        initialize();
    }

    @Test
    public void testValidateOIDClient() throws ServerException {
        FederationRelayState relayStateWithValidaRedirectUri = federationRelayState(CLIENT_ID, REDIRECT_URI.toString());
        FederationRelayState relayStateWithInvalidClientId = federationRelayState(UUID.randomUUID().toString(), REDIRECT_URI.toString());
        FederationRelayState relayStateWithInvalidRedirectUri = federationRelayState(CLIENT_ID, "https://localhost/" + UUID.randomUUID().toString());
        FederationRelayState relayStateWithValidRedirectUriTemplate = federationRelayState(CLIENT_ID, REDIRECT_URI_TEMPLATE);
        FederationRelayState relayStateWithInvalidRedirectUriTemplate = federationRelayState(CLIENT_ID, "https://localhost/" + UUID.randomUUID().toString() + "/{tenant}");
        CasIdmClient idmClient = idmClientBuilder()
                .tenantName(TENANT_NAME)
                .redirectUri(REDIRECT_URI.toString())
                .redirectUriTemplate(REDIRECT_URI_TEMPLATE)
                .clientId(CLIENT_ID)
                .multiTenant(true)
                .tenantCertificate(TENANT_CERT)
                .tenantPrivateKey(TENANT_PRIVATE_KEY)
                .build();

        CSPIdentityProcessor identityProcessor = new CSPIdentityProcessor(idmClient, new SessionManager());
        identityProcessor.validateOIDCClient(relayStateWithValidaRedirectUri);
        identityProcessor.validateOIDCClient(relayStateWithValidRedirectUriTemplate);

        Exception ex = null;
        ServerException serverEx = null;
        try {
            identityProcessor.validateOIDCClient(relayStateWithInvalidClientId);
        } catch (Exception e) {
            ex = e;
        }
        if (ex == null || ! (ex instanceof ServerException)) {
            fail("Unexpected exception!");
        }
        serverEx = (ServerException) ex;
        Assert.assertTrue(serverEx.getErrorObject().getDescription().equals("invalid oidc client"));

        ex = null;
        try {
            identityProcessor.validateOIDCClient(relayStateWithInvalidRedirectUri);
        } catch (Exception e) {
            ex = e;
        }
        if (ex == null || ! (ex instanceof ServerException)) {
            fail("Unexpected exception!");
        }
        serverEx = (ServerException) ex;
        Assert.assertTrue(serverEx.getErrorObject().getDescription().equals("unregistered redirect_uri"));

        ex = null;
        try {
            identityProcessor.validateOIDCClient(relayStateWithInvalidRedirectUriTemplate);
        } catch (Exception e) {
            ex = e;
        }
        if (ex == null || ! (ex instanceof ServerException)) {
            fail("Unexpected exception!");
        }
        serverEx = (ServerException) ex;
        Assert.assertTrue(serverEx.getErrorObject().getDescription().equals("unregistered redirect_uri"));
    }
}
