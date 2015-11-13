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

import java.net.URI;
import java.security.KeyStore;

import org.easymock.EasyMock;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.easymock.PowerMock;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import com.nimbusds.oauth2.sdk.AccessTokenResponse;
import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.TokenErrorResponse;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;
import com.nimbusds.oauth2.sdk.token.BearerAccessToken;
import com.nimbusds.oauth2.sdk.token.RefreshToken;

/**
 * Negotiate GSS Response Test
 *
 * @author Jun Sun
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({OIDCClientUtils.class, GssTicketGrant.class})
public class NegotiateGssResponseTest {

    private static NegotiationHandler negotiationHandler;
    private static GssTicketGrant gssTicketGrantInit = PowerMock.createMock(GssTicketGrant.class);
    private static GssTicketGrant gssTicketGrantContinue = PowerMock.createMock(GssTicketGrant.class);
    private static HTTPResponse httpResponseGssContinue;
    private static HTTPResponse httpResponseGssContinueWrongClientId;
    private static HTTPResponse httpResponseGssSuccess;
    private static HTTPResponse httpResponseError;
    private static TokenSpec tokenSpec = PowerMock.createMock(TokenSpec.class);
    private static URI tokenEndpointURI;
    private static ClientID clientId = PowerMock.createMock(ClientID.class);
    private static HolderOfKeyConfig holderOfKeyConfig = PowerMock.createMock(HolderOfKeyConfig.class);
    private static KeyStore keyStore = PowerMock.createMock(KeyStore.class);
    private static String contextId = "abcd1234";
    private static String wrongContextId = "efgh5678";
    private static byte[] firstLeg = new byte[]{ 0 };
    private static byte[] secondLeg = new byte[]{ 1 };

    @BeforeClass
    public static void setUp() throws Exception {
        tokenEndpointURI = new URI("https://abc.com/token");

        negotiationHandler = new NegotiationHandler() {
            @Override
            public byte[] negotiate(byte[] leg) {
                if (leg == null) {
                    return firstLeg;
                }
                return secondLeg;
            }
        };

        String message = String.format("gss_continue_needed:%s:%s", contextId, "response");
        httpResponseGssContinue = new TokenErrorResponse(OAuth2Error.INVALID_GRANT.setDescription(message)).toHTTPResponse();

        message = String.format("gss_continue_needed:%s:%s", wrongContextId, "response");
        httpResponseGssContinueWrongClientId = new TokenErrorResponse(OAuth2Error.INVALID_GRANT.setDescription(message)).toHTTPResponse();

        httpResponseGssSuccess = new AccessTokenResponse(new BearerAccessToken(), new RefreshToken()).toHTTPResponse();

        httpResponseError = new TokenErrorResponse(OAuth2Error.SERVER_ERROR).toHTTPResponse();
    }

    @Test
    public void testNegotiateGssResponseSuccess() throws Exception {

        PowerMock.expectNew(GssTicketGrant.class, contextId, firstLeg).andReturn(gssTicketGrantInit);
        PowerMock.expectNew(GssTicketGrant.class, contextId, secondLeg).andReturn(gssTicketGrantContinue);
        PowerMock.replay(GssTicketGrant.class);

        PowerMock.mockStaticPartial(OIDCClientUtils.class, "buildAndSendTokenRequest");
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantInit,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseGssContinue);
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantContinue,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseGssSuccess);
        PowerMock.replay(OIDCClientUtils.class);

        OIDCClientUtils.negotiateGssResponse(
                negotiationHandler,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore,
                contextId);
    }

    @Test(expected=OIDCServerException.class)
    public void testNegotiateGssResponseServerError() throws Exception {

        PowerMock.expectNew(GssTicketGrant.class, contextId, firstLeg).andReturn(gssTicketGrantInit);
        PowerMock.replay(GssTicketGrant.class);

        PowerMock.mockStaticPartial(OIDCClientUtils.class, "buildAndSendTokenRequest");
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantInit,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseError);
        PowerMock.replay(OIDCClientUtils.class);

        OIDCClientUtils.negotiateGssResponse(
                negotiationHandler,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore,
                contextId);
    }

    @Test(expected=OIDCClientException.class)
    public void testNegotiateGssResponseClientIdNotMatch() throws Exception {

        PowerMock.expectNew(GssTicketGrant.class, contextId, firstLeg).andReturn(gssTicketGrantInit);
        PowerMock.replay(GssTicketGrant.class);

        PowerMock.mockStaticPartial(OIDCClientUtils.class, "buildAndSendTokenRequest");
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantInit,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseGssContinueWrongClientId);
        PowerMock.replay(OIDCClientUtils.class);

        OIDCClientUtils.negotiateGssResponse(
                negotiationHandler,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                holderOfKeyConfig,
                keyStore,
                contextId);
    }
}
