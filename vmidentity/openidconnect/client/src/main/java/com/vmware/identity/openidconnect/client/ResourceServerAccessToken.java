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
import com.vmware.identity.openidconnect.protocol.AccessToken;

/**
 * @author Yehia Zayour
 * @author Jun Sun
 */
public final class ResourceServerAccessToken {
    private final AccessToken accessToken;

    private ResourceServerAccessToken(AccessToken accessToken) {
        this.accessToken = accessToken;
    }

    /**
     * Access token builder, including signature verification and validation.
     *
     * @param value                             Opaque access token string.
     * @param providerPublicKey                 public key retrieved from Authorization Server jwks endpoint
     * @param issuer                            OIDC issuer.
     * @param resourceServer                    Name of resource server.
     * @param clockToleranceInSeconds           Clock tolerance in seconds.
     * @return                                  Access token including claims.
     * @throws TokenValidationException         Token validation exception.
     */
    public static ResourceServerAccessToken build(
            String value,
            RSAPublicKey providerPublicKey,
            Issuer issuer,
            String resourceServer,
            long clockToleranceInSeconds) throws TokenValidationException {
        Validate.notEmpty(value, "value");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(issuer, "issuer");
        // resourceServer is nullable
        Validate.isTrue(0 <= clockToleranceInSeconds && clockToleranceInSeconds <= 10 * 60L, "0 <= clockToleranceInSeconds && clockToleranceInSeconds <= 10 * 60L");

        AccessToken accessToken;
        try {
            accessToken = AccessToken.parse(value);
        } catch (ParseException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token parsing failed." + e.getMessage(), e);
        }

        try {
            if (!accessToken.hasValidSignature(providerPublicKey)) {
                throw new TokenValidationException(TokenValidationError.INVALID_SIGNATURE, "Token signature validation failed.");
            }
        } catch (JOSEException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token Signature verification process failed." + e.getMessage(), e);
        }

        if (resourceServer != null && !accessToken.getAudience().contains(resourceServer)) {
            throw new TokenValidationException(TokenValidationError.INVALID_AUDIENCE, "Audience in claim set does not contain the specified resource server.");
        }

        Date now = new Date();
        Date notBefore = new Date(accessToken.getIssueTime().getTime() - clockToleranceInSeconds * 1000L);
        Date notAfter = new Date(accessToken.getExpirationTime().getTime() + clockToleranceInSeconds * 1000L);
        if (now.before(notBefore)) {
            throw new TokenValidationException(TokenValidationError.TOKEN_NOT_YET_VALID, "Token is not yet valid.");
        }
        if (now.after(notAfter)) {
            throw new TokenValidationException(TokenValidationError.EXPIRED_TOKEN, "Token has expired.");
        }

        return new ResourceServerAccessToken(accessToken);
    }

    public String serialize() {
        return this.accessToken.serialize();
    }

    public TokenClass getTokenClass() {
        return this.accessToken.getTokenClass();
    }

    public TokenType getTokenType() {
        return this.accessToken.getTokenType();
    }

    public JWTID getJWTID() {
        return this.accessToken.getJWTID();
    }

    public Issuer getIssuer() {
        return this.accessToken.getIssuer();
    }

    public Subject getSubject() {
        return this.accessToken.getSubject();
    }

    public List<String> getAudience() {
        return this.accessToken.getAudience();
    }

    public Date getIssueTime() {
        return this.accessToken.getIssueTime();
    }

    public Date getExpirationTime() {
        return this.accessToken.getExpirationTime();
    }

    public long getLifetimeSeconds() {
        return this.accessToken.getLifetimeSeconds();
    }

    public Scope getScope() {
        return this.accessToken.getScope();
    }

    public String getTenant() {
        return this.accessToken.getTenant();
    }

    public ClientID getClientID() {
        return this.accessToken.getClientID();
    }

    public SessionID getSessionID() {
        return this.accessToken.getSessionID();
    }

    public RSAPublicKey getHolderOfKey() {
        return this.accessToken.getHolderOfKey();
    }

    public Subject getActAs() {
        return this.accessToken.getActAs();
    }

    public Nonce getNonce() {
        return this.accessToken.getNonce();
    }

    public Collection<String> getGroups() {
        return this.accessToken.getGroups();
    }

    public String getAdminServerRole() {
        return this.accessToken.getAdminServerRole();
    }
}