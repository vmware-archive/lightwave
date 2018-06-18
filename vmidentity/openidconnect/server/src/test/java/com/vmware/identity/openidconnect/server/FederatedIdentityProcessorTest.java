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

import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.client.CasIdmClient;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;

import static com.vmware.identity.openidconnect.server.TestContext.federationRelayState;
import static com.vmware.identity.openidconnect.server.TestContext.CLIENT_ID;
import static com.vmware.identity.openidconnect.server.TestContext.initialize;
import static com.vmware.identity.openidconnect.server.TestContext.REDIRECT_URI;
import static com.vmware.identity.openidconnect.server.TestContext.REDIRECT_URI_TEMPLATE;
import static com.vmware.identity.openidconnect.server.TestContext.idmClientBuilder;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_NAME;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_CERT;
import static com.vmware.identity.openidconnect.server.TestContext.TENANT_PRIVATE_KEY;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.fail;

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

        CSPIdentityProcessor identityProcessor = new CSPIdentityProcessor(idmClient, new SessionManager(), new FederationAuthenticationRequestTracker());
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

    @Test
    public void testUpdateUserGroups() throws Exception {
        String groupExtAdmins = "LWExtAdmins";
        String groupExtUsers = "LWExtUsers";
        Map<TokenClaimAttribute,List<String>> mappings = new HashMap<>();
        List<String> groupListLWUsers = new ArrayList<>();
        List<String> groupListLWAdmins = new ArrayList<>();
        groupListLWUsers.add(groupExtUsers);
        groupListLWAdmins.add(groupExtAdmins);
        mappings.put(new TokenClaimAttribute("perms", "ext_admin"), groupListLWAdmins);
        mappings.put(new TokenClaimAttribute("perms", "ext_user"), groupListLWUsers);

        IDPConfig externalIDP = TestContext.externalIDPConfig();
        externalIDP.setJitAttribute(true);
        externalIDP.setMultiTenant(true);
        externalIDP.setTokenClaimGroupMappings(mappings);

        CasIdmClient idmClient = createMock(CasIdmClient.class);

        FederatedIdentityProvider federatedIdp = new FederatedIdentityProvider(TENANT_NAME, idmClient);
        PrincipalId user = new PrincipalId("Administrator", TENANT_NAME);
        List<String> permissions = new ArrayList<>();
        permissions.add("ext_admin");

        Group userGroup = new Group(new PrincipalId(groupExtUsers, TENANT_NAME), null);
        Set<Group> parentGroups = new HashSet<>();
        parentGroups.add(userGroup);
        // existing group is users, token permission is admin. JIT admin group, place user in group and remove from users
        expect(idmClient.findGroup(TENANT_NAME, new PrincipalId(groupExtAdmins, TENANT_NAME))).andReturn(null);
        expect(idmClient.addGroup(TENANT_NAME, groupExtAdmins, new GroupDetail("JIT created group from federated IDP"))).andReturn(null);
        expect(idmClient.findGroup(TENANT_NAME, new PrincipalId(groupExtUsers, TENANT_NAME))).andReturn(userGroup);
        expect(idmClient.findDirectParentGroups(TENANT_NAME, user)).andReturn(parentGroups);
        expect(idmClient.addUserToGroup(TENANT_NAME, user, groupExtAdmins)).andReturn(true);
        expect(idmClient.removeFromLocalGroup(TENANT_NAME, user, groupExtUsers)).andReturn(true);
        replay(idmClient);

        federatedIdp.provisionIDPGroups(mappings);
        federatedIdp.updateUserGroups(user, permissions, mappings);

        verify(idmClient);
    }
}
