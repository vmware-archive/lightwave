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
import java.util.Date;
import java.util.Objects;

import org.apache.commons.lang3.StringUtils;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jose.util.Base64;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.AuthorizationCode;
import com.nimbusds.oauth2.sdk.AuthorizationCodeGrant;
import com.nimbusds.oauth2.sdk.ClientCredentialsGrant;
import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.GrantType;
import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.RefreshTokenGrant;
import com.nimbusds.oauth2.sdk.ResourceOwnerPasswordCredentialsGrant;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.TokenResponse;
import com.nimbusds.oauth2.sdk.auth.Secret;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.oauth2.sdk.token.AccessToken;
import com.nimbusds.oauth2.sdk.token.RefreshToken;
import com.nimbusds.openid.connect.sdk.Nonce;
import com.nimbusds.openid.connect.sdk.OIDCScopeValue;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.openidconnect.common.AuthenticationRequest;
import com.vmware.identity.openidconnect.common.GssTicketGrant;
import com.vmware.identity.openidconnect.common.HttpRequest;
import com.vmware.identity.openidconnect.common.IDToken;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.SolutionUserCredentialsGrant;
import com.vmware.identity.openidconnect.common.TokenErrorResponse;
import com.vmware.identity.openidconnect.common.TokenRequest;
import com.vmware.identity.openidconnect.common.TokenSuccessResponse;

/**
 * @author Yehia Zayour
 */
public class TokenRequestProcessor {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TokenRequestProcessor.class);

    private final TenantInfoRetriever tenantInfoRetriever;
    private final ClientInfoRetriever clientInfoRetriever;
    private final UserInfoRetriever userInfoRetriever;
    private final PersonUserAuthenticator personUserAuthenticator;
    private final SolutionUserAuthenticator solutionUserAuthenticator;

    private final AuthorizationCodeManager authzCodeManager;
    private final HttpRequest httpRequest;
    private final String tenant;

    private TenantInformation tenantInfo;
    private OIDCClientInformation clientInfo;
    private TokenRequest tokenRequest;

    public TokenRequestProcessor(
            IdmClient idmClient,
            AuthorizationCodeManager authzCodeManager,
            HttpRequest httpRequest,
            String tenant) {
        this.tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        this.clientInfoRetriever = new ClientInfoRetriever(idmClient);
        this.userInfoRetriever = new UserInfoRetriever(idmClient);
        this.personUserAuthenticator = new PersonUserAuthenticator(idmClient);
        this.solutionUserAuthenticator = new SolutionUserAuthenticator(idmClient);

        this.authzCodeManager = authzCodeManager;
        this.httpRequest = httpRequest;
        this.tenant = tenant;

        // set by initialize()
        this.tenantInfo = null;
        this.clientInfo = null;
        this.tokenRequest = null;
    }

    public HttpResponse process() {
        TokenResponse tokenResponse;

        try {
            initialize();
            tokenResponse = processInternal(); // TokenSuccessResponse
        } catch (ServerException e) {
            Shared.logFailedRequest(logger, e);
            tokenResponse = new TokenErrorResponse(e.getErrorObject());
        }

        return HttpResponse.success(tokenResponse);
    }

    private void initialize() throws ServerException {
        try {
            this.tokenRequest = TokenRequest.parse(this.httpRequest);
        } catch (ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription(e.getMessage()), e);
        }

        ErrorObject error = validate();
        if (error != null) {
            throw new ServerException(error);
        }

        String tenantName = this.tenant;
        if (tenantName == null) {
            tenantName = this.tenantInfoRetriever.getDefaultTenantName();
        }
        this.tenantInfo = this.tenantInfoRetriever.retrieveTenantInfo(tenantName);

        if (this.tokenRequest.getClientID() != null) {
            this.clientInfo = this.clientInfoRetriever.retrieveClientInfo(tenantName, this.tokenRequest.getClientID());
        }
    }

    private TokenSuccessResponse processInternal() throws ServerException {
        GrantType grantType = this.tokenRequest.getAuthorizationGrant().getType();

        SolutionUser solutionUser = null;
        try {
            if (this.tokenRequest.getClientAssertion() != null) {
                solutionUser = this.solutionUserAuthenticator.authenticateByClientAssertion(
                        this.tokenRequest.getClientAssertion(),
                        this.httpRequest.getRequestUrl(),
                        this.tenantInfo,
                        this.clientInfo);
            } else if (this.tokenRequest.getSolutionAssertion() != null) {
                solutionUser = this.solutionUserAuthenticator.authenticateBySolutionAssertion(
                        this.tokenRequest.getSolutionAssertion(),
                        this.httpRequest.getRequestUrl(),
                        this.tenantInfo);
            }
        } catch (ServerException e) {
            if ((grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE) || grantType.equals(ClientCredentialsGrant.GRANT_TYPE)) &&
                !Objects.equals(e.getErrorObject().getCode(), OAuth2Error.SERVER_ERROR.getCode())) {
                throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription(e.getErrorObject().getDescription()));
            } else {
                throw e;
            }
        }

        TokenSuccessResponse tokenSuccessResponse;
        if (grantType.equals(GrantType.AUTHORIZATION_CODE)) {
            tokenSuccessResponse = processAuthzCodeGrant(solutionUser);
        } else if (grantType.equals(GrantType.PASSWORD)) {
            tokenSuccessResponse = processPasswordCredentialsGrant(solutionUser);
        } else if (grantType.equals(GrantType.CLIENT_CREDENTIALS)) {
            tokenSuccessResponse = processClientCredentialsGrant(solutionUser);
        } else if (grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE)) {
            tokenSuccessResponse = processSolutionUserCredentialsGrant(solutionUser);
        } else if (grantType.equals(GssTicketGrant.GRANT_TYPE)) {
            tokenSuccessResponse = processGssTicketGrant(solutionUser);
        } else if (grantType.equals(GrantType.REFRESH_TOKEN)) {
            tokenSuccessResponse = processRefreshTokenGrant(solutionUser);
        } else {
            throw new IllegalStateException("unexpected grant_type: " + grantType);
        }
        return tokenSuccessResponse;
    }

    private TokenSuccessResponse processAuthzCodeGrant(SolutionUser solutionUser) throws ServerException {
        AuthorizationCodeGrant authzCodeGrant = (AuthorizationCodeGrant) this.tokenRequest.getAuthorizationGrant();
        AuthorizationCode authzCode = authzCodeGrant.getAuthorizationCode();
        URI redirectUri = authzCodeGrant.getRedirectionURI();

        AuthorizationCodeEntry entry = this.authzCodeManager.remove(authzCode);
        ErrorObject error = validateAuthzCode(entry, redirectUri);
        if (error != null) {
            throw new ServerException(error);
        }

        return process(
                entry.getPersonUser(),
                solutionUser,
                entry.getAuthnRequest().getClientID(),
                entry.getAuthnRequest().getScope(),
                entry.getAuthnRequest().getNonce(),
                entry.getSessionId(),
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processClientCredentialsGrant(SolutionUser solutionUser) throws ServerException {
        assert this.tokenRequest.getAuthorizationGrant() instanceof ClientCredentialsGrant;
        return process(
                (PersonUser) null,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                null /* nonce */,
                null /* sessionId*/,
                false /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processSolutionUserCredentialsGrant(SolutionUser solutionUser) throws ServerException {
        assert this.tokenRequest.getAuthorizationGrant() instanceof SolutionUserCredentialsGrant;
        return process(
                (PersonUser) null,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                null /* nonce */,
                null /* sessionId*/,
                false /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processPasswordCredentialsGrant(SolutionUser solutionUser) throws ServerException {
        ResourceOwnerPasswordCredentialsGrant passwordGrant = (ResourceOwnerPasswordCredentialsGrant) this.tokenRequest.getAuthorizationGrant();
        String username = passwordGrant.getUsername();
        Secret password = passwordGrant.getPassword();

        PersonUser personUser;
        try {
            personUser = this.personUserAuthenticator.authenticate(this.tenantInfo.getName(), username, password.getValue());
        } catch (InvalidCredentialsException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("incorrect username or password"), e);
        }

        return process(
                personUser,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                null /* nonce */,
                null /* sessionId */,
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processGssTicketGrant(SolutionUser solutionUser) throws ServerException {
        GssTicketGrant gssTicketGrant = (GssTicketGrant) this.tokenRequest.getAuthorizationGrant();
        byte[] gssTicket = gssTicketGrant.getGssTicket();
        String contextId = gssTicketGrant.getContextId();

        GSSResult gssResult;
        try {
            gssResult = this.personUserAuthenticator.authenticate(this.tenantInfo.getName(), contextId, gssTicket);
        } catch (InvalidCredentialsException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("invalid gss ticket"), e);
        }

        PersonUser personUser;
        if (gssResult.complete()) {
            personUser = new PersonUser(gssResult.getPrincipalId(), this.tenantInfo.getName());
        } else {
            String base64OfServerLeg = Base64.encode(gssResult.getServerLeg()).toString();
            String message = String.format("gss_continue_needed:%s:%s", contextId, base64OfServerLeg);
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription(message));
        }

        return process(
                personUser,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                null /* nonce */,
                null /* sessionId*/,
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processRefreshTokenGrant(SolutionUser solutionUser) throws ServerException {
        RefreshTokenGrant refreshTokenGrant = (RefreshTokenGrant) this.tokenRequest.getAuthorizationGrant();
        RefreshToken refreshToken = refreshTokenGrant.getRefreshToken();

        SignedJWT signedJwt;
        try {
            signedJwt = SignedJWT.parse(refreshToken.getValue());
        } catch (java.text.ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("failed to parse SignedJWT out of refresh_token: " + e.getMessage()), e);
        }

        boolean validSignature;
        try {
            validSignature = signedJwt.verify(new RSASSAVerifier(this.tenantInfo.getPublicKey()));
        } catch (JOSEException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("error while verifying refresh_token signature"), e);
        }
        if (!validSignature) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("jwt has an invalid signature"));
        }

        ReadOnlyJWTClaimsSet claimsSet;
        try {
            claimsSet = signedJwt.getJWTClaimsSet();
        } catch (java.text.ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("failed to parse ClaimsSet out of refresh_token: " + e.getMessage()), e);
        }

        ErrorObject error = validateRefreshToken(claimsSet, solutionUser);
        if (error != null) {
            throw new ServerException(error);
        }

        String scopeString;
        String clientIdString;
        String sessionIdString;
        try {
            scopeString     = claimsSet.getStringClaim("scope");
            clientIdString  = claimsSet.getStringClaim("client_id");
            sessionIdString = claimsSet.getStringClaim("sid");
        } catch (java.text.ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("failed to parse claims out of refresh_token: " + e.getMessage()), e);
        }

        PersonUser personUser;
        try {
            personUser = PersonUser.parse(claimsSet.getSubject(), this.tenantInfo.getName());
        } catch (java.text.ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_GRANT.setDescription("failed to parse subject into a PersonUser"), e);
        }

        return process(
                personUser,
                solutionUser,
                (clientIdString == null) ? null : new ClientID(clientIdString),
                Scope.parse(scopeString),
                (Nonce) null,
                (sessionIdString == null) ? null : new SessionID(sessionIdString),
                false /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse process(
            PersonUser personUser,
            SolutionUser solutionUser,
            ClientID clientId,
            Scope scope,
            Nonce nonce,
            SessionID sessionId,
            boolean refreshTokenAllowed) throws ServerException {
        User user = (personUser != null) ? personUser : solutionUser;
        UserInformation userInfo = this.userInfoRetriever.retrieveUserInfo(user, scope);

        if (personUser != null && solutionUser != null) {
            boolean isMemberOfActAsGroup = this.userInfoRetriever.isMemberOfActAsGroup(solutionUser);
            if (!isMemberOfActAsGroup) {
                throw new ServerException(OAuth2Error.ACCESS_DENIED.setDescription("solution user acting as a person user must be a member of ActAsUsers group"));
            }
        }

        IDToken idToken = TokenIssuer.issueIdToken(
                personUser,
                solutionUser,
                userInfo,
                this.tenantInfo,
                clientId,
                scope,
                nonce,
                sessionId);

        AccessToken accessToken = TokenIssuer.issueAccessToken(
                personUser,
                solutionUser,
                userInfo,
                this.tenantInfo,
                clientId,
                scope,
                nonce);

        RefreshToken refreshToken = null;
        if (refreshTokenAllowed && scope.contains(OIDCScopeValue.OFFLINE_ACCESS)) {
            refreshToken = TokenIssuer.issueRefreshToken(
                    personUser,
                    solutionUser,
                    this.tenantInfo,
                    clientId,
                    scope,
                    sessionId);
        }

        return new TokenSuccessResponse(idToken, accessToken, refreshToken);
    }

    private ErrorObject validate() {
        ErrorObject error = null;

        GrantType grantType = this.tokenRequest.getAuthorizationGrant().getType();
        boolean grantTypeSupported =
                (grantType.equals(GrantType.AUTHORIZATION_CODE)) ||
                (grantType.equals(GrantType.PASSWORD)) ||
                (grantType.equals(GrantType.CLIENT_CREDENTIALS)) ||
                (grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE)) ||
                (grantType.equals(GssTicketGrant.GRANT_TYPE)) ||
                (grantType.equals(GrantType.REFRESH_TOKEN));
        if (!grantTypeSupported) {
            error = OAuth2Error.UNSUPPORTED_GRANT_TYPE;
        }

        Scope scope = this.tokenRequest.getScope();
        if (error == null &&
                (grantType.equals(ResourceOwnerPasswordCredentialsGrant.GRANT_TYPE) ||
                 grantType.equals(ClientCredentialsGrant.GRANT_TYPE) ||
                 grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE) ||
                 grantType.equals(GssTicketGrant.GRANT_TYPE))) {
            error = Shared.validateScope(scope, grantType);
        }

        if (error == null && grantType.equals(GrantType.AUTHORIZATION_CODE) && scope != null) {
            error = OAuth2Error.INVALID_REQUEST.setDescription("scope parameter is not allowed in token request for authz code flow");
        }

        if (error == null && grantType.equals(GrantType.REFRESH_TOKEN) && scope != null) {
            error = OAuth2Error.INVALID_REQUEST.setDescription("scope parameter is not allowed in token request for refresh token flow");
        }

        return error;
    }

    private ErrorObject validateAuthzCode(AuthorizationCodeEntry entry, URI tokenRequestRedirectUri) {
        String error = null;

        if (entry == null) {
            error = "invalid authorization code";
        }

        if (error == null) {
            // in authz code flow, the client_id and redirect_uri in the token request must match those in the original authn request
            AuthenticationRequest originalAuthnRequest = entry.getAuthnRequest();
            if (!originalAuthnRequest.getClientID().equals(this.tokenRequest.getClientID())) {
                error = "client_id does not match that of the original authn request";
            } else if (!originalAuthnRequest.getRedirectionURI().equals(tokenRequestRedirectUri)) {
                error = "redirect_uri does not match that of the original authn request";
            }
        }

        if (error == null && !entry.getPersonUser().getTenant().equals(this.tenantInfo.getName())) {
            error = "tenant does not match that of the original authn request";
        }

        return (error == null) ? null : OAuth2Error.INVALID_GRANT.setDescription(error);
    }

    private ErrorObject validateRefreshToken(ReadOnlyJWTClaimsSet claimsSet, SolutionUser solutionUser) {
        String error = null;

        String tokenClass = null;
        String scope      = null;
        String actAs      = null;
        String clientId   = null;
        String tenant     = null;
        try {
            tokenClass  = claimsSet.getStringClaim("token_class");
            scope       = claimsSet.getStringClaim("scope");
            actAs       = claimsSet.getStringClaim("act_as");
            clientId    = claimsSet.getStringClaim("client_id");
            tenant      = claimsSet.getStringClaim("tenant");
        } catch (java.text.ParseException e) {
            error = "failed to parse claims out of jwt";
        }

        if (error == null && !("refresh_token").equals(tokenClass)) {
            error = "jwt is missing a token_class=refresh_token claim";
        }

        if (error == null && StringUtils.isEmpty(claimsSet.getSubject())) {
            error = "jwt is missing sub (subject) claim";
        }

        Date expirationTime = claimsSet.getExpirationTime();
        if (error == null && expirationTime == null) {
            error = "jwt is missing exp (expiration) claim";
        }

        Date now = new Date();
        if (error == null && expirationTime.before(now)) {
            error = "jwt has expired";
        }

        if (error == null && StringUtils.isEmpty(scope)) {
            error = "jwt is missing scope claim";
        }

        if (error == null && !this.tenantInfo.getName().equals(tenant)) {
            error = "refresh_token was not issued to this tenant";
        }

        String expectedClientId = (this.tokenRequest.getClientID() == null) ? null : this.tokenRequest.getClientID().getValue();
        if (error == null && !Objects.equals(clientId, expectedClientId)) {
            error = "refresh_token was not issued to this client";
        }

        String expectedActAs = (solutionUser == null) ? null : solutionUser.getSubject().getValue();
        if (error == null && !Objects.equals(actAs, expectedActAs)) {
            error = "refresh_token was not issued to this solution user";
        }

        return (error == null) ? null : OAuth2Error.INVALID_GRANT.setDescription(error);
    }
}
