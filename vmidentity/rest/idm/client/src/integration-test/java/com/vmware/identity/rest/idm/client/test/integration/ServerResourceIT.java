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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;
import com.vmware.identity.rest.idm.data.attributes.ComputerType;

public class ServerResourceIT extends IntegrationTestBase {

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(false);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(false);
    }

    @Test
    public void testGetComputers_All() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<ServerDetailsDTO> computers = systemAdminClient.server().getComputers(ComputerType.ALL);
        assertFalse(computers.isEmpty());
    }

    @Test
    public void testGetComputers_DCs() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<ServerDetailsDTO> computers = systemAdminClient.server().getComputers(ComputerType.DC);

        for (ServerDetailsDTO details : computers) {
            assertTrue(details.isDomainController());
        }
    }

    @Test
    public void testGetComputers_NotDCs() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<ServerDetailsDTO> computers = systemAdminClient.server().getComputers(ComputerType.COMPUTER);

        for (ServerDetailsDTO details : computers) {
            assertFalse(details.isDomainController());
        }
    }

}
