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


import java.util.Collection;
import java.util.Collections;
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

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.PasswordResetRequestDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.directory.rest.server.PathParameters;
import com.vmware.directory.rest.server.mapper.GroupMapper;
import com.vmware.directory.rest.server.mapper.UserDetailsMapper;
import com.vmware.directory.rest.server.mapper.UserMapper;
import com.vmware.directory.rest.server.util.Config;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordPolicyViolationException;
import com.vmware.identity.idm.PersonDetail;
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
/**
 * Web service resource to manage all person user operations.
 *
 *
 * https://[address]/vmdir/tenant/<tenant name>/users/
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
     * Creates a user for a provided tenant
     * @param user Details of the user to be created.
     * @return Details of the user created
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public UserDTO create(UserDTO user) {
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(user.getDomain()), sm.getString("valid.not.systemdomain", user.getDomain(), tenant));
        try {
            boolean disabled = user.isDisabled() != null ? user.isDisabled() : false;
            boolean locked = user.isLocked() != null ? user.isLocked() : false;
            if (disabled && locked) {
                throw new BadRequestException("A disabled user cannot be locked");
            }

            PersonUser personUser = UserMapper.getPersonUser(user);
            String name = personUser.getId().getName();
            PersonDetail personDetail = personUser.getDetail();
            char[] password = user.getPasswordDetails().getPassword().toCharArray();
            PrincipalId principal = getIDMClient().addPersonUser(tenant, name, personDetail, password);
            if (disabled) {
                getIDMClient().disableUserAccount(tenant, personUser.getId());
            }

            return UserMapper.getUserDTO(getIDMClient().findPersonUser(tenant, principal), true);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to create user '{}'", user.getName(), e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (PasswordPolicyViolationException | DTOMapperException | InvalidArgumentException | InvalidPrincipalException | NoSuchIdpException e) {
            log.debug("Failed to create user '{}' on tenant '{}' due to a client side error", user.getName(), tenant, e);
            throw new BadRequestException(sm.getString("res.user.create.failed", user.getName(), tenant), e);
        } catch (Exception e) {
            log.error("Failed to create user '{}' on tenant '{}' due to a server side error", user.getName(), tenant, e);
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

        try {
            PersonUser idmPersonUser = getIDMClient().findPersonUser(tenant, id);
            if (idmPersonUser == null) {
                throw new InvalidPrincipalException(String.format("User '%s' does not exist in tenant '%s'", name, tenant), name);
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
     * Deletes person user for a given tenant
     *
     * @param name Username of person to be deleted.
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
            getIDMClient().deletePrincipal(tenant, id.getName());
        } catch (NoSuchTenantException | InvalidPrincipalException e) {
            log.debug("Failed to delete user '{}' from tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.debug("Failed to delete user '{}' from tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.user.delete.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete user '{}' from tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Update various entities of person user in given tenant. This API can be used to :
     *
     * <li> Update details of person ({@link PersonDetailDTO})</li>
     * <li> Unlock user </li>
     * <li> Enable/Disable user </li>
     *
     * @param name name of user on which update operation is performed.
     * @param userDTO Details of user to be updated.
     * @return
     *      Return 200 (OK) if update operation succeeds. <br/>
     *      Return 500 (Internal Server Error) if update operation fails.
     */
    @PUT @Path("/{userName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public UserDTO update(@PathParam("userName") String name, UserDTO userDTO){
        PrincipalId userId = PrincipalUtil.fromName(name);
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(userId.getDomain()), sm.getString("valid.not.systemdomain", userId.getDomain(), tenant));
        UserDetailsDTO personDetailsToUpdate = userDTO.getDetails();

        try {
            if (personDetailsToUpdate != null) {
                PersonDetail personDetail = UserDetailsMapper.getPersonDetail(personDetailsToUpdate);
                getIDMClient().updatePersonUserDetail(tenant, userId.getName(), personDetail);
            }

            if (Boolean.TRUE.equals(userDTO.isDisabled())) {
                getIDMClient().disableUserAccount(tenant, userId);
            } else if (Boolean.FALSE.equals(userDTO.isDisabled())) {
                getIDMClient().enableUserAccount(tenant, userId);
            }

            if (Boolean.FALSE.equals(userDTO.isLocked())) {
                getIDMClient().unlockUserAccount(tenant, userId);
            }
            return UserMapper.getUserDTO(getIDMClient().findPersonUser(tenant, userId), true);
        } catch (NoSuchIdpException | NoSuchTenantException | InvalidPrincipalException e) {
            log.debug("Failed to update user '{}' in tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (DTOMapperException | InvalidArgumentException e) {
            log.debug("Failed to update user '{}' in tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.user.update.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to update user '{}' in tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

    }

    /**
     * Change/update password for a user in tenant
     *
     * @param name username of principal whose password needs to be changed
     * @param currentPassword current password of user
     * @param newPassword new password to be set for user
     * @return Details of updated user
     */
    @PUT @Path("/{userName}/" + PathParameters.PASSWORD)
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @DynamicRole
    public UserDTO updatePassword(@PathParam("userName") String name, PasswordResetRequestDTO passwordResetRequest) {
        PrincipalId userId = PrincipalUtil.fromName(name);
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(userId.getDomain()), sm.getString("valid.not.systemdomain", userId.getDomain(), tenant));
        Validate.notEmpty(passwordResetRequest.getNewPassword(), sm.getString("valid.not.empty", "new password"));
        boolean isAdmin = getSecurityContext().isUserInRole(Role.ADMINISTRATOR.name());
        if (!isAdmin) {
            // Validate if current password is provided by non-admin users
            Validate.notEmpty(passwordResetRequest.getCurrentPassword(), sm.getString("valid.not.empty", "current password"));
        }
        try {
            if (isAdmin) {
                getIDMClient().setUserPassword(tenant, userId.getName(), passwordResetRequest.getNewPassword().toCharArray());
            } else {
                getIDMClient().changeUserPassword(tenant, userId.getName(), passwordResetRequest.getCurrentPassword().toCharArray(), passwordResetRequest.getNewPassword().toCharArray());
            }
            return UserMapper.getUserDTO(getIDMClient().findPersonUser(tenant, userId), includePasswordDetails(name));
        } catch (NoSuchTenantException e) {
            log.warn("Failed to update password for user '{}' in tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidPrincipalException | PasswordPolicyViolationException e) {
            log.warn("Failed to update password for user '{}' in tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.user.update.password.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to update password for user '{}' in tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
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
