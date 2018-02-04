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
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.TokenClass;

import net.minidev.json.JSONObject;

public class FederationToken {

    private static final String CLAIM_CONTEXT_NAME = "context_name";
    private static final String CLAIM_USER_NAME = "username";
    private static final String CLAIM_DOMAIN = "domain";
    private static final String CLAIM_EMAIL = "email";
    private static final String CLAIM_PERMS = "perms";

    private final TokenClass tokenClass;
    private final String jwtId;
    private final String issuer;
    private final String subject;
    private final Date issueTime;
    private final Date expirationTime;
    private String orgId;
    private String userId;
    private String domain;
    private String emailId;
    private final Set<String> permissions;

    private SignedJWT signedJwt;

    private FederationToken(TokenClass tokenClass, SignedJWT signedJwt) throws ParseException {
        Validate.notNull(tokenClass, "tokenClass");
        Validate.notNull(signedJwt, "signedJwt");
        this.tokenClass = tokenClass;
        this.signedJwt = signedJwt;
        JWTClaimsSet claims = JWTUtils.getClaimsSet(signedJwt);
        this.jwtId = claims.getJWTID();
        this.issuer = claims.getIssuer();
        this.issueTime = claims.getIssueTime();
        this.expirationTime = claims.getExpirationTime();
        if (claims.getClaims().containsKey(CLAIM_CONTEXT_NAME)) {
            this.orgId = JWTUtils.getString(claims, tokenClass, CLAIM_CONTEXT_NAME);
        }
        if (claims.getClaims().containsKey(CLAIM_USER_NAME)) {
            this.userId = JWTUtils.getString(claims, tokenClass, CLAIM_USER_NAME);
        }
        if (claims.getClaims().containsKey(CLAIM_DOMAIN)) {
            this.domain = JWTUtils.getString(claims, tokenClass, CLAIM_DOMAIN);
        }
        if (claims.getClaims().containsKey(CLAIM_EMAIL)) {
            this.emailId = JWTUtils.getString(claims, tokenClass, CLAIM_EMAIL);
        }
        if (claims.getClaims().containsKey(CLAIM_PERMS)) {
            String[] permissions = JWTUtils.getStringArray(claims, tokenClass, CLAIM_PERMS);
            this.permissions = new HashSet<>(Arrays.asList(permissions));
        } else {
            this.permissions = Collections.emptySet();
        }
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

    public String getOrgId() throws ParseException {
    	return this.orgId;
    }

    public String getUserId() throws ParseException {
    	return this.userId;
    }

    public String getDomain() throws ParseException {
    	return this.domain;
    }

    public String getEmailAddress() throws ParseException {
        return this.emailId;
    }

    public Set<String> getPermissions() throws Exception {
        return this.permissions;
    }

    public String serialize() throws IllegalArgumentException {
        return this.signedJwt.serialize();
    }

    public static FederationToken parse(JSONObject jsonObject, TokenClass tokenClass) throws ParseException {
      Validate.notNull(jsonObject, "jsonObject");
      return new FederationToken(tokenClass, JSONUtils.getSignedJWT(jsonObject, tokenClass.getValue()));
    }

    public static FederationToken parse( Map<String, String> parameters,  TokenClass tokenClass) throws ParseException {
      Validate.notNull(parameters, "parameters");
      return new FederationToken(tokenClass, ParameterMapUtils.getSignedJWT(parameters, tokenClass.getValue()));
    }

    public static FederationToken parse(String signedJwtString, TokenClass tokenClass) throws ParseException {
      Validate.notEmpty(signedJwtString, "signedJwtString");
      return new FederationToken(tokenClass, JWTUtils.parseSignedJWT(signedJwtString));
    }

    public static FederationToken parse(SignedJWT signedJwt, TokenClass tokenClass) throws ParseException {
      Validate.notNull(signedJwt, "signedJwt");
      return new FederationToken(tokenClass, signedJwt);
    }
  }
