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

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertContainsRP;
import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertRPsEqual;

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
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;

@RunWith(value = Parameterized.class)
public class RelyingPartyResourceIT extends IntegrationTestBase {

    @Parameters
    public static Object[] data() {
           return new Object[] { AccessToken.Type.JWT, AccessToken.Type.SAML };
    }

    public RelyingPartyResourceIT(Type tokenType) throws Exception {
        super(true, tokenType);
    }

    private static RelyingPartyDTO testRP;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException, SOAPException {
        IntegrationTestBase.init(true);

        testRP = TestGenerator.generateRelyingParty(testTenant.getCredentials().getCertificates().get(0));

        testAdminClient.relyingParty().register(testTenant.getName(), testRP);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        try {
            testAdminClient.relyingParty().delete(testTenant.getName(), testRP.getName());
        } finally {
            IntegrationTestBase.cleanup(true);
        }
    }

    @Test
    public void testGetAll() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<RelyingPartyDTO> rps = testAdminClient.relyingParty().getAll(testTenant.getName());

        assertContainsRP(testRP, rps);
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        RelyingPartyDTO rp = testAdminClient.relyingParty().get(testTenant.getName(), testRP.getName());

        assertRPsEqual(testRP, rp);
    }

    @Test
    public void testUpdate() throws ClientProtocolException, HttpException, ClientException, IOException {
        RelyingPartyDTO rp = testAdminClient.relyingParty().update(testTenant.getName(), testRP.getName(), testRP);

        assertRPsEqual(testRP, rp);
    }

}
