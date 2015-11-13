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
package com.vmware.identity.rest.core.server.authorization.token;

/**
 * An object that describes the information necessary
 * to build an access token.
 */
public class TokenInfo {

    private TokenStyle style;
    private TokenType type;
    private String token;
    private String signature;

    /**
     * Constructs a {@link TokenInfo} with the specified style, type, and token contents.
     *
     * @param style the style of token that was specified by the request
     * @param type the type of the token to construct
     * @param token the string containing the token contents
     */
    public TokenInfo(TokenStyle style, TokenType type, String token) {
        this(style, type, token, null);
    }

    /**
     * Constructs a {@link TokenInfo} with the specified style, type, token contents, and
     * request signature.
     *
     * @param style the style of token that was specified by the request
     * @param type the type of the token to construct
     * @param token the string containing the token contents
     * @param signature the string containing the signed request
     */
    public TokenInfo(TokenStyle style, TokenType type, String token, String signature) {
        this.style = style;
        this.type = type;
        this.token = token;
        this.signature = signature;
    }

    /**
     * Retrieve the token style.
     *
     * @return the token style
     */
    public TokenStyle getStyle() {
        return style;
    }

    /**
     * Retrieve the token type.
     *
     * @return the token type
     */
    public TokenType getType() {
        return type;
    }

    /**
     * Retrieve the token contents.
     *
     * @return the token contents
     */
    public String getToken() {
        return token;
    }

    /**
     * Retrieve the signed request.
     *
     * @return the signed request.
     */
    public String getSignature() {
        return signature;
    }

}
