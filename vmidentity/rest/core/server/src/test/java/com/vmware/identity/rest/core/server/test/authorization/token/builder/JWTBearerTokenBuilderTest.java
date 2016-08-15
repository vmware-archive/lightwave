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
import java.util.Arrays;
import java.util.List;

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
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerToken;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerTokenBuilder;
import com.vmware.identity.rest.core.server.test.util.JWTBuilder;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;

public class JWTBearerTokenBuilderTest extends AccessTokenBuilderTest {

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
        List<String> groups = Arrays.asList( new String[] { "vsphere.local\\Administrators" } );

        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .groups(groups)
            .build();

        JWTBearerTokenBuilder builder = new JWTBearerTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM);

        AccessToken token = build(jwt, TokenStyle.HEADER, TokenType.BEARER, builder);

        assertTrue("Token is not a JWTBearerToken", token instanceof JWTBearerToken);
        assertEquals("Roles do not match", Role.ADMINISTRATOR, Role.findByRoleName(token.getRole()));
        assertEquals("Groups do not match", groups, token.getGroupList());
    }

    @Test
    public void testBuilding_BadRole() throws JOSEException, ParseException {
        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role("Junk")
            .build();

        JWTBearerTokenBuilder builder = new JWTBearerTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM);

        AccessToken token = build(jwt, TokenStyle.HEADER, TokenType.BEARER, builder);

        assertTrue("Token is not a JWTBearerToken", token instanceof JWTBearerToken);
        assertNull("Role was not null", Role.findByRoleName(token.getRole()));
    }

    @Test(expected = InvalidTokenException.class)
    public void testBuilding_BadToken() {
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.BEARER, "ABCDEFG", null);
        JWTBearerTokenBuilder builder = new JWTBearerTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM);
        builder.build(info);
    }

}
