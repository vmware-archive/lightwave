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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import static com.vmware.directory.rest.server.test.integration.util.PrincipalAssertor.assertUser;

import java.security.Principal;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.directory.rest.server.annotation.IntegrationTest;
import com.vmware.directory.rest.server.mapper.UserMapper;
import com.vmware.directory.rest.server.resources.UserResource;
import com.vmware.directory.rest.server.resources.UserResourceTest;
import com.vmware.directory.rest.server.test.integration.datagenerator.UserDataGenerator;
import com.vmware.directory.rest.server.test.integration.util.PrincipalAssertor;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.context.AuthorizationContext;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.directory.rest.common.data.PasswordResetRequestDTO;

/**
 * Integration tests for User Resource
 */
@Category(IntegrationTest.class)
public class UserResourceIT extends TestBase {

    private static final String USERNAME = "testUser";
    private static final String USER_UPN = USERNAME + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String USER_UPN_UNKNOWN_USERNAME = "unknownUser" + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String USER_UPN_UNKNOWN_TENANT = USERNAME + "@" + "unknown.local";
    private static final boolean DISABLED = false;
    private static final boolean LOCKED = false;

    private IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(UserResourceTest.class);

    private UserResource userResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        userResource = new UserResource(DEFAULT_TENANT, request, null);
        userResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetUser() throws Exception {
        try {
            // Prepare test setup [Create user]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // Retrieve user
            UserDTO userDTO = userResource.get(USER_UPN);

            UserDTO expectedUserDTO = UserMapper.getUserDTO(userToCreate, false);
            PrincipalAssertor.assertUser(expectedUserDTO, userDTO);

        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);// clean setup stuff
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetUser_WithUnknownUser_ThrowsNotFoundEx() {
        userResource.get(USER_UPN_UNKNOWN_USERNAME);
    }

    @Test(expected=NotFoundException.class)
    public void testGetUser_WithUknownTenant_ThrowsNotFoundEx() {
        userResource.get(USER_UPN_UNKNOWN_TENANT);
    }

    @Test
    public void testCreateUser() throws Exception {
        try {
            UserDTO userToCreate = UserDataGenerator.generateTestUserDTO(USERNAME, DEFAULT_TENANT);
            UserDTO userDTO = userResource.create(userToCreate);
            assertUser(userToCreate, userDTO);
        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }

    @Test(expected=BadRequestException.class)
    public void testCreateUser_WithUnknownTenant_ThrowsBadRequestException() {
        String unknownTenant = "unknown.local";
        UserDTO userToCreate = UserDataGenerator.generateTestUserDTO(USERNAME, unknownTenant);
        userResource.create(userToCreate);
    }

    @Test
    public void testDeleteUser() throws Exception {
        // Prepare test setup [Create user]
        PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, LOCKED);
        userHelper.createUser(DEFAULT_TENANT, userToCreate);

        // Delete user
        userResource.delete(USER_UPN);

        PersonUser user = userHelper.findUser(DEFAULT_TENANT, USERNAME);
        assertNull(user);
    }

    @Test(expected=NotFoundException.class)
    public void testDeleteUser_WithUnknownUser_ThrowsNotFoundException() {
        userResource.delete(USER_UPN_UNKNOWN_USERNAME);
    }

    @Test
    public void testUpdateUser() throws Exception{
        try{
            // Prepare test setup [Create user]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // Update user details
            String newDescription = "This is new description of user updated while running update integration tests";
            UserDetailsDTO personDetails = UserDetailsDTO.builder()
                                                           .withDescription(newDescription)
                                                           .build();
            UserDTO user =  UserDTO.builder()
                                      .withName(USERNAME)
                                      .withDomain(DEFAULT_TENANT)
                                      .withDetails(personDetails)
                                      .build();

            UserDTO updatedUser = userResource.update(USER_UPN, user);
            assertEquals(newDescription, updatedUser.getDetails().getDescription());
        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }

    @Test
    public void testUpdateUser_Disable() throws Exception{
        try {
            // Prepare test setup [Create user with enabled]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // Disable user
            UserDTO user =  UserDTO.builder()
                                      .withName(USERNAME)
                                      .withDomain(DEFAULT_TENANT)
                                      .withDisabled(true)
                                      .build();

            UserDTO updatedUser = userResource.update(USER_UPN, user);
            assertTrue(updatedUser.isDisabled());
        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }

    @Test
    public void testUpdateUser_Enable() throws Exception{
        try {
            // Prepare test setup [Create user with disabled]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, !DISABLED, LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // Enable user
            UserDTO user =  UserDTO.builder()
                                      .withName(USERNAME)
                                      .withDomain(DEFAULT_TENANT)
                                      .withDisabled(false)
                                      .build();

            UserDTO updatedUser = userResource.update(USER_UPN, user);
            assertFalse(updatedUser.isDisabled());
        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }

    @Test
    public void testUpdateUser_Unlock() throws Exception{
        try {
            // Prepare test setup [Create user with locked]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, !LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // Unlock user
            UserDTO user =  UserDTO.builder()
                                      .withName(USERNAME)
                                      .withDomain(DEFAULT_TENANT)
                                      .withLocked(false)
                                      .build();

            UserDTO updatedUser = userResource.update(USER_UPN, user);
            assertFalse(updatedUser.isLocked());
        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }


    @Test(expected=NotFoundException.class)
    public void testUpdate_WithNonExistentUser_ThrowsNotFoundEx() {
        UserDTO user =  UserDTO.builder()
                                  .withName(USERNAME)
                                  .withDomain(DEFAULT_TENANT)
                                  .withLocked(false)
                                  .build();
        userResource.update(USER_UPN_UNKNOWN_USERNAME, user);
    }

    @Test
    public void testUpdatePassword() throws Exception {
        try {
            // Prepare test setup [Create user]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, !LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // update user passwords as ADMIN
            userResource = new UserResource(DEFAULT_TENANT, request, getSecurityContext(Role.ADMINISTRATOR));
            userResource.setIDMClient(idmClient);
            UserDTO updatedUser = userResource.updatePassword(USER_UPN, new PasswordResetRequestDTO(null, "Admin!234"));
            assertEquals(USERNAME, updatedUser.getName());
        } finally {
            log.info("Deleting user : {}", USERNAME);
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }

    @Test(expected=BadRequestException.class)
    public void testUpdatePassword_ViolationPwdPolicy_ThrowsBadRequestEx() throws Exception {
        try {
            // Prepare test setup [Create user]
            PersonUser userToCreate = UserDataGenerator.generateTestUser(USERNAME, DEFAULT_TENANT, DISABLED, !LOCKED);
            userHelper.createUser(DEFAULT_TENANT, userToCreate);

            // update user passwords as ADMIN
            userResource = new UserResource(DEFAULT_TENANT, request, getSecurityContext(Role.ADMINISTRATOR));
            userResource.setIDMClient(idmClient);
            UserDTO updatedUser = userResource.updatePassword(USER_UPN, new PasswordResetRequestDTO(null, "Admin!"));
            assertEquals(USERNAME, updatedUser.getName());
        } finally {
            userHelper.deleteUser(DEFAULT_TENANT, USERNAME);
        }
    }

    private SecurityContext getSecurityContext(Role role){
        return new AuthorizationContext(getPrincipal(), role, true);
    }

    private Principal getPrincipal() {
        return new Principal() {
            @Override
            public String getName() {
                return USERNAME;
            }
        };
    }

}
