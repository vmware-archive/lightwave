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
import java.security.interfaces.RSAPublicKey;
import java.util.Date;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
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
public abstract class ServerIssuedToken extends JWTToken {
    private final Date expirationTime;
    private final Scope scope;
    private final String tenant;
    private final ClientID clientId;
    private final SessionID sessionId;
    private final RSAPublicKey holderOfKey;
    private final Subject actAs;
    private final Nonce nonce;

    protected ServerIssuedToken(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        super(tokenClass, signedJwt);

        JWTClaimsSet claims = JWTUtils.getClaimsSet(signedJwt);

        this.expirationTime = JWTUtils.getExpirationTime(claims, tokenClass);

        String scopeString = JWTUtils.getString(claims, tokenClass, "scope");
        try {
            this.scope = Scope.parse(scopeString);
        } catch (ParseException e) {
            throw new ParseException(tokenClass.getValue() + " has invalid scope claim", e);
        }

        this.tenant = JWTUtils.getString(claims, tokenClass, "tenant");

        ClientID clientId = null;
        if (claims.getClaims().containsKey("client_id")) {
            clientId = new ClientID(JWTUtils.getString(claims, tokenClass, "client_id"));
        }
        this.clientId = clientId;

        SessionID sessionId = null;
        if (claims.getClaims().containsKey("sid")) {
            sessionId = new SessionID(JWTUtils.getString(claims, tokenClass, "sid"));
        }
        this.sessionId = sessionId;

        JWKSet holderOfKeyJwkSet = null;
        if (claims.getClaims().containsKey("hotk")) {
            holderOfKeyJwkSet = JWTUtils.getJWKSet(claims, tokenClass, "hotk");
        }
        this.holderOfKey = (holderOfKeyJwkSet == null) ? null : JWTUtils.getPublicKey(holderOfKeyJwkSet);

        Subject actAs = null;
        if (claims.getClaims().containsKey("act_as")) {
            actAs = new Subject(JWTUtils.getString(claims, tokenClass, "act_as"));
        }
        this.actAs = actAs;

        Nonce nonce = null;
        if (claims.getClaims().containsKey("nonce")) {
            nonce = new Nonce(JWTUtils.getString(claims, tokenClass, "nonce"));
        }
        this.nonce = nonce;

        if (!(super.getIssueTime().before(this.expirationTime))) {
            throw new ParseException(tokenClass.getValue() + " iat must be before exp");
        }
    }

    protected ServerIssuedToken(
            TokenClass tokenClass,
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
            Nonce nonce) {
        super(tokenClass, tokenType, jwtId, issuer, subject, audience, issueTime);

        Validate.notNull(expirationTime, "expirationTime");
        Validate.notNull(scope, "scope");
        Validate.notEmpty(tenant, "tenant");

        this.expirationTime = new Date(expirationTime.getTime());
        this.scope = scope;
        this.tenant = tenant;
        this.clientId = clientId;
        this.sessionId = sessionId;
        this.holderOfKey = holderOfKey;
        this.actAs = actAs;
        this.nonce = nonce;

        Validate.isTrue(super.getIssueTime().before(this.expirationTime), "issueTime must be before expirationTime");
    }

    @Override
    protected JWTClaimsSet.Builder claimsBuilder() {
        JWTClaimsSet.Builder claimsBuilder = super.claimsBuilder();

        claimsBuilder = claimsBuilder.expirationTime(this.expirationTime);
        claimsBuilder = claimsBuilder.claim("scope", this.scope.toString());
        claimsBuilder = claimsBuilder.claim("tenant", this.tenant);

        if (this.clientId != null) {
            claimsBuilder = claimsBuilder.claim("client_id", this.clientId.getValue());
        }
        if (this.sessionId != null) {
            claimsBuilder = claimsBuilder.claim("sid", this.sessionId.getValue());
        }
        if (this.holderOfKey != null) {
            RSAKey rsaKey = new RSAKey(this.holderOfKey, KeyUse.SIGNATURE, null, JWSAlgorithm.RS256, null, null, null, null);
            claimsBuilder = claimsBuilder.claim("hotk", (new JWKSet(rsaKey)).toJSONObject());
        }
        if (this.actAs != null) {
            claimsBuilder = claimsBuilder.claim("act_as", this.actAs.getValue());
        }
        if (this.nonce != null) {
            claimsBuilder = claimsBuilder.claim("nonce", this.nonce.getValue());
        }

        return claimsBuilder;
    }

    public long getLifetimeSeconds() {
        return (getExpirationTime().getTime() - getIssueTime().getTime()) / 1000L;
    }

    public Date getExpirationTime() {
        return new Date(this.expirationTime.getTime());
    }

    public Scope getScope() {
        return this.scope;
    }

    public String getTenant() {
        return this.tenant;
    }

    public ClientID getClientID() {
        return this.clientId;
    }

    public SessionID getSessionID() {
        return this.sessionId;
    }

    public RSAPublicKey getHolderOfKey() {
        return this.holderOfKey;
    }

    public Subject getActAs() {
        return this.actAs;
    }

    public Nonce getNonce() {
        return this.nonce;
    }
}