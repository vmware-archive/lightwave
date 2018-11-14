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

import java.io.IOException;
import java.security.NoSuchProviderException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.OidcConfig;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;

import io.prometheus.client.Histogram.Timer;

@Controller
public class FederationTokenController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederationTokenController.class);

    public static final String metricsResource = "federation";
    private static final String QUERY_PARAM_ORG_LINK = "orgLink";
    private static final String QUERY_PARAM_CODE = "code";
    private static final String QUERY_PARAM_STATE = "state";

    @Autowired
    private CasIdmClient idmClient;

    @Autowired
    private SessionManager sessionManager;

    @Autowired
    private FederationAuthenticationRequestTracker authnRequestTracker;

    @Autowired
    private FederatedIdentityProcessor cspProcessor;

    @RequestMapping(
            value = Endpoints.BASE + Endpoints.FEDERATION,
            method = RequestMethod.GET
    )
    public void federate(
            HttpServletRequest request,
            HttpServletResponse response
    ) throws IOException {
        String metricsOperation = "authenticate";
        Timer requestTimer = null;
        String tenant = null;
        HttpResponse httpResponse = null;
        try {
            FederationRelayState relayState;
            final String code = request.getParameter(QUERY_PARAM_CODE);
            final String state = request.getParameter(QUERY_PARAM_STATE);
            // check if it is redirection response with auth code
            if (StringUtils.isEmpty(code)) {
                relayState = FederationRelayState.build(state);
                final String orgLink = request.getParameter(QUERY_PARAM_ORG_LINK);
                Validate.notEmpty(orgLink, "Org link is not found from the request.");
                // check tenant name is valid
                int index = orgLink.lastIndexOf("/") + 1;
                // tenant name is the org id from org link
                tenant = orgLink.substring(index);
                Validate.notEmpty(tenant, "Org id is not found from org link.");
                if (StringUtils.isNotEmpty(relayState.getTenant())
                        && !StringUtils.equalsIgnoreCase(relayState.getTenant(), tenant)) {
                    ErrorObject errorObject = ErrorObject.invalidRequest("Invalid tenant name");
                    throw new ServerException(errorObject);
                }

                logger.info("Processing IDP-initiated authentication request for tenant {}.", tenant);
                relayState = new FederationRelayState.Builder(relayState.getIssuer(),
                        relayState.getClientId(), relayState.getRedirectURI())
                        .withTenant(tenant)
                        .withState(new State())
                        .withNonce(new Nonce())
                        .build();
            } else {
                // validate state
                relayState = authnRequestTracker.remove(State.parse(state));
                if (relayState == null) {
                    throw new ServerException(ErrorObject.invalidRequest("Request state is not found."));
                }
                tenant = relayState.getTenant();
            }

            // start timer after tenant is available
            requestTimer = MetricUtils.startRequestTimer(metricsResource, metricsOperation);

            final IDPConfig idpConfig = findFederatedIDP(relayState.getIssuer()); // External IDP corresponding to Issuer
            final OidcConfig oidcConfig = idpConfig.getOidcConfig();
            if (oidcConfig == null) {
                throw new ServerException(ErrorObject.invalidRequest("Oidc configuration not found"));
            }
            final FederatedIdentityProcessor processor = findProcessor(oidcConfig.getIssuerType());
            httpResponse = processor.processRequest(request, relayState, idpConfig);
        } catch (IllegalArgumentException e) {
            ErrorObject errorObject = ErrorObject.invalidRequest("Invalid request.");
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
            httpResponse = HttpResponse.createJsonResponse(e.getErrorObject());
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.serverError(
                    String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        } finally {
            if (httpResponse != null) {
                MetricUtils.increaseRequestCount(String.valueOf(httpResponse.getStatusCode().getValue()), metricsResource, metricsOperation);
            }
            if (requestTimer != null) {
                requestTimer.observeDuration();
            }
        }

        httpResponse.applyTo(response);
    }

    private IDPConfig findFederatedIDP(String issuer) throws Exception {
        String systemTenantName = idmClient.getSystemTenant();
        IDPConfig result = idmClient.getExternalIdpConfigForTenant(systemTenantName, issuer);
        if (result == null) {
            throw new ServerException(ErrorObject.invalidRequest("no federated idp config found"));
        }
        if (!result.getProtocol().equals(IDPConfig.IDP_PROTOCOL_OAUTH_2_0)) {
            ErrorObject errorObject = ErrorObject.serverError(
                String.format("Failed to find federated OIDC IDP config for issuer: %s", issuer)
            );
            throw new ServerException(errorObject);
        }
        return result;
    }

    private FederatedIdentityProcessor findProcessor(String issuerType) throws Exception {
        if (issuerType == null || !issuerType.equals("csp")) {
            throw new NoSuchProviderException("Error: Unsupported Issuer Type - " + issuerType);
        }
        return cspProcessor;
    }

    @CrossOrigin(allowCredentials = "true")
    @RequestMapping(value = Endpoints.BASE + Endpoints.EXTERNAL_TOKEN + "/{tenant:.*}", method = RequestMethod.GET)
    public void acquireExternalTokens(
            HttpServletRequest request,
            HttpServletResponse response,
            @PathVariable("tenant") String tenant) throws IOException {
        String metricsOperation = "acquireFederationTokens";
        Timer requestTimer = MetricUtils.startRequestTimer(metricsResource, metricsOperation);
        HttpResponse httpResponse = null;
        IDiagnosticsContextScope context = null;

        try {
            HttpRequest httpRequest;
            try {
                httpRequest = HttpRequest.from(request);
            } catch (IllegalArgumentException e) {
                ErrorObject errorObject = ErrorObject.invalidRequest(e.getMessage());
                LoggerUtils.logFailedRequest(logger, errorObject, e);
                httpResponse = HttpResponse.createJsonResponse(errorObject);
                httpResponse.applyTo(response);
                return;
            }
            context = DiagnosticsContextFactory.createContext(LoggerUtils.getCorrelationID(httpRequest).getValue(), tenant);
            FederatedTokenRequestProcessor p = new FederatedTokenRequestProcessor(
                    this.idmClient,
                    this.sessionManager,
                    httpRequest,
                    tenant);
            httpResponse = p.process();
        } catch (Exception e) {
            ErrorObject errorObject = ErrorObject.serverError(String.format("unhandled %s: %s", e.getClass().getName(), e.getMessage()));
            LoggerUtils.logFailedRequest(logger, errorObject, e);
            httpResponse = HttpResponse.createJsonResponse(errorObject);
        } finally {
            if (context != null) {
                context.close();
            }
            if (httpResponse != null) {
                MetricUtils.increaseRequestCount(String.valueOf(httpResponse.getStatusCode().getValue()),
                        metricsResource, metricsOperation);
            }
            if (requestTimer != null) {
                requestTimer.observeDuration();
            }
        }

        httpResponse.applyTo(response);
    }
}
