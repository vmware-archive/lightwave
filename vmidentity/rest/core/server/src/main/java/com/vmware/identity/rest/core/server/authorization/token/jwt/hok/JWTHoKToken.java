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
package com.vmware.identity.rest.core.server.authorization.token.jwt.hok;

import java.security.PublicKey;
import java.text.ParseException;

import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerToken;

public class JWTHoKToken extends JWTBearerToken {

    private JWKSet jwkset;
    private PublicKey key;

    public JWTHoKToken(SignedJWT jwt, String tokenTypeField, String roleField, String groupsField, JWKSet jwkset, PublicKey key) throws ParseException {
        super(jwt, tokenTypeField, roleField, groupsField);
        this.jwkset = jwkset;
        this.key = key;
    }

    public JWKSet getJWKSet() {
        return this.jwkset;
    }

    public PublicKey getPublicKey() {
        return key;
    }

}
