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
package com.vmware.identity.rest.idm.server.test.resources;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Locale;

import javax.ws.rs.BadRequestException;
import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.VmHostData;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;
import com.vmware.identity.rest.idm.data.attributes.ComputerType;
import com.vmware.identity.rest.idm.server.resources.ServerResource;

/**
 *
 * Unit tests for Server Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ServerResourceTest {

    private static final String TENANT_NAME = "vsphere.local";
    private static final String HOST_NAME = "testHost";
    private ServerResource serverResource;
    private CasIdmClient mockIDMClient;
    private IMocksControl mControl;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(request);

        mockIDMClient = mControl.createMock(CasIdmClient.class);
        serverResource = new ServerResource(request, null);
        serverResource.setIDMClient(mockIDMClient);
    }

    @Test
    public void testGetAllComputers() throws Exception {
        Collection<VmHostData> testVMs = new ArrayList<VmHostData>();
        testVMs.add(new VmHostData(HOST_NAME));
        expect(mockIDMClient.getComputers(TENANT_NAME, false)).andReturn(testVMs);
        expect(mockIDMClient.getSystemTenant()).andReturn(TENANT_NAME);
        mControl.replay();
        Collection<ServerDetailsDTO> resultMachines = serverResource.getComputers(ComputerType.ALL.name());
        assertEquals(1, resultMachines.size());
        mControl.verify();
    }

    @Test(expected=BadRequestException.class)
    public void testGetComputersOnInvalidComputerType_ThrowsBadRequestEx() {
        final String INVALID_COMP_TYPE = "invalid_comp";
        mControl.replay();
        serverResource.getComputers(INVALID_COMP_TYPE);
    }

    @Test(expected=InternalServerErrorException.class)
    public void testGetComputersOnsIDMError_ThrowsInternalServerError() throws Exception {
        mockIDMClient.getSystemTenant();
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        serverResource.getComputers(ComputerType.ALL.name());
        mControl.verify();
    }

    @Test
    public void testGetDomainControllerCompsOnly() throws Exception {
        Collection<VmHostData> testVMs = new ArrayList<VmHostData>();
        testVMs.add(new VmHostData(HOST_NAME));
        expect(mockIDMClient.getComputers(TENANT_NAME, true)).andReturn(testVMs);
        expect(mockIDMClient.getSystemTenant()).andReturn(TENANT_NAME);

        mControl.replay();
        Collection<ServerDetailsDTO> resultMachines = serverResource.getComputers(ComputerType.DC.name());
        assertEquals(1, resultMachines.size());
        ServerDetailsDTO virt = null;
        for (ServerDetailsDTO vm : resultMachines) {
            virt = vm;
        }
        assertEquals(HOST_NAME, virt.getHostname());
        mControl.verify();
    }

    @Test
    public void testGetComputers() throws Exception {

        VmHostData computer1 = new VmHostData("testHost-Type-COMPUTER-1", false);
        VmHostData computer2 = new VmHostData("testHost-Type-COMPUTER-2", false);
        VmHostData dc1 = new VmHostData("testHost-Type-DC-1", true);

        // VMs + DCs
        Collection<VmHostData> testVMs = new ArrayList<VmHostData>();
        testVMs.add(computer1);
        testVMs.add(computer2);
        testVMs.add(dc1);

        expect(mockIDMClient.getSystemTenant()).andReturn(TENANT_NAME);
        expect(mockIDMClient.getComputers(TENANT_NAME, false)).andReturn(testVMs);

        mControl.replay();
        Collection<ServerDetailsDTO> computerVMs = serverResource.getComputers(ComputerType.COMPUTER.name());
        assertEquals(2, computerVMs.size());
        mControl.verify();
    }
}
