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

import java.security.Principal;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.context.AuthorizationContext;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.server.mapper.UserMapper;
import com.vmware.identity.rest.idm.server.resources.UserResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.PrincipalAssertor;
import com.vmware.identity.rest.idm.server.test.integration.util.data.UserDataGenerator;
import com.vmware.identity.rest.idm.server.test.resources.UserResourceTest;

/**
 * Integration tests for User Resource
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class UserResourceIT extends TestBase {

    private static final String USERNAME = "testUser";
    private static final String USER_UPN = USERNAME + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String USER_UPN_UNKNOWN_USERNAME = "unknownUser" + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String USER_UPN_UNKNOWN_TENANT = USERNAME + "@" + "unknown.local";
    private static final boolean DISABLED = false;
    private static final boolean LOCKED = false;

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
