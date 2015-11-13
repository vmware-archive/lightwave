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
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.junit.Test;

import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenExtractor;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenHeaderExtractor;

public class AccessTokenHeaderExtractorTest {

    private static String ACCESS_TOKEN_HEADER = "Authorization";

    @Test
    public void testValidHeaderExtraction_TokenWithoutSignature() {
        String token = "ABCDEFG";
        String header = buildHeader(TokenType.BEARER.toString(), token, null);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getHeaderString(ACCESS_TOKEN_HEADER)).andReturn(header);
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenHeaderExtractor(context, ACCESS_TOKEN_HEADER);
        assertTrue("AccessTokenHeaderExtractor failed to recognize a valid token", extractor.exists());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not HEADER", TokenStyle.HEADER, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
        assertNull("Token signature is not null", tokenInfo.getSignature());
    }

    @Test
    public void testValidHeaderExtraction_TokenAndSignature() {
        String token = "ABCDEFG";
        String signature = "FGHIJKL";
        String header = buildHeader(TokenType.HOK.toString(), token, signature);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getHeaderString(ACCESS_TOKEN_HEADER)).andReturn(header);
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenHeaderExtractor(context, ACCESS_TOKEN_HEADER);
        assertTrue("AccessTokenHeaderExtractor failed to recognize a valid token", extractor.exists());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not HEADER", TokenStyle.HEADER, tokenInfo.getStyle());
        assertEquals("Token type is not HOK", TokenType.HOK, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
        assertEquals("Token signature does not match", signature, tokenInfo.getSignature());
    }

    @Test(expected = InvalidRequestException.class)
    public void testInvalidHeaderExtraction_Empty() {
        String token = null;
        String header = buildHeader(TokenType.BEARER.toString(), token, null);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getHeaderString(ACCESS_TOKEN_HEADER)).andReturn(header);
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenHeaderExtractor(context, ACCESS_TOKEN_HEADER);
        assertFalse("AccessTokenHeaderExtractor recognized an invalid token", extractor.exists());

        extractor.extract();
    }

    public static String buildHeader(String tokenType, String accessToken, String signature) {
        StringBuilder builder = new StringBuilder();
        if (tokenType != null) {
            builder.append(tokenType).append(" ");
        }

        if (accessToken != null) {
            builder.append(accessToken);
        }

        if (signature!= null) {
            builder.append(":").append(signature);
        }

        return builder.toString();
    }

}
