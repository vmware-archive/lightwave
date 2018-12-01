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

import java.io.IOException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.MetricUtils;
import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.ParameterMapUtils;

import io.prometheus.client.Histogram.Timer;

/**
 * @author Yehia Zayour
 */
@Controller
public class TokenController {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TokenController.class);

    public static final String metricsResource = "token";

    @Autowired
    private CasIdmClient idmClient;

    @Autowired
    private AuthorizationCodeManager authzCodeManager;

    public TokenController() {
    }

    // for unit tests
    TokenController(CasIdmClient idmClient, AuthorizationCodeManager authzCodeManager) {
        this.idmClient = idmClient;
        this.authzCodeManager = authzCodeManager;
    }

    // for unit tests
    void setAuthorizationCodeManager(AuthorizationCodeManager authzCodeManager) {
        this.authzCodeManager = authzCodeManager;
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.TOKEN, method = RequestMethod.POST, consumes = "application/x-www-form-urlencoded")
    public void acquireTokens(
            HttpServletRequest request,
            HttpServletResponse response) throws IOException {
        acquireTokens(request, response, null);
    }

    @RequestMapping(value = Endpoints.BASE + Endpoints.TOKEN + "/{tenant:.*}", method = RequestMethod.POST, consumes = "application/x-www-form-urlencoded")
    public void acquireTokens(
            HttpServletRequest request,
            HttpServletResponse response,
            @PathVariable("tenant") String tenant) throws IOException {
        String metricsOperation = "acquireTokens";
        Timer requestTimer = null;
        HttpResponse httpResponse = null;
        IDiagnosticsContextScope context = null;

        try {
            if (tenant == null) {
                tenant = new TenantInfoRetriever(this.idmClient).getDefaultTenantName();
            }
            if (request.getQueryString() != null && !request.getQueryString().isEmpty()) {
                ErrorObject errorObject = ErrorObject.invalidRequest("query parameters are not allowed at token endpoint");
                LoggerUtils.logFailedRequest(logger, errorObject);
                httpResponse = HttpResponse.createJsonResponse(errorObject);
            } else {
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
                GrantType grantType = GrantType.parse(ParameterMapUtils.getString(httpRequest.getParameters(), "grant_type"));
                metricsOperation += "_" + grantType.toString(); // group sub metrics based on grant type
                requestTimer = MetricUtils.startRequestTimer(metricsResource, metricsOperation);

                TokenRequestProcessor p = new TokenRequestProcessor(
                        this.idmClient,
                        this.authzCodeManager,
                        httpRequest,
                        tenant);
                httpResponse = p.process();
            }
        } catch (ParseException e) {
            LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
            httpResponse = HttpResponse.createJsonResponse(e.getErrorObject());
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
            httpResponse = HttpResponse.createJsonResponse(e.getErrorObject());
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
