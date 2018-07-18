package com.vmware.directory.rest.client.test.integration;

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

import static com.vmware.directory.rest.client.test.integration.util.Assert.assertContainsGroup;
import static com.vmware.directory.rest.client.test.integration.util.Assert.assertContainsUser;
import static com.vmware.directory.rest.client.test.integration.util.Assert.assertGroupsEqual;
import static com.vmware.directory.rest.client.test.integration.util.Assert.assertUsersEqual;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.directory.rest.client.test.integration.util.TestGenerator;
import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.SearchResultDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.identity.rest.core.client.UPNUtil;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.BadRequestException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;

public class GroupResourceIT extends IntegrationTestBase {

    private static final String TEST_GROUP_NAME = "test.group";
    private static final String TEST_GROUP_DESCRIPTION = "This is a test group";

    private static final String TEST_USER_NAME = "testUser";
    private static final String TEST_USER_DESCRIPTION = "This is a description";

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
    public void testCreateAndDelete() throws ClientProtocolException, HttpException, ClientException, IOException {
        GroupDTO group = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);
        deleteGroup(group);
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        GroupDTO testGroup = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);

        GroupDTO group = systemAdminClient.group().get(systemTenant, testGroup.getName(), testGroup.getDomain());

        assertNotNull(group);
        assertGroupsEqual(testGroup, group);

        deleteGroup(testGroup);
    }

    @Test
    public void testUpdate() throws ClientProtocolException, HttpException, ClientException, IOException {
        GroupDTO testGroup = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);

        GroupDetailsDTO updatedDetails = new GroupDetailsDTO.Builder()
            .withDescription(testGroup.getDetails().getDescription() + " that we've updated")
            .build();

        GroupDTO updatedGroup = new GroupDTO.Builder()
            .withDetails(updatedDetails)
            .build();

        GroupDTO up = systemAdminClient.group().update(systemTenant, testGroup.getName(), testGroup.getDomain(), updatedGroup);

        assertEquals(updatedDetails.getDescription(), up.getDetails().getDescription());

        deleteGroup(testGroup);
    }

    @Test(expected = BadRequestException.class)
    public void testGetMembersWithInvalidGroupName() throws Exception {
        systemAdminClient.group().getMembers(systemTenant, "", systemTenant, MemberType.USER, 200);
    }

    @Test
    public void testMembership() throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = createUser(TEST_USER_NAME, TEST_USER_DESCRIPTION);
        String testUserUPN = UPNUtil.buildUPN(testUser.getName(), testUser.getDomain());

        GroupDTO testGroup = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);

        systemAdminClient.group().addMembers(systemTenant, testGroup.getName(), testGroup.getDomain(),
                Arrays.asList(testUserUPN), MemberType.USER);

        SearchResultDTO result = systemAdminClient.group().getMembers(systemTenant, testGroup.getName(), testGroup.getDomain(),
                MemberType.USER, 200);

        assertContainsUser(testUser, new ArrayList<UserDTO>(result.getUsers()));

        systemAdminClient.group().removeMembers(systemTenant, testGroup.getName(), testGroup.getDomain(),
                Arrays.asList(testUserUPN), MemberType.USER);

        result = systemAdminClient.group().getMembers(systemTenant, testGroup.getName(), testGroup.getDomain(),
                MemberType.USER, 200);

        assertNull(result.getUsers());

        deleteUser(testUser);
        deleteGroup(testGroup);
    }

    @Test
    public void testGetParents() throws ClientProtocolException, HttpException, ClientException, IOException {
        GroupDTO testGroup = createGroup(TEST_GROUP_NAME, TEST_GROUP_DESCRIPTION);
        String testGroupUPN = UPNUtil.buildUPN(testGroup.getName(), testGroup.getDomain());

        GroupDTO parentGroup = createGroup(TEST_GROUP_NAME + "parent", TEST_GROUP_DESCRIPTION);

        systemAdminClient.group().addMembers(systemTenant, parentGroup.getName(), parentGroup.getDomain(),
                Arrays.asList(testGroupUPN), MemberType.GROUP);

        List<GroupDTO> parents = systemAdminClient.group().getParents(systemTenant, testGroup.getName(), testGroup.getDomain());

        assertContainsGroup(parentGroup, parents);

        deleteGroup(parentGroup);
        deleteGroup(testGroup);
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

    private static UserDTO createUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException {
        UserDTO testUser = TestGenerator.generateUser(name, systemTenant, description);
        UserDTO user = systemAdminClient.user().create(systemTenant, testUser);

        assertUsersEqual(testUser, user);
        return user;
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

}
