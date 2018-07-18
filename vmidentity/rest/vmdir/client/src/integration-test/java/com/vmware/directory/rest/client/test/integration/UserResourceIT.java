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
package com.vmware.directory.rest.client.test.integration;

import static com.vmware.directory.rest.client.test.integration.util.Assert.assertContainsGroup;
import static com.vmware.directory.rest.client.test.integration.util.Assert.assertGroupsEqual;
import static com.vmware.directory.rest.client.test.integration.util.Assert.assertUsersEqual;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.Arrays;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.directory.rest.client.test.integration.util.TestClientFactory;
import com.vmware.directory.rest.client.test.integration.util.TestGenerator;
import com.vmware.directory.rest.client.test.integration.util.UserGenerator;
import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.BadRequestException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;

public class UserResourceIT extends IntegrationTestBase {

    private static final String TEST_USER_NAME = "testUser";
    private static final String TEST_USER_DESCRIPTION = "This is a description";

    private static final String TEST_GROUP_NAME = "test.group";
    private static final String TEST_GROUP_DESCRIPTION = "This is a test group";

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init();
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        UserDTO user = null;
        GroupDTO group = null;
        // clean up user if it exists
        try {
            user = systemAdminClient.user().get(systemTenant, TEST_USER_NAME, systemTenant);
        }  catch (NotFoundException e) {
            // Ignore it
        } finally {
            if (user != null) {
                deleteUser(user);
            }
        }
        // clean up group if it exists
        try {
            group = systemAdminClient.group().get(systemTenant, TEST_GROUP_NAME, systemTenant);
        }  catch (NotFoundException e) {
            // Ignore it
        } finally {
            if (group != null) {
                deleteGroup(group);
            }
        }
    }

    @Test
    public void testCreateAndDelete() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO user = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        deleteUser(user);
    }

    @Test
    public void testGet() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        UserDTO user = systemAdminClient.user().get(systemTenant, testUser.getName(), testUser.getDomain());

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

        UserDTO up = systemAdminClient.user().update(systemTenant, testUser.getName(), testUser.getDomain(), updatedUser);

        assertEquals(updatedDetails.getDescription(), up.getDetails().getDescription());

        deleteUser(testUser);
    }

    @Test
    public void testUpdatePassword() throws Exception {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        VmdirClient testClient = TestClientFactory.createClient(hostname, systemTenant, testUser.getName(),
                testUser.getDomain(), testUser.getPasswordDetails().getPassword());

        testClient.user().updatePassword(systemTenant, testUser.getName(), testUser.getDomain(),
                testUser.getPasswordDetails().getPassword(), UserGenerator.PASSWORD + "a");

        deleteUser(testUser);
    }

    @Test(expected = BadRequestException.class)
    public void testUpdatePasswordWithInvalidCurrentPassword() throws Exception {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        try {
            VmdirClient testClient = TestClientFactory.createClient(hostname, systemTenant, testUser.getName(),
                    testUser.getDomain(), testUser.getPasswordDetails().getPassword());

            testClient.user().updatePassword(systemTenant, testUser.getName(), testUser.getDomain(),
                    testUser.getPasswordDetails().getPassword() + "a", UserGenerator.PASSWORD + "a");
        } finally {
            deleteUser(testUser);
        }
    }

    @Test
    public void testResetPassword() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        systemAdminClient.user().resetPassword(systemTenant, testUser.getName(), testUser.getDomain(), UserGenerator.PASSWORD + "a");

        deleteUser(testUser);
    }

    @Test
    public void testGetGroups() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        String testUserUPN = UPNUtil.buildUPN(testUser.getName(), testUser.getDomain());

        GroupDTO testGroup = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);

        systemAdminClient.group().addMembers(systemTenant, testGroup.getName(), testGroup.getDomain(),
                Arrays.asList(testUserUPN), MemberType.USER);

        List<GroupDTO> groups = systemAdminClient.user().getGroups(systemTenant, testUser.getName(), testUser.getDomain(), false);

        assertContainsGroup(testGroup, groups);

        deleteGroup(testGroup);
        deleteUser(testUser);
    }

    private static UserDTO createUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = TestGenerator.generateUser(name, systemTenant, description);
        UserDTO user = systemAdminClient.user().create(systemTenant, testUser);

        assertUsersEqual(testUser, user);
        return testUser;
    }

    private static void deleteUser(UserDTO user) throws ClientProtocolException, ClientException, HttpException, IOException {
        systemAdminClient.user().delete(systemTenant, user.getName(), user.getDomain());

        try {
            systemAdminClient.user().get(systemTenant, user.getName(), user.getDomain());
            fail("Found user that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

    private static GroupDTO createGroup(String name, String description) throws ClientProtocolException, HttpException, ClientException, IOException {
        GroupDTO testGroup = TestGenerator.generateGroup(name, systemTenant, description);
        GroupDTO group = systemAdminClient.group().create(systemTenant, testGroup);

        assertGroupsEqual(testGroup, group);
        return group;
    }

    private static void deleteGroup(GroupDTO group) throws ClientProtocolException, HttpException, ClientException, IOException {
        systemAdminClient.group().delete(systemTenant, group.getName(), group.getDomain());

        try {
            systemAdminClient.group().get(systemTenant, group.getName(), group.getDomain());
            fail("Found group that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

}