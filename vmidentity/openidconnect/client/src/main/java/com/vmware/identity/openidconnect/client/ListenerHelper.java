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

import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jwt.SignedJWT;

/**
 * OIDC listener helper class for parsing data in response
 *
 * @author Jun Sun
 */
public class ListenerHelper {

    /**
     * Helper method to parse authentication response related data
     *
     * @param parameterMap                      Map containing response parameters.
     * @param clientConfig                      Configuration required in creating a client.
     * @return                                  Authentication response object containing response related data.
     * @throws OIDCClientException              Client side exception.
     * @throws OIDCServerException              Server side exception.
     * @throws TokenValidationException         Token validation exception.
     */
    public static AuthenticationResponse parseAuthenticationResponse(Map<String, String> parameterMap, ClientConfig clientConfig) throws OIDCClientException, OIDCServerException, TokenValidationException {
        Validate.notNull(parameterMap, "parameterMap");

        State state = null;
        if (parameterMap.containsKey("state")) {
            state = new State(parameterMap.get("state"));
        }

        if (parameterMap.containsKey("error")) {
            return new AuthenticationErrorResponse(state, parameterMap.get("error"), parameterMap.get("error_description"));
        }

        if (parameterMap.containsKey("code")) {
            return new AuthenticationCodeResponse(state, new AuthorizationCode(parameterMap.get("code")));
        } else if (parameterMap.containsKey("id_token")) {
            try {
                if (clientConfig == null) {
                    throw new OIDCClientException("Client config is required for parsing implicit flow response.");
                }
                IDToken idToken = IDToken.build(
                        SignedJWT.parse(parameterMap.get("id_token")),
                        clientConfig.getConnectionConfig().getProviderPublicKey(),
                        clientConfig.getClientId(),
                        clientConfig.getConnectionConfig().getIssuer());
                AccessToken accessToken = null;
                if (parameterMap.containsKey("access_token")) {
                    accessToken = new AccessToken(parameterMap.get("access_token"));
                }
                return new AuthenticationTokensResponse(state, new OIDCTokens(
                        accessToken,
                        idToken,
                        null));
            } catch (java.text.ParseException e) {
                throw new OIDCClientException("Parse signed JWT from id token failed: " + e.getMessage(), e);
            }
        } else {
            throw new OIDCClientException("Invalid authentication response.");
        }
    }

    /**
     * Helper method to parse session Id from SLO request
     *
     * @param parameterMap                      Map containing response parameters.
     * @return                                  Session Id in logout response.
     * @throws OIDCClientException              Client side exception.
     */
    public static SessionID parseSLORequest(Map<String, String> parameterMap) throws OIDCClientException {
        Validate.notNull(parameterMap, "parameterMap");

        if (parameterMap.containsKey("sid")) {
            return new SessionID(parameterMap.get("sid"));
        } else {
            return null;
        }
    }

    /**
     * Helper method to parse state value from logout response
     *
     * @param parameterMap                      Map containing response parameters.
     * @return                                  State value in post logout response.
     * @throws OIDCClientException              Client side exception.
     */
    public static State parseLogoutResponse(Map<String, String> parameterMap) throws OIDCClientException {
        Validate.notNull(parameterMap, "parameterMap");

        if (parameterMap.containsKey("state")) {
            return new State(parameterMap.get("state"));
        } else {
            return null;
        }
    }
}
