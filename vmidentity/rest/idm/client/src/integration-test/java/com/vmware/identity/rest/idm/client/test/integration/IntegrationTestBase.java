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

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertTenantEquals;

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.test.integration.IntegrationTestProperties;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.client.test.integration.util.TestClientFactory;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.TenantDTO;

public class IntegrationTestBase {

    protected static IntegrationTestProperties properties;
    protected static TenantDTO testTenant;

    protected static IdmClient systemAdminClient;
    protected static IdmClient testAdminClient;

    public static void init(boolean withTestTenant) throws IOException, GeneralSecurityException, ClientException, HttpException {
        properties = new IntegrationTestProperties();

        systemAdminClient = TestClientFactory.createClient(properties.getHost(),
                properties.getSystemTenant(),
                properties.getSystemAdminUsername(),
                properties.getSystemDomain(),
                properties.getSystemAdminPassword());

        if (withTestTenant) {
            testTenant = TestGenerator.generateTenant();

            // Acts as a test, though it won't show in the JUnit output...
            TenantDTO created = systemAdminClient.tenant().create(testTenant);
            assertTenantEquals(testTenant, created);

            testAdminClient = TestClientFactory.createClient(properties.getHost(),
                    testTenant.getName(),
                    testTenant.getUsername(),
                    testTenant.getPassword());
        }
    }

    public static void cleanup(boolean withTestTenant) throws ClientProtocolException, HttpException, ClientException, IOException {
        if (withTestTenant && testAdminClient != null) {
            testAdminClient.tenant().delete(testTenant.getName());
        }
    }

}
