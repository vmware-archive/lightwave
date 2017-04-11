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
package com.vmware.identity.rest.core.server.test.util;

import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.JWSSigner;
import com.nimbusds.jose.crypto.RSASSASigner;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;

public class JWTBuilder {

    public static String ACCESS_TOKEN_CLASS = "access_token";
    public static String GROUPS_CLAIM = "groups";
    public static String HOK_CLAIM = "hotk";
    public static String ROLE_CLAIM = "admin_server_role";
    public static String TOKEN_CLASS_CLAIM = "token_class";
    public static String TOKEN_TYPE_CLAIM = "token_type";

    private String tokenClass;
    private TokenType type;
    private PrivateKey privateKey;
    private String subject;
    private String issuer;
    private Date issuedAt;
    private int duration;
    private List<String> audiences;
    private List<String> groups;
    private String role;
    private JWKSet hotk;

    public JWTBuilder(PrivateKey privateKey) {
        this(ACCESS_TOKEN_CLASS, TokenType.BEARER, privateKey);
    }

    public JWTBuilder(TokenType type, PrivateKey privateKey) {
        this(ACCESS_TOKEN_CLASS, type, privateKey);
    }

    public JWTBuilder(String tokenClass, TokenType type, PrivateKey privateKey) {
        this.tokenClass = tokenClass;
        this.type = type;
        this.privateKey = privateKey;
        this.duration = 10;
        this.audiences = new LinkedList<String>();
        this.groups = new LinkedList<String>();
    }

    public JWTBuilder subject(String subject) {
        this.subject = subject;
        return this;
    }

    public JWTBuilder issuer(String issuer) {
        this.issuer = issuer;
        return this;
    }

    public JWTBuilder privateKey(PrivateKey privateKey) {
        this.privateKey = privateKey;
        return this;
    }

    public JWTBuilder issuedAt(Date issuedAt) {
        this.issuedAt = issuedAt;
        return this;
    }

    public JWTBuilder duration(int duration) {
        this.duration = duration;
        return this;
    }

    public JWTBuilder audience(String audience) {
        this.audiences.add(audience);
        return this;
    }

    public JWTBuilder audiences(List<String> audiences) {
        this.audiences = audiences;
        return this;
    }

    public JWTBuilder group(String group) {
        this.groups.add(group);
        return this;
    }

    public JWTBuilder groups(List<String> groups) {
        this.groups = groups;
        return this;
    }

    public JWTBuilder role(String role) {
        this.role = role;
        return this;
    }

    public JWTBuilder hotk(PublicKey key) {
        RSAKey rsaKey = new RSAKey((RSAPublicKey) key, KeyUse.SIGNATURE, null, JWSAlgorithm.RS256, null, null, null, null);
        this.hotk = new JWKSet(rsaKey);
        return this;
    }

    public SignedJWT build() throws JOSEException {
        JWSSigner signer = new RSASSASigner((RSAPrivateKey) this.privateKey);

        JWTClaimsSet claimsSet;
        JWTClaimsSet.Builder jwtTokenBuilder = new JWTClaimsSet.Builder()
        .subject(this.subject)
        .issuer(this.issuer)
        .claim(TOKEN_CLASS_CLAIM, this.tokenClass)
        .claim(TOKEN_TYPE_CLAIM, this.type.getJsonName());

        if (!this.groups.isEmpty()) {
            jwtTokenBuilder.claim(GROUPS_CLAIM, this.groups);
        }

        if (this.hotk != null) {
            jwtTokenBuilder.claim(HOK_CLAIM, this.hotk.toJSONObject());
        }

        Calendar cal = Calendar.getInstance();
        if (this.issuedAt != null) {
            cal.setTime(this.issuedAt);
        } else {
            this.issuedAt = cal.getTime();
        }

        cal.add(Calendar.MINUTE, this.duration);
        Date expiresAt = cal.getTime();

        jwtTokenBuilder.issueTime(this.issuedAt);
        jwtTokenBuilder.expirationTime(expiresAt);
        jwtTokenBuilder.audience(this.audiences);

        if (this.role != null) {
            jwtTokenBuilder.claim(ROLE_CLAIM, this.role);
        }

        claimsSet = jwtTokenBuilder.build();
        SignedJWT signedJWT = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claimsSet);
        signedJWT.sign(signer);

        return signedJWT;
    }

}
