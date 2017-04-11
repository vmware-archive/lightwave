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
import java.security.interfaces.RSAPublicKey;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.protocol.AuthenticationErrorResponse;
import com.vmware.identity.openidconnect.protocol.AuthenticationResponse;
import com.vmware.identity.openidconnect.protocol.AuthenticationSuccessResponse;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.LogoutErrorResponse;
import com.vmware.identity.openidconnect.protocol.LogoutResponse;
import com.vmware.identity.openidconnect.protocol.LogoutSuccessResponse;
import com.vmware.identity.openidconnect.protocol.URIUtils;

/**
 * OIDC listener helper class for parsing data in response
 *
 * @author Yehia Zayour
 * @author Jun Sun
 */
public final class ListenerHelper {
    private static final URI DUMMY_URI;

    static {
        try {
            DUMMY_URI = URIUtils.parseURI("https://dummy.uri.com");
        } catch (ParseException e) {
            throw new IllegalStateException("failed to instantiate DUMMY_URI", e);
        }
    }

    private static HttpRequest httpRequestFromMap(Map<String, String> parameterMap) {
        // we are passing in DUMMY_URI because authn/logout response ctors have a required non-null requestURI parameter
        // this requestURI parameter is needed on the server but not on the client
        // this is because authn/logout responses arrive at the oidc client in the form of a request
        return HttpRequest.createGetRequest(DUMMY_URI, parameterMap);
    }

    public static AuthenticationCodeResponse parseAuthenticationCodeResponse(
            HttpServletRequest request) throws OIDCClientException, OIDCServerException {
        Validate.notNull(request, "request");
        return parseAuthenticationCodeResponse(HttpRequest.from(request));
    }

    /**
     * Helper method to parse authentication response for authorization code flow (response contains authz code)
     *
     * @param parameterMap                      Map containing response parameters.
     * @return                                  Authentication response object containing response related data.
     * @throws OIDCClientException              Client side exception.
     * @throws OIDCServerException              Server side exception.
     */
    public static AuthenticationCodeResponse parseAuthenticationCodeResponse(
            Map<String, String> parameterMap) throws OIDCClientException, OIDCServerException {
        Validate.notNull(parameterMap, "parameterMap");
        return parseAuthenticationCodeResponse(httpRequestFromMap(parameterMap));
    }

    private static AuthenticationCodeResponse parseAuthenticationCodeResponse(
            HttpRequest httpRequest) throws OIDCClientException, OIDCServerException {
        AuthenticationResponse authnResponse;
        try {
            authnResponse = AuthenticationResponse.parse(httpRequest);
        } catch (ParseException e) {
            throw new OIDCClientException("failed to parse authentication response", e);
        }

        if (authnResponse instanceof AuthenticationSuccessResponse) {
            AuthenticationSuccessResponse authnSuccessResponse = (AuthenticationSuccessResponse) authnResponse;
            if (authnSuccessResponse.getAuthorizationCode() != null) {
                return new AuthenticationCodeResponse(authnSuccessResponse.getState(), authnSuccessResponse.getAuthorizationCode());
            } else {
                throw new OIDCClientException("response is missing authorization code");
            }
        } else {
            AuthenticationErrorResponse authnErrorResponse = (AuthenticationErrorResponse) authnResponse;
            throw new OIDCServerException(authnErrorResponse.getErrorObject(), authnErrorResponse.getState());
        }
    }

    public static AuthenticationTokensResponse parseAuthenticationTokensResponse(
            HttpServletRequest request,
            RSAPublicKey publicKey,
            Issuer issuer,
            ClientID clientId,
            long clockToleranceSeconds) throws OIDCClientException, OIDCServerException, TokenValidationException {
        Validate.notNull(request, "request");
        Validate.notNull(publicKey, "publicKey");
        Validate.notNull(issuer, "issuer");
        Validate.notNull(clientId, "clientId");
        Validate.isTrue(0 <= clockToleranceSeconds && clockToleranceSeconds <= 10 * 60L, "0 <= clockToleranceSeconds && clockToleranceSeconds <= 10 * 60L");
        return parseAuthenticationTokensResponse(HttpRequest.from(request), publicKey, issuer, clientId, clockToleranceSeconds);
    }

    /**
     * Helper method to parse authentication response for implicit flow (response contains id_token and access_token)
     *
     * @param parameterMap                      Map containing response parameters.
     * @return                                  Authentication response object containing response related data.
     * @throws OIDCClientException              Client side exception.
     * @throws OIDCServerException              Server side exception.
     * @throws TokenValidationException         if id_token does not look right
     */
    public static AuthenticationTokensResponse parseAuthenticationTokensResponse(
            Map<String, String> parameterMap,
            RSAPublicKey publicKey,
            Issuer issuer,
            ClientID clientId,
            long clockToleranceSeconds) throws OIDCClientException, OIDCServerException, TokenValidationException {
        Validate.notNull(parameterMap, "parameterMap");
        Validate.notNull(publicKey, "publicKey");
        Validate.notNull(issuer, "issuer");
        Validate.notNull(clientId, "clientId");
        Validate.isTrue(0 <= clockToleranceSeconds && clockToleranceSeconds <= 10 * 60L, "0 <= clockToleranceSeconds && clockToleranceSeconds <= 10 * 60L");
        return parseAuthenticationTokensResponse(httpRequestFromMap(parameterMap), publicKey, issuer, clientId, clockToleranceSeconds);
    }

    private static AuthenticationTokensResponse parseAuthenticationTokensResponse(
            HttpRequest httpRequest,
            RSAPublicKey publicKey,
            Issuer issuer,
            ClientID clientId,
            long clockToleranceSeconds) throws OIDCClientException, OIDCServerException, TokenValidationException {
        AuthenticationResponse authnResponse;
        try {
            authnResponse = AuthenticationResponse.parse(httpRequest);
        } catch (ParseException e) {
            throw new OIDCClientException("failed to parse authentication response", e);
        }

        if (authnResponse instanceof AuthenticationSuccessResponse) {
            AuthenticationSuccessResponse authnSuccessResponse = (AuthenticationSuccessResponse) authnResponse;
            if (authnSuccessResponse.getIDToken() != null) {
                IDToken idToken = IDToken.build(authnSuccessResponse.getIDToken(), publicKey, issuer, clientId, clockToleranceSeconds);
                AccessToken accessToken = authnSuccessResponse.getAccessToken() == null ? null : new AccessToken(authnSuccessResponse.getAccessToken().serialize());
                OIDCTokens tokens = new OIDCTokens(idToken, accessToken, (RefreshToken) null);
                return new AuthenticationTokensResponse(authnSuccessResponse.getState(), tokens);
            } else {
                throw new OIDCClientException("response is missing id_token");
            }
        } else {
            AuthenticationErrorResponse authnErrorResponse = (AuthenticationErrorResponse) authnResponse;
            throw new OIDCServerException(authnErrorResponse.getErrorObject(), authnErrorResponse.getState());
        }
    }

    public static SessionID parseSLORequest(
            HttpServletRequest request) throws OIDCClientException {
        Validate.notNull(request, "request");
        return parseSLORequest(HttpRequest.from(request).getParameters());
    }

    /**
     * Helper method to parse session Id from SLO request
     *
     * @param parameterMap                      Map containing response parameters.
     * @return                                  Session Id in logout response.
     * @throws OIDCClientException              Client side exception.
     */
    public static SessionID parseSLORequest(
            Map<String, String> parameterMap) throws OIDCClientException {
        Validate.notNull(parameterMap, "parameterMap");

        String sessionIdString = parameterMap.get("sid");
        if (StringUtils.isEmpty(sessionIdString)) {
            throw new OIDCClientException("missing sid parameter");
        }
        return new SessionID(sessionIdString);
    }

    public static State parseLogoutResponse(
            HttpServletRequest request) throws OIDCClientException, OIDCServerException {
        Validate.notNull(request, "request");
        return parseLogoutResponse(HttpRequest.from(request));
    }

    /**
     * Helper method to parse state value from logout response
     *
     * @param parameterMap                      Map containing response parameters.
     * @return                                  State value in post logout response.
     * @throws OIDCClientException              Client side exception.
     * @throws OIDCServerException
     */
    public static State parseLogoutResponse(
            Map<String, String> parameterMap) throws OIDCClientException, OIDCServerException {
        Validate.notNull(parameterMap, "parameterMap");
        return parseLogoutResponse(httpRequestFromMap(parameterMap));
    }

    private static State parseLogoutResponse(
            HttpRequest httpRequest) throws OIDCClientException, OIDCServerException {
        LogoutResponse logoutResponse;
        try {
            logoutResponse = LogoutResponse.parse(httpRequest);
        } catch (ParseException e) {
            throw new OIDCClientException("failed to parse logout response", e);
        }

        if (logoutResponse instanceof LogoutSuccessResponse) {
            LogoutSuccessResponse logoutSuccessResponse = (LogoutSuccessResponse) logoutResponse;
            return logoutSuccessResponse.getState();
        } else {
            LogoutErrorResponse logoutErrorResponse = (LogoutErrorResponse) logoutResponse;
            throw new OIDCServerException(logoutErrorResponse.getErrorObject(), logoutErrorResponse.getState());
        }
    }
}