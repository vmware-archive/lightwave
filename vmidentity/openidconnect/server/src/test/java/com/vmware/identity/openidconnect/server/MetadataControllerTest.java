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

package com.vmware.identity.openidconnect.server;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;
import java.util.List;

import javax.servlet.http.HttpServletResponse;

import net.minidev.json.JSONObject;
import net.minidev.json.parser.JSONParser;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.openidconnect.protocol.JSONUtils;
import com.vmware.identity.openidconnect.protocol.ProviderMetadataMapper;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
public class MetadataControllerTest {
    private static MetadataController metadataController;
    private static String tenant;
    private static String nonExistTenant;
    private static String issuer;

    @BeforeClass
    public static void setUp() throws Exception {
        TestContext.initialize();
        metadataController = new MetadataController(TestContext.idmClient());
        tenant = TestContext.TENANT_NAME;
        nonExistTenant = "tenant_not_exist";
        issuer = TestContext.ISSUER;
    }

    @Test
    public void testMetadataSuccess()
            throws IOException, net.minidev.json.parser.ParseException, com.vmware.identity.openidconnect.common.ParseException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");

        MockHttpServletResponse response = new MockHttpServletResponse();
        metadataController.metadata(request, response, tenant);

        validateSuccessResponse(response);
    }

    @Test
    public void testMetadataSuccessDefaultTenant()
            throws IOException, net.minidev.json.parser.ParseException, com.vmware.identity.openidconnect.common.ParseException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");

        MockHttpServletResponse response = new MockHttpServletResponse();
        metadataController.metadata(request, response);

        validateSuccessResponse(response);
    }

    @Test
    public void testMetadataNonExistentTenant()
            throws IOException, net.minidev.json.parser.ParseException, com.vmware.identity.openidconnect.common.ParseException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");
        request.setServerName("abc.com");

        MockHttpServletResponse response = new MockHttpServletResponse();
        metadataController.metadata(request, response, nonExistTenant);

        Assert.assertEquals(HttpServletResponse.SC_BAD_REQUEST, response.getStatus());
        Assert.assertEquals("invalid_request", JSONUtils.parseJSONObject(response.getContentAsString()).get("error"));
        Assert.assertEquals("non-existent tenant", JSONUtils.parseJSONObject(response.getContentAsString()).get("error_description"));
    }

    private static void validateSuccessResponse(MockHttpServletResponse response)
            throws UnsupportedEncodingException, net.minidev.json.parser.ParseException, com.vmware.identity.openidconnect.common.ParseException {
        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(response.getContentAsString());

        ProviderMetadata providerMetadata = ProviderMetadataMapper.parse(jsonObject);

        String actualIssuer = providerMetadata.getIssuer().getValue();
        String actualJwksURI = providerMetadata.getJWKSetURI().toString();
        String actualAuthzEndpoint = providerMetadata.getAuthorizationEndpointURI().toString();
        String actualTokenEndpoint = providerMetadata.getTokenEndpointURI().toString();
        String actualEndSessionEndpoint = providerMetadata.getEndSessionEndpointURI().toString();
        List<String> actualSubjectTypesSupported = providerMetadata.getSubjectTypesSupported();
        List<String> actualResponseTypesSupported = providerMetadata.getResponseTypesSupported();
        List<String> actualIDTokenSigningAlgorithmValuesSupported = providerMetadata.getIDTokenSigningAlgorithmValuesSupported();

        String expectedIssuer = issuer;
        String expectedJwksURI = TestContext.JWKS_ENDPOINT_URI.toString();
        String expectedAuthzEndpoint = TestContext.AUTHZ_ENDPOINT_URI.toString();
        String expectedTokenEndpoint = TestContext.TOKEN_ENDPOINT_URI.toString();
        String expectedEndSessionEndpoint = TestContext.LOGOUT_ENDPOINT_URI.toString();
        List<String> expectedSubjectTypesSupported = Arrays.asList("public");
        List<String> expectedResponseTypesSupported = Arrays.asList("code", "id_token", "token id_token");
        List<String> expectedDTokenSigningAlgorithmValuesSupported = Arrays.asList("RS256");

        Assert.assertEquals(expectedIssuer, actualIssuer);
        Assert.assertEquals(expectedJwksURI, actualJwksURI);
        Assert.assertEquals(expectedAuthzEndpoint, actualAuthzEndpoint);
        Assert.assertEquals(expectedTokenEndpoint, actualTokenEndpoint);
        Assert.assertEquals(expectedEndSessionEndpoint, actualEndSessionEndpoint);
        Assert.assertEquals(expectedSubjectTypesSupported, actualSubjectTypesSupported);
        Assert.assertEquals(expectedResponseTypesSupported, actualResponseTypesSupported);
        Assert.assertEquals(expectedDTokenSigningAlgorithmValuesSupported, actualIDTokenSigningAlgorithmValuesSupported);
    }
}