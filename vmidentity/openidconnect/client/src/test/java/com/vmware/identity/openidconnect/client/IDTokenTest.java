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
 * ID Token Test
 *
 * @author Jun Sun
 */
public class IDTokenTest {

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
    public void testBuildIdToken() throws Exception {

        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime);
        IDToken idToken = IDToken.build(idTokenString, providerPublicKey, issuer, clientID, 0L);
        Assert.assertTrue(idToken.getAudience().contains(clientID.getValue()));
        Assert.assertEquals(issuer.getValue(), idToken.getIssuer().getValue());
    }

    @Test
    public void testBuildIdTokenByBuilder() throws Exception {

        Date now = new Date();
        Date later = new Date(now.getTime() + 10 * 1000L);

        String idTokenString = JWTBuilder.idTokenBuilder(providerPrivateKey).
                tokenType(TokenType.BEARER).
                jwtId(new JWTID()).
                issuer(issuer).
                subject(new Subject("me")).
                audience(Collections.<String>singletonList(clientID.getValue())).
                issueTime(now).
                expirationTime(later).
                scope(Scope.OPENID).
                tenant("test.tenant").
                clientId(clientID).
                sessionId((SessionID) null).
                holderOfKey((RSAPublicKey) null).
                actAs((Subject) null).
                nonce((Nonce) null).
                groups(Collections.<String>singletonList("Administrator")).
                givenName(null).
                familyName(null).build();

        IDToken idToken = IDToken.build(idTokenString, providerPublicKey, issuer, clientID, 0L);
        Assert.assertTrue(idToken.getAudience().contains(clientID.getValue()));
        Assert.assertEquals(issuer.getValue(), idToken.getIssuer().getValue());
    }

    @Test
    public void testBuildIdTokenInvalidSignature() throws Exception {

        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair providerKeyPair = keyGen.generateKeyPair();
        RSAPublicKey anotherProviderPublicKey = (RSAPublicKey) providerKeyPair.getPublic();

        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime);
        try {
            IDToken.build(idTokenString, anotherProviderPublicKey, issuer, clientID, 0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.INVALID_SIGNATURE, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildIdTokenWrongAudience() throws Exception {

        ClientID anotherClientID = new ClientID("another-client");

        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime);
        try {
            IDToken.build(idTokenString, providerPublicKey, issuer, anotherClientID, 0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.INVALID_AUDIENCE, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildIdTokenNotYetValid() throws Exception {

        Date now = new Date();
        Date issueTime = new Date(now.getTime() + 1*60*1000L); // issued in the future
        Date expirationTime = new Date(now.getTime() + 2*60*1000L);
        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, issueTime, expirationTime);
        try {
            IDToken.build(idTokenString, providerPublicKey, issuer, clientID, 0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.TOKEN_NOT_YET_VALID, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildIdTokenExpired() throws Exception {

        Date now = new Date();
        Date issueTime = new Date(now.getTime() - 2*60*1000L);
        Date expirationTime = new Date(now.getTime() - 1*60*1000L); // expired
        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, issueTime, expirationTime);
        try {
            IDToken.build(idTokenString, providerPublicKey, issuer, clientID, 0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.EXPIRED_TOKEN, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildIdTokenInvalidTokenClass() throws Exception {

        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ACCESS_TOKEN.getValue(), providerPrivateKey, tokenLifeTime);
        try {
            IDToken.build(idTokenString, providerPublicKey, issuer, clientID, 0L);
            Assert.fail("expecting TokenValidationException");
        } catch (TokenValidationException e) {
            Assert.assertEquals(TokenValidationError.PARSE_ERROR, e.getTokenValidationError());
        }
    }

    @Test
    public void testBuildIdTokenValidWithTolerance() throws Exception {

        String idTokenString = TestUtils.buildBaseToken(issuer, clientID.getValue(), TokenClass.ID_TOKEN.getValue(), providerPrivateKey, tokenLifeTime);
        IDToken idToken = IDToken.build(idTokenString, providerPublicKey, issuer, clientID, tokenLifeTime);
        Assert.assertTrue(idToken.getAudience().contains(clientID.getValue()));
        Assert.assertEquals(issuer.getValue(), idToken.getIssuer().getValue());
    }
}
