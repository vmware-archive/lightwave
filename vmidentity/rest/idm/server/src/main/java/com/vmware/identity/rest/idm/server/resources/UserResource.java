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
import java.util.Collections;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

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

import org.apache.logging.log4j.ThreadContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordPolicyViolationException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.DynamicRole;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.DTOMapperException;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.PasswordResetRequestDTO;
import com.vmware.identity.rest.idm.data.PrincipalDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.server.PathParameters;
import com.vmware.identity.rest.idm.server.mapper.GroupMapper;
import com.vmware.identity.rest.idm.server.mapper.PrincipalMapper;
import com.vmware.identity.rest.idm.server.mapper.UserMapper;
import com.vmware.identity.rest.idm.server.util.Config;

/**
 * Web service resource to manage all person user operations.
 *
 *
 * https://[address]/idm/tenant/<tenant name>/users/
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class UserResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(UserResource.class);

    public UserResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Creates FSP(Foreign Security Principal) or external IDP user for a provided tenant
     * @param externalUser Details of the FSP user to be created.
     * @return true On successfull creation of external user.
     *         false On failure
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public Boolean createExternalUser(PrincipalDTO externalUser) {
        Boolean registrationStatus = false;
        try {
            PrincipalId externalUserId = PrincipalMapper.getPrincipal(externalUser);
            registrationStatus = getIDMClient().registerThirdPartyIDPUser(getTenant(), externalUserId);
            return registrationStatus;
        } catch (NoSuchTenantException e) {
            log.debug("Failed to create external IDP user(FSP) '{}'", externalUser.getName(), e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (PasswordPolicyViolationException | DTOMapperException | InvalidArgumentException | InvalidPrincipalException | NoSuchIdpException e) {
            log.debug("Failed to create external IDP user(FSP) '{}' on tenant '{}' due to a client side error", externalUser.getName(), tenant, e);
            throw new BadRequestException(sm.getString("res.user.create.failed", externalUser.getName(), tenant), e);
        } catch (Exception e) {
            log.error("Failed to create external IDP user(FSP) '{}' on tenant '{}' due to a server side error", externalUser.getName(), tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Deletes FSP(Foreign Security Principal) or external IDP user for a given tenant
     *
     * @param name Username of third party user to be deleted.
     * @return
     *      Return 200 (OK) if update operation succeeds.
     *      Return 500 (Internal Server Error) if update operation fails.
     */
    @DELETE @Path("/{userName}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("userName") String name) {
        PrincipalId id = PrincipalUtil.fromName(name);
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(id.getDomain()), sm.getString("valid.not.systemdomain", id.getDomain(), tenant));
        try {
            getIDMClient().removeThirdPartyIDPUser(getTenant(), id);
        } catch (NoSuchTenantException | InvalidPrincipalException e) {
            log.debug("Failed to delete external IDP user (FSP) '{}' from tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.debug("Failed to delete external IDP user (FSP) '{}' from tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.user.delete.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete external IDP user (FSP) '{}' from tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the details of the given user
     *
     * SHOULD BE IDP INDEPENDENT!
     *
     * @param name Name of the user taken as path parameter. (Required @Non-Null. @Non-Empty)
     * @return Details of the user requested
     */
    @GET @Path("/{userName}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public UserDTO get(@PathParam("userName") String name) {
        PrincipalId id = PrincipalUtil.fromName(name);
        Map<String,String> params = ThreadContext.getContext();
        for(Entry<String,String> e : params.entrySet()) {
        	log.info("MDC key : " + e.getKey() + " MDC Value : " + e.getValue());
        }
        log.info("================================");

        try {
            PersonUser idmPersonUser = getIDMClient().findPersonUser(tenant, id);
            if (idmPersonUser == null) {
                throw new InvalidPrincipalException(String.format("User '%s' does not exist in tenant '%s'", name, tenant));
            }
            return UserMapper.getUserDTO(idmPersonUser, includePasswordDetails(name));
        } catch (NoSuchIdpException | NoSuchTenantException | InvalidPrincipalException e) {
            log.debug("Failed to retrieve user '{}' from tenant '{}'", name, tenant);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to retrieve user '{}' from tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.user.get.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve user '{}' from tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Retrieve list of groups that a user is associated with for a given tenant.
     *
     * @param name Name of user to be queried
     * @param nested Boolean value to indicate if nested groups of user to be queried.
     * @return Set of Groups associated with user
     */
    @GET @Path("/{userName}/groups")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public Collection<GroupDTO> getGroups(@PathParam("userName") String name, @DefaultValue("false") @QueryParam("nested") boolean nested) {
        PrincipalId id = PrincipalUtil.fromName(name);

        Collection<GroupDTO> userAffiliatedGroups = Collections.emptySet();
        try {
            // Retrieve all the direct groups associated with user.
            Set<Group> idmGroups = getIDMClient().findDirectParentGroups(tenant, id);

            // Retrieve all nested groups associated with user
            if (nested) {
                Set<Group> nestedParentGroups = getIDMClient().findNestedParentGroups(tenant, id);
                idmGroups.addAll(nestedParentGroups);
            }

            userAffiliatedGroups = GroupMapper.getGroupDTOs(idmGroups);
        } catch (NoSuchIdpException | NoSuchTenantException | InvalidPrincipalException e) {
            log.debug("Failed to retrieve groups associated with user '{}' on tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to retrieve groups associated with user '{}' on tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.user.get.groups.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve groups associated with user '{}' on tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

       return userAffiliatedGroups;
    }

    /**
     * Check if the requesting user is the same as the user requested
     *
     * @param username
     * @return
     */
    private boolean includePasswordDetails(String username) {
        if (getSecurityContext() == null) {
            return false;
        }

        return getSecurityContext().isUserInRole(Role.ADMINISTRATOR.toString()) || getSecurityContext().getUserPrincipal().getName().equals(username);
    }

}
