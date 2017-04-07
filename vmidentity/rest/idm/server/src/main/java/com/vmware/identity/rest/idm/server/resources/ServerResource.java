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

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Locale;

import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.VmHostData;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;
import com.vmware.identity.rest.idm.data.attributes.ComputerType;
import com.vmware.identity.rest.idm.server.mapper.ServerDetailsMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Server resource. Serves global information about the IDM server.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Path("/server")
public class ServerResource extends BaseResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(ServerResource.class);

    public ServerResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    public ServerResource(Locale locale, String correlationId, SecurityContext securityContext) {
        super(locale, correlationId, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Retrieve all computers associated with tenant
     *
     * @param tenantName Name of tenant
     * @param computerType Type of resource{Valid : computer, dc, all}
     * @return List of Computers associated with tenant. @see {@link VmHostData}
     * @throws IllegalArgumentException On invalid params in client request
     * @throws InternalServerErrorException Otherwise
     */
    @GET @Path("/computers")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.CONFIGURATION_USER)
    public Collection<ServerDetailsDTO> getComputers(@DefaultValue("all") @QueryParam("type") String computerType)
    {
        Collection<VmHostData> computers = new HashSet<VmHostData>();
        String systemTenant = null;
        ComputerType compType = getComputerType(computerType);
        Validate.notNull(compType, sm.getString("valid.invalid.type", "type", Arrays.toString(ComputerType.values())));
        try {
            systemTenant = getIDMClient().getSystemTenant();
            switch (compType) {
                case DC:
                    computers = getIDMClient().getComputers(systemTenant, true);
                    break;
                case COMPUTER:
                    // TODO : Refactor after implementing cleaner IDM API.
                    computers = getIDMClient().getComputers(systemTenant, false); // Retrieves both dc + computer accounts

                    // Remove the DCs
                    Iterator<VmHostData> iter = computers.iterator();
                    while (iter.hasNext()) {
                        VmHostData data = iter.next();
                        if (data.isDomainController()) {
                            iter.remove();
                        }
                    }
                    break;
                case ALL:
                    computers = getIDMClient().getComputers(systemTenant, false);
                    break;
            }
            return ServerDetailsMapper.getServerDetailsDTOs(computers);
        } catch (Exception e) {
            log.error("Failed to retrieve computers from system tenant ('{}') due to a server side error", systemTenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Retrieve server status information.
     *
     * @return
     */
    @GET @Path("/status")
    public void getStatus() {
        // TODO : Yet to implement
        // throw new NotImplementedError("ServerResource#getStatus not implemented");
    }

    private ComputerType getComputerType(String computerType) {
        try {
            return ComputerType.valueOf(computerType.toUpperCase());
        } catch (IllegalArgumentException | NullPointerException e) {
            // ignore and we'll just return null
        }
        return null;
    }
}
