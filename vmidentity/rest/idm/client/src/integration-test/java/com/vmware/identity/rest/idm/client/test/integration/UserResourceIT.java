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
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.security.GeneralSecurityException;
import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.client.test.integration.util.UserGenerator;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.data.UserDetailsDTO;

public class UserResourceIT extends IntegrationTestBase {

    private static final String TEST_USER_NAME = "testUser";
    private static final String TEST_USER_DESCRIPTION = "This is a description";

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testCreateAndDelete() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO user = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        deleteUser(user);
    }

    @Test
    public void testGet() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        UserDTO user = testAdminClient.user().get(testTenant.getName(), testUser.getName(), testUser.getDomain());

        assertNotNull(user);
        assertUsersEqual(testUser, user);

        deleteUser(testUser);
    }

    @Test
    public void testUpdate() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        UserDetailsDTO updatedDetails = new UserDetailsDTO.Builder()
            .withDescription(testUser.getDetails().getDescription() + " that we've updated")
            .build();

        UserDTO updatedUser = new UserDTO.Builder()
            .withDetails(updatedDetails)
            .build();

        UserDTO up = testAdminClient.user().update(testTenant.getName(), testUser.getName(), testUser.getDomain(), updatedUser);

        assertEquals(updatedDetails.getDescription(), up.getDetails().getDescription());

        deleteUser(testUser);
    }

    @Test
    public void testUpdatePassword() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        testAdminClient.user().updatePassword(testTenant.getName(), testUser.getName(), testUser.getDomain(),
                testUser.getPasswordDetails().getPassword(), UserGenerator.PASSWORD + "a");

        deleteUser(testUser);
    }

    @Test
    public void testResetPassword() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        testAdminClient.user().resetPassword(testTenant.getName(), testUser.getName(), testUser.getDomain(), UserGenerator.PASSWORD + "a");

        deleteUser(testUser);
    }

    private static UserDTO createUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = TestGenerator.generateUser(name, testTenant.getName(), description);
        UserDTO user = testAdminClient.user().create(testTenant.getName(), testUser);

        assertUsersEqual(testUser, user);
        return user;
    }

    private static void deleteUser(UserDTO user) throws ClientProtocolException, ClientException, HttpException, IOException {
        testAdminClient.user().delete(testTenant.getName(), user.getName(), user.getDomain());

        try {
            testAdminClient.user().get(testTenant.getName(), user.getName(), user.getDomain());
            fail("Found user that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

}
