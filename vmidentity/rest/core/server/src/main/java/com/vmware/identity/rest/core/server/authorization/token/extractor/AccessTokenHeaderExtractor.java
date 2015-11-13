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

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.ws.rs.container.ContainerRequestContext;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.extractor.AccessTokenExtractor;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * An extractor implementation that operates against HTTP request headers.
 */
public class AccessTokenHeaderExtractor implements AccessTokenExtractor {

    /**
     * Pattern that matches the authorization header of the form: {@code <Token Type> <Access Token>(:<Signed Request>)}
     */
    private static final Pattern AUTHORIZATION_PATTERN = Pattern.compile("([a-zA-Z]+) ([^:]+)(:(.*))?");

    private StringManager sm;
    private Matcher matcher;

    /**
     * Constructs a new extractor that operates against the HTTP headers
     * contained as part of the request.
     *
     * @param context request context from which to fetch the HTTP headers
     */
    public AccessTokenHeaderExtractor(ContainerRequestContext context, String header) {
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
        String authorizationHeader = context.getHeaderString(header);

        if (authorizationHeader != null && !authorizationHeader.isEmpty()) {
            this.matcher = AUTHORIZATION_PATTERN.matcher(authorizationHeader);
        }
    }

    @Override
    public boolean exists() {
        boolean exists = false;

        if (matcher != null) {
            exists = matcher.matches();
        }

        return exists;
    }

    @Override
    public TokenInfo extract() throws InvalidRequestException {
        if (!exists()) {
            throw new InvalidRequestException(sm.getString("auth.ire.no.token"));
        }

        String type = matcher.group(1).toUpperCase();
        if (!TokenType.contains(type)) {
            throw new InvalidRequestException(sm.getString("auth.ire.invalid.type", type));
        }

        return new TokenInfo(TokenStyle.HEADER, TokenType.valueOf(type), matcher.group(2), matcher.group(4));
    }

}
