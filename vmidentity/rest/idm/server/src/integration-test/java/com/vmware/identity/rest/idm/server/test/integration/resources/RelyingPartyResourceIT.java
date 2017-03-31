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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.AssertionConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.AttributeConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.server.resources.RelyingPartyResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.data.RelyingPartyDataGenerator;

/**
 * Integration test for Relyingparty Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class RelyingPartyResourceIT extends TestBase {

    private static final String RELYING_PARTY_NAME = "relyingParty_CreatedFromIntegrationTest";
    private static final String RELYING_PARTY_URI = "http://dummy:8080";

    private RelyingPartyResource rpResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);
        rpResource = new RelyingPartyResource(DEFAULT_TENANT, request, null);
        rpResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetRelyingParty() throws Exception {
        try {
            // prepare test input
            relyingPartyHelper.addRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME, RELYING_PARTY_URI);

            RelyingPartyDTO result = rpResource.get(RELYING_PARTY_NAME);
            assertRelyingParty(result);
        } finally {
            relyingPartyHelper.deleteRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME);
        }
    }

    @Test
    public void testGetAllRelyingParties() throws Exception {
        try {
            // Add relying party on new tenant and test
            // prepare test input
            relyingPartyHelper.addRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME, RELYING_PARTY_URI);
            Collection<RelyingPartyDTO> result = rpResource.getAll();
            List<String> relyingPartyNames = new ArrayList<String>();
            for(RelyingPartyDTO relyingParty : result) {
                relyingPartyNames.add(relyingParty.getName());
            }
            assertTrue(relyingPartyNames.contains(RELYING_PARTY_NAME));
        } finally {
            relyingPartyHelper.deleteRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME);
        }
    }

    @Test
    public void testDeleteRelyingParty() throws Exception {
        // prepare test input
        relyingPartyHelper.addRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME, RELYING_PARTY_URI);

        rpResource.delete(RELYING_PARTY_NAME);
        RelyingPartyDTO resultRP = rpResource.get(RELYING_PARTY_NAME);
        assertNull(resultRP);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteRelyingParty_OnNonExistentRelyingParty_ThrowsNotFoundEx() {
        rpResource.delete("unknownRelyingParty");
    }

    @Test
    public void testAddRelyingParty() throws Exception {
        try {
            RelyingPartyDTO rpToAdd = RelyingPartyDataGenerator.generateRelyingPartyDTO(RELYING_PARTY_NAME, RELYING_PARTY_URI);
            RelyingPartyDTO response = rpResource.add(rpToAdd);
            assertRelyingParty(response);
        } finally {
            relyingPartyHelper.deleteRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME);
        }
    }

    @Test
    public void testUpdateRelyingParty() throws Exception {
        try {
            // Prepare test input
            relyingPartyHelper.addRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME, RELYING_PARTY_URI);

            String updatedURI = "http://new-dummy:8080";
            RelyingPartyDTO rpToUpdate = RelyingPartyDTO.builder().withName(RELYING_PARTY_NAME).withUrl(updatedURI).build();
            RelyingPartyDTO updatedRP = rpResource.update(rpToUpdate);

            assertEquals(updatedURI, updatedRP.getUrl());
        } finally {
            relyingPartyHelper.deleteRelyingParty(DEFAULT_TENANT, RELYING_PARTY_NAME);
        }
    }

    /**
     * Hard coded asserts, Not a generic one
     */
    private void assertRelyingParty(RelyingPartyDTO relyingParty) {
        assertEquals(RELYING_PARTY_NAME, relyingParty.getName());
        assertEquals(RELYING_PARTY_URI, relyingParty.getUrl());
        assertNotNull(relyingParty.getCertificate());
        assertFalse(relyingParty.isAuthnRequestsSigned());
        assertEquals(RelyingPartyDataGenerator.ASSERTION_CS_NAME, relyingParty.getDefaultAssertionConsumerService());
        assertEquals(RelyingPartyDataGenerator.ATTRIBUTE_CS_NAME, relyingParty.getDefaultAttributeConsumerService());
        assertEquals(1, relyingParty.getAssertionConsumerServices().size());
        assertEquals(1, relyingParty.getAttributeConsumerServices().size());
        assertEquals(1, relyingParty.getSingleLogoutServices().size());
        assertLogoutServiceEndPoint(relyingParty.getSingleLogoutServices().iterator().next());
        assertAttributeConsumerService(relyingParty.getAttributeConsumerServices().iterator().next());
        assertAssertionConsumerService(relyingParty.getAssertionConsumerServices().iterator().next());
    }

    private void assertAttributeConsumerService(AttributeConsumerServiceDTO attributeCS) {
        assertEquals(RelyingPartyDataGenerator.ATTRIBUTE_CS_NAME, attributeCS.getName());
        assertEquals(0, (int) attributeCS.getIndex());
        assertEquals(3, attributeCS.getAttributes().size());
    }

    private void assertAssertionConsumerService(AssertionConsumerServiceDTO assertionCS) {
        assertEquals(RelyingPartyDataGenerator.ASSERTION_CS_NAME, assertionCS.getName());
        assertEquals(RelyingPartyDataGenerator.ASSERTION_CS_BINDING, assertionCS.getBinding());
        assertEquals(RelyingPartyDataGenerator.ASSERTION_CS_ENDPOINT, assertionCS.getEndpoint());
        assertEquals(0, (int) assertionCS.getIndex());
    }

    private void assertLogoutServiceEndPoint(ServiceEndpointDTO logoutServiceEndpoint) {
        assertEquals(RelyingPartyDataGenerator.SP_BINDING, logoutServiceEndpoint.getBinding());
        assertEquals(RelyingPartyDataGenerator.SP_ENDPOINT, logoutServiceEndpoint.getEndpoint());
    }

}
