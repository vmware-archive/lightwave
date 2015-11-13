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
package com.vmware.identity.rest.idm.server.test.integration.util.data;

import java.io.IOException;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeConsumerService;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.SignatureAlgorithm;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.server.mapper.RelyingPartyMapper;
import com.vmware.identity.rest.idm.server.test.util.CertificateUtil;

/**
 * Data provider for relying party resource integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class RelyingPartyDataGenerator {

    // Assertion consumer service related test constants
    public static final String ASSERTION_CS_NAME = "testAssertionConsumerService";
    public static final String ASSERTION_CS_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";
    public static final String ASSERTION_CS_ENDPOINT = "https://test:8080";

    // Attribute consumer service related test constants
    public static final String ATTRIBUTE_CS_NAME = "testAttributeConsumerService";

    // Attributes related test constants
    public static final String ATTRIBUTE_NAME_GROUP = "Group";
    public static final String ATTRIBUTE_NAME_FN = "FirstName";
    public static final String ATTRIBUTE_NAME_LN = "LastName";
    public static final String ATTRIBUTE_NAME_FORMAT = "urn:oasis:names:tc:SAML:20.blah";

    // Service end point related test constants
    public static final String SP_ENDPOINT = "https://dummy:8080";
    public static final String SP_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";

    public static RelyingParty generateRelyingParty(String relyingPartyName, String relyingPartyURI) throws CertificateException, IOException {
        RelyingParty rp = new RelyingParty(relyingPartyName);
        rp.setUrl(relyingPartyURI);
        rp.setAssertionConsumerServices(Arrays.asList(getTestAssertionConsumerService()));
        rp.setAttributeConsumerServices(Arrays.asList(getTestAttributeConsumerService()));
        rp.setAuthnRequestsSigned(false);
        rp.setCertificate(CertificateUtil.getTestCertificate());
        rp.setDefaultAssertionConsumerService(ASSERTION_CS_NAME);
        rp.setDefaultAttributeConsumerService(ATTRIBUTE_CS_NAME);
        rp.setSignatureAlgorithms(Arrays.asList(getTestSignatureAlgorithm()));
        rp.setSingleLogoutServices(Arrays.asList(getTestServiceEndPoint()));
        return rp;
    }

    public static RelyingPartyDTO generateRelyingPartyDTO(String relyingPartyName, String relyingPartyURI) throws CertificateException, IOException {
        return RelyingPartyMapper.getRelyingPartyDTO(generateRelyingParty(relyingPartyName, relyingPartyURI));
    }

    private static AssertionConsumerService getTestAssertionConsumerService() {
        AssertionConsumerService acs = new AssertionConsumerService(ASSERTION_CS_NAME, ASSERTION_CS_BINDING, ASSERTION_CS_ENDPOINT);
        acs.setIndex(0);
        return acs;
    }

    private static AttributeConsumerService getTestAttributeConsumerService() {
        AttributeConsumerService acs = new AttributeConsumerService(ATTRIBUTE_CS_NAME);
        acs.setAttributes(getTestAttributes());
        acs.setIndex(0);
        return acs;
    }

    private static List<Attribute> getTestAttributes() {
        List<Attribute> attrList = new ArrayList<Attribute>();
        attrList.add(new Attribute(ATTRIBUTE_NAME_GROUP, ATTRIBUTE_NAME_FORMAT, ATTRIBUTE_NAME_GROUP));
        attrList.add(new Attribute(ATTRIBUTE_NAME_FN, ATTRIBUTE_NAME_FORMAT, ATTRIBUTE_NAME_GROUP));
        attrList.add(new Attribute(ATTRIBUTE_NAME_LN, ATTRIBUTE_NAME_FORMAT, ATTRIBUTE_NAME_GROUP));
        return attrList;
    }

    private static SignatureAlgorithm getTestSignatureAlgorithm() {
        SignatureAlgorithm signAlgo = new SignatureAlgorithm();
        signAlgo.setMaximumKeySize(1024);
        signAlgo.setMinimumKeySize(256);
        signAlgo.setPriority(1);
        return signAlgo;
    }

    private static ServiceEndpoint getTestServiceEndPoint() {
        return new ServiceEndpoint(SP_ENDPOINT, SP_BINDING);
    }
}
