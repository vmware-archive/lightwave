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

package com.vmware.identity.openidconnect.client;

import java.security.interfaces.RSAPublicKey;
import java.util.Collection;
import java.util.Date;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
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
 * @author Jun Sun
 */
public final class IDToken {
    private final com.vmware.identity.openidconnect.protocol.IDToken idToken;

    private IDToken(com.vmware.identity.openidconnect.protocol.IDToken idToken) {
        this.idToken = idToken;
    }

    public static IDToken build(
            String value,
            RSAPublicKey providerPublicKey,
            Issuer issuer,
            ClientID clientId,
            long clockToleranceInSeconds) throws TokenValidationException {
        Validate.notEmpty(value, "value");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(issuer, "issuer");

        com.vmware.identity.openidconnect.protocol.IDToken idToken;
        try {
            idToken = com.vmware.identity.openidconnect.protocol.IDToken.parse(value);
        } catch (ParseException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token parsing failed." + e.getMessage(), e);
        }

        return build(idToken, providerPublicKey, issuer, clientId, clockToleranceInSeconds);
    }

    static IDToken build(
            com.vmware.identity.openidconnect.protocol.IDToken idToken,
            RSAPublicKey providerPublicKey,
            Issuer issuer,
            ClientID clientId,
            long clockToleranceInSeconds) throws TokenValidationException {
        Validate.notNull(idToken, "idToken");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(issuer, "issuer");

        try {
            if (!idToken.hasValidSignature(providerPublicKey)) {
                throw new TokenValidationException(TokenValidationError.INVALID_SIGNATURE, "Token signature validation failed.");
            }
        } catch (JOSEException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token Signature verification process failed." + e.getMessage(), e);
        }

        if (clientId != null && !idToken.getAudience().contains(clientId.getValue())) {
            throw new TokenValidationException(TokenValidationError.INVALID_AUDIENCE, "Audience in claim set does not match with client identifier.");
        }

        Date now = new Date();
        Date notBefore = new Date(idToken.getIssueTime().getTime() - clockToleranceInSeconds * 1000L);
        Date notAfter = new Date(idToken.getExpirationTime().getTime() + clockToleranceInSeconds * 1000L);
        if (now.before(notBefore)) {
            throw new TokenValidationException(TokenValidationError.TOKEN_NOT_YET_VALID, "Token is not yet valid.");
        }
        if (now.after(notAfter)) {
            throw new TokenValidationException(TokenValidationError.EXPIRED_TOKEN, "Token has expired.");
        }

        return new IDToken(idToken);
    }

    public String serialize() {
        return this.idToken.serialize();
    }

    public TokenClass getTokenClass() {
        return this.idToken.getTokenClass();
    }

    public TokenType getTokenType() {
        return this.idToken.getTokenType();
    }

    public JWTID getJWTID() {
        return this.idToken.getJWTID();
    }

    public Issuer getIssuer() {
        return this.idToken.getIssuer();
    }

    public Subject getSubject() {
        return this.idToken.getSubject();
    }

    public List<String> getAudience() {
        return this.idToken.getAudience();
    }

    public Date getIssueTime() {
        return this.idToken.getIssueTime();
    }

    public Date getExpirationTime() {
        return this.idToken.getExpirationTime();
    }

    public long getLifetimeSeconds() {
        return this.idToken.getLifetimeSeconds();
    }

    public Scope getScope() {
        return this.idToken.getScope();
    }

    public String getTenant() {
        return this.idToken.getTenant();
    }

    public ClientID getClientID() {
        return this.idToken.getClientID();
    }

    public SessionID getSessionID() {
        return this.idToken.getSessionID();
    }

    public RSAPublicKey getHolderOfKey() {
        return this.idToken.getHolderOfKey();
    }

    public Subject getActAs() {
        return this.idToken.getActAs();
    }

    public Nonce getNonce() {
        return this.idToken.getNonce();
    }

    public Collection<String> getGroups() {
        return this.idToken.getGroups();
    }

    public String getGivenName() {
        return this.idToken.getGivenName();
    }

    public String getFamilyName() {
        return this.idToken.getFamilyName();
    }

    com.vmware.identity.openidconnect.protocol.IDToken getIDToken() {
        return this.idToken;
    }
}