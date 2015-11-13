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
package com.vmware.identity.rest.core.server.authorization.token.extractor;

import javax.ws.rs.container.ContainerRequestContext;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenExtractor;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * An extractor implementation that operates against HTTP request parameters
 */
public class AccessTokenQueryExtractor implements AccessTokenExtractor {

    private ContainerRequestContext context;
    private String accessTokenParameter;
    private String tokenTypeParameter;
    private String tokenSignatureParameter;

    private StringManager sm;

    /**
     * Constructs a new extractor that operates against the specified request parameters.
     * <p>
     * If no token type is specified, the token type is assumed to be {@link TokenType#BEARER}.
     * If the parameters are specified in list format, the extractor uses the first occurrence.
     *
     * @param context request context from which to fetch request parameters
     * @param accessTokenParameter name of the parameter containing the access token
     * @param tokenTypeParameter name of the parameter containing the token type
     * @param tokenSignatureParameter name of the parameter containing the token signature
     */
    public AccessTokenQueryExtractor(ContainerRequestContext context, String accessTokenParameter, String tokenTypeParameter, String tokenSignatureParameter) {
        this.context = context;
        this.accessTokenParameter = accessTokenParameter;
        this.tokenTypeParameter = tokenTypeParameter;
        this.tokenSignatureParameter = tokenSignatureParameter;
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
    }

    @Override
    public boolean exists() {
        return context.getUriInfo().getQueryParameters().containsKey(accessTokenParameter);
    }

    @Override
    public TokenInfo extract() throws InvalidRequestException {
        if (!exists()) {
            throw new InvalidRequestException(sm.getString("auth.ire.invalid.param", accessTokenParameter));
        }

        return new TokenInfo(TokenStyle.QUERY, getTokenType(), getAccessToken(), getTokenSignature());
    }

    /**
     * Fetches the token type from the request parameters.
     *
     * @return token type specified in the request parameter, or {@link TokenType#BEARER}
     * if the parameter is unspecified
     * @throws InvalidRequestException if the type specified is not a valid {@link TokenType}
     */
    private TokenType getTokenType() throws InvalidRequestException {
        TokenType type = TokenType.BEARER;

        if (context.getUriInfo().getQueryParameters().containsKey(tokenTypeParameter)) {
            String tokenType = context.getUriInfo().getQueryParameters().getFirst(tokenTypeParameter).toUpperCase();

            if (!TokenType.contains(tokenType)) {
                throw new InvalidRequestException(sm.getString("auth.ire.invalid.type", tokenType));
            }

            type = TokenType.valueOf(tokenType);
        }

        return type;
    }

    private String getAccessToken() {
        return context.getUriInfo().getQueryParameters().getFirst(accessTokenParameter);
    }

    private String getTokenSignature() {
        return context.getUriInfo().getQueryParameters().getFirst(tokenSignatureParameter);
    }

}
