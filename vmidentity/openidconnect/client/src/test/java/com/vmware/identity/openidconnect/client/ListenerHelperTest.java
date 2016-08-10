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

package com.vmware.identity.openidconnect.client;

import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * ListenerHelper Test
 *
 * @author Jun Sun
 */
public class ListenerHelperTest {

    private static String error = "invalid_request";
    private static String error_description = "invalid request";
    private static String code = "abc";
    private static String state = "123";

    private static RSAPublicKey providerPublicKey;
    private static RSAPrivateKey providerPrivateKey;
    private static ClientID clientID = new ClientID("test-client");
    private static Issuer issuer = new Issuer("https://abc.com/openidconnect");
    private static Long tokenLifeTime = 2 * 60 * 1000L;

    @BeforeClass
    public static void setUp() throws Exception {

        // create key pair and client private key, certificate
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair providerKeyPair = keyGen.generateKeyPair();
        providerPrivateKey = (RSAPrivateKey) providerKeyPair.getPrivate();
        providerPublicKey = (RSAPublicKey) providerKeyPair.getPublic();
    }

    @Test
    public void testParseAuthenticationErrorResponse() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("error", error);
        parameterMap.put("error_description", error_description);
        parameterMap.put("state", state);

        OIDCServerException exception;
        try {
            ListenerHelper.parseAuthenticationCodeResponse(parameterMap);
            exception = null;
            Assert.fail("expecting OIDCServerException to be thrown");
        } catch (OIDCServerException e) {
            exception = e;
        }

        Assert.assertEquals(error, exception.getErrorObject().getErrorCode().getValue());
        Assert.assertEquals(error_description, exception.getErrorObject().getDescription());
        Assert.assertEquals(state, exception.getState().getValue());
    }

    @Test
    public void testParseAuthenticationCodeResponse() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("code", code);
        parameterMap.put("state", state);

        AuthenticationCodeResponse response = ListenerHelper.parseAuthenticationCodeResponse(parameterMap);

        Assert.assertEquals(code, response.getAuthorizationCode().getValue());
        Assert.assertEquals(state, response.getState().getValue());
    }

    @Test
    public void testParseAuthenticationTokensResponse() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("id_token", TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime));
        parameterMap.put("state", state);

        AuthenticationTokensResponse response = ListenerHelper.parseAuthenticationTokensResponse(parameterMap, providerPublicKey, issuer, clientID, 0L);
        Assert.assertNotNull(response.getTokens().getIDToken());
        Assert.assertNull(response.getTokens().getAccessToken());
        Assert.assertEquals(state, response.getState().getValue());
    }

    @Test(expected=OIDCClientException.class)
    public void testParseAuthenticationTokensResponseException() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("id_token", TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime));

        ListenerHelper.parseAuthenticationCodeResponse(parameterMap);
    }

    @Test(expected=OIDCClientException.class)
    public void testParseAuthenticationResponseException() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        ListenerHelper.parseAuthenticationCodeResponse(parameterMap);
    }
}
