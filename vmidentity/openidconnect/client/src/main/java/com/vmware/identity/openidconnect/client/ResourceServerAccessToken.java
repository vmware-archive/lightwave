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
import java.text.ParseException;
import java.util.Date;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSVerifier;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * Resource Server Access Token
 *
 * @author Jun Sun
 */
public class ResourceServerAccessToken {

    private final ReadOnlyJWTClaimsSet jwtClaimsSet;

    private ResourceServerAccessToken(ReadOnlyJWTClaimsSet jwtClaimsSet) {
        Validate.notNull(jwtClaimsSet, "jwtClaimsSet");

        this.jwtClaimsSet = jwtClaimsSet;
    }

    /**
     * Access token builder, including signature verification and validation.
     *
     * @param value                             Opaque access token string.
     * @param providerPublicKey                 OIDC server metadata from querying metadata endpoint. It contains server endpoints URI,
     *                                          issuer and other server supported capability configurations.
     * @param resourceServer                    Name of resource server.
     * @param issuer                            OIDC issuer.
     * @return                                  Access token including claims.
     * @throws TokenValidationException         Token validation exception.
     */
    public static ResourceServerAccessToken build(
            String value,
            RSAPublicKey providerPublicKey,
            String resourceServer,
            Issuer issuer) throws TokenValidationException {
        Validate.notNull(value, "value");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(resourceServer, "resourceServer");
        Validate.notNull(issuer, "issuer");

        // verify the signature
        JWSVerifier verifier = new RSASSAVerifier(providerPublicKey);
        SignedJWT signedJWT;
        try {
            signedJWT = SignedJWT.parse(value);

            if (!signedJWT.verify(verifier)) {
                throw new TokenValidationException(TokenValidationError.INVALID_SIGNATURE, "Token signature validation failed.");
            }
        } catch (ParseException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token parsing failed." + e.getMessage(), e);
        } catch (JOSEException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token Signature verification process failed." + e.getMessage(), e);
        }

        // verify claims
        Date now = new Date();
        ReadOnlyJWTClaimsSet jwtClaimsSet = null;
        try {
            jwtClaimsSet = signedJWT.getJWTClaimsSet();

            if (!jwtClaimsSet.getAudience().contains(resourceServer)) {
                throw new TokenValidationException(TokenValidationError.INVALID_AUDIENCE, "Audience in claim set does not contain the specified resource server.");
            }

            // TODO: add clock skew
            if ((jwtClaimsSet.getExpirationTime().before(now /* + clockSkew */))) {
                throw new TokenValidationException(TokenValidationError.EXPIRED_TOKEN, "Token is expired.");
            }

            // validate id token class & type
            if (!TokenClass.ACCESS_TOKEN.getName().equals(jwtClaimsSet.getStringClaim("token_class"))) {
                throw new TokenValidationException(TokenValidationError.INVALID_TOKEN_CLASS, "Acess token class must be \"access_token\".");
            }
        } catch (java.text.ParseException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Parse exception when getting a claim from claim set.", e);
        }

        return new ResourceServerAccessToken(jwtClaimsSet);
    }

    // all claim strings should be moved to common for sharing between client and server
    /**
     * Get group claims
     *
     * @return                          List of groups
     */
    @SuppressWarnings("unchecked")
    public List<String> getGroups() {
        return (List<String>) getClaim("groups");
    }

    /**
     * Get token class claim
     *
     * @return                          Token class
     */
    public String getTokenClass() {
        return (String) getClaim("token_class");
    }

    /**
     * Get token type claim
     *
     * @return                          Token type
     */
    public String getTokenType() {
        return (String) getClaim("token_type");
    }

    /**
     * Get subject claim
     *
     * @return                          Subject
     */
    public String getSubject() {
        return this.jwtClaimsSet.getSubject();
    }

    /**
     * Get issuer claim
     *
     * @return                          Token issuer
     */
    public String getIssuer() {
        return this.jwtClaimsSet.getIssuer();
    }

    /**
     * Get audience claim
     *
     * @return                          Token audience
     */
    public List<String> getAudience() {
        return this.jwtClaimsSet.getAudience();
    }

    /**
     * Get issue time  claim
     *
     * @return                          Token issue time
     */
    public Date getIssueTime() {
        return this.jwtClaimsSet.getIssueTime();
    }

    /**
     * Get expiration claim
     *
     * @return                          Token expiration time
     */
    public Date getExpirationTime() {
        return this.jwtClaimsSet.getExpirationTime();
    }

    /**
     * Get JWT id claim
     *
     * @return                          JWT id
     */
    public String getJWTID() {
        return this.jwtClaimsSet.getJWTID();
    }

    /**
     * Get a general claim
     *
     * @return                          Claim object
     */
    public Object getClaim(String claim) {
        return this.jwtClaimsSet.getClaim(claim);
    }
}
