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
import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.Objects;
import java.util.Set;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.ScopeValue;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.protocol.AccessToken;
import com.vmware.identity.openidconnect.protocol.AuthorizationCodeGrant;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.ClientCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.GSSTicketGrant;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.PasswordGrant;
import com.vmware.identity.openidconnect.protocol.PersonUserAssertion;
import com.vmware.identity.openidconnect.protocol.PersonUserCertificateGrant;
import com.vmware.identity.openidconnect.protocol.RefreshToken;
import com.vmware.identity.openidconnect.protocol.RefreshTokenGrant;
import com.vmware.identity.openidconnect.protocol.SecurIDGrant;
import com.vmware.identity.openidconnect.protocol.SolutionUserCredentialsGrant;
import com.vmware.identity.openidconnect.protocol.TokenErrorResponse;
import com.vmware.identity.openidconnect.protocol.TokenRequest;
import com.vmware.identity.openidconnect.protocol.TokenSuccessResponse;

/**
 * @author Yehia Zayour
 */
public class TokenRequestProcessor {
    private static final long ASSERTION_LIFETIME_MS = 1 * 60 * 1000L; // 1 minute

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TokenRequestProcessor.class);

    private final TenantInfoRetriever tenantInfoRetriever;
    private final ClientInfoRetriever clientInfoRetriever;
    private final ServerInfoRetriever serverInfoRetriever;
    private final UserInfoRetriever userInfoRetriever;
    private final PersonUserAuthenticator personUserAuthenticator;
    private final SolutionUserAuthenticator solutionUserAuthenticator;

    private final AuthorizationCodeManager authzCodeManager;
    private final HttpRequest httpRequest;
    private String tenant;

    private TenantInfo tenantInfo;
    private TokenRequest tokenRequest;

    public TokenRequestProcessor(
            CasIdmClient idmClient,
            AuthorizationCodeManager authzCodeManager,
            HttpRequest httpRequest,
            String tenant) {
        this.tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        this.clientInfoRetriever = new ClientInfoRetriever(idmClient);
        this.serverInfoRetriever = new ServerInfoRetriever(idmClient);
        this.userInfoRetriever = new UserInfoRetriever(idmClient);
        this.personUserAuthenticator = new PersonUserAuthenticator(idmClient);
        this.solutionUserAuthenticator = new SolutionUserAuthenticator(idmClient);

        this.authzCodeManager = authzCodeManager;
        this.httpRequest = httpRequest;
        this.tenant = tenant;

        this.tenantInfo = null;
        this.tokenRequest = null;
    }

    public HttpResponse process() {
        try {
            this.tokenRequest = TokenRequest.parse(this.httpRequest);
        } catch (ParseException e) {
            LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
            TokenErrorResponse tokenErrorResponse = new TokenErrorResponse(e.getErrorObject());
            return tokenErrorResponse.toHttpResponse();
        }

        try {
            if (this.tenant == null) {
                this.tenant = this.tenantInfoRetriever.getDefaultTenantName();
            }
            this.tenantInfo = this.tenantInfoRetriever.retrieveTenantInfo(this.tenant);
            this.tenant = this.tenantInfo.getName(); // use tenant name as it appears in directory

            TokenSuccessResponse tokenSuccessResponse = processInternal();
            HttpResponse httpResponse = tokenSuccessResponse.toHttpResponse();
            logger.info(
                    "subject [{}] grant_type [{}]",
                    tokenSuccessResponse.getIDToken().getSubject().getValue(),
                    this.tokenRequest.getAuthorizationGrant().getGrantType().getValue());
            return httpResponse;
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            TokenErrorResponse tokenErrorResponse = new TokenErrorResponse(e.getErrorObject());
            return tokenErrorResponse.toHttpResponse();
        }
    }

    private TokenSuccessResponse processInternal() throws ServerException {
        GrantType grantType = this.tokenRequest.getAuthorizationGrant().getGrantType();

        SolutionUser solutionUser = null;
        if (this.tokenRequest.getClientID() != null) {
            ClientInfo clientInfo = this.clientInfoRetriever.retrieveClientInfo(this.tenant, this.tokenRequest.getClientID());
            boolean certRegistered = clientInfo.getCertSubjectDN() != null;
            if (certRegistered && this.tokenRequest.getClientAssertion() != null) {
                solutionUser = this.solutionUserAuthenticator.authenticateByClientAssertion(
                        this.tokenRequest.getClientAssertion(),
                        ASSERTION_LIFETIME_MS,
                        this.httpRequest.getURI(),
                        this.tenantInfo,
                        clientInfo);
            } else if (certRegistered && this.tokenRequest.getClientAssertion() == null) {
                throw new ServerException(ErrorObject.invalidClient("client_assertion parameter is required since client has registered a cert"));
            } else if (!certRegistered && this.tokenRequest.getClientAssertion() != null) {
                throw new ServerException(ErrorObject.invalidClient("client_assertion parameter is not allowed since client did not register a cert"));
            }
        }
        if (this.tokenRequest.getSolutionUserAssertion() != null) {
            solutionUser = this.solutionUserAuthenticator.authenticateBySolutionUserAssertion(
                    this.tokenRequest.getSolutionUserAssertion(),
                    ASSERTION_LIFETIME_MS,
                    this.httpRequest.getURI(),
                    this.tenantInfo);
        }

        TokenSuccessResponse tokenSuccessResponse;
        switch (grantType) {
            case AUTHORIZATION_CODE:
                tokenSuccessResponse = processAuthzCodeGrant(solutionUser);
                break;
            case PASSWORD:
                tokenSuccessResponse = processPasswordGrant(solutionUser);
                break;
            case SOLUTION_USER_CREDENTIALS:
                tokenSuccessResponse = processSolutionUserCredentialsGrant(solutionUser);
                break;
            case CLIENT_CREDENTIALS:
                tokenSuccessResponse = processClientCredentialsGrant(solutionUser);
                break;
            case PERSON_USER_CERTIFICATE:
                tokenSuccessResponse = processPersonUserCertificateGrant(solutionUser);
                break;
            case GSS_TICKET:
                tokenSuccessResponse = processGssTicketGrant(solutionUser);
                break;
            case SECURID:
                tokenSuccessResponse = processSecurIDGrant(solutionUser);
                break;
            case REFRESH_TOKEN:
                tokenSuccessResponse = processRefreshTokenGrant(solutionUser);
                break;
            default:
                throw new IllegalStateException("unrecognized grant type: " + grantType.getValue());
        }
        return tokenSuccessResponse;
    }

    private TokenSuccessResponse processAuthzCodeGrant(SolutionUser solutionUser) throws ServerException {
        AuthorizationCodeGrant authzCodeGrant = (AuthorizationCodeGrant) this.tokenRequest.getAuthorizationGrant();
        AuthorizationCode authzCode = authzCodeGrant.getAuthorizationCode();
        URI redirectUri = authzCodeGrant.getRedirectURI();

        AuthorizationCodeManager.Entry entry = this.authzCodeManager.remove(authzCode);
        validateAuthzCode(entry, redirectUri);

        return process(
                entry.getPersonUser(),
                solutionUser,
                entry.getAuthenticationRequest().getClientID(),
                entry.getAuthenticationRequest().getScope(),
                entry.getAuthenticationRequest().getNonce(),
                entry.getSessionId(),
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processPasswordGrant(SolutionUser solutionUser) throws ServerException {
        PasswordGrant passwordGrant = (PasswordGrant) this.tokenRequest.getAuthorizationGrant();
        String username = passwordGrant.getUsername();
        String password = passwordGrant.getPassword();

        PersonUser personUser;
        try {
            personUser = this.personUserAuthenticator.authenticateByPassword(this.tenant, username, password);
        } catch (InvalidCredentialsException e) {
            throw new ServerException(ErrorObject.invalidGrant("incorrect username or password"), e);
        }

        return process(
                personUser,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                (Nonce) null,
                (SessionID) null,
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processSolutionUserCredentialsGrant(SolutionUser solutionUser) throws ServerException {
        assert this.tokenRequest.getAuthorizationGrant() instanceof SolutionUserCredentialsGrant;
        return process(
                (PersonUser) null,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                (Nonce) null,
                (SessionID) null,
                false /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processClientCredentialsGrant(SolutionUser solutionUser) throws ServerException {
        assert this.tokenRequest.getAuthorizationGrant() instanceof ClientCredentialsGrant;
        return process(
                (PersonUser) null,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                (Nonce) null,
                (SessionID) null,
                false /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processPersonUserCertificateGrant(SolutionUser solutionUser) throws ServerException {
        PersonUserCertificateGrant personUserCertificateGrant = (PersonUserCertificateGrant) this.tokenRequest.getAuthorizationGrant();
        X509Certificate personUserCertificate = personUserCertificateGrant.getPersonUserCertificate();
        PersonUserAssertion personUserAssertion = personUserCertificateGrant.getPersonUserAssertion();

        PersonUser personUser;
        try {
            personUser = this.personUserAuthenticator.authenticateByPersonUserCertificate(
                    this.tenant,
                    personUserCertificate,
                    personUserAssertion,
                    ASSERTION_LIFETIME_MS,
                    this.httpRequest.getURI(),
                    this.tenantInfo.getClockToleranceMs());
        } catch (InvalidCredentialsException e) {
            throw new ServerException(ErrorObject.invalidGrant("invalid person user cert"), e);
        }

        return process(
                personUser,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                (Nonce) null,
                (SessionID) null,
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processGssTicketGrant(SolutionUser solutionUser) throws ServerException {
        GSSTicketGrant gssTicketGrant = (GSSTicketGrant) this.tokenRequest.getAuthorizationGrant();
        byte[] gssTicket = gssTicketGrant.getGSSTicket();
        String contextId = gssTicketGrant.getContextID();

        GSSResult result;
        try {
            result = this.personUserAuthenticator.authenticateByGssTicket(this.tenant, contextId, gssTicket);
        } catch (InvalidCredentialsException e) {
            throw new ServerException(ErrorObject.invalidGrant("invalid gss ticket"), e);
        }

        PersonUser personUser;
        if (result.complete()) {
            personUser = new PersonUser(result.getPrincipalId(), this.tenant);
        } else {
            String base64OfServerLeg = Base64Utils.encodeToString(result.getServerLeg());
            String message = String.format("gss_continue_needed:%s:%s", contextId, base64OfServerLeg);
            throw new ServerException(ErrorObject.invalidGrant(message));
        }

        return process(
                personUser,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                (Nonce) null,
                (SessionID) null,
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processSecurIDGrant(SolutionUser solutionUser) throws ServerException {
        SecurIDGrant securIdGrant = (SecurIDGrant) this.tokenRequest.getAuthorizationGrant();
        String username = securIdGrant.getUsername();
        String passcode = securIdGrant.getPasscode();
        String sessionId = securIdGrant.getSessionID();

        RSAAMResult result;
        try {
            result = this.personUserAuthenticator.authenticateBySecurID(this.tenant, username, passcode, sessionId);
        } catch (InvalidCredentialsException e) {
            throw new ServerException(ErrorObject.invalidGrant("incorrect securid username or passcode"));
        } catch (IDMSecureIDNewPinException e) {
            throw new ServerException(ErrorObject.invalidGrant("new securid pin required"));
        }

        PersonUser personUser;
        if (result.complete()) {
            personUser = new PersonUser(result.getPrincipalId(), this.tenant);
        } else {
            String base64OfSessionId = Base64Utils.encodeToString(result.getRsaSessionID());
            String message = String.format("securid_next_code_required:%s", base64OfSessionId);
            throw new ServerException(ErrorObject.invalidGrant(message));
        }

        return process(
                personUser,
                solutionUser,
                this.tokenRequest.getClientID(),
                this.tokenRequest.getScope(),
                (Nonce) null,
                (SessionID) null,
                true /* refreshTokenAllowed */);
    }

    private TokenSuccessResponse processRefreshTokenGrant(SolutionUser solutionUser) throws ServerException {
        RefreshTokenGrant refreshTokenGrant = (RefreshTokenGrant) this.tokenRequest.getAuthorizationGrant();
        RefreshToken refreshToken = refreshTokenGrant.getRefreshToken();

        validateRefreshToken(refreshToken, solutionUser);

        PersonUser personUser;
        try {
            personUser = PersonUser.parse(refreshToken.getSubject().getValue(), this.tenant);
        } catch (ParseException e) {
            throw new ServerException(ErrorObject.invalidGrant("failed to parse subject into a PersonUser"), e);
        }

        return process(
                personUser,
                solutionUser,
                refreshToken.getClientID(),
                refreshToken.getScope(),
                refreshToken.getNonce(),
                refreshToken.getSessionID(),
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
        Set<ResourceServerInfo> resourceServerInfos = this.serverInfoRetriever.retrieveResourceServerInfos(this.tenant, scope);
        UserInfo userInfo = this.userInfoRetriever.retrieveUserInfo(user, scope, resourceServerInfos);

        if (personUser != null && solutionUser != null) {
            boolean isMemberOfActAsGroup = this.userInfoRetriever.isMemberOfGroup(solutionUser, "ActAsUsers");
            if (!isMemberOfActAsGroup) {
                throw new ServerException(ErrorObject.accessDenied("solution user acting as a person user must be a member of ActAsUsers group"));
            }
        }

        TokenIssuer tokenIssuer = new TokenIssuer(
                personUser,
                solutionUser,
                userInfo,
                this.tenantInfo,
                scope,
                nonce,
                clientId,
                sessionId);

        IDToken idToken = tokenIssuer.issueIDToken();
        AccessToken accessToken = tokenIssuer.issueAccessToken();
        RefreshToken refreshToken = null;
        if (refreshTokenAllowed && scope.contains(ScopeValue.OFFLINE_ACCESS)) {
            refreshToken = tokenIssuer.issueRefreshToken();
        }

        return new TokenSuccessResponse(idToken, accessToken, refreshToken);
    }

    private void validateAuthzCode(AuthorizationCodeManager.Entry entry, URI tokenRequestRedirectUri) throws ServerException {
        if (entry == null) {
            throw new ServerException(ErrorObject.invalidGrant("invalid authorization code"));
        }

        if (!Objects.equals(entry.getAuthenticationRequest().getClientID(), this.tokenRequest.getClientID())) {
            throw new ServerException(ErrorObject.invalidGrant("client_id does not match that of the original authn request"));
        }

        if (!Objects.equals(entry.getAuthenticationRequest().getRedirectURI(), tokenRequestRedirectUri)) {
            throw new ServerException(ErrorObject.invalidGrant("redirect_uri does not match that of the original authn request"));
        }

        if (!Objects.equals(entry.getPersonUser().getTenant(), this.tenant)) {
            throw new ServerException(ErrorObject.invalidGrant("tenant does not match that of the original authn request"));
        }
    }

    private void validateRefreshToken(RefreshToken refreshToken, SolutionUser solutionUser) throws ServerException {
        try {
            if (!refreshToken.hasValidSignature(this.tenantInfo.getPublicKey())) {
                throw new ServerException(ErrorObject.invalidGrant("refresh_token has an invalid signature"));
            }
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError("error while verifying refresh_token signature"), e);
        }

        if (!Objects.equals(refreshToken.getTenant(), this.tenant)) {
            throw new ServerException(ErrorObject.invalidGrant("refresh_token was not issued to this tenant"));
        }

        if (!Objects.equals(refreshToken.getClientID(), this.tokenRequest.getClientID())) {
            throw new ServerException(ErrorObject.invalidGrant("refresh_token was not issued to this client"));
        }

        if (!Objects.equals(refreshToken.getActAs(), (solutionUser == null) ? null : solutionUser.getSubject())) {
            throw new ServerException(ErrorObject.invalidGrant("refresh_token was not issued to this solution user"));
        }

        Date now = new Date();
        Date notBefore = new Date(refreshToken.getIssueTime().getTime() - this.tenantInfo.getClockToleranceMs());
        Date notAfter = new Date(refreshToken.getExpirationTime().getTime() + this.tenantInfo.getClockToleranceMs());
        if (now.before(notBefore)) {
            throw new ServerException(ErrorObject.invalidGrant("refresh_token is not yet valid"));
        }
        if (now.after(notAfter)) {
            throw new ServerException(ErrorObject.invalidGrant("refresh_token has expired"));
        }
    }
}