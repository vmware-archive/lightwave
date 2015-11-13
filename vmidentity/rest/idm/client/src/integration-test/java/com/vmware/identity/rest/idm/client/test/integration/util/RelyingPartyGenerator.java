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
package com.vmware.identity.rest.idm.client.test.integration.util;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.AssertionConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.AttributeConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.AttributeDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.SignatureAlgorithmDTO;

public class RelyingPartyGenerator {

    private static final String RELYING_PARTY_NAME = "relyingParty_CreatedFromIntegrationTest";
    private static final String RELYING_PARTY_URI = "http://dummy:8080";

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
    private static final String SP_NAME = "logout";
    private static final String SP_ENDPOINT = "https://dummy:8080";
    private static final String SP_BINDING = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";

    public static RelyingPartyDTO generateRelyingParty(CertificateDTO certificate) {
        return new RelyingPartyDTO.Builder()
            .withName(RELYING_PARTY_NAME)
            .withUrl(RELYING_PARTY_URI)
            .withAssertionConsumerServices(generateAssertionConsumerServices())
            .withAttributeConsumerServices(generateAttributeConsumerServices())
            .withAuthnRequestsSigned(false)
            .withCertificate(certificate)
            .withDefaultAssertionConsumerService(ASSERTION_CS_NAME)
            .withDefaultAttributeConsumerService(ATTRIBUTE_CS_NAME)
            .withSignatureAlgorithms(generateSignatureAlgorithms())
            .withSingleLogoutServices(generateSingleLogoutServices())
            .build();
    }

    private static List<AssertionConsumerServiceDTO> generateAssertionConsumerServices() {
        AssertionConsumerServiceDTO acs = new AssertionConsumerServiceDTO.Builder()
            .withName(ASSERTION_CS_NAME)
            .withBinding(ASSERTION_CS_BINDING)
            .withEndpoint(ASSERTION_CS_ENDPOINT)
            .withIndex(0)
            .build();

        return Arrays.asList(acs);
    }

    private static List<AttributeConsumerServiceDTO> generateAttributeConsumerServices() {
        AttributeConsumerServiceDTO acs = new AttributeConsumerServiceDTO.Builder()
            .withName(ATTRIBUTE_CS_NAME)
            .withAttributes(generateAttributes())
            .withIndex(0)
            .build();

        return Arrays.asList(acs);
    }

    private static List<AttributeDTO> generateAttributes() {
        List<AttributeDTO> attributes = new ArrayList<AttributeDTO>();

        attributes.add(generateAttribute(ATTRIBUTE_NAME_GROUP));
        attributes.add(generateAttribute(ATTRIBUTE_NAME_FN));
        attributes.add(generateAttribute(ATTRIBUTE_NAME_LN));

        return attributes;
    }

    private static AttributeDTO generateAttribute(String name) {
        return new AttributeDTO.Builder()
            .withName(name)
            .withNameFormat(ATTRIBUTE_NAME_FORMAT)
            .withFriendlyName(name)
            .build();
    }

    private static List<SignatureAlgorithmDTO> generateSignatureAlgorithms() {
        return Arrays.asList(new SignatureAlgorithmDTO.Builder()
            .withMaxKeySize(1024)
            .withMinKeySize(256)
            .withPriority(1)
            .build());
    }

    private static List<ServiceEndpointDTO> generateSingleLogoutServices() {
        return Arrays.asList(new ServiceEndpointDTO.Builder()
            .withName(SP_NAME)
            .withEndpoint(SP_ENDPOINT)
            .withBinding(SP_BINDING)
            .build());
    }

}
