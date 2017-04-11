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

import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.server.resources.SolutionUserResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;

/**
 * Integration tests for Solution user Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class SolutionUserResourceIT extends TestBase {

    private static final String SOLUTION_USERNAME = "testSolutionUser";
    private static final String SOLUTIONUSER_UPN_UNKNOWN_USERNAME = "unknownSolutionUser" + "@" + DEFAULT_SYSTEM_DOMAIN;
    private static final String SOLUTIONUSER_UPN_UNKNOWN_TENANT = SOLUTION_USERNAME + "@" + "unknown.local";

    private SolutionUserResource solutionUserResource;
    private ContainerRequestContext request;

    @Before
    public void testSetUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        solutionUserResource = new SolutionUserResource(DEFAULT_TENANT, request, null);
        solutionUserResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetSolutionUser() throws Exception {
        try {
            // Test setup [Create solution user]
            solutionUserHelper.createSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);

            // Retrieve solution user
            SolutionUserDTO solutionUser = solutionUserResource.get(SOLUTION_USERNAME);

            assertEquals(SOLUTION_USERNAME, solutionUser.getName());
            assertEquals(DEFAULT_SYSTEM_DOMAIN, solutionUser.getDomain());

        } finally {
            solutionUserHelper.deleteSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetSolutionUser_WithNonExistentTenant_ThrowsNotFoundEx() {
        solutionUserResource = new SolutionUserResource("unknown.local", request, null);
        solutionUserResource.setIDMClient(idmClient);
        solutionUserResource.get(SOLUTION_USERNAME);
    }

    @Test(expected = NotFoundException.class)
    public void testGetSolutionUser_WithUnknownUser_ThrowsNotFoundEx() {
        solutionUserResource.get(SOLUTIONUSER_UPN_UNKNOWN_USERNAME);
    }

    @Test(expected = NotImplementedError.class)
    public void testGetGroupsOfSolutionuser_ThrowsNotImplementedError() {
        solutionUserResource.getGroups(SOLUTION_USERNAME, 200);
    }

}
