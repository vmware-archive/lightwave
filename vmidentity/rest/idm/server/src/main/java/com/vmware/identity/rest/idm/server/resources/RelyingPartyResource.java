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

import java.io.ByteArrayInputStream;
import java.io.IOException;
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
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchRelyingPartyException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.server.mapper.RelyingPartyMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Web service resource to manage relying parties associated per tenant basis.
 *
 * https://[address]/idm/tenant/<tenant name>/relyingparty/
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class RelyingPartyResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(RelyingPartyResource.class);

    public RelyingPartyResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Add a relying party for tenant
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public RelyingPartyDTO add(RelyingPartyDTO relyingParty) {
        try {
            RelyingParty rp = RelyingPartyMapper.getRelyingParty(relyingParty);
            getIDMClient().addRelyingParty(tenant, rp);
            return RelyingPartyMapper.getRelyingPartyDTO(getIDMClient().getRelyingParty(tenant, relyingParty.getName()));
        } catch (NoSuchTenantException e) {
            log.debug("Failed to add a relying party for tenant '{}' due to missing tenant", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to add a relying party for tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.relyingparty.create.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to add a relying party for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Register relying party via XML
     */
    @POST
    @Consumes(MediaType.APPLICATION_XML)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void add(String relyingPartyXMLString) throws Exception {
        try{
            Document xmlDoc = parseToDocument(relyingPartyXMLString);
            getIDMClient().importTenantConfiguration(tenant, xmlDoc);
        }  catch (NoSuchTenantException e) {
            log.debug("Failed to add a relying party for tenant '{}' due to missing tenant", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to add a relying party for tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.relyingparty.create.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to add a relying party for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    public static Document parseToDocument(String relyingPartyConfigXML)
            throws IOException, SAXException {

            assert relyingPartyConfigXML != null;

            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setNamespaceAware(true);
            dbf.setValidating(false);

            DocumentBuilder builder;
            try {
               builder = dbf.newDocumentBuilder();

            } catch (ParserConfigurationException e) {
               throw new IllegalStateException(e);
            }
            ByteArrayInputStream strIn = new ByteArrayInputStream(relyingPartyConfigXML.getBytes()); // No need to close this stream!

            return builder.parse(strIn);
         }

    /**
     * Get the details of all relying parties on requested tenant.
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Collection<RelyingPartyDTO> getAll() {
        try {
            Collection<RelyingParty> idmRelyingParties = getIDMClient().getRelyingParties(tenant);
            return RelyingPartyMapper.getRelyingPartyDTOs(idmRelyingParties);

        } catch (NoSuchTenantException e) {
            log.debug("Failed to get relying parties from tenant '{}' due to missing tenant", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to get relying parties from tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.relyingparty.getAll.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to get relying parties from tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the details of relying party on requested tenant. The name of relying party can be used as unique identifier on per tenant basis.
     */
    @GET @Path("/{relyingPartyName}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public RelyingPartyDTO get(@PathParam("relyingPartyName") String relyingPartyName) {

        RelyingPartyDTO relyingParty = null;
        try {
            RelyingParty idmRelyingParty = null;
            idmRelyingParty = getIDMClient().getRelyingParty(tenant, relyingPartyName);

            if (idmRelyingParty != null) {
                relyingParty = RelyingPartyMapper.getRelyingPartyDTO(idmRelyingParty);
            }

            return relyingParty;
        } catch (NoSuchTenantException | NoSuchRelyingPartyException e) {
            log.debug("Failed to get a relying party '{}' from tenant '{}' due to missing tenant or a relying party", relyingPartyName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to get a relying party '{}' from tenant '{}' due to a client side error", relyingPartyName, tenant, e);
            throw new BadRequestException(sm.getString("res.relyingparty.get.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to get a relying party '{}' from tenant '{}' due to a server side error", relyingPartyName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Delete a relying party from requested tenant.
     */
    @DELETE @Path("/{relyingPartyName}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("relyingPartyName") String relyingPartyName) {
        try {
            getIDMClient().deleteRelyingParty(tenant, relyingPartyName);
        } catch (NoSuchTenantException | NoSuchRelyingPartyException e) {
            log.debug("Failed to delete a relying party '{}' from tenant '{}' due to missing tenant or a relying party", relyingPartyName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException | InvalidPrincipalException e) {
            log.debug("Failed to delete a relying party '{}' from tenant '{}' due to a client side error", relyingPartyName, tenant, e);
            throw new BadRequestException(sm.getString("res.relyingparty.delete.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete a relying party '{}' from tenant '{}' due to a server side error", relyingPartyName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Update a relying party
     */
    @PUT @Path("/{relyingPartyName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public RelyingPartyDTO update(RelyingPartyDTO relyingParty) {
        try {
            RelyingParty rpToUpdate = RelyingPartyMapper.getRelyingParty(relyingParty);
            getIDMClient().setRelyingParty(tenant, rpToUpdate);
            return RelyingPartyMapper.getRelyingPartyDTO(getIDMClient().getRelyingParty(tenant, relyingParty.getName()));
        } catch (NoSuchTenantException | NoSuchRelyingPartyException e) {
            log.debug("Failed to update a relying party '{}' on tenant '{}' due to missing tenant or a relying party", relyingParty.getName(), tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException  e) {
            log.debug("Failed to update a relying party '{}' on tenant '{}' due to a client side error", relyingParty.getName(), tenant, e);
            throw new BadRequestException(sm.getString("res.relyingparty.update.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to update a relying party '{}' on tenant '{}' due to a server side error", relyingParty.getName(), tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }
}
