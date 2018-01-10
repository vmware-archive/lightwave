/*
 *  Copyright (c) 2012-2017 VMware, Inc.  All Rights Reserved.
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
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.FederatedIdpDTO;
import com.vmware.identity.rest.idm.server.mapper.ExternalIDPMapper;
import com.vmware.identity.rest.idm.server.mapper.FederatedIdpMapper;
import com.vmware.identity.rest.idm.server.util.Config;
import org.apache.commons.codec.binary.Base64;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import javax.ws.rs.*;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.net.URLDecoder;
import java.util.Collection;

/**
 * All operations related to Federated identity providers are implemented in this
 * class. All of the operations are tenant-based.
 *
 * NOTE Ideally this would fit under the same concept as an Identity Provider, but IDM models them completely
 * separate, meaning that for now, REST IDM must as well.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 * @author Sriram Nambakam
 */
public class FederatedIdpResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(FederatedIdpResource.class);

    public FederatedIdpResource(
            String tenant,
            @Context ContainerRequestContext request,
            @Context SecurityContext securityContext
    ) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Collection<FederatedIdpDTO> getAll() {
        Collection<FederatedIdpDTO> federatedIDPs = null;

        try {
            Collection<IDPConfig> configs = getIDMClient().getAllExternalIdpConfig(tenant);
            federatedIDPs = FederatedIdpMapper.getFederatedIdpDTOs(configs);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to retrieve federated identity providers for tenant '{}'", tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error(
                "Failed to retrieve federated identity providers for tenant '{}' due to a server side error",
                tenant,
                e
            );
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

        return federatedIDPs;
    }

    /**
     * Register external identity provider (ADFS, Shibboleth etc) with provided metadata
     *
     */
    @POST
    @Path("/json")
    @Consumes(MediaType.APPLICATION_FORM_URLENCODED)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public FederatedIdpDTO registerWithMetadata(String jsonConfig) {
        try {
            Validate.notEmpty(jsonConfig, "IDP Configuration");

            String entityId = getIDMClient().importFederatedIdpConfiguration(
                                                    tenant,
                                                    new String(
                                                            Base64.decodeBase64(
                                                                    URLDecoder.decode(jsonConfig, "UTF-8")
                                                            ),
                                                            "UTF-8"
                                                    )
            );
            return FederatedIdpMapper.getFederatedIdpDTO(getIDMClient().getExternalIdpConfigForTenant(tenant, entityId));
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

    @POST
    @Consumes(MediaType.APPLICATION_JSON)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public FederatedIdpDTO register(FederatedIdpDTO federatedIdp) {
        try {
            Validate.notNull(federatedIdp, "Federated IDP");

            IDPConfig config = FederatedIdpMapper.getIDPConfig(federatedIdp);
            getIDMClient().setExternalIdpConfig(tenant, config);
            return FederatedIdpMapper.getFederatedIdpDTO(
                                        getIDMClient().getExternalIdpConfigForTenant(
                                                            tenant,
                                                            config.getEntityID()
                                        )
            );
        } catch (NoSuchTenantException e) {
            log.warn(
                    "Failed to register external identity provider '{}' for tenant '{}'",
                    federatedIdp.getEntityID(),
                    tenant,
                    e
            );
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | DTOMapperException e) {
            log.warn(
                    "Failed to register external identity provider '{}' for tenant '{}' due to a client side error",
                    federatedIdp.getEntityID(),
                    tenant,
                    e
            );
            throw new BadRequestException(
                            sm.getString("res.external.add.failed", federatedIdp.getEntityID(), tenant),
                            e
            );
        } catch (Exception e) {
            log.error(
                    "Failed to register external identity provider '{}' for tenant '{}' due to a server side error",
                    federatedIdp.getEntityID(),
                    tenant,
                    e
            );
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    @GET
    @Path("/{entityID}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public FederatedIdpDTO get(@PathParam("entityID") String entityID) {
        try {
            Validate.notEmpty(entityID, "Entity ID");

            String decodedEntityId = new String(Base64.decodeBase64(entityID.getBytes("UTF-8")), "UTF-8");
            IDPConfig config = getIDMClient().getExternalIdpConfigForTenant(tenant, decodedEntityId);

            if (config == null) {
                throw new NoSuchExternalIdpConfigException(String.format(
                                "External IDP config [%s] does not exist for tenant [%s]",
                                entityID,
                                tenant
                            )
                );
            }

            return FederatedIdpMapper.getFederatedIdpDTO(config);
        } catch (NoSuchTenantException | NoSuchExternalIdpConfigException e) {
            log.warn(
                    "Failed to retrieve external identity provider '{}' from tenant '{}'",
                    entityID,
                    tenant,
                    e
            );
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error(
                    "Failed to retrieve external identity provider '{}' from tenant '{}' doe to a server side error",
                    entityID,
                    tenant,
                    e
            );
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    @DELETE
    @Path("/{entityID}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void
    delete(
            @PathParam("entityID") String entityID,
            @DefaultValue("false") @QueryParam("remove") Boolean removeJitUsers
    ) {
        try {
            String decodedEntityId = new String(Base64.decodeBase64(entityID.getBytes("UTF-8")), "UTF-8");
            getIDMClient().removeExternalIdpConfig(tenant, decodedEntityId, removeJitUsers);
        } catch (NoSuchTenantException | NoSuchExternalIdpConfigException e) {
            log.warn("Failed to remove external identity provider '{}' from tenant '{}'", entityID, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error(
                "Failed to remove external identity provider '{}' from tenant '{}' due to a server side error",
                entityID,
                tenant,
                e
            );
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }
}
