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

import java.io.IOException;
import java.security.cert.CertificateException;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.server.annotation.IntegrationTest;
import com.vmware.directory.rest.server.resources.SolutionUserResource;
import com.vmware.directory.rest.server.test.integration.datagenerator.SolutionUserDataGenerator;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;

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
    public void testCreateSolnUser() throws Exception {
        try {
            SolutionUserDTO solnUserToCreate = SolutionUserDataGenerator.createTestSolutionUserDTO(SOLUTION_USERNAME, DEFAULT_SYSTEM_DOMAIN, false);
            SolutionUserDTO solutionUser = solutionUserResource.create(solnUserToCreate);

            assertEquals(SOLUTION_USERNAME, solutionUser.getName());
            assertEquals(DEFAULT_SYSTEM_DOMAIN, solutionUser.getDomain());
        } finally {
            solutionUserHelper.deleteSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testCreateSolnUser_WithNonexistentTenant_ThrowsNotFoundEx() throws CertificateException, IOException {
        SolutionUserDTO solnUserToCreate = SolutionUserDataGenerator.createTestSolutionUserDTO(SOLUTION_USERNAME, "unknown.local", false);
        solutionUserResource = new SolutionUserResource("unknown.local", request, null);
        solutionUserResource.setIDMClient(idmClient);
        solutionUserResource.create(solnUserToCreate);
    }

    @Test(expected = BadRequestException.class)
    public void testCreateSolnUser_WithDuplicateSubjectDN_ThrowsInternalServerEx() throws Exception {
        try {
            SolutionUserDTO solnUserToCreate = SolutionUserDataGenerator.createTestSolutionUserDTO(SOLUTION_USERNAME, DEFAULT_SYSTEM_DOMAIN, false);
            solutionUserResource.create(solnUserToCreate);
            SolutionUserDTO duplicateSolutionUser = SolutionUserDataGenerator.createTestSolutionUserDTO("DuplicateSolutionUser", DEFAULT_SYSTEM_DOMAIN, false);
            solutionUserResource.create(duplicateSolutionUser);
        } finally {
            solutionUserHelper.deleteSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);
        }
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

    @Test
    public void testDeleteSolutionUser() throws Exception {

        // Test setup [Create solution user]
        solutionUserHelper.createSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME);

        // Delete solution user
        solutionUserResource.delete(SOLUTION_USERNAME);
        assertNull(solutionUserHelper.findSolutionUser(DEFAULT_SYSTEM_DOMAIN, SOLUTION_USERNAME));
    }

    @Test(expected=NotFoundException.class)
    public void testDeleteSolutionUser_WithNonExistentUser_ThrowsNotFoundEx() {
        solutionUserResource.delete(SOLUTIONUSER_UPN_UNKNOWN_USERNAME);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteSolutionUser_WithNonExistentTenant_ThrowsNotFoundEx() {
        solutionUserResource = new SolutionUserResource("unknown.local", request, null);
        solutionUserResource.setIDMClient(idmClient);
        solutionUserResource.delete(SOLUTIONUSER_UPN_UNKNOWN_TENANT);
    }

    @Test(expected = NotImplementedError.class)
    public void testGetGroupsOfSolutionuser_ThrowsNotImplementedError() {
        solutionUserResource.getGroups(SOLUTION_USERNAME, 200);
    }

}
