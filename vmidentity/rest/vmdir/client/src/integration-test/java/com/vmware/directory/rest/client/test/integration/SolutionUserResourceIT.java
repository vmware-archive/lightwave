package com.vmware.directory.rest.client.test.integration;


import static com.vmware.directory.rest.client.test.integration.util.Assert.assertSolutionUsersEqual;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.directory.rest.client.test.integration.util.TestGenerator;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.client.NotFoundException;

public class SolutionUserResourceIT extends IntegrationTestBase {

    private static final String TEST_SOLUTION_USER_NAME = "testSolutionUser";
    private static final String TEST_SOLUTION_USER_DESCRIPTION = "Test description";

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init();
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        SolutionUserDTO solutionUser = null;
        // clean up solution user if it exists
        try {
            solutionUser = systemAdminClient.solutionUser().get(systemTenant, TEST_SOLUTION_USER_NAME);
        }  catch (NotFoundException e) {
            // Ignore it
        } finally {
            if (solutionUser != null) {
                deleteSolutionUser(TEST_SOLUTION_USER_NAME);
            }
        }
    }

    @Test
    public void testCreateAndDelete() throws ClientProtocolException, ClientException, HttpException, IOException, GeneralSecurityException {
        SolutionUserDTO user = createSolutionUser(TEST_SOLUTION_USER_NAME, TEST_SOLUTION_USER_DESCRIPTION);
        deleteSolutionUser(user.getName());
    }

    @Test
    public void testGet() throws ClientProtocolException, ClientException, HttpException, IOException, GeneralSecurityException {
        SolutionUserDTO testUser = createSolutionUser(TEST_SOLUTION_USER_NAME, TEST_SOLUTION_USER_DESCRIPTION);

        SolutionUserDTO user = systemAdminClient.solutionUser().get(systemTenant,  testUser.getName());

        assertNotNull(user);
        assertSolutionUsersEqual(testUser, user);

        deleteSolutionUser(user.getName());
    }

    @Test
    public void testUpdate() throws ClientProtocolException, ClientException, HttpException, IOException, GeneralSecurityException {
        SolutionUserDTO testUser = createSolutionUser(TEST_SOLUTION_USER_NAME, TEST_SOLUTION_USER_DESCRIPTION);

        SolutionUserDTO updated = systemAdminClient.solutionUser().update(systemTenant, testUser.getName(), testUser);

        assertSolutionUsersEqual(testUser, updated);

        deleteSolutionUser(testUser.getName());
    }

    private static SolutionUserDTO createSolutionUser(String name, String description) throws ClientProtocolException, ClientException, HttpException, IOException, GeneralSecurityException {
        SolutionUserDTO testUser = TestGenerator.generateSolutionUser(name, systemTenant, description, TestGenerator.generateCertificate("cn=SomeTestUser"));
        SolutionUserDTO user = systemAdminClient.solutionUser().create(systemTenant, testUser);

        assertSolutionUsersEqual(testUser, user);
        return user;
    }

    private static void deleteSolutionUser(String user) throws ClientProtocolException, ClientException, HttpException, IOException {
        systemAdminClient.solutionUser().delete(systemTenant, user);

        try {
            systemAdminClient.solutionUser().get(systemTenant, user);
            fail("Found solution user that was previously deleted");
        } catch (NotFoundException e) {
            // Ignore it
        }
    }

}
