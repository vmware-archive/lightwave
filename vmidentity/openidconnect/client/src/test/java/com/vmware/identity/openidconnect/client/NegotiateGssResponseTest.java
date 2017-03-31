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

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.protocol.AccessToken;
import com.vmware.identity.openidconnect.protocol.GSSTicketGrant;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.RefreshToken;
import com.vmware.identity.openidconnect.protocol.TokenErrorResponse;
import com.vmware.identity.openidconnect.protocol.TokenSuccessResponse;

/**
 * Negotiate GSS Response Test
 *
 * @author Jun Sun
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({OIDCClientUtils.class, GSSTicketGrant.class})
public class NegotiateGssResponseTest {

    private static final String ID_TOKEN_STRING = "eyJhbGciOiJSUzI1NiJ9.eyJleHAiOjE0NTA0NjIwMTUsInN1YiI6IkFkbWluaXN0cmF0b3JAdnNwaGVyZS5sb2NhbCIsInRva2VuX2NsYXNzIjoiaWRfdG9rZW4iLCJzY29wZSI6Im9wZW5pZCIsInRlbmFudCI6InZzcGhlcmUubG9jYWwiLCJhdWQiOiJBZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJpc3MiOiJodHRwczpcL1wvc2VhMi1vZmZpY2UtZGhjcC05Ni0xNjQuZW5nLnZtd2FyZS5jb21cL29wZW5pZGNvbm5lY3RcL3ZzcGhlcmUubG9jYWwiLCJmYW1pbHlfbmFtZSI6InZzcGhlcmUubG9jYWwiLCJqdGkiOiJVQkZEeDA5eWZVS1VSSWc0QXJUOEI2NmplT0ptYlV2Rkp2TlNhcWtmeXVjIiwidG9rZW5fdHlwZSI6IkJlYXJlciIsImdpdmVuX25hbWUiOiJBZG1pbmlzdHJhdG9yIiwiaWF0IjoxNDUwNDYxNzE1fQ.Oz_smbwcuk_qv1M5vFsbs_j34LqvW7qgqVc9V4__zeqdhER9SsAn5kTeU0auiQpUpe9Ztmqvc0pW20iaC1sZDmGHP5ZWu1QGCgybst037ZgeStNVsPM86tLaw2On7lW6Rkj1m8OMShJ76jBSxC0m3Zc0c1jPHloMjg738WunNpRAXIsJewt9LQOxxJKvflFJM5gm_IMYeP-R9rtuAAKeWkc2CBjZYtQOGjUV9rzZzNUy9QWVi5iGK6zxFXZD79aWiJpn8-WgX0suNO1i-F-ASmye5NK83DuGcnmtrz3gEVmNkXIk8Uc0iY5ccL9dcMwErfRRs8qmSFqPNsX3ttCYaDcTzngei8qUwTslpvNEtX2I6ShiZvBBT4Uh4sAlAkoFm2IAojgKDj51q98J_Vf6gS81Ylzq1SL9YwI2_QfYK3590I9fsUXcWluldBvGgG816TAHyF0ZPypynJvEZRomMOSUIrLA7-wX6lqJjfeZJfmGh8KC76JdEEwIjE1KbqEJWjwhFyWDIGDQksIXg78hBe7bT_-cOivLFtdLiH7JFRAlfbXz_oswaOfZlAttEXsu0E6MuOne9mBLnk4HpgJ6PuCJzMizsEwKb0hiLoR53Ike4Rh0sF8t2VLTDQ3DO4UbY0mmxBeNchX7qNDYhafBkWWq8rlH3pi_Xm_dGPRk0iU" ;
    private static final String ACCESS_TOKEN_STRING = "eyJhbGciOiJSUzI1NiJ9.eyJleHAiOjE0NTA0NjIwMTYsInN1YiI6IkFkbWluaXN0cmF0b3JAdnNwaGVyZS5sb2NhbCIsInRva2VuX2NsYXNzIjoiYWNjZXNzX3Rva2VuIiwic2NvcGUiOiJvcGVuaWQiLCJ0ZW5hbnQiOiJ2c3BoZXJlLmxvY2FsIiwiYXVkIjoiQWRtaW5pc3RyYXRvckB2c3BoZXJlLmxvY2FsIiwiaXNzIjoiaHR0cHM6XC9cL3NlYTItb2ZmaWNlLWRoY3AtOTYtMTY0LmVuZy52bXdhcmUuY29tXC9vcGVuaWRjb25uZWN0XC92c3BoZXJlLmxvY2FsIiwianRpIjoiSlhGLWVFbGNnVkNwa19DYlA0dFdsMnlEc2laVjU0a3Bnam9yczEyQzlJTSIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJpYXQiOjE0NTA0NjE3MTZ9.ahGZYFJUSmF9t7FINXuTtpZavhkmrsSVaK_k_SS5rHbpnw8BMi_QSKsB6xJ44RvfozV5gOvdvj2n9q3gJznnnqOw4M6wre1W2NqpVFbFkR7BpmK3wqu44R8GvHGvs3qvvyQB466QGMbYZ5llcGvaU8avAFllcxz_mTDDe-A4BUODSHRcxkCPSdqemDAyFMHbdxLbWqJCW9AagnBqfl4pIStfpOOkVo5Bvj0evLUfFCBIqD21RIn5jjc0sSyg_kZhWhTW85_u55RQc_Wmhl4d7i4-YSttT9R4hNkJYnS8ZeD2OLXsBiO_pzlEFBMjt2LBCiHWrX1YxBPbgGvfyAiHDfWOALPv4PjjBYUVPB0PHIwl-EhbF0v2c0vFRrI6woJFm9r8o_RgPsT3wuZrlTkdz_U4ISNHiVK8mROQ3SXnsxfvw-Tia09s18qMXvrtDSeZtBytUiMadC5e9iDyYDCTFJG5TC5fZfGnwseBXgZc_Y-bqGuJ69zftTN0on_7CjVlCHeKtZK-7DM60zQUX8b9hwvmB4A2Z749Z9lnIyIlKkUvdajxHtKgLveh6RiQZj_jVL7ZOlX3-wf3cUYORXzKD05eG0FSb36rI42wXRrWtaSRgXHB86FQpgeZ6ZfSLo2HT-CHy_EZ_9GgIS2QWZnfEfFhbI1tgd6dlxnaJUR0TTY";
    private static GSSNegotiationHandler gssNegotiationHandler;
    private static GSSTicketGrant gssTicketGrantInit = PowerMock.createMock(GSSTicketGrant.class);
    private static GSSTicketGrant gssTicketGrantContinue = PowerMock.createMock(GSSTicketGrant.class);
    private static HttpResponse httpResponseGssContinue;
    private static HttpResponse httpResponseGssContinueWrongClientId;
    private static HttpResponse httpResponseGssSuccess;
    private static HttpResponse httpResponseError;
    private static TokenSpec tokenSpec = TokenSpec.EMPTY;
    private static URI tokenEndpointURI = URI.create("https://abc.com/token");
    private static ClientID clientId = new ClientID("__client_id__");
    private static ClientAuthenticationMethod clientAuthenticationMethod = ClientAuthenticationMethod.NONE;
    private static HolderOfKeyConfig holderOfKeyConfig = null;
    private static KeyStore keyStore = PowerMock.createMock(KeyStore.class);
    private static String contextId = "abcd1234";
    private static String wrongContextId = "efgh5678";
    private static byte[] firstLeg = new byte[]{ 0 };
    private static byte[] secondLeg = new byte[]{ 1 };

    @BeforeClass
    public static void setUp() throws ParseException {
        gssNegotiationHandler = new GSSNegotiationHandler() {
            @Override
            public byte[] negotiate(byte[] leg) {
                if (leg == null) {
                    return firstLeg;
                }
                return secondLeg;
            }
        };

        String message = String.format("gss_continue_needed:%s:%s", contextId, "response");
        httpResponseGssContinue = new TokenErrorResponse(ErrorObject.invalidGrant(message)).toHttpResponse();

        message = String.format("gss_continue_needed:%s:%s", wrongContextId, "response");
        httpResponseGssContinueWrongClientId = new TokenErrorResponse(ErrorObject.invalidGrant(message)).toHttpResponse();

        IDToken idToken = IDToken.parse(ID_TOKEN_STRING);
        AccessToken accessToken = AccessToken.parse(ACCESS_TOKEN_STRING);
        httpResponseGssSuccess = new TokenSuccessResponse(idToken, accessToken, (RefreshToken) null).toHttpResponse();

        httpResponseError = new TokenErrorResponse(ErrorObject.serverError("internal server error")).toHttpResponse();
    }

    @Test
    public void testNegotiateGssResponseSuccess() throws Exception {

        PowerMock.expectNew(GSSTicketGrant.class, contextId, firstLeg).andReturn(gssTicketGrantInit);
        PowerMock.expectNew(GSSTicketGrant.class, contextId, secondLeg).andReturn(gssTicketGrantContinue);
        PowerMock.replay(GSSTicketGrant.class);

        PowerMock.mockStaticPartial(OIDCClientUtils.class, "buildAndSendTokenRequest");
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantInit,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseGssContinue);
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantContinue,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseGssSuccess);
        PowerMock.replay(OIDCClientUtils.class);

        OIDCClientUtils.negotiateGssResponse(
                gssNegotiationHandler,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore,
                contextId);
    }

    @Test(expected=OIDCServerException.class)
    public void testNegotiateGssResponseServerError() throws Exception {

        PowerMock.expectNew(GSSTicketGrant.class, contextId, firstLeg).andReturn(gssTicketGrantInit);
        PowerMock.replay(GSSTicketGrant.class);

        PowerMock.mockStaticPartial(OIDCClientUtils.class, "buildAndSendTokenRequest");
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantInit,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseError);
        PowerMock.replay(OIDCClientUtils.class);

        OIDCClientUtils.negotiateGssResponse(
                gssNegotiationHandler,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore,
                contextId);
    }

    @Test(expected=OIDCClientException.class)
    public void testNegotiateGssResponseClientIdNotMatch() throws Exception {

        PowerMock.expectNew(GSSTicketGrant.class, contextId, firstLeg).andReturn(gssTicketGrantInit);
        PowerMock.replay(GSSTicketGrant.class);

        PowerMock.mockStaticPartial(OIDCClientUtils.class, "buildAndSendTokenRequest");
        EasyMock.expect(OIDCClientUtils.buildAndSendTokenRequest(
                gssTicketGrantInit,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore)).andReturn(httpResponseGssContinueWrongClientId);
        PowerMock.replay(OIDCClientUtils.class);

        OIDCClientUtils.negotiateGssResponse(
                gssNegotiationHandler,
                tokenSpec,
                tokenEndpointURI,
                clientId,
                clientAuthenticationMethod,
                holderOfKeyConfig,
                keyStore,
                contextId);
    }
}
