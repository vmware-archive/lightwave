package com.vmware.identity.rest.idm.client.test.integration;

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertFederatedIDPsEqual;
import static org.junit.Assert.assertFalse;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.FederatedIdpDTO;

public class FederatedIdpResourceIT extends IntegrationTestBase {

    private static FederatedIdpDTO testIDP;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);

        testIDP = TestGenerator.generateFederatedIdp();

        systemAdminClient.federatedIdp().register(systemTenantName, testIDP);
    }

    @Test
    public void testGetAll() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<FederatedIdpDTO> idps = systemAdminClient.federatedIdp().getAll(systemTenantName);

        assertFalse(idps.isEmpty());

        FederatedIdpDTO idp = idps.get(0);

        assertFederatedIDPsEqual(testIDP, idp);
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        FederatedIdpDTO idp = systemAdminClient.federatedIdp().get(systemTenantName, testIDP.getEntityID());

        assertFederatedIDPsEqual(testIDP, idp);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        try {
            systemAdminClient.federatedIdp().delete(systemTenantName, testIDP.getEntityID());
        } finally {
            IntegrationTestBase.cleanup(true);
        }
    }
}
