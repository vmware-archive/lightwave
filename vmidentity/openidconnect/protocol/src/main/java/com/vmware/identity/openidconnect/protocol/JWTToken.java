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
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.Subject;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * @author Yehia Zayour
 */
public abstract class JWTToken {
    private final TokenClass tokenClass;
    private final TokenType tokenType;
    private final JWTID jwtId;
    private final Issuer issuer;
    private final Subject subject;
    private final List<String> audience;
    private final Date issueTime;

    protected JWTToken(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notNull(signedJwt, "signedJwt");

        this.tokenClass = tokenClass;

        JWTClaimsSet claims = JWTUtils.getClaimsSet(signedJwt);

        String tokenClassStringClaim = JWTUtils.getString(claims, this.tokenClass, "token_class");
        TokenClass tokenClassClaim;
        try {
            tokenClassClaim = TokenClass.parse(tokenClassStringClaim);
        } catch (ParseException e) {
            throw new ParseException(this.tokenClass.getValue() + " has invalid token_class claim", e);
        }
        if (tokenClassClaim != this.tokenClass) {
            throw new ParseException(this.tokenClass.getValue() + " has incorrect token_class claim");
        }

        String tokenTypeString = JWTUtils.getString(claims, this.tokenClass, "token_type");
        try {
            this.tokenType = TokenType.parse(tokenTypeString);
        } catch (ParseException e) {
            throw new ParseException(this.tokenClass.getValue() + " has invalid token_type claim", e);
        }

        this.jwtId = new JWTID(JWTUtils.getString(claims, this.tokenClass, "jti"));
        this.issuer = new Issuer(JWTUtils.getString(claims, this.tokenClass, "iss"));
        this.subject = new Subject(JWTUtils.getString(claims, this.tokenClass, "sub"));
        this.audience = Collections.unmodifiableList(Arrays.asList(JWTUtils.getStringArray(claims, this.tokenClass, "aud")));
        this.issueTime = JWTUtils.getIssueTime(claims, this.tokenClass);

        if (this.audience.isEmpty()) {
            throw new ParseException(this.tokenClass.getValue() + " aud must be non-empty");
        }
    }

    protected JWTToken(
            TokenClass tokenClass,
            TokenType tokenType,
            JWTID jwtId,
            Issuer issuer,
            Subject subject,
            List<String> audience,
            Date issueTime) {
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notNull(tokenType, "tokenType");
        Validate.notNull(jwtId, "jwtId");
        Validate.notNull(issuer, "issuer");
        Validate.notNull(subject, "subject");
        Validate.notNull(audience, "audience");
        Validate.notNull(issueTime, "issueTime");

        this.tokenClass = tokenClass;
        this.tokenType = tokenType;
        this.jwtId = jwtId;
        this.issuer = issuer;
        this.subject = subject;
        this.audience = Collections.unmodifiableList(audience);
        this.issueTime = new Date(issueTime.getTime());

        Validate.notEmpty(this.audience);
    }

    protected JWTClaimsSet.Builder claimsBuilder() {
        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();

        claimsBuilder = claimsBuilder.claim("token_class", this.tokenClass.getValue());
        claimsBuilder = claimsBuilder.claim("token_type", this.tokenType.getValue());
        claimsBuilder = claimsBuilder.jwtID(this.jwtId.getValue());
        claimsBuilder = claimsBuilder.issuer(this.issuer.getValue());
        claimsBuilder = claimsBuilder.subject(this.subject.getValue());
        claimsBuilder = claimsBuilder.audience(this.audience);
        claimsBuilder = claimsBuilder.issueTime(this.issueTime);

        return claimsBuilder;
    }

    protected abstract SignedJWT getSignedJWT();

    public boolean hasValidSignature(RSAPublicKey publicKey) throws JOSEException {
        Validate.notNull(publicKey, "publicKey");
        return getSignedJWT().verify(new RSASSAVerifier(publicKey));
    }

    public String serialize() {
        return getSignedJWT().serialize();
    }

    public TokenClass getTokenClass() {
        return this.tokenClass;
    }

    public TokenType getTokenType() {
        return this.tokenType;
    }

    public JWTID getJWTID() {
        return this.jwtId;
    }

    public Issuer getIssuer() {
        return this.issuer;
    }

    public Subject getSubject() {
        return this.subject;
    }

    public List<String> getAudience() {
        return this.audience;
    }

    public Date getIssueTime() {
        return new Date(this.issueTime.getTime());
    }
}