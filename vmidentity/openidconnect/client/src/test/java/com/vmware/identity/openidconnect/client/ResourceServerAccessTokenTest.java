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

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * Resource Server Access Token Test
 *
 * @author Jun Sun
 */
public class ResourceServerAccessTokenTest {

    private static RSAPublicKey providerPublicKey;
    private static RSAPrivateKey providerPrivateKey;
    private static String resourceServer = "rs_admin_server";
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
    public void testBuildAccessToken() throws Exception {

        ResourceServerAccessToken accessToken = ResourceServerAccessToken.build(
                TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, tokenLifeTime),
                providerPublicKey,
                issuer,
                resourceServer,
                0L);
        Assert.assertTrue(accessToken.getAudience().contains(resourceServer));
        Assert.assertEquals(issuer, accessToken.getIssuer());
    }

    @Test
    public void testBuildAccessTokenInvalidSignature() throws Exception {

        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair providerKeyPair = keyGen.generateKeyPair();
        RSAPublicKey anotherProviderPublicKey = (RSAPublicKey) providerKeyPair.getPublic();

        try {
            ResourceServerAccessToken.build(
                    TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, tokenLifeTime),
                    anotherProviderPublicKey,
                    issuer,
                    resourceServer,
                    0L);
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.INVALID_SIGNATURE, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildAccessTokenWrongAudience() throws Exception {

        String anotherResourceServer = "rs_another_server";

        try {
            ResourceServerAccessToken.build(
                    TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, tokenLifeTime),
                    providerPublicKey,
                    issuer,
                    anotherResourceServer,
                    0L);
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.INVALID_AUDIENCE, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildAccessTokenExpiredToken() throws Exception {

        try {
            ResourceServerAccessToken.build(
                    TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, -tokenLifeTime),
                    providerPublicKey,
                    issuer,
                    resourceServer,
                    0L);
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.PARSE_ERROR, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildAccessTokenInvalidTokenClass() throws Exception {

        try {
            ResourceServerAccessToken.build(
                    TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime),
                    providerPublicKey,
                    issuer,
                    resourceServer,
                    0L);
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.PARSE_ERROR, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildAccessTokenValidWithTolerance() throws Exception {

        ResourceServerAccessToken accessToken =  ResourceServerAccessToken.build(
                TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, tokenLifeTime),
                providerPublicKey,
                issuer,
                resourceServer,
                tokenLifeTime);
        Assert.assertTrue(accessToken.getAudience().contains(resourceServer));
        Assert.assertEquals(issuer, accessToken.getIssuer());
    }
}
