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
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.LockoutPolicyDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;

public class TenantResourceIT extends IntegrationTestBase {

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        TenantDTO got = testAdminClient.tenant().get(testTenant.getName());
        assertTenantEquals(testTenant, got);
    }

    @Test
    public void testGetConfig() throws ClientProtocolException, HttpException, ClientException, IOException {
        TenantConfigurationDTO config = testAdminClient.tenant().getConfig(testTenant.getName());

        assertNotNull(config);
        assertNotNull(config.getAuthenticationPolicy());
        assertNotNull(config.getBrandPolicy());
        assertNotNull(config.getLockoutPolicy());
        assertNotNull(config.getPasswordPolicy());
        assertNotNull(config.getProviderPolicy());
        assertNotNull(config.getTokenPolicy());
    }

    @Test
    public void testUpdateConfig() throws ClientProtocolException, HttpException, ClientException, IOException {
        LockoutPolicyDTO lockout = new LockoutPolicyDTO.Builder()
            .withDescription("Updated description")
            .withAutoUnlockIntervalSec(30L)
            .withFailedAttemptIntervalSec(30L)
            .withMaxFailedAttempts(5)
            .build();

        TenantConfigurationDTO config = new TenantConfigurationDTO.Builder()
            .withLockoutPolicy(lockout)
            .build();

        TenantConfigurationDTO actual = testAdminClient.tenant().updateConfig(testTenant.getName(), config);

        assertEquals(lockout.getDescription(), actual.getLockoutPolicy().getDescription());
        assertEquals(lockout.getAutoUnlockIntervalSec(), actual.getLockoutPolicy().getAutoUnlockIntervalSec());
        assertEquals(lockout.getFailedAttemptIntervalSec(), actual.getLockoutPolicy().getFailedAttemptIntervalSec());
        assertEquals(lockout.getMaxFailedAttempts(), actual.getLockoutPolicy().getMaxFailedAttempts());
    }

}
