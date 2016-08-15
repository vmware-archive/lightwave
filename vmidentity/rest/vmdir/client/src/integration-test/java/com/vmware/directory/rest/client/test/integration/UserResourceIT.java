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

import com.vmware.directory.rest.client.test.integration.util.TestGenerator;
import com.vmware.directory.rest.client.test.integration.util.UserGenerator;
import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;

public class UserResourceIT extends IntegrationTestBase {

    private static final String TEST_USER_NAME = "testUser";
    private static final String TEST_USER_DESCRIPTION = "This is a description";

    private static final String TEST_GROUP_NAME = "test.group";
    private static final String TEST_GROUP_DESCRIPTION = "This is a test group";

    private static final String TENANT_NAME = "vsphere.local";

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

        UserDTO user = testAdminClient.user().get(TENANT_NAME, testUser.getName(), testUser.getDomain());

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

        UserDTO up = testAdminClient.user().update(TENANT_NAME, testUser.getName(), testUser.getDomain(), updatedUser);

        assertEquals(updatedDetails.getDescription(), up.getDetails().getDescription());

        deleteUser(testUser);
    }

    @Test
    public void testUpdatePassword() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        testAdminClient.user().updatePassword(TENANT_NAME, testUser.getName(), testUser.getDomain(),
                testUser.getPasswordDetails().getPassword(), UserGenerator.PASSWORD + "a");

        deleteUser(testUser);
    }

    @Test
    public void testResetPassword() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);

        testAdminClient.user().resetPassword(TENANT_NAME, testUser.getName(), testUser.getDomain(), UserGenerator.PASSWORD + "a");

        deleteUser(testUser);
    }

    @Test
    public void testGetGroups() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        String testUserUPN = UPNUtil.buildUPN(testUser.getName(), testUser.getDomain());

        GroupDTO testGroup = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);

        testAdminClient.group().addMembers(TENANT_NAME, testGroup.getName(), testGroup.getDomain(),
                Arrays.asList(testUserUPN), MemberType.USER);

        List<GroupDTO> groups = testAdminClient.user().getGroups(TENANT_NAME, testUser.getName(), testUser.getDomain(), false);

        assertContainsGroup(testGroup, groups);

        deleteGroup(testGroup);
        deleteUser(testUser);
    }

    private static UserDTO createUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = TestGenerator.generateUser(name, TENANT_NAME, description);
        UserDTO user = testAdminClient.user().create(TENANT_NAME, testUser);

        assertUsersEqual(testUser, user);
        return user;
    }

    private static void deleteUser(UserDTO user) throws ClientProtocolException, ClientException, HttpException, IOException {
        testAdminClient.user().delete(TENANT_NAME, user.getName(), user.getDomain());

        try {
            testAdminClient.user().get(TENANT_NAME, user.getName(), user.getDomain());
            fail("Found user that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

    private static GroupDTO createGroup(String name, String description) throws ClientProtocolException, HttpException, ClientException, IOException {
        GroupDTO testGroup = TestGenerator.generateGroup(name, TENANT_NAME, description);
        GroupDTO group = testAdminClient.group().create(TENANT_NAME, testGroup);

        assertGroupsEqual(testGroup, group);
        return group;
    }

    private static void deleteGroup(GroupDTO group) throws ClientProtocolException, HttpException, ClientException, IOException {
        testAdminClient.group().delete(TENANT_NAME, group.getName(), group.getDomain());

        try {
            testAdminClient.group().get(TENANT_NAME, group.getName(), group.getDomain());
            fail("Found group that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

}