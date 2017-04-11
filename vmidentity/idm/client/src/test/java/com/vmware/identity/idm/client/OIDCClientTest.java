/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/**
 * Copyright (c) 2015 VMware, Inc. All rights reserved.
 */

package com.vmware.identity.idm.client;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.OIDCClient.Builder;
import com.vmware.identity.idm.Tenant;

public class OIDCClientTest {

    private static Properties props;
    private static CasIdmClient idmClient;
    private static OIDCClient oidcClient;

    private final static String clientID = "client";
    private final static List<String> redirectUris = Arrays.asList("https://www.vmware.com/redirect1", "https://www.vmware.com/redirect2");
    private final static String tokenEndpointAuthMethod = "private_key_jwt";
    private final static String tokenEndpointAuthSigningAlg = "RS256";
    private final static String idTokenSignedResponseAlg = "RS256";
    private final static List<String> postLogoutRedirectUris = Arrays.asList("https://www.vmware.com/postlogoutredirect1");
    private final static String logoutUri = "https://www.vmware.com/logout";
    private final static String certSubDN = "OU=mID-2400d17e-d4f4-4753-98fd-fb9ecbf098ae,C=US,DC=local,DC=vsphere,CN=oidc-client-123";
    private final static long authnRequestClientAssertionLifetimeMS = 1234L;

    @BeforeClass
    public static void setUp() throws Exception {
        props = IdmClientTestUtil.getProps();
        String hostname = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_HOSTNAME);
        Assert.assertNotNull(hostname);
        idmClient = new CasIdmClient(hostname);

        // Build oidcClient object
        Builder oidcClientBuilder = new Builder(clientID);
        oidcClientBuilder.redirectUris(redirectUris);
        oidcClientBuilder.tokenEndpointAuthMethod(tokenEndpointAuthMethod);
        oidcClientBuilder.tokenEndpointAuthSigningAlg(tokenEndpointAuthSigningAlg);
        oidcClientBuilder.idTokenSignedResponseAlg(idTokenSignedResponseAlg);
        oidcClientBuilder.postLogoutRedirectUris(postLogoutRedirectUris);
        oidcClientBuilder.logoutUri(logoutUri);
        oidcClientBuilder.certSubjectDN(certSubDN);
        oidcClientBuilder.authnRequestClientAssertionLifetimeMS(authnRequestClientAssertionLifetimeMS);
        oidcClient = oidcClientBuilder.build();
    }

    @Test
    public void testOIDCClientOps() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        try {
            // Add a client
            idmClient.addOIDCClient(tenantName, oidcClient);

            // Get a client
            OIDCClient result = idmClient.getOIDCClient(tenantName, oidcClient.getClientId());
            Assert.assertNotNull(result);

            // Get all clients in a tenant
            Collection<OIDCClient> results = null;
            results = idmClient.getOIDCClients(tenantName);
            Assert.assertTrue(!results.isEmpty());

            // Set a client
            idmClient.setOIDCClient(tenantName, oidcClient);
            result = idmClient.getOIDCClient(tenantName, oidcClient.getClientId());
            Assert.assertNotNull(result);

            // Delete a client
            idmClient.deleteOIDCClient(tenantName, oidcClient.getClientId());

            // Verify a client is delete
            boolean thrown = false;
            try {
                result = idmClient.getOIDCClient(tenantName, oidcClient.getClientId());
            } catch (NoSuchOIDCClientException e) {
                thrown = true;
            }
            Assert.assertTrue(thrown);
        } finally {
            // clean up tenant
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        }
    }

    @Test(expected=NoSuchOIDCClientException.class)
    public void testDeleteNonExistOIDCClient() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        try {
            // Add a client
            idmClient.addOIDCClient(tenantName, oidcClient);

            // Delete the same client twice
            idmClient.deleteOIDCClient(tenantName, oidcClient.getClientId());
            idmClient.deleteOIDCClient(tenantName, oidcClient.getClientId());
        } finally {
            // clean up tenant
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        }
    }

    @Test
    public void testOIDCEntityID() throws Exception, IDMException
    {
        String tenantName = props.getProperty(IdmClientTestUtil.CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        try {
            String expected = "https://abc.com:1234/openidconnect/" + tenantName;

            // Set correct websso entity id
            idmClient.setEntityID(tenantName, "https://abc.com:1234/websso/SAML2/Metadata/" + tenantName);

            // Get OIDC entity id
            String actual = idmClient.getOIDCEntityID(tenantName);
            Assert.assertEquals(expected, actual);

            // Set wrong websso entity id
            idmClient.setEntityID(tenantName, "https://xyz.com:5678/endpoint");

            String oidcSuffix = String.format("/openidconnect/%s", tenantName);

            // Get OIDC entity id
            actual = idmClient.getOIDCEntityID(tenantName);
            Assert.assertTrue(actual.endsWith(oidcSuffix));
        } finally {
            // clean up tenant
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        }
    }

    @Test(expected=IllegalArgumentException.class)
    public void testBuildClientInvalidRedirectURL() throws Exception, IDMException
    {
        Builder oidcClientBuilder = new Builder(clientID);
        oidcClientBuilder.redirectUris(Arrays.asList("http://www.vmware.com/redirect1"));
        oidcClientBuilder.tokenEndpointAuthMethod(tokenEndpointAuthMethod);
        oidcClientBuilder.tokenEndpointAuthSigningAlg(tokenEndpointAuthSigningAlg);
        oidcClientBuilder.idTokenSignedResponseAlg(idTokenSignedResponseAlg);
        oidcClientBuilder.postLogoutRedirectUris(postLogoutRedirectUris);
        oidcClientBuilder.logoutUri(logoutUri);
        oidcClientBuilder.certSubjectDN(certSubDN);
        oidcClientBuilder.authnRequestClientAssertionLifetimeMS(authnRequestClientAssertionLifetimeMS);
        oidcClientBuilder.build();
    }
}
