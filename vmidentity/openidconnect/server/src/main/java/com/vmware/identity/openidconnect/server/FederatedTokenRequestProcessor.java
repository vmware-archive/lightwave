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

package com.vmware.identity.openidconnect.server;

import java.util.Objects;

import org.apache.commons.lang.Validate;

import com.nimbusds.jose.JOSEException;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.FederatedTokenRequest;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.JSONUtils;
import com.vmware.identity.openidconnect.protocol.TokenErrorResponse;

import net.minidev.json.JSONObject;

public class FederatedTokenRequestProcessor {

    private static final long CLIENT_ASSERTION_LIFETIME_MS = 2 * 60 * 1000L; // 2 minutes

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(FederatedTokenRequestProcessor.class);

    private final TenantInfoRetriever tenantInfoRetriever;
    private final ClientInfoRetriever clientInfoRetriever;
    private final SolutionUserAuthenticator solutionUserAuthenticator;
    private final SessionManager sessionManager;
    private final HttpRequest httpRequest;
    private String tenant;
    private TenantInfo tenantInfo;
    private ClientInfo clientInfo;
    private FederatedTokenRequest extTokenRequest;

    public FederatedTokenRequestProcessor(CasIdmClient idmClient, SessionManager sessionManager, HttpRequest httpRequest,
            String tenant) throws Exception {
        this.tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        this.clientInfoRetriever = new ClientInfoRetriever(idmClient);
        this.solutionUserAuthenticator = new SolutionUserAuthenticator(idmClient);
        this.sessionManager = sessionManager;
        this.httpRequest = httpRequest;
        this.tenant = tenant;
        this.tenantInfo = null;
        this.extTokenRequest = null;
    }

    public HttpResponse process() throws ParseException {
        try {
            this.extTokenRequest = FederatedTokenRequest.parse(this.httpRequest);
        } catch (ParseException e) {
            LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
            TokenErrorResponse tokenErrorResponse = new TokenErrorResponse(e.getErrorObject());
            return tokenErrorResponse.toHttpResponse();
        }

        try {
            this.tenantInfo = this.tenantInfoRetriever.retrieveTenantInfo(this.tenant);
            this.tenant = this.tenantInfo.getName(); // use tenant name as it appears in directory
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            return HttpResponse.createErrorResponse(e.getErrorObject());
        }

        try {
            authenticateClient();
            JSONObject jsonObject = processInternal();
            logger.info("Successfully processed external token request for tenant [{}].", this.tenant);
            return HttpResponse.createJsonResponse(StatusCode.OK, jsonObject);
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            TokenErrorResponse tokenErrorResponse = new TokenErrorResponse(e.getErrorObject());
            return tokenErrorResponse.toHttpResponse();
        }
    }

    private JSONObject processInternal() throws ServerException, ParseException {
        String sessionIdString = this.httpRequest.getCookieValue(SessionManager.getSessionCookieName(this.tenant));
        SessionID sessionId = null;
        SessionManager.Entry entry = null;
        if (sessionIdString != null) {
            sessionId = new SessionID(sessionIdString);
            entry = this.sessionManager.get(sessionId);
        }

        if (entry == null) {
            throw new ServerException(ErrorObject.invalidRequest("session is not found"));
        }

        Validate.notEmpty(entry.getExternalJWTContent(), "external idp token content");
        validateIDToken(this.extTokenRequest.getIDTokenHint(), entry);

        return JSONUtils.parseJSONObject(entry.getExternalJWTContent());
    }

    private void authenticateClient() throws ServerException {
        this.clientInfo = this.clientInfoRetriever.retrieveClientInfo(this.tenant,
                this.extTokenRequest.getClientID());
        if (this.clientInfo.getCertSubjectDN() == null) {
            return; // no client authentication since no cert was specified at client registration
                    // time
        }

        if (this.extTokenRequest.getClientAssertion() == null) {
            throw new ServerException(ErrorObject
                    .invalidClient("client_assertion parameter is required since client has registered a cert"));
        }

        this.solutionUserAuthenticator.authenticateByClientAssertion(this.extTokenRequest.getClientAssertion(),
                CLIENT_ASSERTION_LIFETIME_MS, this.httpRequest.getURI(), this.tenantInfo, this.clientInfo);
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
}
