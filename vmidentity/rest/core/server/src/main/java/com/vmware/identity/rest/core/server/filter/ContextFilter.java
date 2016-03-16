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
package com.vmware.identity.rest.core.server.filter;

import java.io.IOException;
import java.util.UUID;

import javax.annotation.Priority;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerRequestFilter;
import javax.ws.rs.container.PreMatching;

import org.apache.logging.log4j.ThreadContext;
import org.slf4j.MDC;

import com.vmware.identity.rest.core.server.authorization.Config;

/**
 * Request context filter. Used to set up any contextual
 * information necessary for fulfilling a request.
 */
@PreMatching
@Priority(Integer.MIN_VALUE)
public class ContextFilter implements ContainerRequestFilter {

    @Override
    public void filter(ContainerRequestContext context) throws IOException {
        setCorrelationId(context);
    }

    /**
     * Sets the Correlation ID for the incoming request. Uses the value supplied
     * in the "id" header or generates one and sets the header (so the resources
     * can make use of it).
     *
     * @param context request context
     */
    private void setCorrelationId(ContainerRequestContext context) {
        String correlationId = context.getHeaderString(Config.CORRELATION_ID_HEADER);
        if (correlationId == null || correlationId.isEmpty()) {
            correlationId = UUID.randomUUID().toString();
            // Set the header so we can pass it along to the resources
            context.getHeaders().add(Config.CORRELATION_ID_HEADER, correlationId);
        }

        // Set the correlation ID for both slf4j (used by Jersey) and
        // log4j2 (used by everything else)
        MDC.put(Config.CORRELATION_ID_HEADER, correlationId);
        ThreadContext.put(Config.CORRELATION_ID_HEADER, correlationId);
    }

}
