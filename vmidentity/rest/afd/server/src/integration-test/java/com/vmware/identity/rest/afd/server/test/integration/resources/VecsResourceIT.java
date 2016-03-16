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
package com.vmware.identity.rest.afd.server.test.integration.resources;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertFalse;

import java.util.Collection;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.rest.afd.server.resources.VecsResource;
import com.vmware.identity.rest.afd.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Config;

@Category(IntegrationTest.class)
public class VecsResourceIT extends TestBase {

    private VecsResource vecsResource;
    private ContainerRequestContext request;

    @Before
    public void setup() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(request);

        vecsResource = new VecsResource(request, null);
    }

    @Test
    public void testGetSSLCerts() {
        Collection<CertificateDTO> sslCerts = vecsResource.getSSLCertificates();
        assertFalse(sslCerts.isEmpty());
    }

}
