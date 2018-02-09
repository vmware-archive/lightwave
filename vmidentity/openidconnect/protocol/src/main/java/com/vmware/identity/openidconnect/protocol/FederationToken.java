/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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

import java.security.interfaces.RSAPublicKey;
import java.util.Collection;
import java.util.Date;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.TokenClass;

import net.minidev.json.JSONObject;

public abstract class FederationToken {

    protected final TokenClass tokenClass;
    private final String jwtId;
    private final String issuer;
    private final String subject;
    private final Date issueTime;
    private final Date expirationTime;

    private SignedJWT signedJwt;

    protected FederationToken(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notNull(signedJwt, "signedJwt");
        this.tokenClass = tokenClass;
        this.signedJwt = signedJwt;
        JWTClaimsSet claims = JWTUtils.getClaimsSet(signedJwt);
        this.jwtId = claims.getJWTID();
        this.issuer = claims.getIssuer();
        this.issueTime = claims.getIssueTime();
        this.expirationTime = claims.getExpirationTime();
        this.subject = claims.getSubject();
    }

    public boolean hasValidSignature(RSAPublicKey publicKey) throws JOSEException {
        Validate.notNull(publicKey, "publicKey");
        return this.signedJwt.verify(new RSASSAVerifier(publicKey));
    }

    public JWTID getJWTID() {
        if (this.jwtId != null) {
            return new JWTID(this.jwtId);
        }
        return null;
    }

    public TokenClass getTokenClass() {
        return this.tokenClass;
    }

    public String getIssuer() {
        return this.issuer;
    }

    public String getSubject() {
        return this.subject;
    }

    public Date getIssueTime() {
        return this.issueTime;
    }

    public Date getExpirationTime() {
        return this.expirationTime;
    }

    public String serialize() throws IllegalArgumentException {
        return this.signedJwt.serialize();
    }

    public abstract String getUsername();

    public abstract String getDomain();

    public abstract Collection<String> getPermissions();

    public static FederationToken parse(JSONObject jsonObject, TokenClass tokenClass,
            FederationIDPIssuerType issuerType) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");
        return parse(JSONUtils.getSignedJWT(jsonObject, tokenClass.getValue()), tokenClass, issuerType);
    }

    public static FederationToken parse(Map<String, String> parameters, TokenClass tokenClass,
            FederationIDPIssuerType issuerType) throws ParseException {
        Validate.notNull(parameters, "parameters");
        return parse(ParameterMapUtils.getSignedJWT(parameters, tokenClass.getValue()), tokenClass, issuerType);
    }

    public static FederationToken parse(String signedJwtString, TokenClass tokenClass,
            FederationIDPIssuerType issuerType) throws ParseException {
        Validate.notEmpty(signedJwtString, "signedJwtString");
        return parse(JWTUtils.parseSignedJWT(signedJwtString), tokenClass, issuerType);
    }

    public static FederationToken parse(SignedJWT signedJwt, TokenClass tokenClass, FederationIDPIssuerType issuerType)
            throws ParseException {
        Validate.notNull(signedJwt, "signedJwt");
        if (issuerType == FederationIDPIssuerType.CSP) {
            return new CSPToken(tokenClass, signedJwt);
        } else {
            throw new IllegalArgumentException("unsupported federation idp issuer type " + issuerType.getType());
        }
    }
  }
