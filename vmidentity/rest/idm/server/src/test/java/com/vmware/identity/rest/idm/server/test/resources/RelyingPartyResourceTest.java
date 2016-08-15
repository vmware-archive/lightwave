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
import static org.junit.Assert.assertNotNull;

import java.io.File;
import java.io.IOException;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Locale;
import java.util.Scanner;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.Test;
import org.w3c.dom.Document;

import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeConsumerService;
import com.vmware.identity.idm.NoSuchRelyingPartyException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.SignatureAlgorithm;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.idm.data.AssertionConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.AttributeConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.server.mapper.RelyingPartyMapper;
import com.vmware.identity.rest.idm.server.resources.RelyingPartyResource;
import com.vmware.identity.rest.idm.server.test.util.CertificateUtil;

/**
 *
 * Unit tests for RelyingParty Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class RelyingPartyResourceTest {

    private static final String TENANT = "test.local";
    private static final String RELYING_PARTY_NAME = "testRelyingParty";
    private static final String RELYING_PARTY_URI = "https://dummy:8080";
    private static final String TEST_DATA_RELYING_PARTY_PATH = "src/test/resources/data/relying_party.xml";

    // Assertion consumer service related test constants
    private static final String ASSERTION_CS_NAME = "testAssertionConsumerService";
    private static final String ASSERTION_CS_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";
    private static final String ASSERTION_CS_ENDPOINT = "https://test:8080";

    // Attribute consumer service related test constants
    private static final String ATTRIBUTE_CS_NAME = "testAttributeConsumerService";

    // Attributes related test constants
    private static final String ATTRIBUTE_NAME_GROUP = "Group";
    private static final String ATTRIBUTE_NAME_FN = "FirstName";
    private static final String ATTRIBUTE_NAME_LN = "LastName";
    private static final String ATTRIBUTE_NAME_FORMAT = "urn:oasis:names:tc:SAML:20.blah";

    // Service end point related test constants
    private static final String SP_ENDPOINT = "https://dummy:8080";
    private static final String SP_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";

    private RelyingPartyResource rpResource;
    private IMocksControl mControl;
    private SecurityContext mockSecurityContext;
    private ContainerRequestContext mockRequest;
    private CasIdmClient mockIDMClient;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();
        mockRequest = EasyMock.createMock(ContainerRequestContext.class);
        expect(mockRequest.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        expect(mockRequest.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        replay(mockRequest);

        mockSecurityContext = mControl.createMock(SecurityContext.class);
        mockIDMClient = mControl.createMock(CasIdmClient.class);
        rpResource = new RelyingPartyResource(TENANT, mockRequest, mockSecurityContext);
        rpResource.setIDMClient(mockIDMClient);

    }

    @Test
    public void testGetRelyingParty_OnName() throws Exception {
        expect(mockIDMClient.getRelyingParty(TENANT, RELYING_PARTY_NAME)).andReturn(getTestRelyingParty());
        mControl.replay();

        RelyingPartyDTO resultRP = rpResource.get(RELYING_PARTY_NAME);
        assertEquals(1, resultRP.getAssertionConsumerServices().size());
        assertAssertionConsumerService(resultRP.getAssertionConsumerServices().iterator().next());
        assertEquals(1, resultRP.getAttributeConsumerServices().size());
        assertAttributeConsumerService(resultRP.getAttributeConsumerServices().iterator().next());
        assertEquals(ASSERTION_CS_NAME, resultRP.getDefaultAssertionConsumerService());
        assertEquals(ATTRIBUTE_CS_NAME, resultRP.getDefaultAttributeConsumerService());
        assertNotNull(resultRP.getCertificate());
        assertEquals(1, resultRP.getSingleLogoutServices().size());
        assertLogoutServiceEndPoint(resultRP.getSingleLogoutServices().iterator().next());
        assertEquals(RELYING_PARTY_URI, resultRP.getUrl());
    }

    @Test
    public void testGetAllRelyingParties() throws Exception {
        expect(mockIDMClient.getRelyingParties(TENANT)).andReturn(Arrays.asList(getTestRelyingParty()));
        mControl.replay();
        Collection<RelyingPartyDTO> relyingParties = rpResource.getAll();
        assertEquals(1, relyingParties.size());
    }

    @Test(expected=NotFoundException.class)
    public void testGetRelyingParty_OnNoSuchTenant() throws Exception {
        expect(mockIDMClient.getRelyingParty(TENANT, RELYING_PARTY_NAME)).andThrow(new NoSuchTenantException("unit test thrown error"));
        mControl.replay();
        rpResource.get(RELYING_PARTY_NAME);
    }

    @Test
    public void testDeleteRelyingParty() throws Exception {
        mockIDMClient.deleteRelyingParty(TENANT, RELYING_PARTY_NAME);
        mControl.replay();

        rpResource.delete(RELYING_PARTY_NAME);
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteRelyingParty_OnNoSuchTenant() throws Exception {
        mockIDMClient.deleteRelyingParty(TENANT, RELYING_PARTY_NAME);
        expectLastCall().andThrow(new NoSuchTenantException("Error thrown from unit testing"));
        mControl.replay();

        rpResource.delete(RELYING_PARTY_NAME);
    }

    @Test
    public void testAddRelyingParty() throws Exception{

        mockIDMClient.addRelyingParty(eq(TENANT),isA(RelyingParty.class));
        expect(mockIDMClient.getRelyingParty(TENANT, RELYING_PARTY_NAME)).andReturn(getTestRelyingParty());
        mControl.replay();

        RelyingPartyDTO rpToAdd = RelyingPartyMapper.getRelyingPartyDTO(getTestRelyingParty());
        RelyingPartyDTO resultRP = rpResource.add(rpToAdd);

        assertRelyingParty(resultRP);

        mControl.verify();
    }

    @Test
    public void testAddRelyingPartyViaXML() throws Exception {

        String relyingPartyXML = new Scanner(new File(TEST_DATA_RELYING_PARTY_PATH)).useDelimiter("\\Z").next();
        mockIDMClient.importTenantConfiguration(eq(TENANT), isA(Document.class));
        mControl.replay();
        rpResource.add(relyingPartyXML);
        mControl.verify();
    }

    @Test
    public void testUpdateRelyingParty() throws Exception {
        mockIDMClient.setRelyingParty(eq(TENANT), isA(RelyingParty.class));
        expect(mockIDMClient.getRelyingParty(TENANT, RELYING_PARTY_NAME)).andReturn(getTestRelyingParty());
        mControl.replay();
        RelyingPartyDTO updatedResults = rpResource.update(RelyingPartyMapper.getRelyingPartyDTO(getTestRelyingParty()));
        assertRelyingParty(updatedResults);
        mControl.verify();
    }

    @Test(expected=NotFoundException.class)
    public void testUpdateRelyingParty_OnNoSuchRp() throws Exception {
        mockIDMClient.setRelyingParty(eq(TENANT), isA(RelyingParty.class));
        expectLastCall().andThrow(new NoSuchRelyingPartyException("Error thrown from unit testing"));
        mControl.replay();
        rpResource.update(RelyingPartyMapper.getRelyingPartyDTO(getTestRelyingParty()));
        mControl.verify();
    }


    @Test(expected=NotFoundException.class)
    public void testUpdateRelyingParty_OnNoSuchTenant() throws Exception {
        mockIDMClient.setRelyingParty(eq(TENANT), isA(RelyingParty.class));
        expectLastCall().andThrow(new NoSuchTenantException("Error thrown from unit testing"));
        mControl.replay();
        rpResource.update(RelyingPartyMapper.getRelyingPartyDTO(getTestRelyingParty()));
        mControl.verify();
    }

    private void assertRelyingParty(RelyingPartyDTO rp) {
        assertEquals(1, rp.getAssertionConsumerServices().size());
        assertAssertionConsumerService(rp.getAssertionConsumerServices().iterator().next());
        assertEquals(1, rp.getAttributeConsumerServices().size());
        assertAttributeConsumerService(rp.getAttributeConsumerServices().iterator().next());
        assertEquals(ASSERTION_CS_NAME, rp.getDefaultAssertionConsumerService());
        assertEquals(ATTRIBUTE_CS_NAME, rp.getDefaultAttributeConsumerService());
        assertNotNull(rp.getCertificate());
        assertEquals(1, rp.getSingleLogoutServices().size());
        assertLogoutServiceEndPoint(rp.getSingleLogoutServices().iterator().next());
        assertEquals(RELYING_PARTY_URI, rp.getUrl());

    }

    private void assertAttributeConsumerService(AttributeConsumerServiceDTO attributeCS) {
        assertEquals(ATTRIBUTE_CS_NAME, attributeCS.getName());
        assertEquals(0, (int) attributeCS.getIndex());
        assertEquals(3, attributeCS.getAttributes().size());
    }

    private void assertAssertionConsumerService(AssertionConsumerServiceDTO assertionCS) {
        assertEquals(ASSERTION_CS_NAME, assertionCS.getName());
        assertEquals(ASSERTION_CS_BINDING, assertionCS.getBinding());
        assertEquals(ASSERTION_CS_ENDPOINT, assertionCS.getEndpoint());
        assertEquals(0, (int) assertionCS.getIndex());
    }

    private void assertLogoutServiceEndPoint(ServiceEndpointDTO logoutServiceEndpoint){
         assertEquals(SP_BINDING, logoutServiceEndpoint.getBinding());
         assertEquals(SP_ENDPOINT, logoutServiceEndpoint.getEndpoint());
    }

    private RelyingParty getTestRelyingParty() throws CertificateException, IOException {
        RelyingParty rp = new RelyingParty(RELYING_PARTY_NAME);
        rp.setAssertionConsumerServices(Arrays.asList(getTestAssertionConsumerService()));
        rp.setAttributeConsumerServices(Arrays.asList(getTestAttributeConsumerService()));
        rp.setAuthnRequestsSigned(false);
        rp.setCertificate(CertificateUtil.getTestCertificate());
        rp.setDefaultAssertionConsumerService(ASSERTION_CS_NAME);
        rp.setDefaultAttributeConsumerService(ATTRIBUTE_CS_NAME);
        rp.setSignatureAlgorithms(Arrays.asList(getTestSignatureAlgorithm()));
        rp.setSingleLogoutServices(Arrays.asList(getTestServiceEndPoint()));
        rp.setUrl(RELYING_PARTY_URI);
        return rp;
    }

    private AssertionConsumerService getTestAssertionConsumerService() {
        AssertionConsumerService acs = new AssertionConsumerService(ASSERTION_CS_NAME, ASSERTION_CS_BINDING, ASSERTION_CS_ENDPOINT);
        acs.setIndex(0);
        return acs;
    }

    private AttributeConsumerService getTestAttributeConsumerService() {
        AttributeConsumerService acs = new AttributeConsumerService(ATTRIBUTE_CS_NAME);
        acs.setAttributes(getTestAttributes());
        acs.setIndex(0);
        return acs;
    }

    private List<Attribute> getTestAttributes() {
        List<Attribute> attrList = new ArrayList<Attribute>();
        attrList.add(new Attribute(ATTRIBUTE_NAME_GROUP, ATTRIBUTE_NAME_FORMAT, ATTRIBUTE_NAME_GROUP));
        attrList.add(new Attribute(ATTRIBUTE_NAME_FN, ATTRIBUTE_NAME_FORMAT, ATTRIBUTE_NAME_GROUP));
        attrList.add(new Attribute(ATTRIBUTE_NAME_LN, ATTRIBUTE_NAME_FORMAT, ATTRIBUTE_NAME_GROUP));
        return attrList;
    }

    private SignatureAlgorithm getTestSignatureAlgorithm() {
        SignatureAlgorithm signAlgo = new SignatureAlgorithm();
        signAlgo.setMaximumKeySize(1024);
        signAlgo.setMinimumKeySize(256);
        signAlgo.setPriority(1);
        return signAlgo;
    }

    private ServiceEndpoint getTestServiceEndPoint() {
        return new ServiceEndpoint(SP_ENDPOINT, SP_BINDING);
    }
}
