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

package com.vmware.identity.openidconnect.server;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.oauth2.sdk.id.JWTID;
import com.nimbusds.oauth2.sdk.id.Subject;
import com.nimbusds.oauth2.sdk.token.AccessToken;
import com.nimbusds.oauth2.sdk.token.BearerAccessToken;
import com.nimbusds.oauth2.sdk.token.RefreshToken;
import com.nimbusds.openid.connect.sdk.Nonce;
import com.vmware.identity.openidconnect.common.HolderOfKeyAccessToken;
import com.vmware.identity.openidconnect.common.IDToken;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * @author Yehia Zayour
 */
public class TokenIssuer {
    private TokenIssuer() {
    }

    public static IDToken issueIdToken(
            PersonUser personUser,
            SolutionUser solutionUser,
            UserInformation userInfo,
            TenantInformation tenantInfo,
            ClientID clientId,
            Scope scope,
            Nonce nonce,
            SessionID sessionId) throws ServerException {
        Validate.isTrue(personUser != null || solutionUser != null, "personUser and solutionUser should not both be null");
        Validate.notNull(userInfo, "userInfo");
        Validate.notNull(tenantInfo, "tenantInfo");
        Validate.notNull(scope, "scope");

        long lifeTimeMs = (solutionUser != null) ?
                tenantInfo.getIdTokenHokLifetimeMs() :
                tenantInfo.getIdTokenBearerLifetimeMs();

        JWTClaimsSet claimsSet = commonClaims(personUser, solutionUser, tenantInfo, clientId, scope, nonce, sessionId, lifeTimeMs, TokenClass.ID_TOKEN);

        if (personUser != null) {
            claimsSet.setClaim("given_name", userInfo.getGivenName());
            claimsSet.setClaim("family_name", userInfo.getFamilyName());
        }
        if (scope.contains(ScopeValue.ID_TOKEN_GROUPS.getName())) {
            String claimName = ScopeValue.ID_TOKEN_GROUPS.getMappedClaimName();
            claimsSet.setClaim(claimName, userInfo.getGroupMembership());
        }

        SignedJWT signedJwt = Shared.sign(claimsSet, tenantInfo.getPrivateKey());
        return new IDToken(signedJwt);
    }

    public static AccessToken issueAccessToken(
            PersonUser personUser,
            SolutionUser solutionUser,
            UserInformation userInfo,
            TenantInformation tenantInfo,
            ClientID clientId,
            Scope scope,
            Nonce nonce) throws ServerException {
        Validate.isTrue(personUser != null || solutionUser != null, "personUser and solutionUser should not both be null");
        Validate.notNull(userInfo, "userInfo");
        Validate.notNull(tenantInfo, "tenantInfo");
        Validate.notNull(scope, "scope");

        long lifeTimeMs = (solutionUser != null) ?
                tenantInfo.getAccessTokenHokLifetimeMs() :
                tenantInfo.getAccessTokenBearerLifetimeMs();

        JWTClaimsSet claimsSet = commonClaims(personUser, solutionUser, tenantInfo, clientId, scope, nonce, (SessionID) null, lifeTimeMs, TokenClass.ACCESS_TOKEN);

        if (scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS.getName())) {
            String claimName = ScopeValue.ACCESS_TOKEN_GROUPS.getMappedClaimName();
            claimsSet.setClaim(claimName, userInfo.getGroupMembership());
        }
        if (scope.contains(ScopeValue.RESOURCE_SERVER_ADMIN_SERVER.getName())) {
            String claimName = ScopeValue.RESOURCE_SERVER_ADMIN_SERVER.getMappedClaimName();
            claimsSet.setClaim(claimName, userInfo.getAdminServerRole());
        }

        SignedJWT signedJWT = Shared.sign(claimsSet, tenantInfo.getPrivateKey());
        return (solutionUser != null) ?
                new HolderOfKeyAccessToken(signedJWT.serialize(), lifeTimeMs / 1000L) :
                new BearerAccessToken(signedJWT.serialize(), lifeTimeMs / 1000L, (Scope) null);
    }

    public static RefreshToken issueRefreshToken(
            PersonUser personUser,
            SolutionUser solutionUser,
            TenantInformation tenantInfo,
            ClientID clientId,
            Scope scope,
            SessionID sessionId) throws ServerException {
        Validate.notNull(personUser, "personUser");
        Validate.notNull(tenantInfo, "tenantInfo");
        Validate.notNull(scope, "scope");

        long lifeTimeMs = (solutionUser != null) ?
                tenantInfo.getRefreshTokenHokLifetimeMs() :
                tenantInfo.getRefreshTokenBearerLifetimeMs();
        JWTClaimsSet claimsSet = commonClaims(personUser, solutionUser, tenantInfo, clientId, scope, (Nonce) null, sessionId, lifeTimeMs, TokenClass.REFRESH_TOKEN);
        SignedJWT signedJWT = Shared.sign(claimsSet, tenantInfo.getPrivateKey());
        return new RefreshToken(signedJWT.serialize());
    }

    private static JWTClaimsSet commonClaims(
            PersonUser personUser,
            SolutionUser solutionUser,
            TenantInformation tenantInfo,
            ClientID clientId,
            Scope scope,
            Nonce nonce,
            SessionID sessionId,
            long lifeTimeMs,
            TokenClass tokenClass) {
        Date now = new Date();

        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setClaim("token_class", tokenClass.getName());
        claimsSet.setClaim("token_type", (solutionUser != null) ? TokenType.HOK.getName() : TokenType.BEARER.getName());
        claimsSet.setJWTID((new JWTID()).toString());

        // bind the public key to the hok access token by inserting it as a claim
        if (solutionUser != null) {
            RSAKey rsaKey = new RSAKey(solutionUser.getPublicKey(), KeyUse.SIGNATURE, null, JWSAlgorithm.RS256, null, null, null, null);
            claimsSet.setClaim("hotk", (new JWKSet(rsaKey)).toJSONObject());
        }

        // this claim represents the identity of the solution user Acting As the person user
        if (personUser != null && solutionUser != null) {
            claimsSet.setClaim("act_as", solutionUser.getSubject().getValue());
        }

        claimsSet.setClaim("tenant", tenantInfo.getName());
        claimsSet.setIssuer(tenantInfo.getIssuer().getValue());

        Subject subject = (personUser != null) ? personUser.getSubject() : solutionUser.getSubject();
        claimsSet.setSubject(subject.getValue());

        List<String> audience = new ArrayList<String>();
        if (clientId != null) {
            audience.add(clientId.getValue());
        } else if (solutionUser != null) {
            audience.add(solutionUser.getSubject().getValue());
        } else {
            audience.add(personUser.getSubject().getValue());
        }
        if (tokenClass == TokenClass.ACCESS_TOKEN) {
            for (String scopeValue : scope.toStringList()) {
                if (scopeValue.startsWith("rs_")) {
                    audience.add(scopeValue);
                }
            }
        }
        claimsSet.setAudience(audience);

        claimsSet.setIssueTime(now);
        claimsSet.setExpirationTime(new Date(now.getTime() + lifeTimeMs));
        claimsSet.setClaim("scope", scope.toString());

        if (clientId != null) {
            claimsSet.setClaim("client_id", clientId.getValue());
        }
        if (nonce != null) {
            claimsSet.setClaim("nonce", nonce.getValue());
        }
        if (sessionId != null) {
            claimsSet.setClaim("sid", sessionId.getValue());
        }

        return claimsSet;
    }
}
