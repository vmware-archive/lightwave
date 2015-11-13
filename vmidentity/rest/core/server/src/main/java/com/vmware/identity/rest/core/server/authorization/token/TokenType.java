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
 * An enumeration describing the types of tokens that are
 * accepted by authorization.
 */
public enum TokenType {
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
    BEARER("Bearer"),

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
    HOK("hotk-pk"),

    /**
     * A SAML security token.
     * <p>
     * This token is expected to be in the SAML format specified by the
     * VMware Websso server.
     */
    SAML("Saml");

    private String jsonName;

    private TokenType(String jsonName) {
        this.jsonName = jsonName;
    }

    public String getJsonName() {
        return jsonName;
    }

    public static boolean contains(String name) {
        if (name == null) {
            return false;
        }

        try {
            TokenType.valueOf(TokenType.class, name);
            return true;
        } catch (IllegalArgumentException ex) {
            return false;
        }
    }
}
