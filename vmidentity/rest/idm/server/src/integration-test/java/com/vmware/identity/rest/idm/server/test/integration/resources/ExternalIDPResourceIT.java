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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Collection;
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;

import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.server.Config;
import com.vmware.identity.rest.idm.server.mapper.ExternalIDPMapper;
import com.vmware.identity.rest.idm.server.resources.ExternalIDPResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.util.IDPConfigUtil;

/**
 * Integration tests for External IDP
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
public class ExternalIDPResourceIT extends TestBase {

    private ExternalIDPResource resource;
    private HttpServletRequest request;

    @Before
    public void setup() {
        request = createMock(HttpServletRequest.class);
        expect(request.getLocale()).andReturn(Locale.getDefault()).anyTimes();
        expect(request.getHeader(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(request);

        resource = new ExternalIDPResource(DEFAULT_TENANT, request, null);
        resource.setIDMClient(idmClient);
    }

    @Test
    public void testGetAll() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        try {
            register(config);

            Collection<ExternalIDPDTO> idps = resource.getAll();

            assertEquals(1, idps.size());

            ExternalIDPDTO idp = idps.iterator().next();
            IDPConfigUtil.assertDTOEqual(config, idp);
        } finally {
            delete(config);
        }
    }

    @Test
    public void testGetAll_NoneYetRegistered() {
        Collection<ExternalIDPDTO> idps = resource.getAll();
        assertTrue(idps.isEmpty());
    }

    @Test(expected = NotFoundException.class)
    public void testGetAll_NoSuchTenant() {
        resource = new ExternalIDPResource("unknown.local", request, null);
        resource.setIDMClient(idmClient);
        resource.getAll();
    }

    @Test
    public void testRegister() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        try {
            ExternalIDPDTO idp = register(config);

            IDPConfigUtil.assertDTOEqual(config, idp);
        } finally {
            delete(config);
        }
    }

    @Ignore
    @Test(expected = NotFoundException.class)
    public void testRegister_NoSuchTenant() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        resource = new ExternalIDPResource("unknown.local", request, null);
        resource.setIDMClient(idmClient);

        resource.register(ExternalIDPMapper.getExternalIDPDTO(config));
    }

    @Test
    public void testGet() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        try {
            register(config);

            ExternalIDPDTO idp = resource.get(config.getEntityID());

            IDPConfigUtil.assertDTOEqual(config, idp);
        } finally {
            delete(config);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testGet_NoSuchEntity() throws Exception {
        resource.get("junk");
    }

    @Test(expected = NotFoundException.class)
    public void testGet_NoSuchTenant() throws Exception {
        resource = new ExternalIDPResource("unknown.local", request, null);
        resource.setIDMClient(idmClient);

        resource.get("junk");
    }

    @Test
    public void testDelete() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        try {
            register(config);
        } finally {
            delete(config);
        }
    }

    @Test(expected = NotFoundException.class)
    public void testDelete_NoSuchEntity() throws Exception {
        resource.delete("junk");
    }

    @Test(expected = NotFoundException.class)
    public void testDelete_NoSuchTenant() throws Exception {
        resource = new ExternalIDPResource("unknown.local", request, null);
        resource.setIDMClient(idmClient);

        resource.delete("junk");
    }

    private ExternalIDPDTO register(IDPConfig config) throws Exception {
        return resource.register(ExternalIDPMapper.getExternalIDPDTO(config));
    }

    private void delete(IDPConfig config) throws Exception {
        resource.delete(config.getEntityID());
    }

}
