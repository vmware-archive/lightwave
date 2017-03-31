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

import java.io.IOException;
import java.security.cert.CertificateException;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.easymock.IAnswer;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.server.resources.OIDCClientResource;

/**
 *
 * Unit tests for OIDCClient Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class OIDCClientResourceTest {

    private static final String TENANT = "test.local";
    private static final String CLIENT_ID = "test.client";

    private static final List<String> REDIRECT_URIS = Arrays.asList("https://www.vmware.com/redirect1", "https://www.vmware.com/redirect2");
    private static final String TOKEN_ENDPOINT_AUTH_METHOD = "private_key_jwt";
    private static final List<String> POST_LOGOUT_REDIRECT_URIS = Arrays.asList("https://www.vmware.com/postlogoutredirect1");
    private static final String LOGOUT_URI = "https://www.vmware.com/logout";
    private static final String CERT_SUBJECT_DN = "OU=mID-2400d17e-d4f4-4753-98fd-fb9ecbf098ae,C=US,DC=local,DC=vsphere,CN=oidc-client-123";
    private static final Long AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS = new Long(1234L);

    private OIDCClientResource oidcClientResource;
    private IMocksControl mControl;
    private SecurityContext mockSecurityContext;
    private ContainerRequestContext mockRequest;
    private CasIdmClient mockIDMClient;

    @Before
    public void setUp() {
        this.mControl = EasyMock.createControl();
        this.mockRequest = EasyMock.createMock(ContainerRequestContext.class);
        expect(this.mockRequest.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(this.mockRequest.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(this.mockRequest);

        this.mockSecurityContext = this.mControl.createMock(SecurityContext.class);
        this.mockIDMClient = this.mControl.createMock(CasIdmClient.class);
        this.oidcClientResource = new OIDCClientResource(TENANT, this.mockRequest, this.mockSecurityContext);
        this.oidcClientResource.setIDMClient(this.mockIDMClient);

    }

    @Test
    public void testGetOIDCClient_OnName() throws Exception {
        expect(this.mockIDMClient.getOIDCClient(TENANT, CLIENT_ID)).andReturn(getTestOIDCClient(CLIENT_ID));
        this.mControl.replay();

        OIDCClientDTO resultOIDCClientDTO = this.oidcClientResource.get(CLIENT_ID);
        assertOIDCClient(resultOIDCClientDTO, CLIENT_ID);
        this.mControl.verify();
    }

    @Test
    public void testGetAllOIDCClients() throws Exception {
        expect(this.mockIDMClient.getOIDCClients(TENANT)).andReturn(Arrays.asList(getTestOIDCClient(CLIENT_ID)));
        this.mControl.replay();
        Collection<OIDCClientDTO> oidcClientDTOs = this.oidcClientResource.getAll();
        assertEquals(1, oidcClientDTOs.size());
        this.mControl.verify();
    }

    @Test(expected=NotFoundException.class)
    public void testGetOIDCClient_OnNoSuchTenant() throws Exception {
        expect(this.mockIDMClient.getOIDCClient(TENANT, CLIENT_ID)).andThrow(new NoSuchTenantException("unit test thrown error"));
        this.mControl.replay();
        this.oidcClientResource.get(CLIENT_ID);
        this.mControl.verify();
    }

    @Test
    public void testDeleteOIDCClient() throws Exception {
        this.mockIDMClient.deleteOIDCClient(TENANT, CLIENT_ID);
        expectLastCall().once();
        this.mControl.replay();

        this.oidcClientResource.delete(CLIENT_ID);
        this.mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteOIDCClient_OnNoSuchTenant() throws Exception {
        this.mockIDMClient.deleteOIDCClient(TENANT, CLIENT_ID);
        expectLastCall().andThrow(new NoSuchTenantException("Error thrown from unit testing"));
        this.mControl.replay();

        this.oidcClientResource.delete(CLIENT_ID);
        this.mControl.verify();
    }

    @Test
    public void testAddOIDCClient() throws Exception{
        OIDCClientMetadataDTO oidcClientMetadataDTOToAdd = new OIDCClientMetadataDTO.Builder().
                withRedirectUris(REDIRECT_URIS).
                withTokenEndpointAuthMethod(TOKEN_ENDPOINT_AUTH_METHOD).
                withPostLogoutRedirectUris(POST_LOGOUT_REDIRECT_URIS).
                withLogoutUri(LOGOUT_URI).
                withCertSubjectDN(CERT_SUBJECT_DN).
                withAuthnRequestClientAssertionLifetimeMS(AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).build();

        this.mockIDMClient.addOIDCClient(eq(TENANT),isA(OIDCClient.class));
        expect(this.mockIDMClient.getOIDCClient(eq(TENANT), isA(String.class))).andAnswer(
                new IAnswer<OIDCClient>() {
                    @Override
                    public OIDCClient answer() throws Throwable {
                        return getTestOIDCClient((String) EasyMock.getCurrentArguments()[1]);
                    }
                }
        );
        this.mControl.replay();
        OIDCClientDTO resultOIDCClientDTO = this.oidcClientResource.add(oidcClientMetadataDTOToAdd);
        assertOIDCClient(resultOIDCClientDTO, resultOIDCClientDTO.getClientId());
        this.mControl.verify();
    }

    @Test
    public void testUpdateOIDCClient() throws Exception {
        this.mockIDMClient.setOIDCClient(eq(TENANT), isA(OIDCClient.class));
        expect(this.mockIDMClient.getOIDCClient(TENANT, CLIENT_ID)).andReturn(getTestOIDCClient(CLIENT_ID));
        this.mControl.replay();
        OIDCClientDTO updatedOIDCClientDTO = this.oidcClientResource.update(CLIENT_ID, getTestOIDCClientMetadataDTO(CLIENT_ID));
        assertOIDCClient(updatedOIDCClientDTO, CLIENT_ID);
        this.mControl.verify();
    }

    @Test(expected=NotFoundException.class)
    public void testUpdateOIDCClient_OnNoSuchRp() throws Exception {
        this.mockIDMClient.setOIDCClient(eq(TENANT), isA(OIDCClient.class));
        expectLastCall().andThrow(new NoSuchOIDCClientException("Error thrown from unit testing"));
        this.mControl.replay();
        this.oidcClientResource.update(CLIENT_ID, getTestOIDCClientMetadataDTO(CLIENT_ID));
        this.mControl.verify();
    }

    @Test(expected=NotFoundException.class)
    public void testUpdateOIDCClient_OnNoSuchTenant() throws Exception {
        this.mockIDMClient.setOIDCClient(eq(TENANT), isA(OIDCClient.class));
        expectLastCall().andThrow(new NoSuchTenantException("Error thrown from unit testing"));
        this.mControl.replay();
        this.oidcClientResource.update(CLIENT_ID, getTestOIDCClientMetadataDTO(CLIENT_ID));
        this.mControl.verify();
    }

    private void assertOIDCClient(OIDCClientDTO oidcClientDTO, String clientId) {
        assertEquals(clientId, oidcClientDTO.getClientId());
        assertEquals(REDIRECT_URIS, oidcClientDTO.getOIDCClientMetadataDTO().getRedirectUris());
        assertEquals(TOKEN_ENDPOINT_AUTH_METHOD, oidcClientDTO.getOIDCClientMetadataDTO().getTokenEndpointAuthMethod());
        assertEquals(POST_LOGOUT_REDIRECT_URIS, oidcClientDTO.getOIDCClientMetadataDTO().getPostLogoutRedirectUris());
        assertEquals(LOGOUT_URI, oidcClientDTO.getOIDCClientMetadataDTO().getLogoutUri());
        assertEquals(CERT_SUBJECT_DN, oidcClientDTO.getOIDCClientMetadataDTO().getCertSubjectDN());
        assertEquals(AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS, oidcClientDTO.getOIDCClientMetadataDTO().getAuthnRequestClientAssertionLifetimeMS());
    }

    private OIDCClient getTestOIDCClient(String clientId) throws CertificateException, IOException {
        return new OIDCClient.Builder(clientId).
                redirectUris(REDIRECT_URIS).
                tokenEndpointAuthMethod(TOKEN_ENDPOINT_AUTH_METHOD).
                postLogoutRedirectUris(POST_LOGOUT_REDIRECT_URIS).
                logoutUri(LOGOUT_URI).
                certSubjectDN(CERT_SUBJECT_DN).
                authnRequestClientAssertionLifetimeMS(AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).build();
    }

    private OIDCClientMetadataDTO getTestOIDCClientMetadataDTO(String clientId) throws CertificateException, IOException {
        return new OIDCClientMetadataDTO.Builder().
                withRedirectUris(REDIRECT_URIS).
                withTokenEndpointAuthMethod(TOKEN_ENDPOINT_AUTH_METHOD).
                withPostLogoutRedirectUris(POST_LOGOUT_REDIRECT_URIS).
                withLogoutUri(LOGOUT_URI).
                withCertSubjectDN(CERT_SUBJECT_DN).
                withAuthnRequestClientAssertionLifetimeMS(AUTHN_REQUEST_CLIENT_ASSERTION_LIFETIME_MS).
                build();
    }
}
