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
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

import javax.xml.soap.SOAPException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.UPNUtil;
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
    protected static VmdirClient systemAdminVmdirClient;
    protected static IdmClient testAdminClient;
    protected static VmdirClient testVmdirClient;
    protected static String systemTenantName;

    protected static final AccessToken.Type defaultTokenType = AccessToken.Type.JWT;

    public static void init(boolean withTestTenant) throws IOException, GeneralSecurityException, ClientException, HttpException, SOAPException {
        properties = new IntegrationTestProperties();
        systemTenantName = properties.getSystemTenant();

        initSystemTenantClients(defaultTokenType);

        if (withTestTenant) {
            testTenant = TestGenerator.generateTenant();

            // Acts as a test, though it won't show in the JUnit output...
            TenantDTO created = systemAdminClient.tenant().create(testTenant);
            assertTenantEquals(testTenant, created);

            initTestTenantClients(defaultTokenType);
        }
    }

    public IntegrationTestBase(boolean withTestTenant, AccessToken.Type tokenType) throws IOException, GeneralSecurityException, ClientException, HttpException, SOAPException {
        initSystemTenantClients(tokenType);
        if (withTestTenant) {
            initTestTenantClients(tokenType);
        }
    }

    private static void initSystemTenantClients(AccessToken.Type tokenType) throws IOException, GeneralSecurityException, ClientException, HttpException, SOAPException {
        systemAdminClient = TestClientFactory.createClient(properties.getHost(),
                systemTenantName,
                properties.getSystemAdminUsername(),
                properties.getSystemDomain(),
                properties.getSystemAdminPassword(),
                tokenType);

        systemAdminVmdirClient = TestClientFactory.createVmdirClient(properties.getHost(),
                properties.getSystemTenant(),
                UPNUtil.buildUPN(properties.getSystemAdminUsername(), properties.getSystemDomain()),
                properties.getSystemAdminPassword(),
                tokenType);
    }

    private static void initTestTenantClients(AccessToken.Type tokenType) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException, SOAPException {
        testAdminClient = TestClientFactory.createClient(properties.getHost(),
                testTenant.getName(),
                testTenant.getUsername(),
                testTenant.getPassword(),
                tokenType);

        testVmdirClient = TestClientFactory.createVmdirClient(properties.getHost(),
                testTenant.getName(),
                testTenant.getUsername(),
                testTenant.getPassword(),
                tokenType);
    }

    public static void cleanup(boolean withTestTenant) throws ClientProtocolException, HttpException, ClientException, IOException {
        if (withTestTenant && systemAdminClient != null) {
            systemAdminClient.tenant().delete(testTenant.getName());
        }
    }

}
