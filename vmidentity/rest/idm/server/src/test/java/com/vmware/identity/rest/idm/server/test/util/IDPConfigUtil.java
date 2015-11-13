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
package com.vmware.identity.rest.idm.server.test.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.vmware.identity.idm.AttributeConfig;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.TokenClaimGroupDTO;
import com.vmware.identity.rest.idm.server.mapper.ExternalIDPMapper;

public class IDPConfigUtil {

    public static IDPConfig createIDPConfig() throws Exception {
        IDPConfig config = new IDPConfig(getEntityID());

        config.setNameIDFormats(createFormats());
        config.setSsoServices(createSSOEndpoints());
        config.setSloServices(createSLOEndpoints());
        config.setSigningCertificateChain(createSigningCertificateChain());
        config.setSubjectFormatMappings(createAttributeConfigs());
        config.setTokenClaimGroupMappings(createTokenClaimGroupMapping());
        config.setJitAttribute(true);
        config.setUpnSuffix(getUpnSuffix());

        return config;
    }

    public static ExternalIDPDTO createExternalIDP() throws Exception {
        return ExternalIDPMapper.getExternalIDPDTO(createIDPConfig());
    }

    private static String getEntityID() {
        return "entity";
    }

    private static List<String> createFormats() {
        return Arrays.asList(new String[] { "urn:oasis:names:tc:SAML:1.1:nameid-format:emailAddress" });
    }

    private static List<ServiceEndpoint> createSSOEndpoints() {
        List<ServiceEndpoint> endpoints = new ArrayList<ServiceEndpoint>();
        endpoints.add(new ServiceEndpoint("login", "http://website.com/place/sso/vmware/idp", "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST"));

        return endpoints;
    }

    private static List<ServiceEndpoint> createSLOEndpoints() {
        List<ServiceEndpoint> endpoints = new ArrayList<ServiceEndpoint>();
        endpoints.add(new ServiceEndpoint("logout", "http://website.com/place/slo/vmware/idp", "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST"));

        return endpoints;
    }

    private static List<X509Certificate> createSigningCertificateChain() throws Exception {
        return Arrays.asList(new X509Certificate[] { CertificateUtil.getTestCertificate() });
    }

    private static AttributeConfig[] createAttributeConfigs() {
        AttributeConfig[] mappings = new AttributeConfig[2];

        mappings[0] = new AttributeConfig("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname", "givenName");
        mappings[1] = new AttributeConfig("http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname", "surname");

        return mappings;
    }

    private static Map<TokenClaimAttribute, List<String>> createTokenClaimGroupMapping() {
        Map<TokenClaimAttribute, List<String>> mapping = new HashMap<TokenClaimAttribute, List<String>>();

        List<String> grouplist = Arrays.asList(new String[] { "Users" });
        mapping.put(new TokenClaimAttribute("http://schemas.xmlsoap.org/claims/UPN", "user@vsphere.local"), grouplist);

        return mapping;
    }

    private static String getUpnSuffix() {
        return "vsphere.local";
    }

    public static void assertDTOEqual(IDPConfig expected, ExternalIDPDTO actual) {
        assertEquals(expected.getEntityID(), actual.getEntityID());
        assertEquals(expected.getNameIDFormats(), actual.getNameIDFormats());
        assertDTOEndpointsEqual(expected.getSsoServices(), actual.getSsoServices());
        assertDTOEndpointsEqual(expected.getSloServices(), actual.getSloServices());
        assertDTOCertificatesEqual(expected.getSigningCertificateChain(), actual.getSigningCertificates().getCertificates());
        assertDTOAttributeConfigsEqual(expected.getSubjectFormatMappings(), actual.getSubjectFormats());
        assertDTOTokenClaimGroupsEqual(expected.getTokenClaimGroupMappings(), actual.getTokenClaimGroups());
        assertEquals(expected.getJitAttribute(), actual.isJitEnabled());
        assertEquals(expected.getUpnSuffix(), actual.getUpnSuffix());
    }

    public static void assertDTOEqual(Collection<IDPConfig> expected, Collection<ExternalIDPDTO> actual) {
        assertEquals(expected.size(), actual.size());

        Iterator<IDPConfig> expectedIter = expected.iterator();
        Iterator<ExternalIDPDTO> actualIter = actual.iterator();

        while (expectedIter.hasNext()) {
            IDPConfig e = expectedIter.next();
            ExternalIDPDTO a = actualIter.next();

            assertDTOEqual(e, a);
        }
    }

    public static void assertIDMEqual(ExternalIDPDTO expected, IDPConfig actual) {
        assertEquals(expected.getEntityID(), actual.getEntityID());
        assertEquals(expected.getNameIDFormats(), actual.getNameIDFormats());
        assertIDMEndpointsEqual(expected.getSsoServices(), actual.getSsoServices());
        assertIDMEndpointsEqual(expected.getSloServices(), actual.getSloServices());
        assertIDMCertificatesEqual(expected.getSigningCertificates().getCertificates(), actual.getSigningCertificateChain());
        assertIDMAttributeConfigsEqual(expected.getSubjectFormats(), actual.getSubjectFormatMappings());
        assertIDMTokenClaimGroupsEqual(expected.getTokenClaimGroups(), actual.getTokenClaimGroupMappings());
        assertEquals(expected.isJitEnabled(), actual.getJitAttribute());
        assertEquals(expected.getUpnSuffix(), actual.getUpnSuffix());
    }

    private static void assertDTOEndpointsEqual(Collection<ServiceEndpoint> expected, Collection<ServiceEndpointDTO> actual) {
        assertEquals(expected.size(), actual.size());

        Iterator<ServiceEndpoint> expectedIter = expected.iterator();
        Iterator<ServiceEndpointDTO> actualIter = actual.iterator();

        while (expectedIter.hasNext()) {
            ServiceEndpoint e = expectedIter.next();
            ServiceEndpointDTO a = actualIter.next();

            assertEquals(e.getName(), a.getName());
            assertEquals(e.getEndpoint(), a.getEndpoint());
            assertEquals(e.getBinding(), a.getBinding());
        }
    }

    private static void assertIDMEndpointsEqual(Collection<ServiceEndpointDTO> expected, Collection<ServiceEndpoint> actual) {
        assertEquals(expected.size(), actual.size());

        Iterator<ServiceEndpointDTO> expectedIter = expected.iterator();
        Iterator<ServiceEndpoint> actualIter = actual.iterator();

        while (expectedIter.hasNext()) {
            ServiceEndpointDTO e = expectedIter.next();
            ServiceEndpoint a = actualIter.next();

            assertEquals(e.getName(), a.getName());
            assertEquals(e.getEndpoint(), a.getEndpoint());
            assertEquals(e.getBinding(), a.getBinding());
        }
    }

    private static void assertDTOCertificatesEqual(List<X509Certificate> expected, List<CertificateDTO> actual) {
        assertEquals(expected.size(), actual.size());

        Iterator<X509Certificate> expectedIter = expected.iterator();
        Iterator<CertificateDTO> actualIter = actual.iterator();

        while (expectedIter.hasNext()) {
            X509Certificate e = expectedIter.next();
            X509Certificate a = actualIter.next().getX509Certificate();

            assertEquals(e.getType(), a.getType());
            assertEquals(e, a);
        }
    }

    private static void assertIDMCertificatesEqual(List<CertificateDTO> expected, List<X509Certificate> actual) {
        assertEquals(expected.size(), actual.size());

        Iterator<CertificateDTO> expectedIter = expected.iterator();
        Iterator<X509Certificate> actualIter = actual.iterator();

        while (expectedIter.hasNext()) {
            X509Certificate e = expectedIter.next().getX509Certificate();
            X509Certificate a = actualIter.next();

            assertEquals(e.getType(), a.getType());
            assertEquals(e, a);
        }
    }

    private static void assertDTOAttributeConfigsEqual(AttributeConfig[] expected, Map<String, String> actual) {
        assertEquals(expected.length, actual.size());

        for (int i = 0; i < expected.length; i++) {
            AttributeConfig e = expected[i];

            assertTrue(actual.containsKey(e.getStoreAttribute()));
            assertEquals(e.getTokenSubjectFormat(), actual.get(e.getStoreAttribute()));
        }
    }

    private static void assertIDMAttributeConfigsEqual(Map<String, String> expected, AttributeConfig[] actual) {
        assertEquals(expected.size(), actual.length);

        for (Map.Entry<String, String> e : expected.entrySet()) {
            AttributeConfig a = findAttributeConfig(actual, e);

            assertNotNull(a);
        }
    }

    private static AttributeConfig findAttributeConfig(AttributeConfig[] list, Map.Entry<String, String> config) {

        for (int i = 0; i < list.length; i++) {
            AttributeConfig e = list[i];
            if (e.getStoreAttribute().equals(config.getKey()) && e.getTokenSubjectFormat().equals(config.getValue())) {
                return e;
            }
        }

        return null;
    }

    private static void assertDTOTokenClaimGroupsEqual(Map<TokenClaimAttribute, List<String>> expected, List<TokenClaimGroupDTO> actual) {
        assertEquals(expected.size(), actual.size());

        for (Map.Entry<TokenClaimAttribute, List<String>> e : expected.entrySet()) {
            TokenClaimGroupDTO a = findTokenClaimGroup(actual, e.getKey());

            assertNotNull(a);
            assertEquals(e.getValue(), a.getGroups());
        }
    }

    private static TokenClaimGroupDTO findTokenClaimGroup(List<TokenClaimGroupDTO> list, TokenClaimAttribute attribute) {
        for (TokenClaimGroupDTO group : list) {
            if (group.getClaimName().equals(attribute.getClaimName()) && group.getClaimValue().equals(attribute.getClaimValue())) {
                return group;
            }
        }

        return null;
    }

    private static void assertIDMTokenClaimGroupsEqual(List<TokenClaimGroupDTO> expected, Map<TokenClaimAttribute, List<String>> actual) {
        assertEquals(expected.size(), actual.size());

        for (TokenClaimGroupDTO e : expected) {
            TokenClaimAttribute key = new TokenClaimAttribute(e.getClaimName(), e.getClaimValue());

            assertTrue(actual.containsKey(key));
            assertEquals(e.getGroups(), actual.get(key));
        }
    }


}
