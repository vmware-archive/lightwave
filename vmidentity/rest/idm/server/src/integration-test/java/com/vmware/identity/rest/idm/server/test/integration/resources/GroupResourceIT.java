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
package com.vmware.identity.rest.idm.server.test.integration.resources;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.idm.Group;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.server.mapper.GroupMapper;
import com.vmware.identity.rest.idm.server.resources.GroupResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.PrincipalAssertor;
import com.vmware.identity.rest.idm.server.test.integration.util.data.UserDataGenerator;

/**
 * Integration tests for Group Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class GroupResourceIT extends TestBase {

    private static final String GROUPNAME = "testGroup";
    private static final String GROUP_DESCRIPTION = "A group created for the purpose of integration testing";
    private static final String GROUP_UPN = GROUPNAME + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String GROUP_UPN_UNKNOWN_GROUPNAME = "unknownGroup" + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String GROUP_UPN_UNKNOWN_TENANT = GROUPNAME + "@" + "unknown.local";

    private GroupResource groupResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        groupResource = new GroupResource(DEFAULT_TENANT, request, null);
        groupResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetGroup() throws Exception {
        try {
            // Prepare test set up [Create group]
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);
            GroupDTO groupDTO = groupResource.get(GROUP_UPN);

            // Retrieve group
            Group expectedGroup = groupHelper.getGroup(DEFAULT_TENANT, GROUPNAME);
            PrincipalAssertor.assertGroup(GroupMapper.getGroupDTO(expectedGroup), groupDTO);
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetGroup_WithNonExistentGroup_ThrowsNotFoundEx() {
        groupResource.get(GROUP_UPN_UNKNOWN_GROUPNAME);
    }

    @Test(expected = NotFoundException.class)
    public void testGetGroup_WithNonExistentTenant_ThrowsNotFoundEx() {
        groupResource = new GroupResource("unknown.local", request, null);
        groupResource.setIDMClient(idmClient);
        groupResource.get(GROUP_UPN_UNKNOWN_TENANT);
    }

    @Test
    public void testGetMembersFromGroup() throws Exception {
        List<String> users = null;
        List<String> groups = null;
        try {
            // Test set up (2 steps)
            // Step 1 : Create a target group ("testGroup")
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);

            // Step 2 : Create users + groups in default tenant, And associate them to target group
            users = Arrays.asList("User1", "User2", "User3");
            groups = Arrays.asList("Group1", "Group2");
            for (String user : users) {
                userHelper.createUser(DEFAULT_TENANT, UserDataGenerator.generateTestUser(user, DEFAULT_TENANT, false, false));
                userHelper.addUserToGroup(DEFAULT_TENANT, user, GROUPNAME);
            }
            for (String group : groups) {
                groupHelper.createGroup(DEFAULT_TENANT, group);
                groupHelper.addGroupToGroup(DEFAULT_TENANT, group, GROUPNAME);
            }

            // Get person users of group only
            SearchResultDTO principalSearchResults = groupResource.getMembers(GROUP_UPN, MemberType.USER.name(), 200);
            assertEquals(3, principalSearchResults.getUsers().size());
            assertNull(principalSearchResults.getGroups());
            assertNull(principalSearchResults.getSolutionUsers());

            // Get groups of group only
            SearchResultDTO groupsOfGroup = groupResource.getMembers(GROUP_UPN, MemberType.GROUP.name(), 200);
            assertNull(groupsOfGroup.getUsers());
            assertEquals(2, groupsOfGroup.getGroups().size());
            assertNull(groupsOfGroup.getSolutionUsers());

            // Get all members of group
            SearchResultDTO membersOfGroup = groupResource.getMembers(GROUP_UPN, MemberType.ALL.name(), 200);
            assertEquals(3, membersOfGroup.getUsers().size());
            assertEquals(2, membersOfGroup.getGroups().size());
            assertEquals(0, membersOfGroup.getSolutionUsers().size());

        } finally {
            userHelper.deleteUsers(DEFAULT_TENANT, users);
            groupHelper.deleteGroups(DEFAULT_TENANT, groups);
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME); // Delete target group
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetMembers_WithNonExistentGroup() {
        groupResource.getMembers(GROUP_UPN_UNKNOWN_GROUPNAME, MemberType.USER.name(), 200);
    }

    @Test(expected = NotFoundException.class)
    public void testGetMembersFromGroup_WithNonExistentTenant_ThrowsNotFoundEx() {
        groupResource = new GroupResource("unknown.local", request, null);
        groupResource.setIDMClient(idmClient);
        groupResource.getMembers(GROUP_UPN_UNKNOWN_TENANT, MemberType.USER.name(), 200);
    }

    @Test
    public void testGetParentsOfGroup_NonNested() throws Exception {
        String parentGroup = "parentGroup";
        String childGroup = "childGroup";
        try {
            groupHelper.createGroup(DEFAULT_TENANT, parentGroup);
            groupHelper.createGroup(DEFAULT_TENANT, childGroup);
            groupHelper.addGroupToGroup(DEFAULT_TENANT, childGroup, parentGroup);

            Collection<GroupDTO> groups = groupResource.getParents(childGroup + "@" + DEFAULT_SYSTEM_DOMAIN, false);
            assertEquals(2, groups.size());
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, parentGroup);
            groupHelper.deleteGroup(DEFAULT_TENANT, childGroup);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetParents_WithNonExistentTenant_ThrowsNotFoundEx() {
        groupResource = new GroupResource("unknown.local", request, null);
        groupResource.setIDMClient(idmClient);
        groupResource.getParents(GROUP_UPN, false);
    }

    @Test(expected = NotFoundException.class)
    public void testGetParent_WithNonExistentGroup_ThrowsNotFoundEx() {
        groupResource.getParents(GROUP_UPN_UNKNOWN_GROUPNAME, false);
    }

}
