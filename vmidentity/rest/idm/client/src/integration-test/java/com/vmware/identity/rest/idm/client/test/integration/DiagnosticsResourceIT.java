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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.client.test.integration.util.TokenFactory;
import com.vmware.identity.rest.idm.data.EventLogDTO;
import com.vmware.identity.rest.idm.data.EventLogStatusDTO;

public class DiagnosticsResourceIT extends IntegrationTestBase {

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Before
    public void startLogging() throws GeneralSecurityException, IOException, HttpException, ClientException {
        testAdminClient.diagnostics().startEventLog(testTenant.getName());
    }

    @After
    public void clearLog() throws GeneralSecurityException, IOException, HttpException, ClientException {
        testAdminClient.diagnostics().clearEventLog(testTenant.getName());
    }

    @Test
    public void testGetEventLog() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, KeyManagementException, NoSuchAlgorithmException, KeyStoreException {
        getAccessToken();

        List<EventLogDTO> events = testAdminClient.diagnostics().getEventLog(testTenant.getName());
        assertNotNull(events);
        assertFalse(events.isEmpty());
    }

    @Test
    public void testGetEventLogStatus() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, KeyManagementException, NoSuchAlgorithmException, KeyStoreException {
        getAccessToken();

        EventLogStatusDTO status = testAdminClient.diagnostics().getEventLogStatus(testTenant.getName());
        assertNotNull(status);
        assertTrue(status.isEnabled());
        assertNotNull(status.getSize());
    }

    @Test
    public void testGetEventLogStatus_stopped() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, KeyManagementException, NoSuchAlgorithmException, KeyStoreException {
        testAdminClient.diagnostics().stopEventLog(testTenant.getName());
        getAccessToken();

        EventLogStatusDTO status = testAdminClient.diagnostics().getEventLogStatus(testTenant.getName());
        assertNotNull(status);
        assertFalse(status.isEnabled());
        assertNotNull(status.getSize());
    }

    @Test
    public void testGetEventLog_sized() throws GeneralSecurityException, IOException, HttpException, ClientException {
        int size = 5;
        testAdminClient.diagnostics().startEventLog(testTenant.getName(), size);

        for (int i = 0; i < size + 1; i++) {
            getAccessToken();
        }

        List<EventLogDTO> events = testAdminClient.diagnostics().getEventLog(testTenant.getName());
        assertNotNull(events);
        assertEquals(events.size(), size);
    }

    @Test
    public void testGetEventLog_stopped() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, KeyManagementException, NoSuchAlgorithmException, KeyStoreException {
        testAdminClient.diagnostics().stopEventLog(testTenant.getName());
        getAccessToken();

        List<EventLogDTO> events = testAdminClient.diagnostics().getEventLog(testTenant.getName());
        assertNotNull(events);
        assertTrue(events.isEmpty());
    }

    @Test
    public void testGetEventLog_cleared() throws ClientProtocolException, WebApplicationException, ClientException, HttpException, IOException, KeyManagementException, NoSuchAlgorithmException, KeyStoreException {
        getAccessToken();
        testAdminClient.diagnostics().clearEventLog(testTenant.getName());

        List<EventLogDTO> events = testAdminClient.diagnostics().getEventLog(testTenant.getName());
        assertNotNull(events);
        assertTrue(events.isEmpty());
    }

    private static void getAccessToken() throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        TokenFactory.getAccessToken(properties.getHost(),
                testTenant.getName(),
                testTenant.getUsername(),
                testTenant.getPassword());
    }

}
