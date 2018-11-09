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

import java.util.ArrayList;
import java.util.Collection;

import javax.ws.rs.Consumes;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Produces;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.idm.data.AttributeDTO;
import com.vmware.identity.rest.idm.data.AttributeDefinitionsDTO;
import com.vmware.identity.rest.idm.data.UpdateAttributesDTO;
import com.vmware.identity.rest.idm.server.mapper.AttributeMapper;
import com.vmware.identity.rest.idm.server.util.Config;

import io.prometheus.client.Histogram;

/**
 * All operations related to identity providers( aka identity sources) are implemented in this
 * class. All the operations implemented here are tenant based( i.e per tenant)
 * <p>
 * The supported operations include : <br/>
 * <li> Get All identity providers associated with tenant </li>
 * <li> Add a new identity provider </li>
 * <li> Get requested identity provider only</li>
 * <li> Update identity provider</li>
 * <li> Delete identity provider from tenant</li>
 * <li> Probe the existing identity provider in tenant </li>
 * </p>
 *
 */

public class AttributesResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(IdentityProviderResource.class);

    private static final String METRICS_COMPONENT = "idm";
    private static final String METRICS_RESOURCE = "AttributesResource";

    public AttributesResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Retrieve all attributes associated with requested tenant
     * @return A Collection of attributes @see {@link AttributeDTO}
     * @throws BadRequestException On client side errors like bad input.
     * @throws InternalServerErrorException Otherwise
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public Collection<AttributeDTO> getAll() {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "getAll").startTimer();
        String responseStatus = HTTP_OK;
        Collection<AttributeDTO> attributes = null;
        try {
            Collection<Attribute> attrs = getIDMClient().getAttributeDefinitions(tenant);
            attributes = AttributeMapper.getAttributeDTOs(attrs);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to retrieve attributes for tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve attributes for tenant {} due to a client side error", tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.ten.attributes.get.all.failed", tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve attributes for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "getAll").inc();
            requestTimer.observeDuration();
        }
        return attributes;
    }

    /**
     * Set attributes for tenant
     * @param attributes list of attributes
     * @return details of added identity provider
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Collection<AttributeDTO> set(AttributeDefinitionsDTO attributesDTO) throws DTOMapperException {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "set").startTimer();
        String responseStatus = HTTP_OK;
        try {
            getIDMClient().setAttributeDefinitions(tenant, AttributeMapper.getAttributes(attributesDTO));
            Collection<Attribute> attrs = getIDMClient().getAttributeDefinitions(tenant);
            return AttributeMapper.getAttributeDTOs(attrs);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to set attributes for tenant '{}'", tenant, e);
            responseStatus = "404";
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve attributes for tenant {} due to a client side error", tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.ten.attributes.set.failed", tenant), e);
        } catch (BadRequestException e) {
            responseStatus = HTTP_BAD_REQUEST;
            throw e;
        } catch (Exception e) {
            log.error("Failed to set attributes 'for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "set").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Set attributes for tenant
     * @param attributes list of attributes
     * @return details of added identity provider
     */
    @PUT
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Collection<AttributeDTO> update(UpdateAttributesDTO attributesUpdate) throws DTOMapperException {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, METRICS_RESOURCE, "update").startTimer();
        String responseStatus = HTTP_OK;
        try {
            if (attributesUpdate != null){
                Collection<Attribute> attrs = getIDMClient().getAttributeDefinitions(tenant);
                ArrayList<Attribute> newAttributes = new ArrayList<Attribute>(attrs);
                if ( attributesUpdate.getRemove() != null ) {
                    for(String attr : attributesUpdate.getRemove()) {
                        if (attr != null){
                            for( Attribute a : attrs ) {
                                if ( attr.equals(a.getName()) ){
                                    newAttributes.remove(a);
                                }
                            }
                        }
                    }
                }
                if ( attributesUpdate.getAdd() != null) {
                    Collection<Attribute> idmAttributes = AttributeMapper.getAttributes(attributesUpdate.getAdd());
                    newAttributes.addAll(idmAttributes);
                }
                getIDMClient().setAttributeDefinitions(tenant, newAttributes);
            }
            return AttributeMapper.getAttributeDTOs(getIDMClient().getAttributeDefinitions(tenant));
        } catch (NoSuchTenantException e) {
            log.warn("Failed to update attributes for tenant '{}'", tenant, e);
            responseStatus = "404";
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to update attributes for tenant {} due to a client side error", tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.ten.attributes.update.failed", tenant), e);
        } catch (BadRequestException e) {
            responseStatus = HTTP_BAD_REQUEST;
            throw e;
        } catch (Exception e) {
            log.error("Failed to update attributes 'for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, responseStatus, METRICS_RESOURCE, "update").inc();
            requestTimer.observeDuration();
        }
    }
}
