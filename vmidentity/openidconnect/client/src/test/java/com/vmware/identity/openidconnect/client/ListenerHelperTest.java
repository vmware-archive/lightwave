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

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;

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

        AuthenticationResponse response = ListenerHelper.parseAuthenticationResponse(parameterMap, null);

        Assert.assertEquals(OIDCServerError.getOIDCServerError(error), ((AuthenticationErrorResponse) response).getOIDCServerError());
        Assert.assertEquals(state, ((AuthenticationErrorResponse) response).getState().getValue());
    }

    @Test
    public void testParseAuthenticationCodeResponse() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("code", code);
        parameterMap.put("state", state);

        AuthenticationResponse response = ListenerHelper.parseAuthenticationResponse(parameterMap, null);

        Assert.assertEquals(code, ((AuthenticationCodeResponse) response).getAuthorizationCode().getValue());
        Assert.assertEquals(state, ((AuthenticationCodeResponse) response).getState().getValue());
    }

    @Test
    public void testParseAuthenticationTokensResponse() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("id_token", TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getName(), providerPrivateKey, tokenLifeTime));

        ConnectionConfig connectionConfigMock = createNiceMock(ConnectionConfig.class);
        expect(connectionConfigMock.getProviderPublicKey()).andReturn(providerPublicKey);
        expect(connectionConfigMock.getIssuer()).andReturn(issuer);
        replay(connectionConfigMock);
        ClientConfig clientConfig = new ClientConfig(
                connectionConfigMock,
                clientID,
                null);
        AuthenticationResponse response = ListenerHelper.parseAuthenticationResponse(parameterMap, clientConfig);
        Assert.assertNotNull(((AuthenticationTokensResponse) response).getOidcTokens().getIdToken());
        Assert.assertNull(((AuthenticationTokensResponse) response).getOidcTokens().getAccessToken());
        Assert.assertNull(((AuthenticationTokensResponse) response).getOidcTokens().getRefreshToken());
    }

    @Test(expected=OIDCClientException.class)
    public void testParseAuthenticationTokensResponseException() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        parameterMap.put("id_token", TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getName(), providerPrivateKey, tokenLifeTime));

        ListenerHelper.parseAuthenticationResponse(parameterMap, null);
    }

    @Test(expected=OIDCClientException.class)
    public void testParseAuthenticationResponseException() throws Exception {

        Map<String, String> parameterMap = new HashMap<String, String>();
        ListenerHelper.parseAuthenticationResponse(parameterMap, null);
    }
}
