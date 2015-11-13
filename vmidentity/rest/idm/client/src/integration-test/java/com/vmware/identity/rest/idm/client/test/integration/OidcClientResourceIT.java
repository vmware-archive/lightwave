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
package com.vmware.identity.rest.idm.client.test.integration;

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertClientsEqual;
import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertContainsClient;
import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertMetadataEqual;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;

public class OidcClientResourceIT extends IntegrationTestBase {

    private static OIDCClientDTO testOidcClient;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);

        OIDCClientMetadataDTO metadata = TestGenerator.generateOIDCClientMetadata();

        testOidcClient = testAdminClient.oidcClient().register(testTenant.getName(), metadata);
        assertMetadataEqual(metadata, testOidcClient.getOIDCClientMetadataDTO());
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        try {
            testAdminClient.oidcClient().delete(testTenant.getName(), testOidcClient.getClientId());
        } finally {
            IntegrationTestBase.cleanup(true);
        }
    }

    @Test
    public void testGetAll() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<OIDCClientDTO> clients = testAdminClient.oidcClient().getAll(testTenant.getName());

        assertContainsClient(testOidcClient, clients);
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        OIDCClientDTO client = testAdminClient.oidcClient().get(testTenant.getName(), testOidcClient.getClientId());

        assertClientsEqual(testOidcClient, client);
    }

    @Test
    public void testUpdate() throws ClientProtocolException, HttpException, ClientException, IOException {
        OIDCClientDTO client = testAdminClient.oidcClient().update(testTenant.getName(), testOidcClient.getClientId(), testOidcClient.getOIDCClientMetadataDTO());

        assertClientsEqual(testOidcClient, client);
    }

}
