package com.vmware.directory.rest.server.resources;

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

import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.Arrays;
import java.util.Collection;
import java.util.EnumSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.PrincipalDTO;
import com.vmware.directory.rest.common.data.SearchResultDTO;
import com.vmware.directory.rest.server.util.TestDataGenerator;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.MemberAlreadyExistException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;

/**
 * Test cases for "GroupResource" web-resource
 */
public class GroupResourceTest {

    private static final String TEST_TENANT = "test.local";
    private static final String TEST_SYSTEM_DOMAIN = "test.local";
    private static final String TEST_GROUP_NAME = "testGroup";
    private static final String TEST_GROUP_UPN = TEST_GROUP_NAME + "@" + TEST_SYSTEM_DOMAIN;
    private static final String TEST_GROUP_ALIAS_NAME = "testAliasGroup";
    private static final String TEST_GROUP_ALIAS_DOMAIN = "testALias.local";
    private static final String TEST_GROUP_DESC = "Group created for unit testing purpose";

    private GroupResource groupResource;
    private IMocksControl mControl;
    private CasIdmClient mockCasIDMClient;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        mockCasIDMClient = mControl.createMock(CasIdmClient.class);
        groupResource = new GroupResource(TEST_TENANT, request, null);
        groupResource.setIDMClient(mockCasIDMClient);
    }

    @Test
    public void testGetUsersInGroup() throws Exception {
        Set<PersonUser> idmPersonUsers = TestDataGenerator.getIdmPersonUsers(1);
        EasyMock.expect(mockCasIDMClient.findPersonUsersInGroup(EasyMock.eq(TEST_TENANT), EasyMock.isA(PrincipalId.class),  EasyMock.eq(""), EasyMock.eq(10)))
                .andReturn(idmPersonUsers);
        mControl.replay();

        SearchResultDTO principalSearchResults = groupResource.getMembers(TEST_GROUP_UPN, MemberType.USER.toString(), 10);
        assertEquals(1, principalSearchResults.getUsers().size());
        assertNull(principalSearchResults.getGroups());
        assertNull(principalSearchResults.getSolutionUsers());

        mControl.verify();
    }

    @Test(expected=BadRequestException.class)
    public void testGetUnknownPrincipalTypeInGroup() throws Exception {
        Set<PersonUser> idmPersonUsers = TestDataGenerator.getIdmPersonUsers(1);
        EasyMock.expect(mockCasIDMClient.findPersonUsersInGroup(EasyMock.eq(TEST_TENANT), EasyMock.isA(PrincipalId.class),  EasyMock.eq(""), EasyMock.eq(10)))
                .andReturn(idmPersonUsers);
        mControl.replay();

        SearchResultDTO principalSearchResults = groupResource.getMembers(TEST_GROUP_UPN, "UNKNOWN_PRINCIPAL_TYPE", 10);
        assertEquals(1, principalSearchResults.getUsers().size());
        assertNull(principalSearchResults.getGroups());
        assertNull(principalSearchResults.getSolutionUsers());

        mControl.verify();
    }

    @Test
    public void testGetGroupsInGroup() throws Exception {
        Set<Group> idmGroups = TestDataGenerator.getIdmGroups(1);
        expect(mockCasIDMClient.findGroupsInGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(""), eq(10)))
                .andReturn(idmGroups);
        mControl.replay();

        SearchResultDTO principalSearchResults = groupResource.getMembers(TEST_GROUP_UPN, MemberType.GROUP.toString(), 10);
        assertEquals(1, principalSearchResults.getGroups().size());
        assertNull(principalSearchResults.getUsers());
        assertNull(principalSearchResults.getSolutionUsers());

        mControl.verify();
    }

    @Test
    public void testGetSolutionUsersInGroup() throws Exception {
        Set<SolutionUser> idmSolutionUsers = TestDataGenerator.getIdmSolutionUsers(1);
        expect(mockCasIDMClient.findSolutionUsersInGroup(eq(TEST_TENANT), eq(TEST_GROUP_NAME), eq(""), eq(10))).andReturn(idmSolutionUsers);
        mControl.replay();

        SearchResultDTO principalSearchResults = groupResource.getMembers(TEST_GROUP_UPN, MemberType.SOLUTIONUSER.toString(), 10);
        assertEquals(1, principalSearchResults.getSolutionUsers().size());
        assertNull(principalSearchResults.getGroups());
        assertNull(principalSearchResults.getUsers());

        mControl.verify();
    }

    @Test
    public void testCreateGroup() throws Exception{
        PrincipalId group = new PrincipalId(TEST_GROUP_NAME, TEST_TENANT);

        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroup(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andReturn(group);
        mControl.replay();

        GroupDTO actualGroup = groupResource.create(getGroupDTO());
        assertEquals(TEST_GROUP_NAME, actualGroup.getName());
        assertEquals(TEST_TENANT, actualGroup.getDomain());

        mControl.verify();
    }

    @Test(expected=NotImplementedError.class)
    public void testCreateGroup_withAlias() throws Exception {
        PrincipalDTO alias = new PrincipalDTO(TEST_GROUP_ALIAS_NAME, TEST_GROUP_ALIAS_DOMAIN);
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mControl.replay();
        GroupDTO input = GroupDTO.builder()
                        .withName(TEST_GROUP_UPN)
                        .withDomain(TEST_SYSTEM_DOMAIN)
                        .withDetails(new GroupDetailsDTO(TEST_GROUP_DESC))
                        .withAlias(alias)
                        .build();
        groupResource.create(input);
    }

    @Test(expected=NotFoundException.class)
    public void testCreateOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroup(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andThrow(new NoSuchTenantException("No such tenant"));
        mControl.replay();
        groupResource.create(getGroupDTO());
        mControl.verify();
    }

    @Test(expected=BadRequestException.class)
    public void testCreateOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroup(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andThrow(new InvalidPrincipalException("Invalid principal"));
        mControl.replay();
        groupResource.create(getGroupDTO());
        mControl.verify();
    }

    @Test(expected=InternalServerErrorException.class)
    public void testCreateOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroup(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.create(getGroupDTO());
        mControl.verify();
    }

    @Test
    public void testUpdateGroup() throws Exception {

        String newGroupDescription = "This is new description being update for a group from unit test";
        GroupDTO replacementGroup = GroupDTO.builder().withDetails(getGroupDetailsDTO()).build();

        PrincipalId updatedGroup = new PrincipalId(TEST_GROUP_NAME, TEST_SYSTEM_DOMAIN);
        GroupDetail getGroupDetail = new GroupDetail(newGroupDescription);
        Group getGroup = new Group(updatedGroup, getGroupDetail);

        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.updateGroupDetail(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andReturn(updatedGroup);
        expect(mockCasIDMClient.findGroup(eq(TEST_TENANT), isA(PrincipalId.class))).andReturn(getGroup);
        mControl.replay();

        GroupDTO actualGroup = groupResource.update(TEST_GROUP_UPN, replacementGroup);
        assertNotNull(actualGroup);
        assertEquals(TEST_GROUP_NAME, actualGroup.getName());
        assertEquals(TEST_SYSTEM_DOMAIN, actualGroup.getDomain());
        assertNull(actualGroup.getAlias());
        assertEquals(newGroupDescription, actualGroup.getDetails().getDescription());

        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        GroupDTO groupToUpdateWith = GroupDTO.builder().withDetails(getGroupDetailsDTO()).build();
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.updateGroupDetail(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        groupResource.update(TEST_GROUP_UPN, groupToUpdateWith);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        GroupDTO groupToUpdateWith = GroupDTO.builder().withDetails(getGroupDetailsDTO()).build();
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.updateGroupDetail(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andThrow(new InvalidArgumentException("Invalid argument"));
        mControl.replay();
        groupResource.update(TEST_GROUP_UPN, groupToUpdateWith);
        mControl.verify();

    }

    @Test(expected = InternalServerErrorException.class)
    public void testUpdateOnIDMError_ThrowsInternalServerError() throws Exception {
        GroupDTO groupToUpdateWith = GroupDTO.builder().withDetails(getGroupDetailsDTO()).build();
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.updateGroupDetail(eq(TEST_TENANT), eq(TEST_GROUP_NAME), isA(GroupDetail.class))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.update(TEST_GROUP_UPN, groupToUpdateWith);
        mControl.verify();
    }

    @Test(expected=NotImplementedError.class)
    public void testUpdateGroupWithGroupNameAsNull() throws Exception{
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        GroupDTO groupToUpdate = GroupDTO.builder()
                        .withName(TEST_GROUP_NAME)
                        .build();
        mControl.replay();
        groupResource.update(TEST_GROUP_UPN, groupToUpdate);
    }

    @Test(expected=NotImplementedError.class)
    public void testUpdateGroupWithDomainAsNull() throws Exception{
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        GroupDTO groupToUpdate = GroupDTO.builder()
                        .withDomain(TEST_SYSTEM_DOMAIN)
                        .build();
        mControl.replay();
        groupResource.update(TEST_GROUP_UPN, groupToUpdate);
    }

    @Test(expected=NotImplementedError.class)
    public void testUpdateGroupWithAliasAsNull() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        GroupDTO groupToUpdate = GroupDTO.builder().withAlias(new PrincipalDTO(TEST_GROUP_NAME, TEST_SYSTEM_DOMAIN)).build();
        mControl.replay();
        groupResource.update(TEST_GROUP_UPN, groupToUpdate);
    }

    @Test(expected=NotImplementedError.class)
    public void testGetParentsWithNestedThrowsNotImplementedEx() {
        mControl.replay();
        groupResource.getParents(TEST_GROUP_UPN, true);
    }

    @Test
    public void testDeleteGroup() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mockCasIDMClient.deletePrincipal(TEST_TENANT,TEST_GROUP_NAME);
        mControl.replay();
        groupResource.delete(TEST_GROUP_UPN);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mockCasIDMClient.deletePrincipal(TEST_TENANT,TEST_GROUP_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        groupResource.delete(TEST_GROUP_UPN);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mockCasIDMClient.deletePrincipal(TEST_TENANT,TEST_GROUP_NAME);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        groupResource.delete(TEST_GROUP_UPN);
        mControl.verify();

    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mockCasIDMClient.deletePrincipal(TEST_TENANT,TEST_GROUP_NAME);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.delete(TEST_GROUP_UPN);
        mControl.verify();
    }

    @Test
    public void testAddGroupsToGroup() throws Exception {

        List<String> groupsToAdd = Arrays.asList("Group1@vsphere.local","Group2@vsphere.local");
        expect(mockCasIDMClient.addGroupToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expectLastCall().once();
        expect(mockCasIDMClient.addGroupToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expectLastCall().once();
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mControl.replay();

        groupResource.addMembers(TEST_GROUP_UPN, groupsToAdd, MemberType.GROUP.toString());

        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testAddMembersOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        List<String> groupsToAdd = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroupToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME)));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        groupResource.addMembers(TEST_GROUP_UPN, groupsToAdd, MemberType.GROUP.toString());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testAddMembersOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        List<String> groupsToAdd = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroupToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME)));
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        groupResource.addMembers(TEST_GROUP_UPN, groupsToAdd, MemberType.GROUP.toString());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testAddMembersOnIDMError_ThrowsInternalServerError() throws Exception {
        List<String> groupsToAdd = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroupToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.addMembers(TEST_GROUP_UPN, groupsToAdd, MemberType.GROUP.toString());
        mControl.verify();
    }

    @Test(expected=BadRequestException.class)
    public void testAddMembersOnDuplicate_Throws_BadRequest() throws Exception{
        List<String> groupsToAdd = Arrays.asList("Group1@vsphere.local");
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        expect(mockCasIDMClient.addGroupToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andThrow(new MemberAlreadyExistException("Group1 already exists"));
        mControl.replay();
        groupResource.addMembers(TEST_GROUP_UPN, groupsToAdd, MemberType.GROUP.toString());
    }

    @Test
    public void testGetParents() throws Exception {
        Set<Group> groups = TestDataGenerator.getIdmGroups(1);
        expect(mockCasIDMClient.findDirectParentGroups(eq(TEST_TENANT), isA(PrincipalId.class))).andReturn(groups);
        mControl.replay();
        Collection<GroupDTO> actualGroups = groupResource.getParents(TEST_GROUP_UPN, false);
        assertEquals(1, actualGroups.size());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetParentsOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.findDirectParentGroups(eq(TEST_TENANT), isA(PrincipalId.class)));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        groupResource.getParents(TEST_GROUP_UPN, false);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetParentsOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.findDirectParentGroups(eq(TEST_TENANT), isA(PrincipalId.class)));
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        groupResource.getParents(TEST_GROUP_UPN, false);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetParentsOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.findDirectParentGroups(eq(TEST_TENANT), isA(PrincipalId.class)));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.getParents(TEST_GROUP_UPN, false);
        mControl.verify();
    }

    @Test
    public void testRemoveGroupsFromGroup() throws Exception {
        List<String> groupsToRemove = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expectLastCall().once();
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expectLastCall().once();
        mControl.replay();

        groupResource.removeMembers(TEST_GROUP_UPN, groupsToRemove, MemberType.GROUP.toString());

        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testRemoveMembersOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andThrow(new NoSuchTenantException("no such tenant"));
        List<String> groupsToRemove = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        mControl.replay();
        groupResource.removeMembers(TEST_GROUP_UPN, groupsToRemove, MemberType.GROUP.toString());
        mControl.verify();

    }

    @Test(expected = BadRequestException.class)
    public void testRemoveMembersOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        List<String> groupsToRemove = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        groupResource.removeMembers(TEST_GROUP_UPN, groupsToRemove, MemberType.GROUP.toString());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testRemoveMembersOnIDMError_ThrowsInternalServerError() throws Exception {
        List<String> groupsToRemove = Arrays.asList("Group1@vsphere.local", "Group2@vsphere.local");
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andThrow(new IDMException("idm error"));
        mControl.replay();
        groupResource.removeMembers(TEST_GROUP_UPN, groupsToRemove, MemberType.GROUP.toString());
        mControl.verify();
    }


    @Test
    public void testGetGroup() throws Exception {
        PrincipalId principal = new PrincipalId(TEST_GROUP_NAME, TEST_SYSTEM_DOMAIN);
        GroupDetail groupDetail = new GroupDetail(TEST_GROUP_DESC);
        expect(mockCasIDMClient.findGroup(eq(TEST_TENANT), isA(PrincipalId.class))).andReturn(new Group(principal, groupDetail));

        mControl.replay();
        GroupDTO group = groupResource.get(TEST_GROUP_UPN);
        assertEquals(TEST_GROUP_NAME, group.getName());
        assertEquals(TEST_SYSTEM_DOMAIN, group.getDomain());
        assertEquals(TEST_GROUP_DESC, group.getDetails().getDescription());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.findGroup(eq(TEST_TENANT), isA(PrincipalId.class))).andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        groupResource.get(TEST_GROUP_UPN);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.findGroup(eq(TEST_TENANT), isA(PrincipalId.class))).andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        groupResource.get(TEST_GROUP_UPN);
        mControl.verify();

    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.findGroup(eq(TEST_TENANT), isA(PrincipalId.class))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.get(TEST_GROUP_UPN);
        mControl.verify();

    }

    @Test
    public void testAddUsersToGroup() throws Exception {

        List<String> usersToAdd = Arrays.asList("User1@vsphere.local", "User2@vsphere.local");
        expect(mockCasIDMClient.addUserToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expect(mockCasIDMClient.addUserToGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        mControl.replay();
        groupResource.addMembers(TEST_GROUP_UPN, usersToAdd, MemberType.USER.toString());
        mControl.verify();
    }

    @Test(expected = NotImplementedError.class)
    public void testAddSolutionUsersToGroupThrowsNotImplementedError() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TEST_TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_SYSTEM_DOMAIN));
        List<String> solutionUsersToAdd = Arrays.asList("solnUser1@test.local", "solnUser2@test.local");
        mControl.replay();
        groupResource.addMembers(TEST_GROUP_UPN, solutionUsersToAdd, MemberType.SOLUTIONUSER.name());
        mControl.verify();
    }

    @Test
    public void testRemoveUsersToGroup() throws Exception {

        List<String> usersToRemove = Arrays.asList("User1@vsphere.local", "User2@vsphere.local");
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        expect(mockCasIDMClient.removeFromLocalGroup(eq(TEST_TENANT), isA(PrincipalId.class), eq(TEST_GROUP_NAME))).andReturn(true);
        mControl.replay();
        groupResource.removeMembers(TEST_GROUP_UPN, usersToRemove, MemberType.USER.toString());
        mControl.verify();
    }

    @Test
    public void testGetMembers() throws Exception {
        expect(mockCasIDMClient.findPersonUsersInGroup(eq(TEST_TENANT), isA(PrincipalId.class), isA(String.class), eq(33))).andReturn(TestDataGenerator.getIdmPersonUsers(2));
        expect(mockCasIDMClient.findGroupsInGroup(eq(TEST_TENANT), isA(PrincipalId.class), isA(String.class), eq(33))).andReturn(TestDataGenerator.getIdmGroups(2));
        expect(mockCasIDMClient.findSolutionUsersInGroup(eq(TEST_TENANT), isA(String.class), isA(String.class), eq(34))).andReturn(TestDataGenerator.getIdmSolutionUsers(2));
        mControl.replay();
        SearchResultDTO principals = groupResource.getMembers(TEST_GROUP_UPN, MemberType.ALL.name(), 100);
        assertEquals(2, principals.getUsers().size());
        assertEquals(2, principals.getGroups().size());
        assertEquals(2, principals.getSolutionUsers().size());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetMembersOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.findPersonUsersInGroup(eq(TEST_TENANT), isA(PrincipalId.class), isA(String.class), eq(66))).andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        groupResource.getMembers(TEST_GROUP_UPN, MemberType.ALL.name(), 200);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetMembersOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.findPersonUsersInGroup(eq(TEST_TENANT), isA(PrincipalId.class), isA(String.class), eq(66))).andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        groupResource.getMembers(TEST_GROUP_UPN, MemberType.ALL.name(), 200);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetMembersOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.findPersonUsersInGroup(eq(TEST_TENANT), isA(PrincipalId.class), isA(String.class), eq(66))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        groupResource.getMembers(TEST_GROUP_UPN, MemberType.ALL.name(), 200);
        mControl.verify();
    }

    @Test(expected = NotImplementedError.class)
    public void testRemoveSolutionUsersFromGroup() throws Exception {
        List<String> usersToRemove = Arrays.asList("solnUser1@vsphere.local", "solnUser2@vsphere.local");
        mControl.replay();
        groupResource.removeMembers(TEST_GROUP_UPN, usersToRemove, MemberType.SOLUTIONUSER.name());
    }

    private Group getGroup() {
        PrincipalId updatedGroup = new PrincipalId(TEST_GROUP_NAME, TEST_SYSTEM_DOMAIN);
        GroupDetail getGroupDetail = new GroupDetail(TEST_GROUP_DESC);
        return new Group(updatedGroup, getGroupDetail);
    }

    private GroupDTO getGroupDTO() {
        return  GroupDTO.builder()
                        .withName(TEST_GROUP_NAME)
                        .withDomain(TEST_SYSTEM_DOMAIN)
                        .withDetails(getGroupDetailsDTO())
                        .build();
    }

    private GroupDetailsDTO getGroupDetailsDTO(){
        return GroupDetailsDTO.builder().withDescription(TEST_GROUP_DESC).build();
    }
}
