package com.vmware.directory.rest.server.test.integration.resources;

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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.ArrayList;
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

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.SearchResultDTO;
import com.vmware.directory.rest.server.annotation.IntegrationTest;
import com.vmware.directory.rest.server.mapper.GroupMapper;
import com.vmware.directory.rest.server.resources.GroupResource;
import com.vmware.directory.rest.server.test.integration.datagenerator.GroupDataGenerator;
import com.vmware.directory.rest.server.test.integration.datagenerator.UserDataGenerator;
import com.vmware.directory.rest.server.test.integration.util.PrincipalAssertor;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;

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
    public void testCreateGroup() throws Exception {
        try {
            GroupDTO groupToCreate = GroupDataGenerator.createGroupDTO(GROUPNAME, DEFAULT_SYSTEM_DOMAIN, GROUP_DESCRIPTION);
            GroupDTO createdGroup = groupResource.create(groupToCreate);
            PrincipalAssertor.assertGroup(groupToCreate, createdGroup);
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME);
        }
    }

    @Test(expected = BadRequestException.class)
    public void testCreateGroup_WithNonSystemTenant_ThrowsBadRequestEx() {
        GroupDTO groupToCreate = GroupDataGenerator.createGroupDTO(GROUPNAME, "unknown.local", GROUP_DESCRIPTION);
        groupResource.create(groupToCreate);
    }

    @Test(expected = InvalidPrincipalException.class)
    public void testDeleteGroup() throws Exception {

        // Prepare test set up [create group]
        groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);

        // Delete the group
        groupResource.delete(GROUP_UPN);

        groupHelper.getGroup(DEFAULT_TENANT, GROUPNAME);
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteGroup_WithNonSystemTenant_ThrowsNotFoundEx() {
        groupResource.delete(GROUPNAME + "@" + "nonsytem.local");
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteGroup_WithNonExistentGroup_ThrowsNotFoundEx() {
        groupResource.delete(GROUP_UPN_UNKNOWN_GROUPNAME);
    }

    @Test
    public void testUpdateGroup() throws Exception {
        String newDescription = "New description that is being updated as part of integration testing";
        try {
            // test set up [Create group with some description]
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);

            // update group with new description
            GroupDTO groupDTO = new GroupDTO(null, null, getTestGroupDetailsDTO(newDescription), null, null);
            groupResource.update(GROUP_UPN, groupDTO);

            // Assert if the description of group is been updated
            Group updatedGroup = groupHelper.getGroup(DEFAULT_TENANT, GROUPNAME);
            assertEquals(newDescription, updatedGroup.getDetail().getDescription());
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateGroup_WithNonExistentGroup_ThrowsNotFoundException() {
        String newDescription = "New description that is being updated as part of integration testing";
        // update unknown group with new description
        GroupDTO groupDTO = new GroupDTO(null, null, getTestGroupDetailsDTO(newDescription), null, null);
        groupResource.update(GROUP_UPN_UNKNOWN_GROUPNAME, groupDTO);
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateGroup_WithNonSystemTenant_ThrowsBadRequestException() {
        String newDescription = "New description that is being updated as part of integration testing";
        // update unknown group with new description
        GroupDTO groupDTO = new GroupDTO(null, null, getTestGroupDetailsDTO(newDescription), null, null);
        groupResource.update(GROUP_UPN_UNKNOWN_TENANT, groupDTO);
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
    public void testAddUsersToGroup() throws Exception {
        List<String> users = null;
        try {
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);
            users = Arrays.asList("User1@vsphere.local", "User2@vsphere.local", "User3@vsphere.local");
            for (String user : users) {
                userHelper.createUser(DEFAULT_TENANT, UserDataGenerator.generateTestUser(PrincipalUtil.fromName(user).getName(), DEFAULT_TENANT, false, false));
            }

            groupResource.addMembers(GROUP_UPN, users, MemberType.USER.name());

            int totalPersonUsers = groupHelper.findPersonUsersInGroup(DEFAULT_TENANT, GROUPNAME, "User").size();
            assertEquals(3, totalPersonUsers);

        } finally {
            userHelper.deleteUsers(DEFAULT_TENANT, users);
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME); // Delete target group
        }
    }

    @Test
    public void testAddGroupToGroup() throws Exception {
        List<String> groups = null;
        try {
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);
            groups = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local", "Group3@vsphere.local");
            for (String group : groups) {
                groupHelper.createGroup(DEFAULT_TENANT, PrincipalUtil.fromName(group).getName());
            }

            groupResource.addMembers(GROUP_UPN, groups, MemberType.GROUP.name());

            int totalGroups = groupHelper.findGroupsInGroup(DEFAULT_TENANT, GROUPNAME, "Group").size();
            assertEquals(3, totalGroups);

        } finally {
            groupHelper.deleteGroups(DEFAULT_TENANT, groups);
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME); // Delete target group
        }
    }

    @Test(expected = NotImplementedError.class)
    public void testAddSolutionUsersToGroup() throws Exception {
        List<String> solutionUsers = null;
        try {
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);
            solutionUsers = Arrays.asList("SolutionUser1@vsphere.local");
            // revisit after implementing support to add solution user to group
            /*
             * for (String solutionUser : solutionUsers) {
             * solutionUserHelper.createSolutionUser(DEFAULT_TENANT, solutionUser); }
             */
            groupResource.addMembers(GROUP_UPN, solutionUsers, MemberType.SOLUTIONUSER.name());
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME); // Delete target group
        }
    }

    @Test(expected = NotFoundException.class)
    public void testAddMember_WithNonExistentTenant_throwsNotFoundEx() {
        groupResource = new GroupResource("unknown.local", request, null);
        groupResource.setIDMClient(idmClient);
        List<String> groupsToAdd = new ArrayList<String>();
        groupResource.addMembers(GROUP_UPN, groupsToAdd, MemberType.GROUP.name());
    }

    @Test(expected = NotFoundException.class)
    public void testAddMember_WithNonExistentGroup_ThrowsNotFoundEx() {
        List<String> unknownGroups = Arrays.asList("unknownGroup1@vsphere.local", "unknownGroup2@vsphere.local");
        groupResource.addMembers(GROUP_UPN, unknownGroups, MemberType.GROUP.name());
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

    @Test
    public void testRemoveMembersFromGroup() throws Exception {
        List<String> groups = null;
        try {
            groupHelper.createGroup(DEFAULT_TENANT, GROUPNAME);
            groups = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
            for (String group : groups) {
                groupHelper.createGroup(DEFAULT_TENANT, PrincipalUtil.fromName(group).getName());
                groupHelper.addGroupToGroup(DEFAULT_TENANT, PrincipalUtil.fromName(group).getName(), GROUPNAME);
            }

            groupResource.removeMembers(GROUP_UPN, groups, MemberType.GROUP.name());
            assertEquals(0, groupResource.getMembers(GROUP_UPN, MemberType.GROUP.name(), 200).getGroups().size());
        } finally {
            groupHelper.deleteGroup(DEFAULT_TENANT, GROUPNAME);
            groupHelper.deleteGroups(DEFAULT_TENANT, groups);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testRemoveMembers_WithNonExistingUser_ThrowsNotFoundEx() {
        List<String> unknownUsers = Arrays.asList("unknownUser1@vsphere.local", "unknownUser2@vsphere.local");
        groupResource.removeMembers(GROUP_UPN, unknownUsers, MemberType.USER.name());
    }

    private GroupDetailsDTO getTestGroupDetailsDTO(String desc) {
        return GroupDetailsDTO.builder().withDescription(desc).build();
    }

}
