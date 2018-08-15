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

import java.net.MalformedURLException;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ADIDSAlreadyExistException;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidProviderException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.UpdateAttributesMapDTO;
import com.vmware.identity.rest.idm.data.attributes.IdentityProviderType;
import com.vmware.identity.rest.idm.server.mapper.IdentityProviderMapper;
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
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *
 */

public class IdentityProviderResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(IdentityProviderResource.class);

    private static final String METRICS_COMPONENT = "idm";
    private static final String METRICS_RESOURCE = "IdentityProviderResource";

    public IdentityProviderResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Retrieve all identity providers/sources associated with requested tenant
     * @return A Collection of identity providers @see {@link IdentityProviderDTO}
     * @throws BadRequestException On client side errors like bad input.
     * @throws InternalServerErrorException Otherwise
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public Collection<IdentityProviderDTO> getAll() {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "getAll").startTimer();
        String responseStatus = HTTP_OK;
        Collection<IdentityProviderDTO> identityProviders = null;
        try {
            Collection<IIdentityStoreData> identitySources = getIDMClient().getProviders(tenant);
            identityProviders = IdentityProviderMapper.getIdentityProviderDTOs(identitySources);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to retrieve identity providers for tenant '{}'", tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve identity providers for tenant {} due to a client side error", tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.get.all.failed"), e);
        } catch (Exception e) {
            log.error("Failed to retrieve identity providers for tenant '{}' due to a server side error", tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "getAll").inc();
            requestTimer.observeDuration();
        }
        return identityProviders;
    }

    /**
     * Add identity provider for tenant / Probe connectivity of identity provider
     * @param identityProvider details of identity provider
     * @param probe URI used to probe/examine connectivity with identity provider. Should be set to test connectivity only.
     * @return details of added identity provider
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public IdentityProviderDTO create(IdentityProviderDTO identityProvider, @DefaultValue("false") @QueryParam("probe") boolean probe) throws DTOMapperException {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "create").startTimer();
        String responseStatus = HTTP_OK;
        try {
            validateProviderType(identityProvider.getType());
            validateAuthenticationType(identityProvider.getType(), identityProvider.getAuthenticationType());
            if (probe) {
                // Currently, Probing AD as LDAP and OpenLDAP are supported only
                validateProbeEssentials(identityProvider);
                IIdentityStoreData identityStoreData = IdentityProviderMapper.getIdentityStoreData(identityProvider);
                getIDMClient().probeProviderConnectivity(tenant, identityStoreData);
                return IdentityProviderMapper.sanitizeProbeResponse(identityProvider);
            } else {
                IIdentityStoreData identityStoreData = IdentityProviderMapper.getIdentityStoreData(identityProvider);
                getIDMClient().addProvider(tenant, identityStoreData);
                return IdentityProviderMapper.getIdentityProviderDTO(getIDMClient().getProvider(tenant, identityProvider.getName()));
            }

        } catch (NoSuchTenantException e) {
            log.warn("Failed to {} identity provider '{}' for tenant '{}'", probe ? "probe" : "add", identityProvider.getName(), tenant, e);
            responseStatus = "404";
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (MalformedURLException | InvalidProviderException | ADIDSAlreadyExistException | IDMLoginException e) {
            log.warn("Failed to {} identity provider '{}' for tenant '{}' due to a client side error", probe ? "probe" : "add", identityProvider.getName(), tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.add.failed", identityProvider.getName(), tenant), e);
        } catch (BadRequestException e) {
            responseStatus = HTTP_BAD_REQUEST;
            throw e;
        } catch (Exception e) {
            log.error("Failed to {} identity provider '{}' for tenant '{}' due to a server side error", probe ? "probe" : "add", identityProvider.getName(), tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "create").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Get information of requested identity provider.
     * @param providerName Name of identity source/provider for which info is requested.
     * @return Information of identity provider @see {@link IdentityProviderDTO}
     */
    @GET @Path("/{providerName}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public IdentityProviderDTO get(@PathParam("providerName") String providerName) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "get").startTimer();
        String responseStatus = HTTP_OK;
        try {
            IIdentityStoreData identitySource = getIDMClient().getProvider(tenant, providerName);

            if (identitySource == null) {
                throw new InvalidProviderException(String.format("Provider '%s' does not exist in tenant '%s'", providerName, tenant), providerName, tenant);
            }

            return IdentityProviderMapper.getIdentityProviderDTO(identitySource);
        } catch (NoSuchTenantException | InvalidProviderException e) {
            log.warn("Failed to retrieve identity provider '{}' from tenant '{}'", providerName, tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve identity provider '{}' from tenant '{}' due to a client side error", providerName, tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.get.failed", providerName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve identity provider '{}' from tenant '{}' due to a server side error", providerName, tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "get").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Update details of identity provider.
     *
     * @param providerName Name of registered identity provider
     * @param identityProvider Details of identity provider to be updated with. @see {@link IdentityProviderDTO}
     * @return Details of updated identity provider
     */
    @PUT @Path("/{providerName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public IdentityProviderDTO update(@PathParam("providerName") String providerName, IdentityProviderDTO identityProvider) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "update").startTimer();
        String responseStatus = HTTP_OK;
        try {
            validateProviderType(identityProvider.getType());
            validateAuthenticationType(identityProvider.getType(), identityProvider.getAuthenticationType());
            IIdentityStoreData identityStoreData = IdentityProviderMapper.getIdentityStoreData(identityProvider);
            getIDMClient().setProvider(tenant, identityStoreData);
            return IdentityProviderMapper.getIdentityProviderDTO(getIDMClient().getProvider(tenant, providerName));
        } catch (NoSuchTenantException e) {
            log.warn("Failed to update identity provider '{}' for tenant '{}'", providerName, tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | IDMLoginException | InvalidProviderException e) {
            log.warn("Failed to update identity provider '{}' for tenant {} due to a client side error", providerName, tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.update.failed", providerName, tenant), e);
        } catch (BadRequestException e) {
            responseStatus = HTTP_BAD_REQUEST;
            throw e;
        } catch (Exception e) {
            log.error("Failed to update identity provider '{}' for tenant '{}' due to a server side error", providerName, tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "update").inc();
            requestTimer.observeDuration();
        }
    }

   /**
     * Get attributes map of requested identity provider.
     * @param providerName Name of identity source/provider for which info is requested.
     * @return Information of identity provider @see {@link IdentityProviderDTO}
     */
    @GET @Path("/{providerName}/attributesMap")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public Map<String,String> getAttributesMap(
            @PathParam("providerName") String providerName) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "getAttributesMap").startTimer();
        String responseStatus = HTTP_OK;
        try {
            return getIDMClient().getProviderAttributesMap(tenant, providerName);
        } catch (NoSuchTenantException | InvalidProviderException | NoSuchIdpException e) {
            log.warn("Failed to retrieve attributes map for provider '{}' from tenant '{}'", providerName, tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve attributes map for provider '{}' from tenant '{}' due to a client side error", providerName, tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.getAttributesMap.failed", providerName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve identity provider '{}' from tenant '{}' due to a server side error", providerName, tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "get").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Update identity provider attributesMap.
     *
     * @param providerName Name of registered identity provider
     * @param updateAttrsDto Details of identity provider to be updated with. @see {@link UpdateAttributesMapDTO}
     * @return Details of updated identity provider
     */
    @PUT @Path("/{providerName}/attributesMap")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Map<String, String> updateAttributesMap(
        @PathParam("providerName") String providerName,
        UpdateAttributesMapDTO updateAttrsDto) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "updateAttributesMap").startTimer();
        String responseStatus = HTTP_OK;
        try {
            if (updateAttrsDto != null) {
                Map<String,String> current = getIDMClient().getProviderAttributesMap(tenant, providerName);
                Map<String,String> newMap = new HashMap<String,String>();
                newMap.putAll(current);
                if(updateAttrsDto.getRemove() != null) {
                    for(String key : updateAttrsDto.getRemove()){
                        newMap.remove(key);
                    }
                }
                if (updateAttrsDto.getAdd() != null) {
                    newMap.putAll(updateAttrsDto.getAdd());
                }
                getIDMClient().setProviderAttributesMap(tenant, providerName, newMap);
            }
            return getIDMClient().getProviderAttributesMap(tenant, providerName);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to update identity provider '{}' for tenant '{}'", providerName, tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | InvalidProviderException | NoSuchIdpException e) {
            log.warn("Failed to update identity provider '{}' for tenant {} due to a client side error", providerName, tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.updateAttributesMap.failed", providerName, tenant), e);
        } catch (BadRequestException e) {
            responseStatus = HTTP_BAD_REQUEST;
            throw e;
        } catch (Exception e) {
            log.error("Failed to update identity provider '{}' for tenant '{}' due to a server side error", providerName, tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "updateAttributesMap").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Set identity provider attributesMap.
     *
     * @param providerName Name of registered identity provider
     * @param identityProvider Details of identity provider to be updated with. @see {@link IdentityProviderDTO}
     * @return Details of updated identity provider
     */
    @POST @Path("/{providerName}/attributesMap")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Map<String, String> setAttributesMap(
        @PathParam("providerName") String providerName,
        Map<String, String> attributesMap) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "setAttributesMap").startTimer();
        String responseStatus = HTTP_OK;
        try {
            getIDMClient().setProviderAttributesMap(tenant, providerName, attributesMap);
            return getIDMClient().getProviderAttributesMap(tenant, providerName);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to set attributes map for identity provider '{}' for tenant '{}'", providerName, tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | InvalidProviderException | NoSuchIdpException e) {
            log.warn("Failed to set attributes map for identity provider '{}' for tenant {} due to a client side error", providerName, tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.setAttributesMap.failed", providerName, tenant), e);
        } catch (BadRequestException e) {
            responseStatus = HTTP_BAD_REQUEST;
            throw e;
        } catch (Exception e) {
            log.error("Failed to set attributes map for identity provider '{}' for tenant '{}' due to a server side error", providerName, tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "setAttributesMap").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Delete identity provider from tenant
     * @param providerName Name of identity source/provider to delete.
     * @return
     * <code> HTTP 200 OK </code> If identity provider delete operation is successful <br/>
     * <code> HTTP 500 InternalServerError </code> Otherwise
     */
    @DELETE @Path("/{providerName}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("providerName") String providerName) {
        Histogram.Timer requestTimer = requestLatency.labels(METRICS_COMPONENT, tenant, METRICS_RESOURCE, "delete").startTimer();
        String responseStatus = HTTP_OK;
        try {
            getIDMClient().deleteProvider(tenant, providerName);
        } catch (NoSuchTenantException e) {
            log.warn("Failed to remove identity provider '{}' from tenant '{}'", providerName, tenant, e);
            responseStatus = HTTP_NOT_FOUND;
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to remove identity provider '{}' from tenant '{}' due to a client side error", providerName, tenant, e);
            responseStatus = HTTP_BAD_REQUEST;
            throw new BadRequestException(sm.getString("res.provider.delete.failed", providerName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete identity provider '{}' from tenant '{}' due to a server side error", providerName, tenant, e);
            responseStatus = HTTP_SERVER_ERROR;
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        } finally {
            totalRequests.labels(METRICS_COMPONENT, tenant, responseStatus, METRICS_RESOURCE, "delete").inc();
            requestTimer.observeDuration();
        }
    }

    /**
     * Validates all required parameters to examine connectivity against identity provider
     */
    private void validateProbeEssentials(IdentityProviderDTO identityProvider) {
        Validate.notNull(identityProvider, sm.getString("valid.not.null", "Details of identity provider"));
        Validate.notEmpty(identityProvider.getUsername(), sm.getString("valid.not.empty", "Username"));
        Validate.notNull(identityProvider.getPassword(), sm.getString("valid.not.null", "password"));
        Validate.notEmpty(identityProvider.getConnectionStrings(), sm.getString("valid.not.null", "connection strings"));
    }

    private void validateProviderType(String provider) throws BadRequestException {
        try {
            IdentityStoreType.valueOf(provider.toUpperCase());
        } catch (IllegalArgumentException | NullPointerException e) {
            throw new BadRequestException(sm.getString("valid.invalid.type", "type", Arrays.toString(IdentityProviderType.values())));
        }
    }

    public void validateAuthenticationType(String providerType, String authenticationType) throws BadRequestException {
        try {
            if(providerType.equalsIgnoreCase(IdentityProviderType.IDENTITY_STORE_TYPE_LDAP.name()) ||
               providerType.equalsIgnoreCase(IdentityProviderType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING.name())) {
                AuthenticationType.valueOf(authenticationType.toUpperCase());
            }
        } catch (IllegalArgumentException | NullPointerException e) {
            throw new BadRequestException(sm.getString("valid.invalid.type", "authenticationType", Arrays.toString(AuthenticationType.values())));
        }
    }

}
