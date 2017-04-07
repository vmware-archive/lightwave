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

import com.vmware.identity.idm.DomainType;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.attributes.IdentityProviderType;
import com.vmware.identity.rest.idm.server.resources.IdentityProviderResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;

/**
 * Integration tests for Identity provider Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class IdentityProviderResourceIT extends TestBase {

    private IdentityProviderResource providerResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        providerResource = new IdentityProviderResource(DEFAULT_TENANT, request, null);
        providerResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetProvider() {
        IdentityProviderDTO providerDTO = providerResource.get(VSPHERE_LOCAL);
        assertEquals(DomainType.SYSTEM_DOMAIN.name(), providerDTO.getDomainType());
        assertEquals(VSPHERE_LOCAL, providerDTO.getName());
    }

    @Test(expected = BadRequestException.class)
    public void testGetProvider_WithNonExistentTenant_ThrowsNotFoundEx() {
        /*
         * Currently, IDM is throwing InvalidArgumentException If tenant doesn't exist.
         * Ideally it should throw NoSuchTenantException. We should fix this in IDM and hence
         * re-visit these integration tests.
         */
        providerResource = new IdentityProviderResource("unknown.local", request, null);
        providerResource.setIDMClient(idmClient);
        providerResource.get(VSPHERE_LOCAL);
    }

    @Test
    public void testGetAllProviders() {
        Collection<IdentityProviderDTO> identityProviders = providerResource.getAll();
        assertEquals(2, identityProviders.size());
    }

    @Test(expected = NotFoundException.class)
    public void testGetAllProviders_WithNonExistentTenant_ThrowsNotFoundEx() {
        providerResource = new IdentityProviderResource("unknown.local", request, null);
        providerResource.setIDMClient(idmClient);
        providerResource.getAll();
    }

    @Ignore("Not ready yet. Require investigation to join AD programatically")
    @Test
    public void updateProvider_TimeOut() throws Exception {

    }

    @Test(expected=BadRequestException.class)
    public void updateProvider_WithNonExistentTenant_ThrowsNotFoundEx() {

        providerResource = new IdentityProviderResource("unknown.local", request, null);
        providerResource.setIDMClient(idmClient);
        IdentityProviderDTO idp = new IdentityProviderDTO.Builder()
                                                        .withType(IdentityProviderType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY.name())
                                                        .withSearchTimeOutInSeconds(100L)
                                                        .withName("unknown.local")
                                                        .withUsername("user@unknown.local")
                                                        .withPassword("Password!23")
                                                        .build();
        providerResource.update("unknown.local", idp);
    }

    @Ignore("Not ready yet")
    @Test
    public void testAddExternalProvider(){

    }

    @Ignore("Not ready yet")
    @Test
    public void testDeleteProvider(){

    }

}
