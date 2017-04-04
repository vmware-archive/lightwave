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

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertUsersEqual;
import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertVmdirUsersEqual;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;
import com.vmware.identity.rest.idm.client.test.integration.util.TestClientFactory;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.UserDTO;

public class UserResourceIT extends IntegrationTestBase {

    private static final String TEST_USER_NAME = "testUser";
    private static final String TEST_USER_DESCRIPTION = "This is a description";

    private static VmdirClient testAdminVmdirClient;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);

        testAdminVmdirClient = TestClientFactory.createVmdirClient(properties.getHost(),
                testTenant.getName(),
                testTenant.getUsername(),
                testTenant.getPassword());
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testGet() throws ClientProtocolException, ClientException, HttpException, IOException {
        com.vmware.directory.rest.common.data.UserDTO testUser = createVmdirUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        UserDTO user = testAdminClient.user().get(testTenant.getName(), testUser.getName(), testUser.getDomain());

        assertNotNull(user);
        assertUsersEqual(testUser, user);

        deleteVmdirUser(testUser);
    }

    private static com.vmware.directory.rest.common.data.UserDTO createVmdirUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException {
        com.vmware.directory.rest.common.data.UserDTO testUser = TestGenerator.generateVmdirUser(name, testTenant.getName(), description);
        com.vmware.directory.rest.common.data.UserDTO user = testAdminVmdirClient.user().create(testTenant.getName(), testUser);

        assertVmdirUsersEqual(testUser, user);
        return user;
    }

    private static void deleteVmdirUser(com.vmware.directory.rest.common.data.UserDTO user) throws ClientProtocolException, ClientException, HttpException, IOException {
        testAdminVmdirClient.user().delete(testTenant.getName(), user.getName(), user.getDomain());

        try {
            testAdminClient.user().get(testTenant.getName(), user.getName(), user.getDomain());
            fail("Found user that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

}
