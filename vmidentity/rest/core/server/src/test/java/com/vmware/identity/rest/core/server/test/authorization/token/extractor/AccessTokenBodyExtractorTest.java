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
package com.vmware.identity.rest.core.server.test.authorization.token.extractor;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.MediaType;

import org.junit.Test;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenBodyExtractor;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenExtractor;

public class AccessTokenBodyExtractorTest {

    private static String JSON = "{ \"attribute\": \"value\", \"attribute2\": \"value\" }";

    @Test
    public void testValidBodyExtraction_NoTokenType() throws JOSEException {
        String token = "ABCDEFG";
        String body = buildBody(Config.ACCESS_TOKEN_PARAMETER, token, null, null, null, null, null);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenBodyExtractor failed to recognize a valid token", extractor.exists());
        verify(context);

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not BODY", TokenStyle.BODY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testValidBodyExtraction_BearerToken() throws JOSEException {
        String token = "ABCDEF";
        String body = buildBody(Config.ACCESS_TOKEN_PARAMETER, token, Config.TOKEN_TYPE_PARAMETER, TokenType.BEARER.toString(), null, null, null);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenBodyExtractor failed to recognize a valid token", extractor.exists());
        verify(context);

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not BODY", TokenStyle.BODY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testValidBodyExtraction_WithJSON() throws JOSEException {
        String token = "ABCDEF";
        String body = buildBody(Config.ACCESS_TOKEN_PARAMETER, token, Config.TOKEN_TYPE_PARAMETER, TokenType.BEARER.toString(), null, null, JSON);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        context.setEntityStream(isA(InputStream.class));
        expectLastCall();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenBodyExtractor failed to recognize a valid token", extractor.exists());
        verify(context);

        assertEquals("AccessTokenBodyExtractor did not recognize valid JSON", JSON, extractor.getJson());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not BODY", TokenStyle.BODY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testValidBodyExtraction_JunkJSON() throws JOSEException {
        String token = "ABCDEF";
        String body = buildBody(Config.ACCESS_TOKEN_PARAMETER, token, Config.TOKEN_TYPE_PARAMETER, TokenType.BEARER.toString(), null, null, "{ crap");

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenBodyExtractor failed to recognize a valid token", extractor.exists());
        verify(context);

        assertNull("AccessTokenBodyExtractor recognized invalid JSON", extractor.getJson());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not BODY", TokenStyle.BODY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testInvalidBodyExtraction_Nothing() throws JOSEException {
        String body = buildBody(null, null, null, null, null, null, null);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertFalse("AccessTokenBodyExtractor found a token where none should exist", extractor.exists());
        verify(context);

        assertNull("AccessTokenBodyExtractor recognized invalid JSON", extractor.getJson());
    }

    @Test
    public void testInvalidBodyExtraction_OnlyJSON() throws JOSEException {
        String body = buildBody(null, null, null, null, null, null, JSON);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        context.setEntityStream(isA(InputStream.class));
        expectLastCall();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertFalse("AccessTokenBodyExtractor found a token where none should exist", extractor.exists());
        verify(context);

        assertEquals("AccessTokenBodyExtractor did not recognize valid JSON", JSON, extractor.getJson());
    }

    @Test
    public void testInvalidBodyExtraction_JunkJSON() throws JOSEException {
        String body = buildBody(null, null, null, null, null, null, "{ crap");

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertFalse("AccessTokenBodyExtractor found a token where none should exist", extractor.exists());
        verify(context);

        assertNull("AccessTokenBodyExtractor recognized invalid JSON", extractor.getJson());
    }

    @Test
    public void testValidBodyExtraction_BraceInToken() throws JOSEException {
        String token = "ABCDE{F";
        String body = buildBody(Config.ACCESS_TOKEN_PARAMETER, token, Config.TOKEN_TYPE_PARAMETER, TokenType.BEARER.toString(), null, null, JSON);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        context.setEntityStream(isA(InputStream.class));
        expectLastCall();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenBodyExtractor failed to recognize a valid token", extractor.exists());
        verify(context);

        assertEquals("AccessTokenBodyExtractor did not recognize valid JSON", JSON, extractor.getJson());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not BODY", TokenStyle.BODY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testValidBodyExtraction_WithSignature() throws JOSEException {
        String token = "ABCDE{F";
        String signature = "ABCDEFGH";
        String body = buildBody(Config.ACCESS_TOKEN_PARAMETER, token, Config.TOKEN_TYPE_PARAMETER, TokenType.HOK.toString(), Config.TOKEN_SIGNATURE_PARAMETER, signature, JSON);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);

        expect(context.getMediaType()).andReturn(MediaType.APPLICATION_JSON_TYPE).anyTimes();
        expect(context.hasEntity()).andReturn(true).anyTimes();
        expect(context.getEntityStream()).andReturn(new ByteArrayInputStream(body.getBytes())).once();
        context.setEntityStream(isA(InputStream.class));
        expectLastCall();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenBodyExtractor extractor = new AccessTokenBodyExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenBodyExtractor failed to recognize a valid token", extractor.exists());
        verify(context);

        assertEquals("AccessTokenBodyExtractor did not recognize valid JSON", JSON, extractor.getJson());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not BODY", TokenStyle.BODY, tokenInfo.getStyle());
        assertEquals("Token type is not HOK", TokenType.HOK, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
        assertEquals("Token signature does not match", signature, tokenInfo.getSignature());
    }

    private static String buildBody(String accessTokenParameter, String accessToken, String tokenTypeParameter, String tokenType, String tokenSignatureParameter, String tokenSignature, String json) {
        StringBuilder builder = new StringBuilder();
        if (accessTokenParameter != null) {
            builder.append(accessTokenParameter).append("=").append(accessToken);
            if (tokenTypeParameter != null) {
                builder.append("&");
            }
        }

        if (tokenTypeParameter != null) {
            builder.append(tokenTypeParameter).append("=").append(tokenType);
            if (tokenSignatureParameter != null) {
                builder.append("&");
            }
        }

        if (tokenSignatureParameter != null) {
            builder.append(tokenSignatureParameter).append("=").append(tokenSignature);
        }

        if (json != null) {
            if (accessTokenParameter != null || tokenTypeParameter != null || tokenSignatureParameter != null) {
                builder.append("&");
            }
            builder.append(json);
        }

        return builder.toString();
    }

}
