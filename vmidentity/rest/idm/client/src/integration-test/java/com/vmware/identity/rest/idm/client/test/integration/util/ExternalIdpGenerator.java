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

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;

public class ExternalIdpGenerator {

    private static final String TEST_IDP_NAME = "external.test.integration";

    public static ExternalIDPDTO generateExternalIDP(CertificateDTO certificate) throws GeneralSecurityException, IOException {
        return new ExternalIDPDTO.Builder()
        .withEntityID(TEST_IDP_NAME)
        .withJitEnabled(true)
        .withUpnSuffix(TEST_IDP_NAME)
        .withSigningCertificates(getSigningCertificateChain(certificate))
        .withSsoServices(getSsoServices(TEST_IDP_NAME))
        .withSloServices(getSloServices(TEST_IDP_NAME))
        .withSubjectFormats(getSubjectFormats())
        .withNameIDFormats(getNameIDFormats())
        .withTokenClaimGroups(getTokenClaimGroups(TEST_IDP_NAME))
        .build();
    }

    private static CertificateChainDTO getSigningCertificateChain(CertificateDTO certificate) throws GeneralSecurityException, IOException {
        return new CertificateChainDTO.Builder()
            .withCertificates(Arrays.asList(new CertificateDTO[] { certificate }))
            .build();
    }

    private static List<ServiceEndpointDTO> getSsoServices(String name) {
        List<ServiceEndpointDTO> endpoints = new ArrayList<ServiceEndpointDTO>();
        endpoints.add(new ServiceEndpointDTO("login", "http://" + name + "/place/sso/vmware/idp", "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST"));
        return endpoints;
    }

    private static List<ServiceEndpointDTO> getSloServices(String name) {
        List<ServiceEndpointDTO> endpoints = new ArrayList<ServiceEndpointDTO>();
        endpoints.add(new ServiceEndpointDTO("logout", "http://" + name + "/place/slo/vmware/idp", "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST"));
        return endpoints;
    }

    private static Map<String, String> getSubjectFormats() {
        Map<String, String> map = new HashMap<String, String>();
        map.put("givenName", "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname");
        map.put("surname", "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname");
        return map;
    }

    private static List<String> getNameIDFormats() {
        List<String> formats = new ArrayList<String>();
        formats.add("urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress");
        return formats;
    }

    private static List<TokenClaimGroupDTO> getTokenClaimGroups(String name) {
        List<TokenClaimGroupDTO> claimGroups = new ArrayList<TokenClaimGroupDTO>();

        TokenClaimGroupDTO group = new TokenClaimGroupDTO.Builder()
            .withClaimName("http://schemas.xmlsoap.org/claims/UPN")
            .withClaimValue("user@" + name)
            .withGroups(Arrays.asList(new String[] { "Users" }))
            .build();

        claimGroups.add(group);
        return claimGroups;
    }

}
