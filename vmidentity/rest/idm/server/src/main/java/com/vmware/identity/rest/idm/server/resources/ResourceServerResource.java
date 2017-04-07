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

import java.util.Collection;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchResourceServerException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.server.mapper.ResourceServerMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Web service resource to manage resource servers associated per tenant basis.
 *
 * https://[address]/idm/tenant/<tenant name>/resourceserver/
 *
 * @author Yehia Zayour
 */
public class ResourceServerResource extends BaseSubResource {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(ResourceServerResource.class);

    public ResourceServerResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Add resource server for tenant
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public ResourceServerDTO add(ResourceServerDTO resourceServerDTO) {
        try {
            ResourceServer resourceServer = ResourceServerMapper.getResourceServer(resourceServerDTO);
            getIDMClient().addResourceServer(this.tenant, resourceServer);
            return ResourceServerMapper.getResourceServerDTO(getIDMClient().getResourceServer(this.tenant, resourceServer.getName()));
        } catch (NoSuchTenantException e) {
            logger.debug("Failed to add resource server for tenant '{}' due to missing tenant", this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            logger.debug("Failed to add resource server for tenant '{}' due to a client side error", this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.resourceserver.create.failed", resourceServerDTO.getName(), this.tenant), e);
        } catch (Exception e) {
            logger.error("Failed to add resource server for tenant '{}' due to a server side error", this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the details of all resource servers on requested tenant.
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Collection<ResourceServerDTO> getAll() {
        try {
            Collection<ResourceServer> resourceServers = getIDMClient().getResourceServers(this.tenant);
            return ResourceServerMapper.getResourceServerDTOs(resourceServers);
        } catch (NoSuchTenantException e) {
            logger.debug("Failed to get resource servers from tenant '{}' due to missing tenant", this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            logger.debug("Failed to get resource servers from tenant '{}' due to a client side error", this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.resourceserver.getAll.failed", this.tenant), e);
        } catch (Exception e) {
            logger.error("Failed to get resource servers from tenant '{}' due to a server side error", this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the details of resource server on requested tenant. The name of resource server can be used as unique identifier on per tenant basis.
     */
    @GET @Path("/{resourceServerName}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public ResourceServerDTO get(@PathParam("resourceServerName") String resourceServerName) {
        try {
            ResourceServer resourceServer = getIDMClient().getResourceServer(this.tenant, resourceServerName);
            return ResourceServerMapper.getResourceServerDTO(resourceServer);
        } catch (NoSuchTenantException | NoSuchResourceServerException e) {
            logger.debug("Failed to get resource server '{}' on tenant '{}' due to missing tenant or resource server", resourceServerName, this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            logger.debug("Failed to get resource server '{}' from tenant '{}' due to a client side error", resourceServerName, this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.resourceserver.get.failed", resourceServerName, this.tenant), e);
        } catch (Exception e) {
            logger.error("Failed to get resource server '{}' from tenant '{}' due to a server side error", resourceServerName, this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Delete resource server from requested tenant.
     */
    @DELETE @Path("/{resourceServerName}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("resourceServerName") String resourceServerName) {
        try {
            getIDMClient().deleteResourceServer(this.tenant, resourceServerName);
        } catch (NoSuchTenantException | NoSuchResourceServerException e) {
            logger.debug("Failed to delete resource server '{}' on tenant '{}' due to missing tenant or resource server", resourceServerName, this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException | InvalidPrincipalException e) {
            logger.debug("Failed to delete resource server '{}' from tenant '{}' due to a client side error", resourceServerName, this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.resourceserver.delete.failed", resourceServerName, this.tenant), e);
        } catch (Exception e) {
            logger.error("Failed to delete resource server '{}' from tenant '{}' due to a server side error", resourceServerName, this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Update resource server
     */
    @PUT @Path("/{resourceServerName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public ResourceServerDTO update(@PathParam("resourceServerName") String resourceServerName, ResourceServerDTO resourceServerDTO) {
        try {
            if (!resourceServerName.equals(resourceServerDTO.getName())) {
                throw new InvalidArgumentException("resource server name in url path does not match that in request body");
            }
            ResourceServer resourceServer = ResourceServerMapper.getResourceServer(resourceServerDTO);
            getIDMClient().setResourceServer(this.tenant, resourceServer);
            return ResourceServerMapper.getResourceServerDTO(getIDMClient().getResourceServer(this.tenant, resourceServer.getName()));
        } catch (NoSuchTenantException | NoSuchResourceServerException e) {
            logger.debug("Failed to update resource server '{}' on tenant '{}' due to missing tenant or resource server", resourceServerName, this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            logger.debug("Failed to update resource server '{}' on tenant '{}' due to a client side error", resourceServerName, this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.resourceserver.update.failed", resourceServerName, this.tenant), e);
        } catch (Exception e) {
            logger.error("Failed to update resource server '{}' on tenant '{}' due to a server side error", resourceServerName, this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }
}
