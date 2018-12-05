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

import java.net.URI;
import java.net.URISyntaxException;
import java.security.interfaces.RSAPublicKey;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.*;
import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.openidconnect.protocol.AccessToken;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.RefreshToken;

import javax.print.URIException;

/**
 * @author Yehia Zayour
 */
public class TokenIssuer {
    private final PersonUser personUser;
    private final SolutionUser solutionUser;
    private final UserInfo userInfo;
    private final TenantInfo tenantInfo;
    private final Scope scope;
    private final Nonce nonce;
    private final ClientID clientId;
    private final SessionID sessionId;
    private final URI requestUri;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TokenIssuer.class);

    public TokenIssuer(
            PersonUser personUser,
            SolutionUser solutionUser,
            UserInfo userInfo,
            TenantInfo tenantInfo,
            Scope scope,
            Nonce nonce,
            ClientID clientId,
            SessionID sessionId) {
        Validate.isTrue(personUser != null || solutionUser != null, "personUser and solutionUser should not both be null");
        Validate.notNull(userInfo, "userInfo");
        Validate.notNull(tenantInfo, "tenantInfo");
        Validate.notNull(scope, "scope");
        // nullable nonce
        // nullable clientId
        // nullable sessionId

        this.personUser = personUser;
        this.solutionUser = solutionUser;
        this.userInfo = userInfo;
        this.tenantInfo = tenantInfo;
        this.scope = scope;
        this.nonce = nonce;
        this.clientId = clientId;
        this.sessionId = sessionId;
        this.requestUri = null;
    }

    /**
     Initializes TokenIssuer with the request's uri for the token flow. If STS is configured to use a secondary issuer
     matching the passed in parameter, the resulting token will have the secondary issuer.
     TODO remove once endpoint migration complete
     */
    public TokenIssuer(
            PersonUser personUser,
            SolutionUser solutionUser,
            UserInfo userInfo,
            TenantInfo tenantInfo,
            Scope scope,
            Nonce nonce,
            ClientID clientId,
            SessionID sessionId,
            URI requestUri) {
        Validate.isTrue(personUser != null || solutionUser != null, "personUser and solutionUser should not both be null");
        Validate.notNull(userInfo, "userInfo");
        Validate.notNull(tenantInfo, "tenantInfo");
        Validate.notNull(scope, "scope");
        Validate.notNull(requestUri, "requestUri");
        // nullable nonce
        // nullable clientId
        // nullable sessionId

        this.personUser = personUser;
        this.solutionUser = solutionUser;
        this.userInfo = userInfo;
        this.tenantInfo = tenantInfo;
        this.scope = scope;
        this.nonce = nonce;
        this.clientId = clientId;
        this.sessionId = sessionId;
        this.requestUri = requestUri;
    }

    // Gets the appropriate issuer claim to use for the token
    private Issuer getIssuer() {
        Issuer issuer = this.tenantInfo.getIssuer();
        if (this.requestUri == null) {
            return issuer;
        }

        String secondaryIssuer = issuer.getSecondaryIssuer();
        if (secondaryIssuer != null && !secondaryIssuer.isEmpty()) {
            // If the incoming request matches the configured secondary issuer, use that instead
            try {
                URI issuerURI = new URI(secondaryIssuer);
                if (issuerURI.getHost().equalsIgnoreCase(this.requestUri.getHost())) {
                    return new Issuer(secondaryIssuer);
                }
            } catch (URISyntaxException e) {
                logger.error("Failed to parse configured issuer URI: {}", secondaryIssuer);
            }
        }
        return issuer;
    }

    public IDToken issueIDToken() throws ServerException {
        Date now = new Date();
        Date issueTime = now;
        long lifeTimeMs = (this.solutionUser != null) ?
                this.tenantInfo.getIDTokenHokLifetimeMs() :
                this.tenantInfo.getIDTokenBearerLifetimeMs();
        Date expirationTime = new Date(now.getTime() + lifeTimeMs);

        Collection<String> groups = null;
        if (this.scope.contains(ScopeValue.ID_TOKEN_GROUPS)) {
            groups = this.userInfo.getGroupMembership();
        } else if (this.scope.contains(ScopeValue.ID_TOKEN_GROUPS_FILTERED)) {
            groups = this.userInfo.getGroupMembershipFiltered() != null ?
                    this.userInfo.getGroupMembershipFiltered() :
                    this.userInfo.getGroupMembership();
        }

        String givenName = null;
        String familyName = null;
        if (this.personUser != null) {
            givenName = this.userInfo.getGivenName();
            familyName = this.userInfo.getFamilyName();
        }

        try {
            return new IDToken(
                    this.tenantInfo.getPrivateKey(),
                    tokenType(),
                    new JWTID(),
                    getIssuer(),
                    subject(),
                    audience(),
                    issueTime,
                    expirationTime,
                    this.scope,
                    this.tenantInfo.getName(),
                    this.clientId,
                    this.sessionId,
                    holderOfKey(),
                    actAs(),
                    this.nonce,
                    groups,
                    givenName,
                    familyName,
                    multiTenant());
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError("faild to sign id_token"), e);
        }
    }

    public AccessToken issueAccessToken() throws ServerException {
        Date now = new Date();
        Date issueTime = now;
        long lifeTimeMs = (this.solutionUser != null) ?
                this.tenantInfo.getAccessTokenHokLifetimeMs() :
                this.tenantInfo.getAccessTokenBearerLifetimeMs();
        Date expirationTime = new Date(now.getTime() + lifeTimeMs);

        List<String> audience = audience();
        for (ScopeValue scopeValue : this.scope.getScopeValues()) {
            if (scopeValue.denotesResourceServer()) {
                audience.add(scopeValue.getValue());
            }
        }

        Collection<String> groups = null;
        if (this.scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS)) {
            groups = this.userInfo.getGroupMembership();
        } else if (this.scope.contains(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED)) {
            groups = this.userInfo.getGroupMembershipFiltered() != null ?
                    this.userInfo.getGroupMembershipFiltered() :
                    this.userInfo.getGroupMembership();
        }

        String adminServerRole = null;
        if (this.scope.contains(ScopeValue.RESOURCE_SERVER_ADMIN_SERVER)) {
            adminServerRole = this.userInfo.getAdminServerRole();
        }

        try {
            return new AccessToken(
                    this.tenantInfo.getPrivateKey(),
                    tokenType(),
                    new JWTID(),
                    getIssuer(),
                    subject(),
                    audience,
                    issueTime,
                    expirationTime,
                    this.scope,
                    this.tenantInfo.getName(),
                    this.clientId,
                    this.sessionId,
                    holderOfKey(),
                    actAs(),
                    this.nonce,
                    groups,
                    adminServerRole,
                    multiTenant());
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError("faild to sign access_token"), e);
        }
    }

    public RefreshToken issueRefreshToken() throws ServerException {
        Date now = new Date();
        Date issueTime = now;
        long lifeTimeMs = (this.solutionUser != null) ?
                this.tenantInfo.getRefreshTokenHokLifetimeMs() :
                this.tenantInfo.getRefreshTokenBearerLifetimeMs();
        Date expirationTime = new Date(now.getTime() + lifeTimeMs);

        try {
            return new RefreshToken(
                    this.tenantInfo.getPrivateKey(),
                    tokenType(),
                    new JWTID(),
                    getIssuer(),
                    subject(),
                    audience(),
                    issueTime,
                    expirationTime,
                    this.scope,
                    this.tenantInfo.getName(),
                    this.clientId,
                    this.sessionId,
                    holderOfKey(),
                    actAs(),
                    this.nonce);
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError("faild to sign refresh_token"), e);
        }
    }

    private TokenType tokenType() {
        return (this.solutionUser != null) ? TokenType.HOK : TokenType.BEARER;
    }

    private Subject subject() {
        return (this.personUser != null) ? this.userInfo.getSubject() : this.solutionUser.getSubject();
    }

    private boolean multiTenant() {
        return (this.personUser == null) && (this.solutionUser != null) && (this.solutionUser.isMultiTenant());
    }

    private List<String> audience() {
        List<String> audience = new ArrayList<String>();
        if (this.clientId != null) {
            audience.add(this.clientId.getValue());
        } else if (this.solutionUser != null) {
            audience.add(this.solutionUser.getSubject().getValue());
        } else {
            audience.add(this.personUser.getSubject().getValue());
        }
        return audience;
    }

    private RSAPublicKey holderOfKey() {
        return (this.solutionUser != null) ? this.solutionUser.getPublicKey() : null;
    }

    private Subject actAs() {
        Subject actAs = null;
        if (this.personUser != null && this.solutionUser != null) {
            actAs = this.solutionUser.getSubject();
        }
        return actAs;
    }
}