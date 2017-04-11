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
import javax.ws.rs.DefaultValue;
import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
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
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.server.mapper.ExternalIDPMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * All operations related to external identity providers are implemented in this
 * class. All of the operations are tenant-based.
 *
 * NOTE Ideally this would fit under the same concept as an Identity Provider, but IDM models them completely
 * separate, meaning that for now, REST IDM must as well.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ExternalIDPResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(ExternalIDPResource.class);

    public ExternalIDPResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Collection<ExternalIDPDTO> getAll() {
        Collection<ExternalIDPDTO> externalIDPs = null;

        try {
            Collection<IDPConfig> configs = getIDMClient().getAllExternalIdpConfig(tenant);
            externalIDPs = ExternalIDPMapper.getExternalIDPDTOs(configs);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to retrieve external identity providers for tenant '{}'", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to retrieve identity providers for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

        return externalIDPs;
    }

    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public ExternalIDPDTO register(ExternalIDPDTO externalIDP) {
        try {
            IDPConfig config = ExternalIDPMapper.getIDPConfig(externalIDP);
            getIDMClient().setExternalIdpConfig(tenant, config);
            return ExternalIDPMapper.getExternalIDPDTO(getIDMClient().getExternalIdpConfigForTenant(tenant, config.getEntityID()));
        } catch (NoSuchTenantException e) {
            log.warn("Failed to register external identity provider '{}' for tenant '{}'", externalIDP.getEntityID(), tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | DTOMapperException e) {
            log.warn("Failed to register external identity provider '{}' for tenant '{}' due to a client side error", externalIDP.getEntityID(), tenant, e);
            throw new BadRequestException(sm.getString("res.external.add.failed", externalIDP.getEntityID(), tenant), e);
        } catch (Exception e) {
            log.error("Failed to register external identity provider '{}' for tenant '{}' due to a server side error", externalIDP.getEntityID(), tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Register external identity provider (ADFS, Shibboleth etc) with provided metadata
     *
     */
    @POST
    @Consumes(MediaType.APPLICATION_XML)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public ExternalIDPDTO registerWithMetadata(String externalIDPConfig) {
        try {
            String entityId = getIDMClient().importExternalIDPConfiguration(tenant, parseSAMLConfig(externalIDPConfig));
            return ExternalIDPMapper.getExternalIDPDTO(getIDMClient().getExternalIdpConfigForTenant(tenant, entityId));
        } catch (NoSuchTenantException e) {
            log.warn("Failed to register external identity provider for tenant '{}'", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | DTOMapperException e) {
            log.warn("Failed to register external identity provider for tenant '{}' due to a client side error", tenant, e);
            throw new BadRequestException(sm.getString("res.external.add.metadata.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to register external identity provider for tenant '{}' due to a server side error", tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    @GET @Path("/{entityID}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public ExternalIDPDTO get(@PathParam("entityID") String entityID) {
        try {
            IDPConfig config = getIDMClient().getExternalIdpConfigForTenant(tenant, entityID);

            if (config == null) {
                throw new NoSuchExternalIdpConfigException(String.format("External IDP config [%s] does not exist for tenant [%s]", entityID, tenant));
            }

            return ExternalIDPMapper.getExternalIDPDTO(config);
        } catch (NoSuchTenantException | NoSuchExternalIdpConfigException e) {
            log.warn("Failed to retrieve external identity provider '{}' from tenant '{}'", entityID, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to retrieve external identity provider '{}' from tenant '{}' doe to a server side error", entityID, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    @DELETE @Path("/{entityID}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("entityID") String entityID, @DefaultValue("false") @QueryParam("remove") Boolean removeJitUsers) {
        try {
            getIDMClient().removeExternalIdpConfig(tenant, entityID, removeJitUsers);
        } catch (NoSuchTenantException | NoSuchExternalIdpConfigException e) {
            log.warn("Failed to remove external identity provider '{}' from tenant '{}'", entityID, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to remove external identity provider '{}' from tenant '{}' due to a server side error", entityID, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Parse SAML configuration value passed as argument to DOM document.
     *
     * @param samlConfigDoc
     *           SAML configuration
     * @return DOM document representing SAML configuration
     *
     * @throws SAXException
     *            If any parse errors occur.
     * @throws IOException
     *            If any IO errors occur
     */
    public static Document parseSAMLConfig(String samlConfigDoc)
       throws IOException, SAXException {

       assert samlConfigDoc != null;

       DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
       dbf.setNamespaceAware(true);
       dbf.setValidating(false);

       DocumentBuilder builder;
       try {
          builder = dbf.newDocumentBuilder();

       } catch (ParserConfigurationException e) {
          throw new IllegalStateException(e);
       }
       ByteArrayInputStream strIn = new ByteArrayInputStream(samlConfigDoc.getBytes()); // No need to close this stream!

       return builder.parse(strIn);
    }

}
