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

import static com.vmware.identity.rest.idm.client.test.integration.util.Assert.assertExternalIDPsEqual;
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
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;

public class ExternalIDPResourceIT extends IntegrationTestBase {

    private static ExternalIDPDTO testIDP;

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException {
        IntegrationTestBase.init(true);

        testIDP = TestGenerator.generateExternalIDP(testTenant.getCredentials().getCertificates().get(0));

        testAdminClient.externalIdp().register(testTenant.getName(), testIDP);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        try {
            testAdminClient.externalIdp().delete(testTenant.getName(), testIDP.getEntityID());
        } finally {
            IntegrationTestBase.cleanup(true);
        }
    }

    @Test
    public void testGetAll() throws ClientProtocolException, HttpException, ClientException, IOException {
        List<ExternalIDPDTO> idps = testAdminClient.externalIdp().getAll(testTenant.getName());

        assertFalse(idps.isEmpty());

        ExternalIDPDTO ex = idps.get(0);

        assertExternalIDPsEqual(testIDP, ex);
    }

    @Test
    public void testGet() throws ClientProtocolException, HttpException, ClientException, IOException {
        ExternalIDPDTO ex = testAdminClient.externalIdp().get(testTenant.getName(), testIDP.getEntityID());

        assertExternalIDPsEqual(testIDP, ex);
    }

}
