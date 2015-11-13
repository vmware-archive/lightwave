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
package com.vmware.identity.rest.idm.client.test.integration;

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertSolutionUsersEqual;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;

public class SolutionUserResourceIT extends IntegrationTestBase {

    private static final String TEST_SOLUTION_USER_NAME = "testSolutionUser";
    private static final String TEST_SOLUTION_USER_DESCRIPTION = "Test description";

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testCreateAndDelete() throws ClientProtocolException, ClientException, HttpException, IOException {
        SolutionUserDTO user = createSolutionUser(TEST_SOLUTION_USER_NAME, TEST_SOLUTION_USER_DESCRIPTION);
        deleteSolutionUser(user.getName());
    }

    @Test
    public void testGet() throws ClientProtocolException, ClientException, HttpException, IOException {
        SolutionUserDTO testUser = createSolutionUser(TEST_SOLUTION_USER_NAME, TEST_SOLUTION_USER_DESCRIPTION);

        SolutionUserDTO user = testAdminClient.solutionUser().get(testTenant.getName(),  testUser.getName());

        assertNotNull(user);
        assertSolutionUsersEqual(testUser, user);

        deleteSolutionUser(user.getName());
    }

    @Test
    public void testUpdate() throws ClientProtocolException, ClientException, HttpException, IOException {
        SolutionUserDTO testUser = createSolutionUser(TEST_SOLUTION_USER_NAME, TEST_SOLUTION_USER_DESCRIPTION);

        SolutionUserDTO updated = testAdminClient.solutionUser().update(testTenant.getName(), testUser.getName(), testUser);

        assertSolutionUsersEqual(testUser, updated);

        deleteSolutionUser(testUser.getName());
    }

    private static SolutionUserDTO createSolutionUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException {
        SolutionUserDTO testUser = TestGenerator.generateSolutionUser(name, testTenant.getName(), description, testTenant.getCredentials().getCertificates().get(0));
        SolutionUserDTO user = testAdminClient.solutionUser().create(testTenant.getName(), testUser);

        assertSolutionUsersEqual(testUser, user);
        return user;
    }

    private static void deleteSolutionUser(String user) throws ClientProtocolException, ClientException, HttpException, IOException {
        testAdminClient.solutionUser().delete(testTenant.getName(), user);

        try {
            testAdminClient.solutionUser().get(testTenant.getName(), user);
            fail("Found solution user that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

}
