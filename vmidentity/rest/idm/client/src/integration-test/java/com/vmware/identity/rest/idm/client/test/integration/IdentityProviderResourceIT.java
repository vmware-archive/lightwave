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
package com.vmware.identity.rest.idm.client.test.integration;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import javax.xml.soap.SOAPException;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.junit.Test;

import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.AccessToken.Type;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.idm.data.UpdateAttributesMapDTO;

@RunWith(value = Parameterized.class)
public class IdentityProviderResourceIT extends IntegrationTestBase {

    @Parameters
    public static Object[] data() {
           return new Object[] { AccessToken.Type.JWT, AccessToken.Type.SAML };
    }

    public IdentityProviderResourceIT(Type tokenType) throws Exception {
        super(true, tokenType);
    }

    private static Map<String, String> customSamlAttributeMap = new HashMap<>();

    static {
        customSamlAttributeMap.put("https://aws.amazon.com/SAML/Attributes/Role", "const:abc,def");
        customSamlAttributeMap.put("https://aws.amazon.com/SAML/Attributes/RoleSessionName", "objectGUID");
    }

    @BeforeClass
    public static void init() throws HttpException, IOException, GeneralSecurityException, ClientException, SOAPException {
        IntegrationTestBase.init(true);
    }

    @AfterClass
    public static void cleanup() throws ClientException, ClientProtocolException, HttpException, IOException {
        IntegrationTestBase.cleanup(true);
    }

    // TODO Tests pending a good way to handle providers...

    @Test
    public void testGetAttributesMap() throws ClientProtocolException, HttpException, ClientException, IOException {
        Map<String, String> attrMap = testAdminClient.provider().getAttributesMap(testTenant.getName(), testTenant.getName());

        assertFalse(attrMap.isEmpty());
    }

    @Test
    public void testSetAttributesMap() throws ClientProtocolException, HttpException, ClientException, IOException {
        Map<String, String> originalAttrMap = testAdminClient.provider().getAttributesMap(testTenant.getName(), testTenant.getName());
        Map<String, String> newAttrMap = new HashMap<>();

        for (Entry<String, String> e : originalAttrMap.entrySet()) {
            newAttrMap.put(e.getKey(), e.getValue());
        }

        for (Entry<String, String> e : customSamlAttributeMap.entrySet()) {
            newAttrMap.put(e.getKey(), e.getValue());
        }

        Map<String, String> attrMap = testAdminClient.provider().setAttributesMap(testTenant.getName(), testTenant.getName(), newAttrMap);
        assertTrue(attrMap.equals(newAttrMap));

        attrMap = testAdminClient.provider().setAttributesMap(testTenant.getName(), testTenant.getName(), originalAttrMap);
        assertTrue(attrMap.equals(originalAttrMap));
    }

    @Test
    public void testUpdateAttributesMap() throws ClientProtocolException, HttpException, ClientException, IOException {
        UpdateAttributesMapDTO updateAttrsDto = new UpdateAttributesMapDTO.Builder()
                                                                          .withAdd(customSamlAttributeMap)
                                                                          .build();
        Map<String, String> attrMap = testAdminClient.provider().updateAttributesMap(testTenant.getName(), testTenant.getName(), updateAttrsDto);

        for (String attrKey : customSamlAttributeMap.keySet()) {
            assertTrue(attrMap.containsKey(attrKey));
        }

        for (String attrValue : customSamlAttributeMap.values()) {
            assertTrue(attrMap.containsValue(attrValue));
        }

        List<String> remove = new ArrayList<>();
        remove.addAll(customSamlAttributeMap.keySet());
        updateAttrsDto = new UpdateAttributesMapDTO.Builder()
                                                   .withRemove(remove)
                                                   .build();
        attrMap = testAdminClient.provider().updateAttributesMap(testTenant.getName(), testTenant.getName(), updateAttrsDto);

        for (String attrKey : customSamlAttributeMap.keySet()) {
            assertFalse(attrMap.containsKey(attrKey));
        }

        for (String attrValue : customSamlAttributeMap.values()) {
            assertFalse(attrMap.containsValue(attrValue));
        }

        updateAttrsDto = new UpdateAttributesMapDTO.Builder()
                .withAddMapping("https://aws.amazon.com/SAML/Attributes/Role", "const:abc,def")
                .build();

        attrMap = testAdminClient.provider().updateAttributesMap(testTenant.getName(), testTenant.getName(), updateAttrsDto);
        assertTrue(attrMap.containsKey("https://aws.amazon.com/SAML/Attributes/Role"));
        assertTrue(attrMap.containsValue("const:abc,def"));

        updateAttrsDto = new UpdateAttributesMapDTO.Builder()
                .withRemoveAttribute("https://aws.amazon.com/SAML/Attributes/Role")
                .build();
        attrMap = testAdminClient.provider().updateAttributesMap(testTenant.getName(), testTenant.getName(), updateAttrsDto);
        assertFalse(attrMap.containsKey("https://aws.amazon.com/SAML/Attributes/Role"));
    }
}
