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
import java.util.Date;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSVerifier;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * Id Token
 *
 * @author Jun Sun
 */
public class IDToken {

    private final SignedJWT signedJWT;
    private final ReadOnlyJWTClaimsSet jwtClaimsSet;

    private IDToken(SignedJWT signedJWT, ReadOnlyJWTClaimsSet jwtClaimsSet) {
        this.signedJWT = signedJWT;
        this.jwtClaimsSet = jwtClaimsSet;
    }

    static IDToken build(
            SignedJWT signedJWT,
            RSAPublicKey providerPublicKey,
            ClientID clientId,
            Issuer issuer) throws TokenValidationException {
        Validate.notNull(signedJWT, "signedJWT");
        Validate.notNull(providerPublicKey, "providerPublicKey");
        Validate.notNull(issuer, "issuer");

        // verify the signature
        JWSVerifier verifier = new RSASSAVerifier(providerPublicKey);
        try {
            if (!signedJWT.verify(verifier)) {
                throw new TokenValidationException(TokenValidationError.INVALID_SIGNATURE, "Token signature validation failed.");
            }
        } catch (JOSEException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Token Signature verification process failed." + e.getMessage(), e);
        }

        // verify claims
        Date now = new Date();
        ReadOnlyJWTClaimsSet jwtClaimsSet = null;
        try {
            jwtClaimsSet = signedJWT.getJWTClaimsSet();

            // only validate audience for HOK tokens
            if (jwtClaimsSet.getStringClaim("token_type").equals(TokenType.HOK.getName())) {
                if (clientId != null && !jwtClaimsSet.getAudience().contains(clientId.getValue())) {
                    throw new TokenValidationException(TokenValidationError.INVALID_AUDIENCE, "Audience in claim set does not match with client identifier.");
                }
            }

            // TODO: add clock skew
            if ((jwtClaimsSet.getExpirationTime().before(now /* + clockSkew */))) {
                throw new TokenValidationException(TokenValidationError.EXPIRED_TOKEN, "Token is expired.");
            }

            // validate id token class & type
            if (!TokenClass.ID_TOKEN.getName().equals(jwtClaimsSet.getStringClaim("token_class"))) {
                throw new TokenValidationException(TokenValidationError.INVALID_TOKEN_CLASS, "Id token class must be \"id_token\".");
            }
        } catch (java.text.ParseException e) {
            throw new TokenValidationException(TokenValidationError.PARSE_ERROR, "Parse exception when getting a claim from claim set.", e);
        }

        return new IDToken(signedJWT, jwtClaimsSet);
    }

    SignedJWT getSignedJWT() {
        return this.signedJWT;
    }

    /**
     * Get id token value
     *
     * @return                          String value of id token
     */
    public String serialize() {
        return this.signedJWT.serialize();
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
     * Get given name claim
     *
     * @return                          Given name
     */
    public String getGivenName() {
        return (String) getClaim("given_name");
    }

    /**
     * Get family name claim
     *
     * @return                          Family name
     */
    public String getFamilyName() {
        return (String) getClaim("family_name");
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
     * Get nonce claim
     *
     * @return                          Nonce
     */
    public Nonce getNonce() {
        return new Nonce((String) getClaim("nonce"));
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
    public Issuer getIssuer() {
        return new Issuer(this.jwtClaimsSet.getIssuer());
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
