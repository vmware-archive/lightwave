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

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.server.resources.ResourceServerResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.data.ResourceServerDataGenerator;

/**
 * Integration test for ResourceServer Resource
 *
 * @author Yehia Zayour
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class ResourceServerResourceIT extends TestBase {

    private static final String NAME = "rs_CreatedFromIntegrationTest";
    private static final String NAME_NON_EXISTENT = "rs_nonexistent_resource_server";

    private ResourceServerResource resource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);
        resource = new ResourceServerResource(DEFAULT_TENANT, request, null);
        resource.setIDMClient(idmClient);
    }

    @Test
    public void testAddResourceServer() throws Exception {
        try {
            ResourceServerDTO result = resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
            assertResourceServer(result);
        } finally {
            resource.delete(NAME);
        }
    }

    @Test(expected = InternalServerErrorException.class)
    public void testAddResourceServerAlreadyExists() throws Exception {
        try {
            resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
            resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
        } finally {
            resource.delete(NAME);
        }
    }

    @Test
    public void testGetResourceServer() throws Exception {
        try {
            resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
            ResourceServerDTO result = resource.get(NAME);
            assertResourceServer(result);
        } finally {
            resource.delete(NAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGetResourceServerDoesNotExist() throws Exception {
        ResourceServerDTO result = resource.get(NAME_NON_EXISTENT);
    }

    @Test
    public void testGetAllResourceServers() throws Exception {
        try {
            resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
            Collection<ResourceServerDTO> result = resource.getAll();
            assertEquals(1, result.size());
            resource.add(ResourceServerDataGenerator.generateResourceServerDTO("rs_another_resource_server"));
            result = resource.getAll();
            assertEquals(2, result.size());
        } finally {
            resource.delete(NAME);
            resource.delete("rs_another_resource_server");
        }
    }

    public void testDeleteResourceServer() throws Exception {
        resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
        resource.delete(NAME);
        Collection<ResourceServerDTO> result = resource.getAll();
        assertEquals(0, result.size());
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteResourceServerDoesNotExist() throws Exception {
        resource.delete(NAME_NON_EXISTENT);
    }

    @Test
    public void testUpdateResourceServer() throws Exception {
        try {
            resource.add(ResourceServerDataGenerator.generateResourceServerDTO(NAME));
            Set<String> groupFilter = new HashSet<String>(Arrays.asList("domain\\a", "domain\\b"));
            ResourceServerDTO rsToUpdate = ResourceServerDTO.builder().withName(NAME).withGroupFilter(groupFilter).build();
            ResourceServerDTO updatedRS = resource.update(NAME, rsToUpdate);
            assertEquals(groupFilter, updatedRS.getGroupFilter());
        } finally {
            resource.delete(NAME);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateResourceServerDoesNotExist() throws Exception {
        ResourceServerDTO rsToUpdate = ResourceServerDTO.builder().withName(NAME_NON_EXISTENT).build();
        resource.update(NAME_NON_EXISTENT, rsToUpdate);
    }

    private static void assertResourceServer(ResourceServerDTO rs) {
        assertEquals(NAME, rs.getName());
        assertEquals(ResourceServerDataGenerator.GROUP_FILTER, rs.getGroupFilter());
    }
}
