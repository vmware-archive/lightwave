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

import static org.junit.Assert.assertNotNull;

import java.io.IOException;
import java.security.GeneralSecurityException;

import javax.xml.soap.SOAPException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import com.vmware.directory.rest.common.data.GroupDetailsDTO;
import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.AccessToken.Type;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.GroupDTO;

@RunWith(value = Parameterized.class)
public class GroupResourceIT extends IntegrationTestBase {

    @Parameters
    public static Object[] data() {
           return new Object[] { AccessToken.Type.JWT, AccessToken.Type.SAML };
    }

    public GroupResourceIT(Type tokenType) throws Exception {
        super(true, tokenType);
    }

    private static final String TEST_GROUP_NAME = "test.group";

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException, SOAPException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientProtocolException, HttpException, ClientException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    @Test
    public void testCreateAndGetGroup() throws ClientProtocolException, HttpException, ClientException, IOException {

        GroupDetailsDTO groupDetails = new GroupDetailsDTO.Builder().withDescription("test tenant group.").build();
        com.vmware.directory.rest.common.data.GroupDTO groupDTO = new com.vmware.directory.rest.common.data.GroupDTO.Builder()
                .withName(TEST_GROUP_NAME)
                .withDomain(testTenant.getName())
                .withDetails(groupDetails)
                .build();
        testVmdirClient.group().create(testTenant.getName(), groupDTO);
        try {
            GroupDTO group = testAdminClient.group().get(testTenant.getName(), TEST_GROUP_NAME, testTenant.getName());
            assertNotNull(group);
        } finally {
            testVmdirClient.group().delete(testTenant.getName(), TEST_GROUP_NAME, testTenant.getName());
        }
    }
}
