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
package com.vmware.identity.rest.afd.server;

import java.util.logging.Logger;

import javax.ws.rs.ApplicationPath;

import org.glassfish.jersey.filter.LoggingFilter;
import org.glassfish.jersey.server.ResourceConfig;
import org.glassfish.jersey.server.ServerProperties;
import org.slf4j.bridge.SLF4JBridgeHandler;

import com.vmware.identity.rest.core.server.authorization.filter.AuthorizationRequestFilter;
import com.vmware.identity.rest.core.server.filter.ContextFilter;
import com.vmware.identity.rest.core.server.resources.BaseRestApplication;

/**
 * Base application for AFD REST.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class AfdRestApplication extends BaseRestApplication {

    private static final String ENTITY_LOG_PROPERTY = "vmware.rest.afd.entitylogging";
    private static final String TRACE_LOG_PROPERTY = "vmware.rest.afd.tracing";

    public AfdRestApplication() {

        super();

        // Register resources and providers under com.vmware.identity.rest.afd.server
        packages(true, "com.vmware.identity.rest.afd.server");

        // Register LoggingFilter with optional entity logging
        Logger log = Logger.getLogger(AfdRestApplication.class.getName());
        boolean entityLog = Boolean.parseBoolean(System.getProperty(ENTITY_LOG_PROPERTY, "false"));
        // Register the LoggingFilter so it occurs after the ContextFilter so Correlation ID works correctly
        register(new LoggingFilter(log, entityLog), Integer.MIN_VALUE + 1);

        // Enable tracing support
        // Supply header during request: "X-Jersey-Tracing-Accept:true"
        boolean traceLog = Boolean.parseBoolean(System.getProperty(TRACE_LOG_PROPERTY, "false"));
        if (traceLog) {
            property(ServerProperties.TRACING, "ON_DEMAND");
        }
    }
}
