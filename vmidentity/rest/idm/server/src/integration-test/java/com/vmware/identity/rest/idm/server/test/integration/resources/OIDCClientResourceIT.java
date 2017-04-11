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
import java.util.List;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.server.resources.OIDCClientResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.data.OIDCClientDataGenerator;

/**
 * OIDCClient Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class OIDCClientResourceIT extends TestBase {

    private OIDCClientResource oidcClientResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        this.request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(this.request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(this.request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(this.request);
        this.oidcClientResource = new OIDCClientResource(DEFAULT_TENANT, this.request, null);
        this.oidcClientResource.setIDMClient(this.idmClient);
    }

    @Test
    public void testAddOIDCClient() throws Exception {
        String clientId1 = null;

        // test add
        OIDCClientMetadataDTO oidcClientMetadataDTOToAdd = OIDCClientDataGenerator.generateOIDCClientMetadataDTO();
        OIDCClientDTO response1 = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        clientId1 = response1.getClientId();
        assertOIDCClient(response1, clientId1);

        Assert.assertNotNull(this.oidcClientResource.get(clientId1));
        this.oidcClientResource.delete(clientId1);
    }

    @Test
    public void testGetOIDCClient() throws Exception {
        String clientId1 = null;

        // test add
        OIDCClientMetadataDTO oidcClientMetadataDTOToAdd = OIDCClientDataGenerator.generateOIDCClientMetadataDTO();
        OIDCClientDTO response1 = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        clientId1 = response1.getClientId();
        assertOIDCClient(response1, clientId1);

        // test get
        OIDCClientDTO result = this.oidcClientResource.get(clientId1);
        assertOIDCClient(result, clientId1);

        Assert.assertNotNull(this.oidcClientResource.get(clientId1));
        this.oidcClientResource.delete(clientId1);
    }

    @Test
    public void testUpdateOIDCClient() throws Exception {
        String clientId1 = null;

        // test add
        OIDCClientMetadataDTO oidcClientMetadataDTOToAdd = OIDCClientDataGenerator.generateOIDCClientMetadataDTO();
        OIDCClientDTO response1 = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        clientId1 = response1.getClientId();
        assertOIDCClient(response1, clientId1);

        // test update client 1
        List<String> redirectURIs = Arrays.asList("https://www.vmware.com/redirect3");
        List<String> postLogoutRedirectUris = Arrays.asList("https://www.vmware.com/postlogoutredirect3");
        OIDCClientMetadataDTO oidcClientToUpdate = new OIDCClientMetadataDTO.Builder().
                withRedirectUris(redirectURIs).
                withTokenEndpointAuthMethod(OIDCClientDataGenerator.TOKEN_ENDPOINT_AUTH_METHOD).
                withPostLogoutRedirectUris(postLogoutRedirectUris).
                withLogoutUri(OIDCClientDataGenerator.LOGOUT_URI).
                withCertSubjectDN(OIDCClientDataGenerator.CERT_SUBJECT_DN).
                withAuthnRequestClientAssertionLifetimeMS(OIDCClientDataGenerator.AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).
                build();
        OIDCClientDTO updatedOIDCClient = this.oidcClientResource.update(clientId1, oidcClientToUpdate);
        assertEquals(redirectURIs, updatedOIDCClient.getOIDCClientMetadataDTO().getRedirectUris());
        assertEquals(postLogoutRedirectUris, updatedOIDCClient.getOIDCClientMetadataDTO().getPostLogoutRedirectUris());

        Assert.assertNotNull(this.oidcClientResource.get(clientId1));
        this.oidcClientResource.delete(clientId1);
    }

    @Test
    public void testGetAllOIDCClient() throws Exception {
        String clientId1 = null;
        String clientId2 = null;

        // test add
        OIDCClientMetadataDTO oidcClientMetadataDTOToAdd = OIDCClientDataGenerator.generateOIDCClientMetadataDTO();
        OIDCClientDTO response1 = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        clientId1 = response1.getClientId();
        assertOIDCClient(response1, clientId1);

        // test add another client
        List<String> redirectURIs = Arrays.asList("https://www.vmware.com/redirect3");
        List<String> postLogoutRedirectUris = Arrays.asList("https://www.vmware.com/postlogoutredirect3");
        oidcClientMetadataDTOToAdd = new OIDCClientMetadataDTO.Builder().
                withRedirectUris(redirectURIs).
                withTokenEndpointAuthMethod(OIDCClientDataGenerator.TOKEN_ENDPOINT_AUTH_METHOD).
                withPostLogoutRedirectUris(postLogoutRedirectUris).
                withLogoutUri(OIDCClientDataGenerator.LOGOUT_URI).
                withCertSubjectDN(OIDCClientDataGenerator.CERT_SUBJECT_DN).
                withAuthnRequestClientAssertionLifetimeMS(OIDCClientDataGenerator.AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).
                build();
        OIDCClientDTO response2 = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        clientId2 = response2.getClientId();

        // test get all
        Collection<OIDCClientDTO> responseAll = this.oidcClientResource.getAll();
        assertEquals(2, responseAll.size());

        Assert.assertNotNull(this.oidcClientResource.get(clientId1));
        this.oidcClientResource.delete(clientId1);
        Assert.assertNotNull(this.oidcClientResource.get(clientId2));
        this.oidcClientResource.delete(clientId2);
    }

    @Test
    public void testDeleteOIDCClient() throws Exception {
        String clientId1 = null;

        // test add
        OIDCClientMetadataDTO oidcClientMetadataDTOToAdd = OIDCClientDataGenerator.generateOIDCClientMetadataDTO();
        OIDCClientDTO response1 = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        clientId1 = response1.getClientId();
        assertOIDCClient(response1, clientId1);

        // test delete client 1
        Assert.assertNotNull(this.oidcClientResource.get(clientId1));
        this.oidcClientResource.delete(clientId1);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteNonExistOIDCClientx() {
        this.oidcClientResource.delete("unknownOIDCClient");
    }

    @Test(expected = NotFoundException.class)
    public void testUpdateNonExistOIDCClientx() {
        OIDCClientMetadataDTO oidcClientToUpdate = new OIDCClientMetadataDTO.Builder().
                withRedirectUris(OIDCClientDataGenerator.REDIRECT_URIS).
                withTokenEndpointAuthMethod(OIDCClientDataGenerator.TOKEN_ENDPOINT_AUTH_METHOD).
                withPostLogoutRedirectUris(OIDCClientDataGenerator.POST_LOGOUT_REDIRECT_URIS).
                withLogoutUri(OIDCClientDataGenerator.LOGOUT_URI).
                withCertSubjectDN(OIDCClientDataGenerator.CERT_SUBJECT_DN).
                withAuthnRequestClientAssertionLifetimeMS(OIDCClientDataGenerator.AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).
                build();
        this.oidcClientResource.update("unknownOIDCClient", oidcClientToUpdate);
    }

    private void assertOIDCClient(OIDCClientDTO oidcClientDTO, String clientId) {
        assertEquals(clientId, oidcClientDTO.getClientId());
        assertEquals(OIDCClientDataGenerator.REDIRECT_URIS, oidcClientDTO.getOIDCClientMetadataDTO().getRedirectUris());
        assertEquals(OIDCClientDataGenerator.TOKEN_ENDPOINT_AUTH_METHOD, oidcClientDTO.getOIDCClientMetadataDTO().getTokenEndpointAuthMethod());
        assertEquals(OIDCClientDataGenerator.POST_LOGOUT_REDIRECT_URIS, oidcClientDTO.getOIDCClientMetadataDTO().getPostLogoutRedirectUris());
        assertEquals(OIDCClientDataGenerator.LOGOUT_URI, oidcClientDTO.getOIDCClientMetadataDTO().getLogoutUri());
        assertEquals(OIDCClientDataGenerator.CERT_SUBJECT_DN, oidcClientDTO.getOIDCClientMetadataDTO().getCertSubjectDN());
        assertEquals(OIDCClientDataGenerator.AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS, oidcClientDTO.getOIDCClientMetadataDTO().getAuthnRequestClientAssertionLifetimeMS());
    }
}
