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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.File;
import java.util.Arrays;
import java.util.Collection;
import java.util.Locale;
import java.util.Scanner;

import javax.ws.rs.container.ContainerRequestContext;

import org.junit.Before;
import org.junit.Test;
import org.w3c.dom.Document;

import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.server.mapper.ExternalIDPMapper;
import com.vmware.identity.rest.idm.server.resources.ExternalIDPResource;
import com.vmware.identity.rest.idm.server.test.util.IDPConfigUtil;

public class ExternalIDPResourceTest {

    private static final String TENANT = "test.tenant";
    private static final String TEST_DATA_EXTERNAL_IDP_PATH = "src/test/resources/data/external_idp_config.xml";

    private ExternalIDPResource resource;
    private CasIdmClient client;
    private ContainerRequestContext request;

    @Before
    public void setup() {
        request = createMock(ContainerRequestContext.class);
        expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(request);

        client = createMock(CasIdmClient.class);

        resource = new ExternalIDPResource(TENANT, request, null);
        resource.setIDMClient(client);
    }

    @Test
    public void testGetAllExternalIDPs() throws Exception {
        Collection<IDPConfig> configs = getIDPConfigs();
        expect(client.getAllExternalIdpConfig(TENANT)).andReturn(configs);
        replay(client);

        Collection<ExternalIDPDTO> externalIDPs = resource.getAll();
        verify(client);

        IDPConfigUtil.assertDTOEqual(configs, externalIDPs);
    }

    @Test(expected = NotFoundException.class)
    public void testGetAllExternalIDPs_NoSuchTenant() throws Exception {
        expect(client.getAllExternalIdpConfig(TENANT)).andThrow(new NoSuchTenantException("No such tenant"));
        replay(client);

        resource.getAll();
        verify(client);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetAllExternalIDPs_ThrowsInternalServerError() throws Exception {
        expect(client.getAllExternalIdpConfig(TENANT)).andThrow(new IDMException("IDM error"));
        replay(client);

        resource.getAll();

        verify(client);
    }

    @Test
    public void testRegisterExternalIDP() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        client.setExternalIdpConfig(eq(TENANT), isA(IDPConfig.class));
        expectLastCall();
        expect(client.getExternalIdpConfigForTenant(TENANT, config.getEntityID())).andReturn(config);
        replay(client);

        ExternalIDPDTO idp = resource.register(ExternalIDPMapper.getExternalIDPDTO(config));
        verify(client);

        IDPConfigUtil.assertDTOEqual(config, idp);
    }

    @Test
    public void testRegisterExternalIDP_WithMetadata() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        String entityId = "https://sc-rdops-vm06-dhcp-183-243.eng.vmware.com/websso/SAML2/Metadata/vsphere.local";
        String externalIDPConfig = new Scanner(new File(TEST_DATA_EXTERNAL_IDP_PATH)).useDelimiter("\\Z").next();
        expect(client.importExternalIDPConfiguration(eq(TENANT), isA(Document.class))).andReturn(entityId);
        expect(client.getExternalIdpConfigForTenant(TENANT, entityId)).andReturn(config);

        replay(client);

        ExternalIDPDTO idp = resource.registerWithMetadata(externalIDPConfig);
        IDPConfigUtil.assertDTOEqual(config, idp);
    }

    @Test(expected = NotFoundException.class)
    public void testRegisterExternalIDPWithMetaData_NoSuchTenant() throws Exception {
        expect(client.importExternalIDPConfiguration(eq(TENANT), isA(Document.class))).andThrow(new NoSuchTenantException("No such tenant"));
        expectLastCall().andThrow(new NoSuchTenantException("No such tenant"));
        replay(client);

        resource.registerWithMetadata(new Scanner(new File(TEST_DATA_EXTERNAL_IDP_PATH)).useDelimiter("\\Z").next());
        verify(client);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testRegisterExternalIDPWithMetadata_ThrowsInternalServerError() throws Exception {
        expect(client.importExternalIDPConfiguration(eq(TENANT), isA(Document.class)));
        expectLastCall().andThrow(new IDMException("IDM Error"));
        replay(client);

        resource.registerWithMetadata(new Scanner(new File(TEST_DATA_EXTERNAL_IDP_PATH)).useDelimiter("\\Z").next());
        verify(client);
    }

    @Test(expected = NotFoundException.class)
    public void testRegisterExternalIDP_NoSuchTenant() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        client.setExternalIdpConfig(eq(TENANT), isA(IDPConfig.class));
        expectLastCall().andThrow(new NoSuchTenantException("No such tenant"));
        replay(client);

        resource.register(ExternalIDPMapper.getExternalIDPDTO(config));
        verify(client);
    }

    @Test(expected = BadRequestException.class)
    public void testRegisterExternalIDP_InvalidArgument() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        client.setExternalIdpConfig(eq(TENANT), isA(IDPConfig.class));
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        replay(client);

        resource.register(ExternalIDPMapper.getExternalIDPDTO(config));
        verify(client);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testRegisterExternalIDP_ThrowsInternalServerError() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();

        client.setExternalIdpConfig(eq(TENANT), isA(IDPConfig.class));
        expectLastCall().andThrow(new IDMException("IDM Error"));
        replay(client);

        resource.register(ExternalIDPMapper.getExternalIDPDTO(config));
        verify(client);
    }

    @Test
    public void testGetExternalIDP() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        expect(client.getExternalIdpConfigForTenant(TENANT, config.getEntityID())).andReturn(config);
        replay(client);

        ExternalIDPDTO externalIDP = resource.get(config.getEntityID());
        verify(client);

        IDPConfigUtil.assertDTOEqual(config, externalIDP);
    }

    @Test(expected = NotFoundException.class)
    public void testGetExternalIDP_NoSuchTenant() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        expect(client.getExternalIdpConfigForTenant(TENANT, config.getEntityID())).andThrow(new NoSuchTenantException("No such tenant"));
        replay(client);

        resource.get(config.getEntityID());
        verify(client);
    }

    @Test(expected = NotFoundException.class)
    public void testGetExternalIDP_NoSuchEntity() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        expect(client.getExternalIdpConfigForTenant(TENANT, config.getEntityID())).andThrow(new NoSuchExternalIdpConfigException("No such entity"));
        replay(client);

        resource.get(config.getEntityID());
        verify(client);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetExternalIDP_ThrowsInternalServerError() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        expect(client.getExternalIdpConfigForTenant(TENANT, config.getEntityID())).andThrow(new IDMException("IDM Error"));
        replay(client);

        resource.get(config.getEntityID());
        verify(client);
    }

    @Test
    public void testDeleteExternalIDP() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        client.removeExternalIdpConfig(TENANT, config.getEntityID(), false);
        replay(client);

        resource.delete(config.getEntityID(), false);
        verify(client);
    }

    @Test
    public void testDeleteExternalIDPAndUsers() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        client.removeExternalIdpConfig(TENANT, config.getEntityID(), true);
        replay(client);

        resource.delete(config.getEntityID(), true);
        verify(client);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteExternalIDP_NoSuchTenant() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        client.removeExternalIdpConfig(TENANT, config.getEntityID(), false);
        expectLastCall().andThrow(new NoSuchTenantException("No such tenant"));
        replay(client);

        resource.delete(config.getEntityID(), false);
        verify(client);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteExternalIDP_NoSuchEntity() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        client.removeExternalIdpConfig(TENANT, config.getEntityID(), false);
        expectLastCall().andThrow(new NoSuchExternalIdpConfigException("No such entity"));
        replay(client);

        resource.delete(config.getEntityID(), false);
        verify(client);
    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteExternalIDP_ThrowsInternalServerError() throws Exception {
        IDPConfig config = IDPConfigUtil.createIDPConfig();
        client.removeExternalIdpConfig(TENANT, config.getEntityID(), false);
        expectLastCall().andThrow(new IDMException("IDM error"));
        replay(client);

        resource.delete(config.getEntityID(), false);
        verify(client);
    }

    private static Collection<IDPConfig> getIDPConfigs() throws Exception {
        return Arrays.asList(new IDPConfig[] { IDPConfigUtil.createIDPConfig() } );
    }

}
