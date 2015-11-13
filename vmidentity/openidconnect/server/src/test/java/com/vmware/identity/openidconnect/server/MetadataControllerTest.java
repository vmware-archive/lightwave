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

import javax.servlet.http.HttpServletResponse;

import net.minidev.json.JSONArray;
import net.minidev.json.JSONObject;
import net.minidev.json.parser.JSONParser;
import net.minidev.json.parser.ParseException;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.oauth2.sdk.util.JSONObjectUtils;

/**
 * @author Jun Sun
 */
public class MetadataControllerTest {

    private static MetadataController metadataController;
    private static String tenant;
    private static String issuer;
    private static String nonExistTenant;

    @BeforeClass
    public static void setUp() throws Exception {
        TestContext.initialize();
        metadataController = new MetadataController(TestContext.idmClient());
        tenant = TestContext.TENANT_NAME;
        issuer = TestContext.ISSUER;
        nonExistTenant = "tenant_not_exist";
    }

    @Test
    public void testMetadataSuccess() throws IOException, ParseException {

        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");

        MockHttpServletResponse response = new MockHttpServletResponse();
        metadataController.metadata(request, response, tenant);

        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(response.getContentAsString());

        String actualIssuer = (String) jsonObject.get("issuer");
        String actualJwksURI = (String) jsonObject.get("jwks_uri");
        String actualAuthzEndpoint = (String) jsonObject.get("authorization_endpoint");
        String actualTokenEndpoint = (String) jsonObject.get("token_endpoint");
        String actualEndSessionEndpoint = (String) jsonObject.get("end_session_endpoint");

        String expectedIssuer = issuer;
        String expectedJwksURI = Shared.replaceLast(issuer, tenant, "jwks/" + tenant);
        String expectedAuthzEndpoint = Shared.replaceLast(issuer, tenant, "oidc/authorize/" + tenant);
        String expectedTokenEndpoint = Shared.replaceLast(issuer, tenant, "token/" + tenant);
        String expectedEndSessionEndpoint = Shared.replaceLast(issuer, tenant, "logout/" + tenant);

        Assert.assertEquals(expectedIssuer, actualIssuer);
        Assert.assertEquals(expectedJwksURI, actualJwksURI);
        Assert.assertEquals(expectedAuthzEndpoint, actualAuthzEndpoint);
        Assert.assertEquals(expectedTokenEndpoint, actualTokenEndpoint);
        Assert.assertEquals(expectedEndSessionEndpoint, actualEndSessionEndpoint);

        // TODO: validate other fields
    }

    @Test
    public void testJwksSuccess() throws IOException, ParseException {

        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");
        request.setServerName("abc.com");

        MockHttpServletResponse response = new MockHttpServletResponse();
        metadataController.jwks(request, response, tenant);

        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(response.getContentAsString());

        JSONArray jsonArray = (JSONArray) jsonObject.get("keys");
        JSONObject jwk = (JSONObject) jsonArray.get(0);

        String alg = (String) jwk.get("alg");
        String use = (String) jwk.get("use");
        String e = (String) jwk.get("e");
        String n = (String) jwk.get("n");

        RSAKey rsaKey = new RSAKey(
                TestContext.TENANT_PUBLIC_KEY,
                KeyUse.SIGNATURE,
                null,
                JWSAlgorithm.RS256,
                null,
                null,
                null,
                null);

        Assert.assertEquals("RS256", alg);
        Assert.assertEquals("sig", use);
        Assert.assertEquals(rsaKey.getPublicExponent().toString(), e);
        Assert.assertEquals(rsaKey.getModulus().toString(), n);
    }

    @Test
    public void testJwksNoSuchTenantException() throws IOException, ParseException, com.nimbusds.oauth2.sdk.ParseException {

        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");
        request.setServerName("abc.com");

        MockHttpServletResponse response = new MockHttpServletResponse();
        metadataController.jwks(request, response, nonExistTenant);

        Assert.assertEquals(HttpServletResponse.SC_BAD_REQUEST, response.getStatus());
        Assert.assertEquals("invalid_request", JSONObjectUtils.parseJSONObject(response.getContentAsString()).get("error"));
        Assert.assertEquals("non-existent tenant", JSONObjectUtils.parseJSONObject(response.getContentAsString()).get("error_description"));
    }
}
