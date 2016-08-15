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

package com.vmware.identity.openidconnect.protocol;

import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Date;
import java.util.List;
import java.util.Map;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.Subject;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * @author Yehia Zayour
 */
public final class RefreshToken extends ServerIssuedToken {
    private static final TokenClass TOKEN_CLASS = TokenClass.REFRESH_TOKEN;

    private final SignedJWT signedJwt;

    private RefreshToken(SignedJWT signedJwt) throws ParseException {
        super(TOKEN_CLASS, signedJwt);

        this.signedJwt = signedJwt;
    }

    public RefreshToken(
            RSAPrivateKey privateKey,
            TokenType tokenType,
            JWTID jwtId,
            Issuer issuer,
            Subject subject,
            List<String> audience,
            Date issueTime,

            Date expirationTime,
            Scope scope,
            String tenant,
            ClientID clientId,
            SessionID sessionId,
            RSAPublicKey holderOfKey,
            Subject actAs,
            Nonce nonce) throws JOSEException {
        super(TOKEN_CLASS, tokenType, jwtId, issuer, subject, audience, issueTime, expirationTime, scope, tenant, clientId, sessionId, holderOfKey, actAs, nonce);

        Validate.notNull(privateKey, "privateKey");

        JWTClaimsSet.Builder claimsBuilder = super.claimsBuilder();
        this.signedJwt = JWTUtils.signClaimsSet(claimsBuilder.build(), privateKey);
    }

    @Override
    protected SignedJWT getSignedJWT() {
        return this.signedJwt;
    }

    public static RefreshToken parse(JSONObject jsonObject) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");
        return new RefreshToken(JSONUtils.getSignedJWT(jsonObject, "refresh_token"));
    }

    public static RefreshToken parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");
        return new RefreshToken(ParameterMapUtils.getSignedJWT(parameters, "refresh_token"));
    }

    public static RefreshToken parse(String signedJwtString) throws ParseException {
        Validate.notEmpty(signedJwtString, "signedJwtString");
        return new RefreshToken(JWTUtils.parseSignedJWT(signedJwtString));
    }

    public static RefreshToken parse(SignedJWT signedJwt) throws ParseException {
        Validate.notNull(signedJwt, "signedJwt");
        return new RefreshToken(signedJwt);
    }
}