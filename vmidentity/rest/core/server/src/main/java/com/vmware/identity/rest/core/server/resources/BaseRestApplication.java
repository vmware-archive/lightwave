/*
 *  Copyright (c) 2016 VMware, Inc.  All Rights Reserved.
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

import javax.ws.rs.ApplicationPath;

import org.glassfish.jersey.server.ResourceConfig;
import org.slf4j.bridge.SLF4JBridgeHandler;

import com.vmware.identity.rest.core.server.authorization.filter.AuthorizationRequestFilter;
import com.vmware.identity.rest.core.server.filter.ContextFilter;
import com.vmware.identity.rest.core.server.filters.VmIdentityCORSResponseFilter;

/**
 * A Jersey Based JAX-RS compliant, Resource registry for RESTful services. This contains the common configuration that can be shared across sub-projects.
 * This will be extended by other Jersey-based services like IDM, AFD and Lookup to register resources.
 *
 * @author Balaji Boggaram Ramanarayan
 */
@ApplicationPath("/")
public class BaseRestApplication extends ResourceConfig {

    public BaseRestApplication() {

        // Install bridge for SLF4J
        SLF4JBridgeHandler.removeHandlersForRootLogger();
        SLF4JBridgeHandler.install();

        // Register the Context Filter
        register(ContextFilter.class);

        // Register Authorization Provider
        register(AuthorizationRequestFilter.class);

        // Register CORS (Cross-Origin Resource Sharing) Provider
        register(VmIdentityCORSResponseFilter.class);

    }
}
