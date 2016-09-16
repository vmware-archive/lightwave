
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
package com.vmware.directory.rest.server.resources;

import static org.easymock.EasyMock.aryEq;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.security.Principal;
import java.util.Collection;
import java.util.EnumSet;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.PasswordDetailsDTO;
import com.vmware.directory.rest.common.data.PasswordResetRequestDTO;
import com.vmware.directory.rest.common.data.PrincipalDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.directory.rest.server.util.TestDataGenerator;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordPolicyViolationException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;

/**
 *
 * Unit tests for User Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class UserResourceTest {

    private static final String TENANT = "test.local";
    private static final String TEST_USERNAME = "testUser";
    private static final String TEST_DOMAIN = "test.local";
    private static final String TEST_USER_UPN = TEST_USERNAME + "@" + TEST_DOMAIN;

    private static final String TEST_ALIAS = "testUserAlias";
    private static final String TEST_ALIAS_DOMAIN = "test.local.alias";

    private static final String FIRST_NAME = "userFN";
    private static final String LAST_NAME = "userLN";
    private static final String DESC = "Test user created from unit tests";
    private static final String EMAIL = "testUser@test.com";
    private static final String UPN = "testUser@test.local";

    private static final String CURRENT_PWD = "oldpassword_That_Needs_To_Be_Updated";
    private static final String NEW_PWD = "newpassword_Being_Updated_With";

    private UserResource userResource;
    private IMocksControl mControl;
    private CasIdmClient mockCasIDMClient;
    private SecurityContext mockSecurityContext;
    private Principal mockPrincipal;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        mockCasIDMClient = mControl.createMock(CasIdmClient.class);
        mockSecurityContext = mControl.createMock(SecurityContext.class);
        mockPrincipal = mControl.createMock(Principal.class);
        userResource = new UserResource(TENANT, request, mockSecurityContext);
        userResource.setIDMClient(mockCasIDMClient);
    }

    @Test
    public void testGetUser() throws Exception {
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andReturn(getTestUser());
        expect(mockSecurityContext.isUserInRole(Role.ADMINISTRATOR.name())).andReturn(true);
        mControl.replay();

        UserDTO user = userResource.get(TEST_USER_UPN);

        assertEquals(TEST_USERNAME, user.getName());
        assertEquals(TEST_DOMAIN, user.getDomain());
        assertNotNull(user.getAlias());
        assertEquals(TEST_ALIAS, user.getAlias().getName());
        assertEquals(TEST_ALIAS_DOMAIN, user.getAlias().getDomain());
        assertFalse(user.isDisabled());
        assertFalse(user.isLocked());
        UserDetailsDTO personDetail = user.getDetails();
        assertNotNull(personDetail);
        assertEquals(FIRST_NAME, personDetail.getFirstName());
        assertEquals(LAST_NAME, personDetail.getLastName());
        assertEquals(DESC, personDetail.getDescription());
        assertEquals(EMAIL, personDetail.getEmail());
        assertEquals(UPN, personDetail.getUPN());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetOnNoSuchTenant_ThrowsNotFoundException() throws Exception {
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andThrow(new NoSuchTenantException("No such tenant"));
        mControl.replay();
        userResource.get(TEST_USER_UPN);
    }

    @Test(expected = BadRequestException.class)
    public void testGetOnInvalidArgument_ThrowsBadRequestException() throws Exception {
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andThrow(new InvalidArgumentException("Invalid Argument"));
        mControl.replay();
        userResource.get(TEST_USER_UPN);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andThrow(new IDMException("Internal server error"));
        mControl.replay();
        userResource.get(TEST_USER_UPN);
    }

    @Test
    public void testDelete() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));

        mockCasIDMClient.deletePrincipal(TENANT, TEST_USERNAME);
        mControl.replay();

        userResource.delete(TEST_USER_UPN);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        mockCasIDMClient.deletePrincipal(eq(TENANT), eq(TEST_USERNAME));
        expectLastCall().andThrow(new NoSuchTenantException("No such tenant"));
        mControl.replay();
        userResource.delete(TEST_USER_UPN);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        mockCasIDMClient.deletePrincipal(eq(TENANT), eq(TEST_USERNAME));
        expectLastCall().andThrow(new InvalidArgumentException("Invalid Argument"));
        mControl.replay();
        userResource.delete(TEST_USER_UPN);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        mockCasIDMClient.deletePrincipal(eq(TENANT), eq(TEST_USERNAME));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        userResource.delete(TEST_USER_UPN);
        mControl.verify();
    }

    @Test
    public void testGetGroups() throws Exception {
        Set<Group> groups = TestDataGenerator.getIdmGroups(2);
        expect(mockCasIDMClient.findDirectParentGroups(eq(TENANT), isA(PrincipalId.class))).andReturn(groups);
        mControl.replay();

        Collection<GroupDTO> groupDTOs = userResource.getGroups(TEST_USER_UPN, false);

        assertEquals(2, groupDTOs.size());

        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetGroupsOnNoSuchIDP_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.findDirectParentGroups(eq(TENANT), isA(PrincipalId.class))).andThrow(new NoSuchIdpException("No such provider"));
        mControl.replay();
        userResource.getGroups(TEST_USER_UPN, false);
    }

    @Test(expected = BadRequestException.class)
    public void testGetGroupsOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.findDirectParentGroups(eq(TENANT), isA(PrincipalId.class))).andThrow(new InvalidArgumentException("Invalid Argument"));
        mControl.replay();
        userResource.getGroups(TEST_USER_UPN, false);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetGroupsOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.findDirectParentGroups(eq(TENANT), isA(PrincipalId.class))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        userResource.getGroups(TEST_USER_UPN, false);
    }

    @Test
    public void testGetNestedGroups() throws Exception {
        Set<Group> directGroups = TestDataGenerator.getIdmGroups("DirectPrincipal", 2);
        Set<Group> indirectGroups = TestDataGenerator.getIdmGroups("IndirectPrincipal", 2);
        expect(mockCasIDMClient.findDirectParentGroups(eq(TENANT), isA(PrincipalId.class))).andReturn(directGroups);
        expect(mockCasIDMClient.findNestedParentGroups(eq(TENANT), isA(PrincipalId.class))).andReturn(indirectGroups);
        mControl.replay();
        Collection<GroupDTO> groupDTOs = userResource.getGroups(TEST_USER_UPN, true);
        assertEquals(4, groupDTOs.size());
    }

    @Test
    public void testCreateuser() throws Exception {
        PersonUser user = getTestUser();

        PrincipalId userId = new PrincipalId(TEST_USERNAME, TEST_DOMAIN);
        expect(mockCasIDMClient.addPersonUser(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class), aryEq("testUser!23".toCharArray()))).andReturn(userId);
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andReturn(user);
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));

        mControl.replay();

        UserDTO userDTO = getTestUserDTO();
        UserDTO userCreated = userResource.create(userDTO);

        assertEquals(TEST_USERNAME, userCreated.getName());
        assertEquals(TEST_DOMAIN, userCreated.getDomain());
        assertNotNull(userCreated.getDetails());
        assertEquals(UPN, userCreated.getDetails().getUPN());

        mControl.verify();
    }

    @Test
    public void testCreateUserWithDisabledStatus() throws Exception {

        PersonDetail personDetail = new PersonDetail.Builder().firstName(FIRST_NAME).lastName(LAST_NAME).description(DESC).emailAddress(EMAIL).userPrincipalName(UPN).build();
        PrincipalId personUserId = new PrincipalId(TEST_USERNAME, TEST_DOMAIN);
        PrincipalId aliasId = new PrincipalId(TEST_ALIAS, TEST_ALIAS_DOMAIN);
        PersonUser user = new PersonUser(personUserId, aliasId, null, personDetail, true, false);

        PrincipalId userId = new PrincipalId(TEST_USERNAME, TEST_DOMAIN);
        expect(mockCasIDMClient.addPersonUser(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class), aryEq("testUser!23".toCharArray()))).andReturn(userId);
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andReturn(user);
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockCasIDMClient.disableUserAccount(eq(TENANT), isA(PrincipalId.class))).andReturn(Boolean.TRUE);

        mControl.replay();

        PrincipalDTO alias = new PrincipalDTO(TEST_ALIAS, TEST_ALIAS_DOMAIN);
        PasswordDetailsDTO passwordDetails = PasswordDetailsDTO.builder().withPassword("testUser!23").build();
        UserDTO userToCreate = UserDTO.builder()
                      .withName(TEST_USERNAME)
                      .withDomain(TEST_DOMAIN)
                      .withAlias(alias)
                      .withDetails(getTestPersonDetailDTO())
                      .withLocked(false)
                      .withDisabled(true)
                      .withPasswordDetails(passwordDetails)
                      .build();
        UserDTO userCreated = userResource.create(userToCreate);
        assertTrue(userCreated.isDisabled());
        assertFalse(userCreated.isLocked());

    }

    @Test(expected=BadRequestException.class)
    public void testCreateUser_OnInvalidPwdPolicy_ThrowsBadRequest() throws Exception {
        expect(mockCasIDMClient.addPersonUser(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class), aryEq("invalidpwd".toCharArray())))
              .andThrow(new PasswordPolicyViolationException("Invalid Password. Password policy voliated"));
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));

        mControl.replay();

        PrincipalDTO alias = new PrincipalDTO(TEST_ALIAS, TEST_ALIAS_DOMAIN);
        PasswordDetailsDTO passwordDetails = PasswordDetailsDTO.builder().withPassword("invalidpwd").build();
        UserDTO userToCreate =  UserDTO.builder().withName(TEST_USERNAME).withDomain(TEST_DOMAIN).withAlias(alias).withDetails(getTestPersonDetailDTO()).withLocked(false).withDisabled(false).withPasswordDetails(passwordDetails).build();
        userResource.create(userToCreate);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testCreateUserOnNoTenant() throws Exception {
        expect(mockCasIDMClient.addPersonUser(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class), aryEq("testUser!23".toCharArray()))).andThrow(new NoSuchTenantException("no such tenant"));
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));

        mControl.replay();
        UserDTO userDTO = getTestUserDTO();
        userResource.create(userDTO);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testCreateUserOnInvalidPrincipal() throws Exception {
        expect(mockCasIDMClient.addPersonUser(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class), aryEq("testUser!23".toCharArray()))).andThrow(new InvalidPrincipalException("no such tenant"));
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));

        mControl.replay();
        UserDTO userDTO = getTestUserDTO();
        userResource.create(userDTO);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testCreateOnException_throwsInternalServerError() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockCasIDMClient.addPersonUser(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class), aryEq("testUser!23".toCharArray()))).andThrow(new Exception("UnitTest : IDM Error"));
        mControl.replay();
        userResource.create(getTestUserDTO());
        mControl.verify();
    }


    @Test
    public void testUpdateUser() throws Exception {
        PersonUser user = getTestUser();

        PrincipalId userId = new PrincipalId(TEST_USERNAME, TEST_DOMAIN);
        expect(mockCasIDMClient.updatePersonUserDetail(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class))).andReturn(userId);
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andReturn(user);
        expect(mockCasIDMClient.enableUserAccount(eq(TENANT), isA(PrincipalId.class))).andReturn(true);
        expect(mockCasIDMClient.unlockUserAccount(eq(TENANT), isA(PrincipalId.class))).andReturn(true);
        mControl.replay();

        UserDTO userDTO = getTestUserDTO();
        UserDTO updated = userResource.update(TEST_USER_UPN, userDTO);

        assertNotNull(updated);
        assertNotNull(updated.getDetails());
        assertEquals(userDTO.getDetails().getDescription(), updated.getDetails().getDescription());
        assertEquals(userDTO.getDetails().getEmail(), updated.getDetails().getEmail());
        assertEquals(userDTO.getDetails().getFirstName(), updated.getDetails().getFirstName());
        assertEquals(userDTO.getDetails().getLastName(), updated.getDetails().getLastName());
        assertEquals(userDTO.getDetails().getUPN(), updated.getDetails().getUPN());
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateOnInvalidPrincipalEx_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockCasIDMClient.updatePersonUserDetail(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class))).andThrow(new InvalidPrincipalException("Invalid Principal"));
        mControl.replay();
        userResource.update(TEST_USER_UPN, getTestUserDTO());
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockCasIDMClient.updatePersonUserDetail(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class))).andThrow(new InvalidArgumentException("Invalid argument"));
        mControl.replay();
        userResource.update(TEST_USER_UPN, getTestUserDTO());
    }

    @Test(expected = InternalServerErrorException.class)
    public void testUpdateOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockCasIDMClient.updatePersonUserDetail(eq(TENANT), eq(TEST_USERNAME), isA(PersonDetail.class))).andThrow(new IDMException("IDM error"));
        mControl.replay();
        userResource.update(TEST_USER_UPN, getTestUserDTO());
    }

    @Test
    public void testChangePasswordBySelf() throws Exception {
        expect(mockSecurityContext.getUserPrincipal()).andReturn(mockPrincipal);
        expect(mockPrincipal.getName()).andReturn(TEST_USER_UPN);
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_DOMAIN));
        expect(mockSecurityContext.isUserInRole(Role.ADMINISTRATOR.name())).andReturn(false).times(2);
        mockCasIDMClient.changeUserPassword(eq(TENANT), eq(TEST_USERNAME), aryEq(CURRENT_PWD.toCharArray()), aryEq(NEW_PWD.toCharArray()));
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andReturn(getTestUser());
        mControl.replay();

        userResource.updatePassword(TEST_USER_UPN, new PasswordResetRequestDTO(CURRENT_PWD, NEW_PWD));
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testChangePasswordBySelfWithoutOldPwd() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_DOMAIN));
        expect(mockSecurityContext.isUserInRole(Role.ADMINISTRATOR.name())).andReturn(false);
        mControl.replay();
        userResource.updatePassword(TEST_USER_UPN, new PasswordResetRequestDTO(null, NEW_PWD));
    }

    @Test(expected = BadRequestException.class)
    public void testUpdatePasswordViolatingPwdPolicy() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TENANT));
        expect(mockSecurityContext.isUserInRole(Role.ADMINISTRATOR.name())).andReturn(false);
        mockCasIDMClient.changeUserPassword(eq(TENANT), eq(TEST_USERNAME), aryEq(CURRENT_PWD.toCharArray()), aryEq(NEW_PWD.toCharArray()));
        expectLastCall().andThrow(new PasswordPolicyViolationException("Unit test : Password policy have been not met"));
        mControl.replay();
        userResource.updatePassword(TEST_USER_UPN, new PasswordResetRequestDTO(CURRENT_PWD, NEW_PWD));
    }

    @Test
    public void testUpdatePasswordWithAdminRole() throws Exception {

        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andReturn(TestDataGenerator.getTestSystemIDP(TEST_DOMAIN));
        expect(mockSecurityContext.isUserInRole(Role.ADMINISTRATOR.name())).andReturn(true).times(2);
        mockCasIDMClient.setUserPassword(eq(TENANT), eq(TEST_USERNAME), aryEq(NEW_PWD.toCharArray()));
        expect(mockCasIDMClient.findPersonUser(eq(TENANT), isA(PrincipalId.class))).andReturn(getTestUser());

        mControl.replay();
        userResource.updatePassword(TEST_USER_UPN, new PasswordResetRequestDTO(CURRENT_PWD, NEW_PWD));
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testUpdatePasswordOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.getProviders(eq(TENANT), isA(EnumSet.class))).andThrow(new NoSuchTenantException("No such tenant"));
        mControl.replay();
        userResource.updatePassword(TEST_USER_UPN, new PasswordResetRequestDTO(CURRENT_PWD, NEW_PWD));
    }

    private PersonUser getTestUser() {
        PersonDetail personDetail = new PersonDetail.Builder().firstName(FIRST_NAME).lastName(LAST_NAME).description(DESC).emailAddress(EMAIL).userPrincipalName(UPN).build();
        PrincipalId personUserId = new PrincipalId(TEST_USERNAME, TEST_DOMAIN);
        PrincipalId aliasId = new PrincipalId(TEST_ALIAS, TEST_ALIAS_DOMAIN);
        return new PersonUser(personUserId, aliasId, null, personDetail, false, false);
    }


    private UserDTO getTestUserDTO() {
        PrincipalDTO alias = new PrincipalDTO(TEST_ALIAS, TEST_ALIAS_DOMAIN);
        PasswordDetailsDTO passwordDetails = PasswordDetailsDTO.builder().withPassword("testUser!23").build();

        return UserDTO.builder().withName(TEST_USERNAME).withDomain(TEST_DOMAIN).withAlias(alias).withDetails(getTestPersonDetailDTO()).withLocked(false).withDisabled(false).withPasswordDetails(passwordDetails).build();
    }

    private UserDetailsDTO getTestPersonDetailDTO() {
        return UserDetailsDTO.builder().withFirstName(FIRST_NAME).withLastName(LAST_NAME).withDescription(DESC).withUPN(UPN).withEmail(EMAIL).build();
    }
    
}
