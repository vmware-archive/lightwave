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
package com.vmware.identity.rest.core.client;

import java.security.PrivateKey;

/**
 * The {@code AccessToken} class represents an access token as issued from one
 * of several token services. This class is the entity used to grant or deny
 * access to any resource or resource method.
 * <p>
 * From the perspective of the user, they should only need to know the encoded
 * string format of the token, the type of token, and if the token is an
 * Holder-of-Key token, the private key that will be used for signing the
 * request as proof of possession.
 */
public class AccessToken {

    /**
     * The {@code Type} enum describes the types of tokens that are accepted
     * by the authorization server.
     */
    public static enum Type {
        /**
         * A security token with the property that any party in possession of
         * the token (a "bearer") can use the token in any way that any other
         * party in possession of it can. Using a bearer token does not
         * require a bearer to prove possession of cryptographic key material
         * (proof-of-possession).
         * <p>
         * This token is expected to be in the JWT format specified by the VMware
         * OIDC server.
         *
         * @see <a href="https://tools.ietf.org/html/rfc6750">The OAuth 2.0 Authorization Framework: Bearer Token Usage</a>
         */
        JWT("Bearer"),

        /**
         * A security token that requires the bearer to prove possession of
         * cryptographic key material (proof-of-possession).
         * <p>
         * "Holder of Key" (<b>HoK</b>) token.
         * <p>
         * This token is expected to be in the JWT format specified by the VMware
         * OIDC server.
         *
         * @see <a href="https://tools.ietf.org/html/rfc2875">Diffie-Hellman Proof-of-Possession Algorithms</a>
         */
        JWT_HOK("HOK"),

        /**
         * A SAML security token.
         * <p>
         * This token is expected to be in the SAML format specified by the
         * VMware Websso server.
         */
        SAML("SAML"),

        /**
         * A SAML security token that requires the bearer to prove possession of
         * cryptographic key material (proof-of-possession).
         * <p>
         * "Holder of Key" (<b>HoK</b>) token.
         * <p>
         * This token is expected to be in the SAML format specified by the
         * VMware Websso server.
         */
        SAML_HOK("SAML");

        private String identifier = null;

        private Type(String identifier) {
            this.identifier = identifier;
        }

        /**
         * Get the type identifier as presented to the remote server.
         *
         * @return the type identifier.
         */
        public String getIdentifier() {
            return identifier;
        }
    }

    private final String token;
    private final Type type;
    private final PrivateKey privateKey;

    /**
     * Construct an {@code AccessToken} with the token contents and its type.
     * Note that this constructor is not sufficient for Holder-of-Key tokens
     * as it does not provide the proof-of-possession. Use
     * {@link AccessToken#AccessToken(String, Type, PrivateKey)} in that case.
     *
     * @param token the string contents of the token.
     * @param type the type of the token in use.
     */
    public AccessToken(String token, Type type) {
        this(token, type, null);
    }

    /**
     * Construct an {@code AccessToken} with the token contents and its type,
     * as well as the private key used for proof-of-possession when using Holder-of-Key
     * tokens.
     *
     * @param token the string contents of the token.
     * @param type the type of the token in use.
     * @param privateKey the private key used for proof-of-possession.
     */
    public AccessToken(String token, Type type, PrivateKey privateKey) {
        this.token = token;
        this.type = type;
        this.privateKey = privateKey;
    }

    /**
     * Get the token string.
     *
     * @return the token in string form.
     */
    public String getToken() {
        return token;
    }

    /**
     * Get the token type.
     *
     * @return the token's type.
     */
    public Type getType() {
        return type;
    }

    /**
     * Get the token's private key.
     *
     * @return the token's private key.
     */
    public PrivateKey getPrivateKey() {
        return privateKey;
    }

}
