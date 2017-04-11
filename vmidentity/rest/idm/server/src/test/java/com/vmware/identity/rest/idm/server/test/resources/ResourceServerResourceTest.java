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

import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.NoSuchResourceServerException;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.server.resources.ResourceServerResource;

/**
 * @author Yehia Zayour
 */
public class ResourceServerResourceTest {

    private static final String TENANT = "test.local";
    private static final String NAME = "rs_test_resource_server";
    private static final Set<String> GROUP_FILTER = new HashSet<String>(Arrays.asList("domain\\group1", "domain\\group2"));
    private static final ResourceServer RESOURCE_SERVER = new ResourceServer.Builder(NAME).groupFilter(GROUP_FILTER).build();
    private static final ResourceServerDTO RESOURCE_SERVER_DTO = new ResourceServerDTO.Builder().withName(NAME).withGroupFilter(GROUP_FILTER).build();

    private ResourceServerResource resource;
    private IMocksControl mControl;
    private SecurityContext mockSecurityContext;
    private ContainerRequestContext mockRequest;
    private CasIdmClient mockIdmClient;

    @Before
    public void setUp() {
        this.mControl = EasyMock.createControl();
        this.mockRequest = EasyMock.createMock(ContainerRequestContext.class);
        expect(this.mockRequest.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(this.mockRequest.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(this.mockRequest);

        this.mockSecurityContext = this.mControl.createMock(SecurityContext.class);
        this.mockIdmClient = this.mControl.createMock(CasIdmClient.class);
        this.resource = new ResourceServerResource(TENANT, this.mockRequest, this.mockSecurityContext);
        this.resource.setIDMClient(this.mockIdmClient);
    }

    @Test
    public void testGetAllResourceServers() throws Exception {
        expect(this.mockIdmClient.getResourceServers(TENANT)).andReturn(Arrays.asList(RESOURCE_SERVER));
        this.mControl.replay();
        Collection<ResourceServerDTO> result = this.resource.getAll();
        assertEquals(1, result.size());
        this.mControl.verify();
    }

    @Test
    public void testGetResourceServer() throws Exception {
        expect(this.mockIdmClient.getResourceServer(TENANT, NAME)).andReturn(RESOURCE_SERVER);
        this.mControl.replay();
        ResourceServerDTO result = this.resource.get(NAME);
        assertResourceServerDTO(result);
        this.mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetResourceServer_OnNoSuchResourceServer() throws Exception {
        expect(this.mockIdmClient.getResourceServer(TENANT, NAME)).andThrow(new NoSuchResourceServerException("unit test thrown error"));
        this.mControl.replay();
        this.resource.get(NAME);
    }

    @Test
    public void testDeleteResourceServer() throws Exception {
        this.mockIdmClient.deleteResourceServer(TENANT, NAME);
        expectLastCall().once();
        this.mControl.replay();
        this.resource.delete(NAME);
        this.mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteResourceServer_OnNoSuchResourceServer() throws Exception {
        this.mockIdmClient.deleteResourceServer(TENANT, NAME);
        expectLastCall().andThrow(new NoSuchResourceServerException("Error thrown from unit testing"));
        this.mControl.replay();
        this.resource.delete(NAME);
    }

    @Test
    public void testUpdateResourceServer() throws Exception {
        this.mockIdmClient.setResourceServer(eq(TENANT), isA(ResourceServer.class));
        expect(this.mockIdmClient.getResourceServer(TENANT, NAME)).andReturn(RESOURCE_SERVER);
        this.mControl.replay();
        ResourceServerDTO result = this.resource.update(NAME, RESOURCE_SERVER_DTO);
        assertResourceServerDTO(result);
        this.mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateResourceServer_OnNoSuchResourceServer() throws Exception {
        this.mockIdmClient.setResourceServer(eq(TENANT), isA(ResourceServer.class));
        expectLastCall().andThrow(new NoSuchResourceServerException("Error thrown from unit testing"));
        this.mControl.replay();
        this.resource.update(NAME, RESOURCE_SERVER_DTO);
    }

    @Test(expected = BadRequestException.class)
    public void testUpdateResourceServer_OnNameInPathDoesNotMatchNameInBody() throws Exception {
        ResourceServerDTO rs = new ResourceServerDTO.Builder().withName(NAME + "non_matching").build();
        this.resource.update(NAME, rs);
    }

    @Test
    public void testAddResourceServer() throws Exception{
        this.mockIdmClient.addResourceServer(eq(TENANT), isA(ResourceServer.class));
        expect(this.mockIdmClient.getResourceServer(TENANT, NAME)).andReturn(RESOURCE_SERVER);
        this.mControl.replay();
        ResourceServerDTO result = this.resource.add(RESOURCE_SERVER_DTO);
        assertResourceServerDTO(result);
        this.mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testAddResourceServer_OnAlreadyExists() throws Exception{
        this.mockIdmClient.addResourceServer(eq(TENANT), isA(ResourceServer.class));
        expectLastCall().andThrow(new IDMException("Already exists"));
        this.mControl.replay();
        this.resource.add(RESOURCE_SERVER_DTO);
    }

    @Test(expected = BadRequestException.class)
    public void testAddResourceServer_OnInvalidName() throws Exception{
        this.mockIdmClient.addResourceServer(eq(TENANT), isA(ResourceServer.class));
        this.mControl.replay();
        ResourceServerDTO rs = new ResourceServerDTO.Builder().withName("should_start_with_rs_but_doesn't").build();
        this.resource.add(rs);
    }

    @Test(expected = BadRequestException.class)
    public void testAddResourceServer_OnInvalidGroupFilterEntry() throws Exception{
        this.mockIdmClient.addResourceServer(eq(TENANT), isA(ResourceServer.class));
        this.mControl.replay();
        Set<String> groupFilter = new HashSet<String>(Arrays.asList("group")); // entry must be of the form domain\name
        ResourceServerDTO rs = new ResourceServerDTO.Builder().withName(NAME).withGroupFilter(groupFilter).build();
        this.resource.add(rs);
    }

    private void assertResourceServerDTO(ResourceServerDTO resourceServerDTO) {
        assertEquals(NAME, resourceServerDTO.getName());
        assertEquals(GROUP_FILTER, resourceServerDTO.getGroupFilter());
    }
}
