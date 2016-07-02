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
public final class IDToken extends ServerIssuedToken {
    private static final TokenClass TOKEN_CLASS = TokenClass.ID_TOKEN;

    private final SignedJWT signedJwt;

    private final Collection<String> groups;
    private final String givenName;
    private final String familyName;

    private IDToken(SignedJWT signedJwt) throws ParseException {
        super(TOKEN_CLASS, signedJwt);

        this.signedJwt = signedJwt;
        JWTClaimsSet claims = JWTUtils.getClaimsSet(this.signedJwt);

        String[] groupsStringArray = null;
        if (claims.getClaims().containsKey("groups")) {
            groupsStringArray = JWTUtils.getStringArray(claims, TOKEN_CLASS, "groups");
        }
        this.groups = (groupsStringArray == null) ? null : Collections.unmodifiableList(Arrays.asList(groupsStringArray));

        String givenName = null;
        if (claims.getClaims().containsKey("given_name")) {
            givenName = JWTUtils.getString(claims, TOKEN_CLASS, "given_name");
        }
        this.givenName = givenName;

        String familyName = null;
        if (claims.getClaims().containsKey("family_name")) {
            familyName = JWTUtils.getString(claims, TOKEN_CLASS, "family_name");
        }
        this.familyName = familyName;
    }

    public IDToken(
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
            String givenName,
            String familyName) throws JOSEException {
        super(TOKEN_CLASS, tokenType, jwtId, issuer, subject, audience, issueTime, expirationTime, scope, tenant, clientId, sessionId, holderOfKey, actAs, nonce);

        Validate.notNull(privateKey, "privateKey");

        this.groups = (groups == null) ? null : Collections.unmodifiableCollection(groups);
        this.givenName = givenName;
        this.familyName = familyName;

        JWTClaimsSet.Builder claimsBuilder = super.claimsBuilder();
        if (this.groups != null) {
            claimsBuilder = claimsBuilder.claim("groups", this.groups);
        }
        if (this.givenName != null) {
            claimsBuilder = claimsBuilder.claim("given_name", this.givenName);
        }
        if (this.familyName != null) {
            claimsBuilder = claimsBuilder.claim("family_name", this.familyName);
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

    public String getGivenName() {
        return this.givenName;
    }

    public String getFamilyName() {
        return this.familyName;
    }

    public static IDToken parse(JSONObject jsonObject) throws ParseException {
        Validate.notNull(jsonObject, "jsonObject");
        return new IDToken(JSONUtils.getSignedJWT(jsonObject, "id_token"));
    }

    public static IDToken parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");
        return new IDToken(ParameterMapUtils.getSignedJWT(parameters, "id_token"));
    }

    public static IDToken parse(String signedJwtString) throws ParseException {
        Validate.notEmpty(signedJwtString, "signedJwtString");
        return new IDToken(JWTUtils.parseSignedJWT(signedJwtString));
    }

    public static IDToken parse(SignedJWT signedJwt) throws ParseException {
        Validate.notNull(signedJwt, "signedJwt");
        return new IDToken(signedJwt);
    }
}