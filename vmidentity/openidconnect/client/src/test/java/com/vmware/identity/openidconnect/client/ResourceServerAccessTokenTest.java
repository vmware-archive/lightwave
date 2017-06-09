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
import java.util.Collections;
import java.util.Date;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.Subject;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;

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
    public void testBuildAccessTokenNoResourceServer() throws Exception {

        ResourceServerAccessToken accessToken = ResourceServerAccessToken.build(
                TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, tokenLifeTime),
                providerPublicKey,
                issuer,
                null, // resourceServer
                0L);
        Assert.assertTrue(accessToken.getAudience().contains(resourceServer));
        Assert.assertEquals(issuer, accessToken.getIssuer());
    }

    @Test
    public void testBuildAccessTokenByBuilder() throws Exception {

        Date now = new Date();
        Date later = new Date(now.getTime() + 10 * 1000L);

        String accessTokenString = JWTBuilder.accessTokenBuilder(providerPrivateKey).
                tokenType(TokenType.BEARER).
                jwtId(new JWTID()).
                issuer(issuer).
                subject(new Subject("me")).
                audience(Collections.<String>singletonList(resourceServer)).
                issueTime(now).
                expirationTime(later).
                scope(Scope.OPENID).
                tenant("test.tenant").
                clientId((ClientID) null).
                sessionId((SessionID) null).
                holderOfKey((RSAPublicKey) null).
                actAs((Subject) null).
                nonce((Nonce) null).
                groups(Collections.<String>singletonList("Administrator")).build();

        ResourceServerAccessToken accessToken = ResourceServerAccessToken.build(
                accessTokenString,
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
            Assert.fail("expecting TokenValidationException");
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
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.INVALID_AUDIENCE, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildAccessTokenNotYetValid() throws Exception {

        Date now = new Date();
        Date issueTime = new Date(now.getTime() + 1*60*1000L); // issued in the future
        Date expirationTime = new Date(now.getTime() + 2*60*1000L);

        try {
            ResourceServerAccessToken.build(
                    TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, issueTime, expirationTime),
                    providerPublicKey,
                    issuer,
                    resourceServer,
                    0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.TOKEN_NOT_YET_VALID, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildAccessTokenExpired() throws Exception {

        Date now = new Date();
        Date issueTime = new Date(now.getTime() - 2*60*1000L);
        Date expirationTime = new Date(now.getTime() - 1*60*1000L); // expired

        try {
            ResourceServerAccessToken.build(
                    TestUtils.buildBaseToken(issuer, resourceServer, TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, issueTime, expirationTime),
                    providerPublicKey,
                    issuer,
                    resourceServer,
                    0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.EXPIRED_TOKEN, e.getTokenValidationError());
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
            Assert.fail("expecting TokenValidationException");
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
                5 * 60L /* clockToleranceInSeconds */);
        Assert.assertTrue(accessToken.getAudience().contains(resourceServer));
        Assert.assertEquals(issuer, accessToken.getIssuer());
    }
}
