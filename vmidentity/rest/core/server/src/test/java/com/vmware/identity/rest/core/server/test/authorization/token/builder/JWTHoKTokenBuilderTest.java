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
package com.vmware.identity.rest.core.server.test.authorization.token.builder;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.text.ParseException;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.jwt.hok.JWTHoKToken;
import com.vmware.identity.rest.core.server.authorization.token.jwt.hok.JWTHoKTokenBuilder;
import com.vmware.identity.rest.core.server.test.util.JWTBuilder;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;

public class JWTHoKTokenBuilderTest extends AccessTokenBuilderTest {

    protected static PublicKey publicKey;
    protected static PrivateKey privateKey;

    @BeforeClass
    public static void setup() {
        KeyPair keypair = KeyPairUtil.getKeyPair();
        publicKey = keypair.getPublic();
        privateKey = keypair.getPrivate();
    }

    @Test
    public void testBuilding() throws JOSEException, ParseException {
        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);

        AccessToken token = build(jwt, TokenStyle.HEADER, TokenType.HOK, builder);

        assertTrue("Token is not a JWTHoKToken", token instanceof JWTHoKToken);
        assertEquals("Roles do not match", Role.ADMINISTRATOR, Role.findByRoleName(token.getRole()));

        JWTHoKToken hokToken = (JWTHoKToken) token;
        assertEquals("Public Key does not match", publicKey, hokToken.getPublicKey());
    }

    @Test
    public void testBuilding_BadRole() throws JOSEException, ParseException {
        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role("Junk")
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);

        AccessToken token = build(jwt, TokenStyle.HEADER, TokenType.HOK, builder);

        assertTrue("Token is not a JWTHoKToken", token instanceof JWTHoKToken);
        assertNull("Role was not null", Role.findByRoleName(token.getRole()));

        JWTHoKToken hokToken = (JWTHoKToken) token;
        assertEquals("Public Key does not match", publicKey, hokToken.getPublicKey());
    }

    @Test(expected = InvalidTokenException.class)
    public void testBuilding_NoHotK() throws JOSEException, ParseException {
        SignedJWT jwt = new JWTBuilder(privateKey)
        .subject("administrator")
        .issuer("OAUTH")
        .audience(RESOURCE_SERVER_AUDIENCE)
        .role(Role.ADMINISTRATOR.getRoleName())
        .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);

        build(jwt, TokenStyle.HEADER, TokenType.HOK, builder);
    }

    @Test(expected = InvalidTokenException.class)
    public void testBuilding_BadToken() {
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, "ABCDEFG");
        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        builder.build(info);
    }

}
