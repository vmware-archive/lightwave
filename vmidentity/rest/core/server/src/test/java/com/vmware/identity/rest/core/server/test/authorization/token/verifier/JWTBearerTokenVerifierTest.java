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
package com.vmware.identity.rest.core.server.test.authorization.token.verifier;

import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.util.Date;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.MediaType;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.token.jwt.hok.JWTHoKTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.verifier.jwt.bearer.JWTBearerTokenVerifier;
import com.vmware.identity.rest.core.server.authorization.verifier.jwt.hok.JWTHoKTokenVerifier;
import com.vmware.identity.rest.core.server.test.util.JWTBuilder;
import com.vmware.identity.rest.core.server.util.VerificationUtil;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;
import com.vmware.identity.rest.core.util.RequestSigner;

public class JWTBearerTokenVerifierTest extends AccessTokenVerifierTest {

    protected static long SKEW = 10 * 60 * 1000;
    protected static PublicKey publicKey;
    protected static PrivateKey privateKey;

    @BeforeClass
    public static void setup() {
        KeyPair keypair = KeyPairUtil.getKeyPair();
        publicKey = keypair.getPublic();
        privateKey = keypair.getPrivate();
    }

    @Test
    public void testVerification() throws Exception {
        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject("administrator")
            .issuer("OATH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .build();

        JWTBearerTokenBuilder builder = new JWTBearerTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.BEARER, jwt.serialize());
        AccessToken token = builder.build(info);

        JWTBearerTokenVerifier verifier = new JWTBearerTokenVerifier(SKEW, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadSignature() throws Exception {
        SignedJWT jwt = new JWTBuilder(privateKey)
            .subject("administrator")
            .issuer("OATH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .build();

        JWTBearerTokenBuilder builder = new JWTBearerTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.BEARER, jwt.serialize());
        AccessToken token = builder.build(info);

        PublicKey badPublicKey = KeyPairUtil.getKeyPair().getPublic();
        JWTBearerTokenVerifier verifier = new JWTBearerTokenVerifier(SKEW, badPublicKey);

        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadType() throws Exception {
        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTBearerTokenBuilder builder = new JWTBearerTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.BEARER, jwt.serialize());
        AccessToken token = builder.build(info);


        JWTBearerTokenVerifier verifier = new JWTBearerTokenVerifier(SKEW, publicKey);
        verifier.verify(token);
    }

}
