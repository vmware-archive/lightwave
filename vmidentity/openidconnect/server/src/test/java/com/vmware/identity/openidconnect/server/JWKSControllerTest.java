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
import java.security.cert.CertificateEncodingException;
import java.util.Arrays;

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
import com.nimbusds.jose.util.Base64;
import com.vmware.identity.openidconnect.protocol.JSONUtils;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
public class JWKSControllerTest {
    private static JWKSController jwksController;
    private static String tenant;
    private static String nonExistTenant;

    @BeforeClass
    public static void setUp() throws Exception {
        TestContext.initialize();
        jwksController = new JWKSController(TestContext.idmClient());
        tenant = TestContext.TENANT_NAME;
        nonExistTenant = "tenant_not_exist";
    }

    @Test
    public void testJwksSuccess() throws IOException, ParseException, CertificateEncodingException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");
        request.setServerName("abc.com");

        MockHttpServletResponse response = new MockHttpServletResponse();
        jwksController.jwks(request, response, tenant);

        validateSuccessResponse(response);
    }

    @Test
    public void testJwksSuccessDefaultTenant() throws IOException, ParseException, CertificateEncodingException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");
        request.setServerName("abc.com");

        MockHttpServletResponse response = new MockHttpServletResponse();
        jwksController.jwks(request, response);

        validateSuccessResponse(response);
    }

    @Test
    public void testJwksNonExistentTenant() throws IOException, ParseException, com.vmware.identity.openidconnect.common.ParseException {
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("GET");
        request.setServerName("abc.com");

        MockHttpServletResponse response = new MockHttpServletResponse();
        jwksController.jwks(request, response, nonExistTenant);

        Assert.assertEquals(HttpServletResponse.SC_BAD_REQUEST, response.getStatus());
        Assert.assertEquals("invalid_request", JSONUtils.parseJSONObject(response.getContentAsString()).get("error"));
        Assert.assertEquals("non-existent tenant", JSONUtils.parseJSONObject(response.getContentAsString()).get("error_description"));
    }

    private static void validateSuccessResponse(MockHttpServletResponse response) throws CertificateEncodingException, UnsupportedEncodingException, ParseException {
        JSONParser jsonParser = new JSONParser(JSONParser.DEFAULT_PERMISSIVE_MODE);
        JSONObject jsonObject = (JSONObject) jsonParser.parse(response.getContentAsString());

        JSONArray jsonArray = (JSONArray) jsonObject.get("keys");
        JSONObject jwk = (JSONObject) jsonArray.get(0);

        String alg = (String) jwk.get("alg");
        String use = (String) jwk.get("use");
        String e = (String) jwk.get("e");
        String n = (String) jwk.get("n");
        String x5c = (String) ((JSONArray) jwk.get("x5c")).get(0);

        RSAKey rsaKey = new RSAKey(
                TestContext.TENANT_PUBLIC_KEY,
                KeyUse.SIGNATURE,
                null,
                JWSAlgorithm.RS256,
                null,
                null,
                null,
                Arrays.asList(Base64.encode(TestContext.TENANT_CERT.getEncoded())));

        Assert.assertEquals("RS256", alg);
        Assert.assertEquals("sig", use);
        Assert.assertEquals(rsaKey.getPublicExponent().toString(), e);
        Assert.assertEquals(rsaKey.getModulus().toString(), n);
        Assert.assertEquals(rsaKey.getX509CertChain().get(0).toString(), x5c);
    }
}