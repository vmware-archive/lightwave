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

import java.io.IOException;
import java.security.GeneralSecurityException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;

import com.vmware.identity.rest.core.client.exceptions.ClientException;

public class IdentityProviderResourceIT extends IntegrationTestBase {

    @BeforeClass
    public static void init() throws ClientException, ClientProtocolException, GeneralSecurityException, HttpException, IOException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientException, ClientProtocolException, HttpException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    // TODO Tests pending a good way to handle providers...

}
