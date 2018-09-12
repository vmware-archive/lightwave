package com.vmware.identity.rest.idm.client.test.integration;

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertFederatedIDPsEqual;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.List;

import javax.xml.soap.SOAPException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.AccessToken.Type;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.client.test.integration.util.TestGenerator;
import com.vmware.identity.rest.idm.data.FederatedIdpDTO;

@RunWith(value = Parameterized.class)
public class FederatedIdpResourceIT extends IntegrationTestBase {

    @Parameters
    public static Object[] data() {
           return new Object[] { AccessToken.Type.JWT, AccessToken.Type.SAML };
    }

    public FederatedIdpResourceIT(Type tokenType) throws Exception {
        super(true, tokenType);
    }

    private static FederatedIdpDTO testIDP;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException, SOAPException {
        IntegrationTestBase.init(true);

        testIDP = TestGenerator.generateFederatedIdp();

        systemAdminClient.federatedIdp().register(systemTenantName, testIDP);
    }

    @Test
    public void testGetAll() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<FederatedIdpDTO> idps = systemAdminClient.federatedIdp().getAll(systemTenantName);

        assertFalse(idps.isEmpty());

        boolean idpFound = false;
        for (FederatedIdpDTO idp : idps)
        {
            if (idp != null && idp.getEntityID().equals(testIDP.getEntityID()))
            {
                assertFederatedIDPsEqual(testIDP, idp);
                idpFound = true;
            }
        }

        assertTrue("Could not find TestIDP", idpFound);
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
