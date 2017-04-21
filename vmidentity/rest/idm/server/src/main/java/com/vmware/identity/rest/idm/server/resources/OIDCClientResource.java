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
import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.OIDCClientMetadataDTO;
import com.vmware.identity.rest.idm.server.mapper.OIDCClientMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Web service resource to manage OIDC clients associated per tenant basis.
 *
 * https://[address]/idm/tenant/<tenant name>/oidcclient/
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class OIDCClientResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(OIDCClientResource.class);

    public OIDCClientResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Add an OIDC client for tenant
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.TRUSTED_USER)
    public OIDCClientDTO add(OIDCClientMetadataDTO oidcClientMetadataDTO) {
        String clientId = null;
        try {
            OIDCClient oidcClient = OIDCClientMapper.getOIDCClient(oidcClientMetadataDTO);
            clientId = oidcClient.getClientId();
            getIDMClient().addOIDCClient(this.tenant, oidcClient);
            return OIDCClientMapper.getOIDCClientDTO(getIDMClient().getOIDCClient(this.tenant, clientId));
        } catch (NoSuchTenantException e) {
            log.debug("Failed to add an OIDC client for tenant '{}' due to missing tenant", this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to add an OIDC client for tenant '{}' due to a client side error", this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.oidcclient.create.failed", clientId, this.tenant), e);
        } catch (Exception e) {
            log.error("Failed to add an OIDC client for tenant '{}' due to a server side error", this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the details of all OIDC clients on requested tenant.
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.TRUSTED_USER)
    public Collection<OIDCClientDTO> getAll() {
        try {
            Collection<OIDCClient> oidcClients = getIDMClient().getOIDCClients(this.tenant);
            return OIDCClientMapper.getOIDCClientDTOs(oidcClients);

        } catch (NoSuchTenantException e) {
            log.debug("Failed to get OIDC clients from tenant '{}' due to missing tenant", this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to get OIDC clients from tenant '{}' due to a client side error", this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.oidcclient.getAll.failed", this.tenant), e);
        } catch (Exception e) {
            log.error("Failed to get OIDC clients from tenant '{}' due to a server side error", this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the details of an OIDC client on requested tenant. The name of an OIDC client can be used as unique identifier on per tenant basis.
     */
    @GET @Path("/{clientId}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.TRUSTED_USER)
    public OIDCClientDTO get(@PathParam("clientId") String clientId) {
        try {
            OIDCClient oidcClient = getIDMClient().getOIDCClient(this.tenant, clientId);

            OIDCClientDTO oidcClientDTO = null;
            if (oidcClient != null) {
                oidcClientDTO = OIDCClientMapper.getOIDCClientDTO(oidcClient);
            }

            return oidcClientDTO;
        } catch (NoSuchTenantException | NoSuchOIDCClientException e) {
            log.debug("Failed to get an OIDC client '{}' from tenant '{}' due to missing tenant or an OIDC client", clientId, this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to get an OIDC client '{}' from tenant '{}' due to a client side error", clientId, this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.oidcclient.get.failed", clientId, this.tenant), e);
        } catch (Exception e) {
            log.error("Failed to get an OIDC client '{}' from tenant '{}' due to a server side error", clientId, this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Delete an OIDC client from requested tenant.
     */
    @DELETE @Path("/{clientId}")
    @RequiresRole(role=Role.TRUSTED_USER)
    public void delete(@PathParam("clientId") String clientId) {
        try {
            getIDMClient().deleteOIDCClient(this.tenant, clientId);
        } catch (NoSuchTenantException | NoSuchOIDCClientException e) {
            log.debug("Failed to delete an OIDC client '{}' from tenant '{}' due to missing tenant or an OIDC client", clientId, this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException | InvalidPrincipalException e) {
            log.debug("Failed to delete an OIDC client '{}' from tenant '{}' due to a client side error", clientId, this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.oidcclient.delete.failed", clientId, this.tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete an OIDC client '{}' from tenant '{}' due to a server side error", clientId, this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }

    /**
     * Update an OIDC client
     */
    @PUT @Path("/{clientId}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.TRUSTED_USER)
    public OIDCClientDTO update(@PathParam("clientId") String clientId, OIDCClientMetadataDTO oidcClientMetadataDTO) {
        try {
            OIDCClientDTO oidcClientDTO = new OIDCClientDTO.Builder().
                    withClientId(clientId).
                    withOIDCClientMetadataDTO(oidcClientMetadataDTO).
                    build();
            OIDCClient oidcClient = OIDCClientMapper.getOIDCClient(oidcClientDTO);
            getIDMClient().setOIDCClient(this.tenant, oidcClient);
            return OIDCClientMapper.getOIDCClientDTO(getIDMClient().getOIDCClient(this.tenant, oidcClient.getClientId()));
        } catch (NoSuchTenantException | NoSuchOIDCClientException e) {
            log.debug("Failed to update an OIDC client '{}' on tenant '{}' due to missing tenant or an OIDC client", clientId, this.tenant, e);
            throw new NotFoundException(this.sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to update an OIDC client '{}' on tenant '{}' due to a client side error", clientId, this.tenant, e);
            throw new BadRequestException(this.sm.getString("res.oidcclient.update.failed", clientId, this.tenant), e);
        } catch (Exception e) {
            log.error("Failed to update an OIDC client '{}' on tenant '{}' due to a server side error", clientId, this.tenant, e);
            throw new InternalServerErrorException(this.sm.getString("ec.500"), e);
        }
    }
}
