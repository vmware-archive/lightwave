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

import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.InvalidKeySpecException;
import java.text.ParseException;

import net.minidev.json.JSONObject;
import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenInfo;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerTokenBuilder;

public class JWTHoKTokenBuilder extends JWTBearerTokenBuilder {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(JWTBearerTokenBuilder.class);

    private String hokField;

    public JWTHoKTokenBuilder(String tokenTypeField, String roleField, String groupsField, String hokField) {
        super(tokenTypeField, roleField, groupsField);
        this.hokField = hokField;
    }

    public String getHoKField() {
        return hokField;
    }

    @Override
    public AccessToken build(TokenInfo info) throws InvalidTokenException {
        try {
            SignedJWT jwt = SignedJWT.parse(info.getToken());

            JSONObject jwks = (JSONObject) jwt.getJWTClaimsSet().getClaim(hokField);

            if (jwks == null) {
                throw new InvalidTokenException(sm.getString("auth.ite.parse.missing.claim", hokField));
            }

            JWKSet jwkset = JWKSet.parse(jwks);

            RSAKey rsaKey = RSAKey.parse(jwkset.getKeys().get(0).toJSONObject());
            PublicKey key = rsaKey.toRSAPublicKey();

            return new JWTHoKToken(jwt, getTokenTypeField(), getRoleField(), getGroupsField(), jwkset, key);
        } catch (ParseException | JOSEException e) {
            log.error("Error parsing the JWT HOK Token", e);
            throw new InvalidTokenException(sm.getString("auth.ite.parse.malformed"));
        }
    }

}
