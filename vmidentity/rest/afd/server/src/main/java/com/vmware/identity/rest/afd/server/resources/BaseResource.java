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
package com.vmware.identity.rest.afd.server.resources;

import java.util.UUID;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.SecurityContext;

import org.slf4j.MDC;

import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.afd.server.util.Config;
import com.vmware.identity.rest.core.server.util.ClientFactory;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * Base resource that be extended by other sub resources such as AD, Vecs etc.
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

    public BaseResource(HttpServletRequest request, SecurityContext securityContext) {
        this.request = request;
        this.securityContext = securityContext;

        StringManager.setThreadLocale(request.getLocale());
        this.sm = StringManager.getManager("i18n");

        this.correlationId = request.getHeader(Config.CORRELATION_ID_HEADER);
        if (this.correlationId == null || this.correlationId.isEmpty()) {
            this.correlationId = UUID.randomUUID().toString();
        }

        MDC.put(Config.CORRELATION_ID_HEADER, this.correlationId);
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
