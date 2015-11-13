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
import java.text.ParseException;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import javax.servlet.http.Cookie;

import org.apache.commons.lang3.tuple.Pair;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.id.ClientID;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientInformation;
import com.nimbusds.openid.connect.sdk.rp.OIDCClientMetadata;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.HttpRequest;
import com.vmware.identity.openidconnect.common.LogoutRequest;
import com.vmware.identity.openidconnect.common.LogoutSuccessResponse;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.TokenClass;

/**
 * @author Yehia Zayour
 */
public class LogoutRequestProcessor {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(LogoutRequestProcessor.class);

    private final TenantInfoRetriever tenantInfoRetriever;
    private final ClientInfoRetriever clientInfoRetriever;
    private final SolutionUserAuthenticator solutionUserAuthenticator;

    private final SessionManager sessionManager;
    private final HttpRequest httpRequest;
    private final String tenant;

    private TenantInformation tenantInfo;
    private LogoutRequest logoutRequest;

    public LogoutRequestProcessor(
            IdmClient idmClient,
            SessionManager sessionManager,
            HttpRequest httpRequest,
            String tenant) {
        this.tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        this.clientInfoRetriever = new ClientInfoRetriever(idmClient);
        this.solutionUserAuthenticator = new SolutionUserAuthenticator(idmClient);

        this.sessionManager = sessionManager;
        this.httpRequest = httpRequest;
        this.tenant = tenant;

        // set by initialize()
        this.tenantInfo = null;
        this.logoutRequest = null;
    }

    public HttpResponse process() {
        HttpResponse httpResponse;

        try {
            initialize();
            Pair<LogoutSuccessResponse, Cookie> result = processInternal();
            httpResponse = HttpResponse.success(result.getLeft(), result.getRight());
        } catch (ServerException e) {
            Shared.logFailedRequest(logger, e);
            httpResponse = HttpResponse.error(e);
        }

        return httpResponse;
    }

    private void initialize() throws ServerException {
        try {
            this.logoutRequest = LogoutRequest.parse(this.httpRequest);
        } catch (com.nimbusds.oauth2.sdk.ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("logout request parse error: " + e.getMessage()), e);
        }

        String tenantName = this.tenant;
        if (tenantName == null) {
            tenantName = this.tenantInfoRetriever.getDefaultTenantName();
        }
        this.tenantInfo = this.tenantInfoRetriever.retrieveTenantInfo(tenantName);
    }

    private Pair<LogoutSuccessResponse, Cookie> processInternal() throws ServerException {
        String sessionIdString = this.httpRequest.getCookieValue(Shared.getSessionCookieName(this.tenantInfo.getName()));
        SessionID sessionId = null;
        SessionEntry entry = null;
        if (sessionIdString != null) {
            sessionId = new SessionID(sessionIdString);
            entry = this.sessionManager.get(sessionId);
        }

        boolean validSignature;
        try {
            validSignature = this.logoutRequest.getIDTokenHint().getSignedJWT().verify(new RSASSAVerifier(this.tenantInfo.getPublicKey()));
        } catch (JOSEException e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("error while verifying id_token_hint signature"), e);
        }
        if (!validSignature) {
            throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("id_token_hint has an invalid signature"));
        }

        ReadOnlyJWTClaimsSet idTokenHintClaimsSet;
        try {
            idTokenHintClaimsSet = this.logoutRequest.getIDTokenHint().getSignedJWT().getJWTClaimsSet();
        } catch (ParseException e) {
            throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("failed to parse claims out of id_token_hint"), e);
        }

        ErrorObject error = validateIdTokenHintClaims(idTokenHintClaimsSet, entry);
        if (error != null) {
            throw new ServerException(error);
        }

        String clientId = idTokenHintClaimsSet.getAudience().get(0);
        OIDCClientInformation clientInfo = this.clientInfoRetriever.retrieveClientInfo(this.tenantInfo.getName(), new ClientID(clientId));
        String certSubjectDn = (String) clientInfo.getOIDCMetadata().getCustomField("cert_subject_dn");
        if (certSubjectDn != null) {
            if (this.logoutRequest.getClientAssertion() != null) {
                this.solutionUserAuthenticator.authenticateByClientAssertion(
                        this.logoutRequest.getClientAssertion(),
                        this.httpRequest.getRequestUrl(),
                        this.tenantInfo,
                        clientInfo);
            } else {
                throw new ServerException(OAuth2Error.INVALID_CLIENT.setDescription("client_assertion parameter is required since client has registered a cert"));
            }
        }

        if (this.logoutRequest.getPostLogoutRedirectionURI() != null) {
            Set<URI> postLogoutRedirectUris = clientInfo.getOIDCMetadata().getPostLogoutRedirectionURIs();
            if (postLogoutRedirectUris == null || !postLogoutRedirectUris.contains(this.logoutRequest.getPostLogoutRedirectionURI())) {
                throw new ServerException(OAuth2Error.INVALID_REQUEST.setDescription("unregistered post_logout_redirect_uri"));
            }
        }

        Set<URI> logoutUris = new HashSet<URI>();
        if (entry != null) {
            for (OIDCClientInformation client : entry.getClients()) {
                URI logoutUri = getClientLogoutUri(client);
                if (logoutUri != null) {
                    logoutUris.add(logoutUri);
                }
            }
            this.sessionManager.remove(sessionId);
        }

        return Pair.of(
                new LogoutSuccessResponse(
                    this.logoutRequest.getPostLogoutRedirectionURI(),
                    this.logoutRequest.getState(),
                    sessionId,
                    logoutUris),
                (sessionId == null) ? null : wipeOutSessionCookie());
    }

    private ErrorObject validateIdTokenHintClaims(ReadOnlyJWTClaimsSet idTokenHintClaimsSet, SessionEntry entry) {
        ErrorObject error = null;

        final String errorPrefix = "id_token_hint validation error: ";

        if (error == null) {
            try {
                idTokenHintClaimsSet = this.logoutRequest.getIDTokenHint().getSignedJWT().getJWTClaimsSet();
            } catch (ParseException e) {
                error = OAuth2Error.INVALID_REQUEST.setDescription("failed to parse claims out of id_token_hint");
            }
        }

        if (error == null) {
            try {
                if (!TokenClass.ID_TOKEN.getName().equals(idTokenHintClaimsSet.getStringClaim("token_class"))) {
                    error = OAuth2Error.INVALID_REQUEST.setDescription(errorPrefix + "jwt must have token_class=id_token");
                }
            } catch (ParseException e) {
                error = OAuth2Error.INVALID_REQUEST.setDescription(errorPrefix + "failed to parse token_class or token_type claims out of jwt");
            }
        }

        if (error == null && !Objects.equals(this.tenantInfo.getIssuer().getValue(), idTokenHintClaimsSet.getIssuer())) {
            error = OAuth2Error.INVALID_REQUEST.setDescription(errorPrefix + "invalid issuer");
        }

        if (error == null && idTokenHintClaimsSet.getSubject() == null) {
            error = OAuth2Error.INVALID_REQUEST.setDescription(errorPrefix + "jwt is missing a sub (subject) claim");
        }

        if (error == null && entry != null && !Objects.equals(entry.getPersonUser().getSubject().getValue(), idTokenHintClaimsSet.getSubject())) {
            error = OAuth2Error.INVALID_REQUEST.setDescription(errorPrefix + "jwt subject does not match the session user");
        }

        if (error == null) {
            List<String> audience = idTokenHintClaimsSet.getAudience();
            if (audience == null || audience.size() != 1) {
                error = OAuth2Error.INVALID_REQUEST.setDescription(errorPrefix + "jwt must have a single audience value containing the client_id");
            }
        }

        return error;
    }

    private Cookie wipeOutSessionCookie() {
        Cookie sessionCookie = new Cookie(Shared.getSessionCookieName(this.tenantInfo.getName()), "");
        sessionCookie.setPath("/openidconnect");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        sessionCookie.setMaxAge(0);
        return sessionCookie;
    }

    private static URI getClientLogoutUri(OIDCClientInformation clientInfo) {
        URI logoutUri = null;
        OIDCClientMetadata clientMetadata = clientInfo.getOIDCMetadata();
        String logoutUriString = (String) clientMetadata.getCustomField("logout_uri");
        if (logoutUriString != null) {
            try {
                logoutUri = new URI(logoutUriString);
            } catch (URISyntaxException e) {
                logoutUri = null;
            }
        }
        return logoutUri;
    }
}
