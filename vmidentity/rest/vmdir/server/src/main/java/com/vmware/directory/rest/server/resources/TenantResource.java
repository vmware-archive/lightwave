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
package com.vmware.directory.rest.server.resources;

import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.SecurityContext;

import com.vmware.directory.rest.server.PathParameters;
import com.vmware.directory.rest.server.util.Config;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.resources.BaseResource;

/**
 * Tenant resource. Serves information specifically about tenants.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Path("/tenant")
public class TenantResource extends BaseResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(TenantResource.class);

    public TenantResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/groups")
    public GroupResource getGroupSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new GroupResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/users")
    public UserResource getUserSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new UserResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/solutionusers")
    public SolutionUserResource getSolutionUserSubResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new SolutionUserResource(tenantName, getRequest(), getSecurityContext());
    }

    @Path(PathParameters.TENANT_NAME_VAR + "/config")
    public ConfigurationResource getConfigurationResource(@PathParam(PathParameters.TENANT_NAME) String tenantName) {
        return new ConfigurationResource(tenantName, getRequest(), getSecurityContext());
    }

}