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
import java.util.HashMap;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.AuthenticationErrorResponse;
import com.vmware.identity.openidconnect.common.AuthenticationResponse;
import com.vmware.identity.openidconnect.common.AuthenticationSuccessResponse;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.HttpRequest;
import com.vmware.identity.openidconnect.common.LogoutErrorResponse;
import com.vmware.identity.openidconnect.common.LogoutResponse;
import com.vmware.identity.openidconnect.common.LogoutSuccessResponse;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.RefreshToken;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.URIUtils;

/**
 * OIDC listener helper class for parsing data in response
 *
 * @author Yehia Zayour
 * @author Jun Sun
 */
public class ListenerHelper {
    public static AuthenticationCodeResponse parseAuthenticationCodeResponse(
            HttpServletRequest request) throws OIDCClientException, OIDCServerException {
        Validate.notNull(request, "request");
        return parseAuthenticationCodeResponse(parameterMap(request), URIUtils.from(request));
    }

    /**
     * Helper method to parse authentication response for authorization code flow (response contains authz code)
     *
     * @param parameterMap                      Map containing response parameters.
     * @param requestURI                        request uri
     * @return                                  Authentication response object containing response related data.
     * @throws OIDCClientException              Client side exception.
     * @throws OIDCServerException              Server side exception.
     */
    public static AuthenticationCodeResponse parseAuthenticationCodeResponse(
            Map<String, String> parameterMap,
            URI requestURI) throws OIDCClientException, OIDCServerException {
        Validate.notNull(parameterMap, "parameterMap");
        Validate.notNull(requestURI, "requestURI");

        AuthenticationResponse authnResponse;
        try {
            authnResponse = AuthenticationResponse.parse(HttpRequest.createGetRequest(requestURI, parameterMap));
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
            ClientID clientId,
            long clockToleranceSeconds) throws OIDCClientException, OIDCServerException, TokenValidationException {
        Validate.notNull(request, "request");
        return parseAuthenticationTokensResponse(parameterMap(request), URIUtils.from(request), publicKey, clientId, clockToleranceSeconds);
    }

    /**
     * Helper method to parse authentication response for implicit flow (response contains id_token and access_token)
     *
     * @param parameterMap                      Map containing response parameters.
     * @param requestURI                        request uri
     * @return                                  Authentication response object containing response related data.
     * @throws OIDCClientException              Client side exception.
     * @throws OIDCServerException              Server side exception.
     * @throws TokenValidationException         if id_token does not look right
     */
    public static AuthenticationTokensResponse parseAuthenticationTokensResponse(
            Map<String, String> parameterMap,
            URI requestURI,
            RSAPublicKey publicKey,
            ClientID clientId,
            long clockToleranceSeconds) throws OIDCClientException, OIDCServerException, TokenValidationException {
        Validate.notNull(parameterMap, "parameterMap");
        Validate.notNull(requestURI, "requestURI");
        Validate.notNull(publicKey, "publicKey");
        Validate.notNull(clientId, "clientId");

        AuthenticationResponse authnResponse;
        try {
            authnResponse = AuthenticationResponse.parse(HttpRequest.createGetRequest(requestURI, parameterMap));
        } catch (ParseException e) {
            throw new OIDCClientException("failed to parse authentication response", e);
        }

        if (authnResponse instanceof AuthenticationSuccessResponse) {
            AuthenticationSuccessResponse authnSuccessResponse = (AuthenticationSuccessResponse) authnResponse;
            if (authnSuccessResponse.getIDToken() != null) {
                ClientIDToken clientIdToken = ClientIDToken.build(authnSuccessResponse.getIDToken(), publicKey, clientId, clockToleranceSeconds);
                OIDCTokens tokens = new OIDCTokens(clientIdToken, authnSuccessResponse.getAccessToken(), (RefreshToken) null);
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
        return parseSLORequest(parameterMap(request));
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
        return parseLogoutResponse(parameterMap(request), URIUtils.from(request));
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
            Map<String, String> parameterMap,
            URI requestURI) throws OIDCClientException, OIDCServerException {
        Validate.notNull(parameterMap, "parameterMap");
        Validate.notNull(requestURI, "requestURI");

        LogoutResponse logoutResponse;
        try {
            logoutResponse = LogoutResponse.parse(HttpRequest.createGetRequest(requestURI, parameterMap));
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

    private static Map<String, String> parameterMap(HttpServletRequest request) throws OIDCClientException {
        Map<String, String> result = new HashMap<String, String>();

        for (Map.Entry<String, String[]> entry : request.getParameterMap().entrySet()) {
            String key = entry.getKey();
            String[] values = entry.getValue();
            if (values.length != 1) {
                throw new OIDCClientException("HttpServletRequest parameter map must contain one value per key. " + entry);
            }
            result.put(key, values[0]);
        }

        return result;
    }
}