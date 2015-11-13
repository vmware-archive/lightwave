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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.util.Scanner;
import java.util.regex.MatchResult;
import java.util.regex.Pattern;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Form;
import javax.ws.rs.core.MediaType;

import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.TokenStyle;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.util.StringManager;

public class AccessTokenBodyExtractor implements AccessTokenExtractor {
    private static Pattern FIELD_PATTERN = Pattern.compile("(\\w+)=?([^&]+)?");
    private static Pattern FIELD_DELIMITER = Pattern.compile("&");
    private static Pattern JSON_DELIMITER = Pattern.compile("(\\{.*\\})");

    private String accessTokenParameter;
    private String tokenTypeParameter;
    private String tokenSignatureParameter;
    private String json;
    private Form form;
    private StringManager sm;

    /**
     * Constructs a new extractor that operates against the specified HTTP entity body parameters.
     *
     * @param context request context from which to request the entity body parameters
     * @param accessTokenParameter name of the parameter containing the access token
     * @param tokenTypeParameter name of the parameter containing the token type
     * @param tokenSignatureParameter name of the parameter containing the token signature
     */
    public AccessTokenBodyExtractor(ContainerRequestContext context, String accessTokenParameter, String tokenTypeParameter, String tokenSignatureParameter) {
        this.accessTokenParameter = accessTokenParameter;
        this.tokenTypeParameter = tokenTypeParameter;
        this.tokenSignatureParameter = tokenSignatureParameter;
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
        this.form = new Form();

        if (MediaType.APPLICATION_JSON_TYPE.isCompatible(context.getMediaType()) && context.hasEntity()) {
            InputStream inputStream = context.getEntityStream();
            Scanner scanner = new Scanner(inputStream);
            scanner.useDelimiter(FIELD_DELIMITER);

            while(scanner.hasNext(FIELD_PATTERN)) {
                scanner.next(FIELD_PATTERN);

                MatchResult matches = scanner.match();
                if (matches.groupCount() == 2) {
                    String name = matches.group(1);
                    String value = matches.group(2);

                    // Only put them into the map if the parameter is correctly formed
                    if (value != null) {
                        form.param(name, value);
                    }
                }
            }

            if (scanner.findWithinHorizon(JSON_DELIMITER, 0) != null) {
                MatchResult matches = scanner.match();
                json = matches.group(1);
            }

            if (json != null) {
                // There was JSON but we already consumed it, so we have to replace the input stream
                // Close out the old input stream since we will replace it
                try {
                    inputStream.close();
                } catch (IOException e) {
                    throw new IllegalArgumentException(e);
                }

                try {
                    // Set the user's specified charset or default to UTF-8
                    String requestCharset = context.getMediaType().getParameters().get(MediaType.CHARSET_PARAMETER);

                    if (requestCharset == null) {
                        requestCharset = StandardCharsets.UTF_8.name();
                    }

                    inputStream = new ByteArrayInputStream(json.getBytes(requestCharset));
                } catch (UnsupportedEncodingException e) {
                    throw new IllegalArgumentException(e);
                }

                context.setEntityStream(inputStream);
            }
        }
    }

    @Override
    public boolean exists() {
        return form.asMap().containsKey(accessTokenParameter);
    }

    @Override
    public TokenInfo extract() throws InvalidRequestException {
        if (!exists()) {
            throw new InvalidRequestException(sm.getString("auth.ire.invalid.param", accessTokenParameter));
        }

        return new TokenInfo(TokenStyle.BODY, getTokenType(), getAccessToken(), getTokenSignature());
    }

    public String getJson() {
        return json;
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

        if (form.asMap().containsKey(tokenTypeParameter)) {
            String tokenType = form.asMap().getFirst(tokenTypeParameter).toUpperCase();

            if (!TokenType.contains(tokenType)) {
                throw new InvalidRequestException(sm.getString("auth.ire.invalid.type", tokenType));
            }

            type = TokenType.valueOf(tokenType);
        }

        return type;
    }

    private String getAccessToken() {
        return form.asMap().getFirst(accessTokenParameter);
    }

    private String getTokenSignature() {
        return form.asMap().getFirst(tokenSignatureParameter);
    }

}
