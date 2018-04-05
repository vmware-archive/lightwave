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
package com.vmware.identity.rest.core.server.resources;

import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.util.ClientFactory;
import com.vmware.identity.rest.core.util.StringManager;

import io.prometheus.client.Counter;
import io.prometheus.client.Histogram;

/**
 * Core Resource that be extended by other resources that perform identity management.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public abstract class BaseResource {

    private ContainerRequestContext request;
    private SecurityContext securityContext;
    private CasIdmClient idmClient;
    private String correlationId;
    private String userId;
    private String sessionId;

    protected StringManager sm;
    protected static final String HTTP_OK = "200";
    protected static final String HTTP_BAD_REQUEST = "400";
    protected static final String HTTP_NOT_FOUND = "404";
    protected static final String HTTP_SERVER_ERROR = "500";

    protected static final Counter totalRequests = Counter.build()
            .name("sts_requests_total").help("Total requests.")
            .labelNames("component", "tenant", "status", "resource", "operation")
            .register();

    protected static final Histogram requestLatency = Histogram.build()
            .name("sts_requests_latency_seconds").help("Request latency in seconds.")
            .labelNames("component", "tenant", "resource", "operation")
            .buckets(0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 3.0, 4.0)
            .register();

    protected BaseResource(Locale locale, String localizationPackage, SecurityContext securityContext) {
        this.securityContext = securityContext;

        if (locale == null) {
            locale = Locale.getDefault();
        }
        StringManager.setThreadLocale(locale);
        this.sm = StringManager.getManager(localizationPackage);
        this.correlationId = DiagnosticsContextFactory.getCurrentDiagnosticsContext().getCorrelationId();
        this.userId = DiagnosticsContextFactory.getCurrentDiagnosticsContext().getUserId();
        this.sessionId = DiagnosticsContextFactory.getCurrentDiagnosticsContext().getSessionId();
    }

    public BaseResource(ContainerRequestContext request, String localizationPackage, SecurityContext securityContext) {
        this(request.getLanguage(), localizationPackage, securityContext);

        this.request = request;
    }
    /**
     * Lazily instantiate the IDM client so it is only connected
     * if it's necessary.
     *
     * @return an instance of the client
     */
    protected CasIdmClient getIDMClient() {
        if (this.idmClient == null) {
            this.idmClient = ClientFactory.getClient(this.correlationId, this.userId, this.sessionId);
        }

        return this.idmClient;
    }

    protected ContainerRequestContext getRequest() {
        return request;
    }

    protected SecurityContext getSecurityContext() {
        return securityContext;
    }

    protected String getCorrelationId() {
        return correlationId;
    }

    protected String getUserId() {
        return userId;
    }

    protected String getSessionId() {
        return sessionId;
    }

    /**
     * Set the IDM client with an already instantiated version.
     * <p>
     * Used for mocking the IDM client.
     *
     * @param idmClient client to load
     */
    public void setIDMClient(CasIdmClient idmClient) {
        this.idmClient = idmClient;
    }

}
