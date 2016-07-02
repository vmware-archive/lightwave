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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
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
public final class AccessToken extends ServerIssuedToken {
    private static final TokenClass TOKEN_CLASS = TokenClass.ACCESS_TOKEN;

    private final SignedJWT signedJwt;

    private final Collection<String> groups;
    private final String adminServerRole;

    private AccessToken(SignedJWT signedJwt) throws ParseException {
        super(TOKEN_CLASS, signedJwt);

        this.signedJwt = signedJwt;
        JWTClaimsSet claims = JWTUtils.getClaimsSet(this.signedJwt);

        String[] groupsStringArray = null;
        if (claims.getClaims().containsKey("groups")) {
            groupsStringArray = JWTUtils.getStringArray(claims, TOKEN_CLASS, "groups");
        }
        this.groups = (groupsStringArray == null) ? null : Collections.unmodifiableList(Arrays.asList(groupsStringArray));

        String adminServerRole = null;
        if (claims.getClaims().containsKey("admin_server_role")) {
            adminServerRole = JWTUtils.getString(claims, TOKEN_CLASS, "admin_server_role");
        }
        this.adminServerRole = adminServerRole;
    }

    public AccessToken(
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
            Nonce nonce,

            Collection<String> groups,
            String adminServerRole) throws JOSEException {
        super(TOKEN_CLASS, tokenType, jwtId, issuer, subject, audience, issueTime, expirationTime, scope, tenant, clientId, sessionId, holderOfKey, actAs, nonce);

        Validate.notNull(privateKey, "privateKey");

        this.groups = (groups == null) ? null : Collections.unmodifiableCollection(groups);
        this.adminServerRole = adminServerRole;

        JWTClaimsSet.Builder claimsBuilder = super.claimsBuilder();
        if (this.groups != null) {
            claimsBuilder = claimsBuilder.claim("groups", this.groups);
        }
        if (this.adminServerRole != null) {
            claimsBuilder = claimsBuilder.claim("admin_server_role", this.adminServerRole);
        }

        this.signedJwt = JWTUtils.signClaimsSet(claimsBuilder.build(), privateKey);
    }

    @Override
    protected SignedJWT getSignedJWT() {
        return this.signedJwt;
    }

    public Collection<String> getGroups() {
        return this.groups;
    }

    public String getAdminServerRole() {
        return this.adminServerRole;
    }

    public static AccessToken parse(JSONObject jsonObject) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");

        SignedJWT signedJwt = JSONUtils.getSignedJWT(jsonObject, "access_token");
        TokenType tokenType = TokenType.parse(JSONUtils.getString(jsonObject, "token_type"));
        long expiresIn = JSONUtils.getLong(jsonObject, "expires_in");

        AccessToken accessToken = new AccessToken(signedJwt);
        if (accessToken.getTokenType() != tokenType) {
            throw new ParseException("token_type from json does not match that from jwt");
        }
        if (accessToken.getLifetimeSeconds() != expiresIn) {
            throw new ParseException("expires_in from json does not match that from jwt");
        }
        return accessToken;
    }

    public static AccessToken parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        SignedJWT signedJwt = ParameterMapUtils.getSignedJWT(parameters, "access_token");
        TokenType tokenType = TokenType.parse(ParameterMapUtils.getString(parameters, "token_type"));
        long expiresIn = ParameterMapUtils.getLong(parameters, "expires_in");

        AccessToken accessToken = new AccessToken(signedJwt);
        if (accessToken.getTokenType() != tokenType) {
            throw new ParseException("token_type from parameter does not match that from jwt");
        }
        if (accessToken.getLifetimeSeconds() != expiresIn) {
            throw new ParseException("expires_in from parameter does not match that from jwt");
        }
        return accessToken;
    }

    public static AccessToken parse(String signedJwtString) throws ParseException {
        Validate.notEmpty(signedJwtString, "signedJwtString");
        return new AccessToken(JWTUtils.parseSignedJWT(signedJwtString));
    }

    public static AccessToken parse(SignedJWT signedJwt) throws ParseException {
        Validate.notNull(signedJwt, "signedJwt");
        return new AccessToken(signedJwt);
    }
}