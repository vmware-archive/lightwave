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
package com.vmware.identity.rest.idm.server.test.resources;

import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.Collection;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.InvalidArgumentException;
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
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.server.resources.GroupResource;
import com.vmware.identity.rest.idm.server.test.util.TestDataGenerator;

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

    @Test(expected=NotImplementedError.class)
    public void testGetParentsWithNestedThrowsNotImplementedEx() {
        mControl.replay();
        groupResource.getParents(TEST_GROUP_UPN, true);
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

}
