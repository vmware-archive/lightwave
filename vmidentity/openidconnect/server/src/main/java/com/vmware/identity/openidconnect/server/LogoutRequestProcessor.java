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
import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

import javax.servlet.http.Cookie;

import org.apache.commons.lang3.tuple.Pair;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.LogoutErrorResponse;
import com.vmware.identity.openidconnect.protocol.LogoutRequest;
import com.vmware.identity.openidconnect.protocol.LogoutSuccessResponse;

/**
 * @author Yehia Zayour
 */
public class LogoutRequestProcessor {
    private static final long CLIENT_ASSERTION_LIFETIME_MS = 2 * 60 * 1000L; // 2 minutes

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(LogoutRequestProcessor.class);

    private final TenantInfoRetriever tenantInfoRetriever;
    private final ClientInfoRetriever clientInfoRetriever;
    private final SolutionUserAuthenticator solutionUserAuthenticator;

    private final SessionManager sessionManager;
    private final HttpRequest httpRequest;
    private String tenant;

    private TenantInfo tenantInfo;
    private ClientInfo clientInfo;
    private LogoutRequest logoutRequest;

    public LogoutRequestProcessor(
            CasIdmClient idmClient,
            SessionManager sessionManager,
            HttpRequest httpRequest,
            String tenant) {
        this.tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        this.clientInfoRetriever = new ClientInfoRetriever(idmClient);
        this.solutionUserAuthenticator = new SolutionUserAuthenticator(idmClient);

        this.sessionManager = sessionManager;
        this.httpRequest = httpRequest;
        this.tenant = tenant;

        this.tenantInfo = null;
        this.logoutRequest = null;
    }

    public HttpResponse process() {
        ClientID clientId;
        URI postLogoutRedirectUri;
        LogoutErrorResponse parseErrorResponse;
        try {
            this.logoutRequest = LogoutRequest.parse(this.httpRequest);
            clientId = this.logoutRequest.getClientID();
            postLogoutRedirectUri = this.logoutRequest.getPostLogoutRedirectURI();
            parseErrorResponse = null;
        } catch (LogoutRequest.ParseException e) {
            if (e.getClientID() != null && e.getPostLogoutRedirectURI() != null && e.createLogoutErrorResponse() != null) {
                clientId = e.getClientID();
                postLogoutRedirectUri = e.getPostLogoutRedirectURI();
                parseErrorResponse = e.createLogoutErrorResponse();
            } else {
                LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
                return HttpResponse.createErrorResponse(e.getErrorObject());
            }
        }

        try {
            this.tenantInfo = this.tenantInfoRetriever.retrieveTenantInfo(this.tenant);
            this.tenant = this.tenantInfo.getName(); // use tenant name as it appears in directory
            this.clientInfo = this.clientInfoRetriever.retrieveClientInfo(this.tenant, clientId);
            if (!this.clientInfo.getPostLogoutRedirectURIs().contains(postLogoutRedirectUri)) {
                throw new ServerException(ErrorObject.invalidRequest("unregistered post_logout_redirect_uri"));
            }
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            return HttpResponse.createErrorResponse(e.getErrorObject());
        }

        if (parseErrorResponse != null) {
            LoggerUtils.logFailedRequest(logger, parseErrorResponse.getErrorObject());
            return parseErrorResponse.toHttpResponse();
        }

        try {
            authenticateClient();

            Pair<LogoutSuccessResponse, Cookie> result = processInternal();
            LogoutSuccessResponse logoutSuccessResponse = result.getLeft();
            Cookie personUserCertificateLoggedOutCookie = result.getRight();

            HttpResponse httpResponse = logoutSuccessResponse.toHttpResponse();
            httpResponse.addCookie(loggedOutSessionCookie());
            if (personUserCertificateLoggedOutCookie != null) {
                httpResponse.addCookie(personUserCertificateLoggedOutCookie);
            }
            logger.info(
                    "subject [{}]",
                    this.logoutRequest.getIDTokenHint().getSubject().getValue());
            return httpResponse;
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            LogoutErrorResponse logoutErrorResponse = new LogoutErrorResponse(
                    this.logoutRequest.getPostLogoutRedirectURI(),
                    this.logoutRequest.getState(),
                    e.getErrorObject());
            return logoutErrorResponse.toHttpResponse();
        }
    }

    private Pair<LogoutSuccessResponse, Cookie> processInternal() throws ServerException {
        String sessionIdString = this.httpRequest.getCookieValue(SessionManager.getSessionCookieName(this.tenant));
        SessionID sessionId = null;
        SessionManager.Entry entry = null;
        if (sessionIdString != null) {
            sessionId = new SessionID(sessionIdString);
            entry = this.sessionManager.get(sessionId);
        }

        validateIDToken(this.logoutRequest.getIDTokenHint(), entry);

        Cookie personUserCertificateLoggedOutCookie = null;
        if (entry != null) {
            this.sessionManager.remove(sessionId);
            if (entry.getLoginMethod() == LoginMethod.PERSON_USER_CERTIFICATE) {
                personUserCertificateLoggedOutCookie = personUserCertificateLoggedOutCookie();
            }
        }

        // SLO using OpenID Connect HTTP-Based Logout 1.0 - draft 03
        // construct iframe links containing logout_uri requests, the browser will send these to other participating clients
        // do not include the client that initiated this logout request as that client has already logged out before sending us this request
        Set<URI> logoutUris = new HashSet<URI>();
        if (entry != null) {
            for (ClientInfo client : entry.getClients()) {
                if (client.getLogoutURI() != null && !Objects.equals(client.getID(), this.logoutRequest.getClientID())) {
                    logoutUris.add(client.getLogoutURI());
                }
            }
        }

        LogoutSuccessResponse logoutSuccessResponse = new LogoutSuccessResponse(
                this.logoutRequest.getPostLogoutRedirectURI(),
                this.logoutRequest.getState(),
                sessionId,
                logoutUris);

        return Pair.of(logoutSuccessResponse, personUserCertificateLoggedOutCookie);
    }

    private void validateIDToken(IDToken idToken, SessionManager.Entry entry) throws ServerException {
        try {
            if (!idToken.hasValidSignature(this.tenantInfo.getPublicKey())) {
                throw new ServerException(ErrorObject.invalidRequest("id_token has an invalid signature"));
            }
        } catch (JOSEException e) {
            throw new ServerException(ErrorObject.serverError("error while verifying id_token signature"), e);
        }

        if (!Objects.equals(idToken.getTenant(), this.tenant)) {
            throw new ServerException(ErrorObject.invalidRequest("id_token has incorrect tenant"));
        }

        if (!Objects.equals(idToken.getIssuer(), this.tenantInfo.getIssuer())) {
            throw new ServerException(ErrorObject.invalidRequest("id_token has incorrect issuer"));
        }

        if (entry != null && !Objects.equals(idToken.getSubject(), entry.getPersonUser().getSubject())) {
            throw new ServerException(ErrorObject.invalidRequest("id_token subject does not match the session user"));
        }
    }

    private void authenticateClient() throws ServerException {
        if (this.clientInfo.getCertSubjectDN() == null) {
            return; // no client authentication since no cert was specified at client registration time
        }

        if (this.logoutRequest.getClientAssertion() == null) {
            throw new ServerException(ErrorObject.invalidClient("client_assertion parameter is required since client has registered a cert"));
        }

        this.solutionUserAuthenticator.authenticateByClientAssertion(
                this.logoutRequest.getClientAssertion(),
                CLIENT_ASSERTION_LIFETIME_MS,
                this.httpRequest.getURI(),
                this.tenantInfo,
                this.clientInfo);
    }

    private Cookie loggedOutSessionCookie() {
        Cookie cookie = new Cookie(SessionManager.getSessionCookieName(this.tenant), "");
        cookie.setPath(Endpoints.BASE);
        cookie.setSecure(true);
        cookie.setHttpOnly(true);
        cookie.setMaxAge(0);
        return cookie;
    }

    private Cookie personUserCertificateLoggedOutCookie() {
        Cookie cookie = new Cookie(SessionManager.getPersonUserCertificateLoggedOutCookieName(this.tenant), "");
        cookie.setPath(Endpoints.BASE);
        cookie.setSecure(true);
        cookie.setHttpOnly(true);
        return cookie;
    }
}