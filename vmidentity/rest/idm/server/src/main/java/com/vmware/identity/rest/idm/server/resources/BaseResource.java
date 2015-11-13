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
package com.vmware.identity.rest.idm.server.resources;

import java.util.Locale;
import java.util.UUID;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.SecurityContext;

import org.slf4j.MDC;

import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.util.ClientFactory;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * Core Resource that be extended by other resources that perform identity management.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public abstract class BaseResource {

    private HttpServletRequest request;
    private SecurityContext securityContext;
    private CasIdmClient idmClient;
    private String correlationId;

    protected StringManager sm;

    public BaseResource(Locale locale, String correlationId, SecurityContext securityContext) {
        this.securityContext = securityContext;

        StringManager.setThreadLocale(locale);
        this.sm = StringManager.getManager("i18n");

        this.correlationId = correlationId;
        if (this.correlationId == null || this.correlationId.isEmpty()) {
            this.correlationId = UUID.randomUUID().toString();
        }

        MDC.put(Config.CORRELATION_ID_HEADER, this.correlationId);
    }

    public BaseResource(HttpServletRequest request, SecurityContext securityContext) {
        this(request.getLocale(), request.getHeader(Config.CORRELATION_ID_HEADER), securityContext);

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
            this.idmClient = ClientFactory.getClient(this.correlationId);
        }

        return this.idmClient;
    }

    protected HttpServletRequest getRequest() {
        return request;
    }

    protected SecurityContext getSecurityContext() {
        return securityContext;
    }

    protected String getCorrelationId() {
        return correlationId;
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
