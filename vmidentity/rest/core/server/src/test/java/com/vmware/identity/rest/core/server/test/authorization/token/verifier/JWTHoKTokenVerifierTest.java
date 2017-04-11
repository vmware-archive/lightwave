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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.util.Calendar;
import java.util.Date;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.UriInfo;

import org.junit.BeforeClass;
import org.junit.Test;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.jwt.hok.JWTHoKTokenBuilder;
import com.vmware.identity.rest.core.server.authorization.verifier.jwt.hok.JWTHoKTokenVerifier;
import com.vmware.identity.rest.core.server.test.util.JWTBuilder;
import com.vmware.identity.rest.core.server.util.VerificationUtil;
import com.vmware.identity.rest.core.test.util.KeyPairUtil;
import com.vmware.identity.rest.core.util.RequestSigner;

public class JWTHoKTokenVerifierTest extends AccessTokenVerifierTest {

    private static final int SKEW_TIME = 15 * 60 * 1000; // 15 minutes

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
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", null, MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidRequestException.class)
    public void testVerification_NoSignature() throws Exception {
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(null, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test
    public void testVerification_DifferingKeypairs() throws Exception {
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());
        KeyPair keypair = KeyPairUtil.getKeyPair();

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(keypair.getPublic())
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, keypair.getPrivate());


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadType() throws Exception {
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());

        SignedJWT jwt = new JWTBuilder(TokenType.BEARER, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadSignature() throws Exception {
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);

        PublicKey badPublicKey = KeyPairUtil.getKeyPair().getPublic();
        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, badPublicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadSignedRequest() throws Exception {
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);

        PrivateKey badPrivateKey = KeyPairUtil.getKeyPair().getPrivate();
        String signedString = RequestSigner.sign(stringToSign, badPrivateKey);

        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadHoK() throws Exception {
        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, new Date());

        PublicKey badPublicKey = KeyPairUtil.getKeyPair().getPublic();
        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(badPublicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);

        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadRequest_DatePast() throws Exception {
        Calendar cal = Calendar.getInstance();
        cal.add(Calendar.MINUTE, -(SKEW_TIME + 1));
        Date date = cal.getTime();

        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadRequest_DateFuture() throws Exception {
        Calendar cal = Calendar.getInstance();
        cal.add(Calendar.MINUTE, SKEW_TIME + 1);
        Date date = cal.getTime();

        ContainerRequestContext context = createMockRequest("some/arbitrary/endpoint", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, context, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadRequest_DifferentEndpoint() throws Exception {
        Date date = new Date();
        String endpoint = "some/arbitrary/endpoint";

        ContainerRequestContext context = createMockRequest(endpoint, "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);
        ContainerRequestContext badContext = createMockRequest(endpoint + "/junk", "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, badContext, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadRequest_DifferentMethod() throws Exception {
        Date date = new Date();
        String endpoint = "some/arbitrary/endpoint";

        ContainerRequestContext context = createMockRequest(endpoint, "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);
        ContainerRequestContext badContext = createMockRequest(endpoint, "POST", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, badContext, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadRequest_DifferentEntity() throws Exception {
        Date date = new Date();
        String endpoint = "some/arbitrary/endpoint";

        ContainerRequestContext context = createMockRequest(endpoint, "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);
        ContainerRequestContext badContext = createMockRequest(endpoint, "PUT", "Hello", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, badContext, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    @Test(expected=InvalidTokenException.class)
    public void testVerification_BadRequest_DifferentMediaType() throws Exception {
        Date date = new Date();
        String endpoint = "some/arbitrary/endpoint";

        ContainerRequestContext context = createMockRequest(endpoint, "PUT", "", MediaType.APPLICATION_FORM_URLENCODED_TYPE, date);
        ContainerRequestContext badContext = createMockRequest(endpoint, "PUT", "", MediaType.APPLICATION_JSON_TYPE, date);

        SignedJWT jwt = new JWTBuilder(TokenType.HOK, privateKey)
            .subject("administrator")
            .issuer("OAUTH")
            .audience(RESOURCE_SERVER_AUDIENCE)
            .role(Role.ADMINISTRATOR.getRoleName())
            .hotk(publicKey)
            .build();

        JWTHoKTokenBuilder builder = new JWTHoKTokenBuilder(JWTBuilder.TOKEN_TYPE_CLAIM, JWTBuilder.ROLE_CLAIM, JWTBuilder.GROUPS_CLAIM, JWTBuilder.HOK_CLAIM);
        TokenInfo info = new TokenInfo(TokenStyle.HEADER, TokenType.HOK, jwt.serialize());
        AccessToken token = builder.build(info);

        String stringToSign = VerificationUtil.buildStringToSign(context);
        String signedString = RequestSigner.sign(stringToSign, privateKey);


        JWTHoKTokenVerifier verifier = new JWTHoKTokenVerifier(signedString, badContext, SKEW_TIME, publicKey);
        verifier.verify(token);
    }

    private static ContainerRequestContext createMockRequest(String uri, String method, String entity, MediaType mediaType, Date date) throws URISyntaxException {
        UriInfo uriInfo = createMock(UriInfo.class);
        expect(uriInfo.getRequestUri()).andReturn(new URI(uri)).anyTimes();
        replay(uriInfo);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getMethod()).andReturn(method).anyTimes();
        if (entity != null) {
            expect(context.hasEntity()).andReturn(!entity.isEmpty()).anyTimes();
            expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(entity.getBytes())).anyTimes();
            context.setEntityStream(isA(InputStream.class));
            expectLastCall().anyTimes();
        } else {
            expect(context.hasEntity()).andReturn(false).anyTimes();
        }
        expect(context.getMediaType()).andReturn(mediaType).anyTimes();
        expect(context.getDate()).andReturn(date).anyTimes();
        expect(context.getUriInfo()).andReturn(uriInfo).anyTimes();
        replay(context);

        return context;
    }

}
