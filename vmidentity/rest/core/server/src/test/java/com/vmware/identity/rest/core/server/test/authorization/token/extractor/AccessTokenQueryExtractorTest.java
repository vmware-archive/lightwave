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
import static org.junit.Assert.assertTrue;

import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.MultivaluedHashMap;
import javax.ws.rs.core.MultivaluedMap;
import javax.ws.rs.core.UriInfo;

import org.junit.Test;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenExtractor;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenQueryExtractor;

public class AccessTokenQueryExtractorTest {

    @Test
    public void testValidQueryExtraction_NoTokenType() throws JOSEException {
        String token = "ABCDEFG";

        MultivaluedMap<String, String> queryParameters = new MultivaluedHashMap<String, String>();
        queryParameters.add(Config.ACCESS_TOKEN_PARAMETER, token);

        UriInfo info = createMock(UriInfo.class);
        expect(info.getQueryParameters()).andReturn(queryParameters).anyTimes();
        replay(info);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getUriInfo()).andReturn(info).anyTimes();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenQueryExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenQueryExtractor failed to recognize a valid token", extractor.exists());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not QUERY", TokenStyle.QUERY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testValidQueryExtraction_BearerToken() throws JOSEException {
        String token = "ABCDEFG";

        MultivaluedMap<String, String> queryParameters = new MultivaluedHashMap<String, String>();
        queryParameters.add(Config.ACCESS_TOKEN_PARAMETER, token);
        queryParameters.add(Config.TOKEN_TYPE_PARAMETER, TokenType.BEARER.toString());

        UriInfo info = createMock(UriInfo.class);
        expect(info.getQueryParameters()).andReturn(queryParameters).anyTimes();
        replay(info);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getUriInfo()).andReturn(info).anyTimes();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenQueryExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenQueryExtractor failed to recognize a valid token", extractor.exists());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not QUERY", TokenStyle.QUERY, tokenInfo.getStyle());
        assertEquals("Token type is not BEARER", TokenType.BEARER, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
    }

    @Test
    public void testValidQueryExtraction_HOKToken() throws JOSEException {
        String token = "ABCDEFG";
        String signature = "ABCDEFGH";

        MultivaluedMap<String, String> queryParameters = new MultivaluedHashMap<String, String>();
        queryParameters.add(Config.ACCESS_TOKEN_PARAMETER, token);
        queryParameters.add(Config.TOKEN_TYPE_PARAMETER, TokenType.HOK.toString());
        queryParameters.add(Config.TOKEN_SIGNATURE_PARAMETER, signature);

        UriInfo info = createMock(UriInfo.class);
        expect(info.getQueryParameters()).andReturn(queryParameters).anyTimes();
        replay(info);

        ContainerRequestContext context = createMock(ContainerRequestContext.class);
        expect(context.getUriInfo()).andReturn(info).anyTimes();
        expect(context.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        replay(context);

        AccessTokenExtractor extractor = new AccessTokenQueryExtractor(context, Config.ACCESS_TOKEN_PARAMETER, Config.TOKEN_TYPE_PARAMETER, Config.TOKEN_SIGNATURE_PARAMETER);
        assertTrue("AccessTokenQueryExtractor failed to recognize a valid token", extractor.exists());

        TokenInfo tokenInfo = extractor.extract();
        assertEquals("Token style is not QUERY", TokenStyle.QUERY, tokenInfo.getStyle());
        assertEquals("Token type is not HOK", TokenType.HOK, tokenInfo.getType());
        assertEquals("Token contents do not match", token, tokenInfo.getToken());
        assertEquals("Token signatures do not match", signature, tokenInfo.getSignature());
    }

}
