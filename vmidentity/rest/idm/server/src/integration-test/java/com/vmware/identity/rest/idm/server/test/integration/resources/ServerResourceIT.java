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
package com.vmware.identity.rest.idm.server.test.integration.resources;

import static org.junit.Assert.assertEquals;

import java.util.Collection;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;
import com.vmware.identity.rest.idm.data.attributes.ComputerType;
import com.vmware.identity.rest.idm.server.resources.ServerResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;

/**
 * Integration tests for Server Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class ServerResourceIT extends TestBase {

    private ServerResource serverResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        serverResource = new ServerResource(request, null);
        serverResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetComputers() {
        Collection<ServerDetailsDTO> computers = serverResource.getComputers(ComputerType.ALL.name());
        assertEquals(1, computers.size());
    }
}
