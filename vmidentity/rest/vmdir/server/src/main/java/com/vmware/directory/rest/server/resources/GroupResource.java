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

import java.util.Arrays;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
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
import com.vmware.directory.rest.common.data.MemberType;
import com.vmware.directory.rest.common.data.SearchResultDTO;
import com.vmware.directory.rest.server.mapper.GroupDetailsMapper;
import com.vmware.directory.rest.server.mapper.GroupMapper;
import com.vmware.directory.rest.server.mapper.SolutionUserMapper;
import com.vmware.directory.rest.server.mapper.UserMapper;
import com.vmware.directory.rest.server.util.Config;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.MemberAlreadyExistException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;
import com.vmware.identity.rest.core.server.util.Validate;

/**
 * Web service resource to manage all group operations.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class GroupResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(GroupResource.class);

    public GroupResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Create a group on a tenant.
     *
     * @param groupDTO Details of group which is been created
     * @return the group object that was just created
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public GroupDTO create(GroupDTO groupDTO) {
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(groupDTO.getDomain()), sm.getString("valid.not.systemdomain", groupDTO.getDomain(), tenant));
        // Alias not supported
        if (groupDTO.getAlias() != null) {
            throw new NotImplementedError("Creation of groups with alias is not supported");
        }

        try {
            GroupDetail groupDetail = GroupDetailsMapper.getGroupDetails(groupDTO.getDetails());
            PrincipalId groupId = getIDMClient().addGroup(tenant, groupDTO.getName(), groupDetail);

            // Assume that the details we supplied were accepted...
            return new GroupDTO(groupId.getName(), groupId.getDomain(), groupDTO.getDetails(), null, null);
        } catch (NoSuchTenantException e) {
            log.debug("Failed to create group '{}' on tenant '{}'", groupDTO.getName(), tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | InvalidPrincipalException e) {
            log.warn("Failed to create group '{}' on tenant '{}' due to a client side error", groupDTO.getName(), tenant, e);
            throw new BadRequestException(sm.getString("res.group.create.failed", groupDTO.getName(), tenant), e);
        } catch (Exception e) {
            log.error("Failed to create group '{}' on tenant '{}' due to a server side error", groupDTO.getName(), tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the group associated with a tenant.
     *
     * @param groupName name of the group to search
     * @return the group object
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @GET @Path("/{groupName}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public GroupDTO get(@PathParam("groupName") String groupName) {
        PrincipalId id = PrincipalUtil.fromName(groupName);

        try {
            Group group = getIDMClient().findGroup(tenant, id);
            return GroupMapper.getGroupDTO(group);
        } catch (InvalidPrincipalException | NoSuchTenantException | NoSuchIdpException e) {
            log.debug("Failed to retrieve group '{}' from tenant '{}'", groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve group '{}' from tenant '{}' due to a client side error", groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.get.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve group '{}' from tenant '{}' due to a server side error", groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Update a group object.
     *
     * <p>Currently supports only replacement of the group description</p>
     *
     * @param groupName name of the group being updated
     * @param group group object containing the fields to update
     * @return the newly updated group object
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @PUT @Path("/{groupName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public GroupDTO update(@PathParam("groupName") String groupName, GroupDTO group) {
        PrincipalId id = PrincipalUtil.fromName(groupName);
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(id.getDomain()), sm.getString("valid.not.systemdomain", id.getDomain(), tenant));
        if (group.getName() != null) {
            throw new NotImplementedError("Group name replacement is not supported");
        }

        if (group.getDomain() != null) {
            throw new NotImplementedError("Group domain replacement is not supported");
        }

        if (group.getAlias() != null) {
            throw new NotImplementedError("Group alias replacement is not supported");
        }

        try {
            if (group.getDetails() != null) {
                GroupDetail groupDetail = GroupDetailsMapper.getGroupDetails(group.getDetails());
                getIDMClient().updateGroupDetail(tenant, id.getName(), groupDetail);
            }

            return GroupMapper.getGroupDTO(getIDMClient().findGroup(tenant, id));
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to update group '{}' in tenant '{}'", groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to update group '{}' in tenant '{}' due to a client side error", groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.update.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to update group '{}' in tenant '{}' due to a server side error", groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Deletes group from tenant
     *
     * @param groupName name of group to delete
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @DELETE @Path("/{groupName}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("groupName") String groupName) {
        PrincipalId id = PrincipalUtil.fromName(groupName);
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(id.getDomain()), sm.getString("valid.not.systemdomain", id.getDomain(), tenant));
        try {
            getIDMClient().deletePrincipal(tenant, id.getName());
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to delete group '{}' from tenant '{}'", groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to delete group '{}' from tenant '{}' due to a client side error", groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.delete.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete group '{}' from tenant '{}' due to a server side error", groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get the members of a group associated with a tenant.
     *
     * @param groupName name of the group to search
     * @param type the type of members to retrieve. See {@link MemberType}.
     * @param limit the number of principals to limit the search to. No limit is set on negative value.
     * @return a collection of principal objects listing the members of the group
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @GET @Path("/{groupName}/members")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public SearchResultDTO getMembers(@PathParam("groupName") String groupName, @DefaultValue("all") @QueryParam("type") String memberType, @DefaultValue("200") @QueryParam("limit") int limit) {
        PrincipalId id = PrincipalUtil.fromName(groupName);
        MemberType type = getPrincipalType(memberType);
        Validate.notNull(type, sm.getString("valid.invalid.type", "member type", Arrays.toString(MemberType.values())));
        try {
            SearchCriteria searchCriteria = new SearchCriteria("", tenant);
            return findPrincipals(type, id, tenant, searchCriteria, limit);
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to retrieve principals for group '{}' in tenant '{}'", groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve principals for group '{}' in tenant '{}' due to a client side error", groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.get.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve principals for group '{}' in tenant '{}' due to a server side error", groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Retrieve parent groups of requested group in a given tenant.
     *
     * @param groupName name of group to find the parents of
     * @param nested flag indicating whether to search for indirect parent groups
     * @return a collection of parent groups of requested group (can be empty)
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @GET @Path("/{groupName}/parents")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public Collection<GroupDTO> getParents(@PathParam("groupName") String groupName, @DefaultValue("false") @QueryParam("nested") boolean nested) {
        PrincipalId id = PrincipalUtil.fromName(groupName);

        if (nested) {
            throw new NotImplementedError("Indirect parent group retrieval is not supported");
        }

        Set<GroupDTO> groups = new HashSet<GroupDTO>();
        try {
            // Add direct groups
            Set<Group> idmDirectGroups = getIDMClient().findDirectParentGroups(tenant, id);
            if (idmDirectGroups != null && !idmDirectGroups.isEmpty()) {
                groups.addAll(GroupMapper.getGroupDTOs(idmDirectGroups));
            }
            // Add indirect groups
            // TODO : Need to implement casIDMClient API. "Get parent groups associated indirectly to given group"
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to retrieve parent groups of group '{}' in tenant '{}'", groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to retrieve parent groups of group '{}' in tenant '{}' due to a client side error", groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.get.parents.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve parent groups of group '{}' in tenant '{}' due to a server side error", groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

        return groups;
    }

    /**
     * Add members to a group in a given tenant.
     *
     * TODO Make atomic
     *
     * @param groupName name of the group to add members to
     * @param members list of members to add
     * @param memberType type of members to add. See {@link MemberType}.
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenant or group does not exist
     */
    @PUT @Path("/{groupName}/members")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void addMembers(@PathParam("groupName") String groupName, @QueryParam("members") List<String> members, @QueryParam("type") String memberType) {
        PrincipalId id = PrincipalUtil.fromName(groupName);
        Validate.isTrue(getSystemDomain().equalsIgnoreCase(id.getDomain()), sm.getString("valid.not.systemdomain", id.getDomain(), tenant));
        MemberType type = getPrincipalType(memberType);
        Collection<PrincipalId> memberPrincipals = PrincipalUtil.fromNames(members);
        Validate.notNull(type, sm.getString("valid.invalid.type", "type", Arrays.toString(MemberType.values())));
        Validate.notEqual(type, MemberType.ALL, sm.getString("valid.not.equal", "type", MemberType.ALL, EnumSet.of(MemberType.USER, MemberType.GROUP, MemberType.SOLUTIONUSER)));

        try {
            if (type == MemberType.GROUP) {
                addGroupsToGroup(id.getName(), memberPrincipals);
            } else if (type == MemberType.USER) {
                addUsersToGroup(id.getName(), memberPrincipals);
            } else if (type == MemberType.SOLUTIONUSER) {
                addSolutionUsersToGroup(id.getName(), memberPrincipals);
            }
        } catch (NotImplementedError e) {
            // TODO remove this catch - only temporary while waiting for addSolutionUsersToGroup
            throw e;
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            // TODO Check if InvalidPrincipalException is thrown for both the group being removed from and the group to remove...
            log.debug("Failed to add members {} of type {} to group '{}' in tenant '{}'", members.toString(), memberType, groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | MemberAlreadyExistException e) {
            log.warn("Failed to add members {} of type {} to group '{}' in tenant '{}' due to a client side error", members.toString(), memberType, groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.add.groups.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to add members {} of type {} to group '{}' in tenant '{}' due to a server side error", members.toString(), groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Remove members from a group in a given tenant.
     *
     * TODO Make atomic
     *
     * @param groupName name of the group to remove members from
     * @param members list of members to remove
     * @param memberType type of members to remove. See {@link MemberType}.
     * @throws BadRequestException on client side errors
     * @throws InternalServerErrorException on server side errors
     * @throws NoSuchTenantException if the tenanRTt or group does not exist
     */
    @DELETE @Path("/{groupName}/members")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void removeMembers(@PathParam("groupName") String groupName, @QueryParam("members") List<String> members, @QueryParam("type") String memberType) {
        PrincipalId id = PrincipalUtil.fromName(groupName);
        MemberType type = getPrincipalType(memberType);
        Collection<PrincipalId> memberPrincipals = PrincipalUtil.fromNames(members);

        Validate.notNull(type, sm.getString("valid.invalid.type", "type", Arrays.toString(MemberType.values())));
        Validate.notEqual(type, MemberType.ALL, sm.getString("valid.not.equal", "type", MemberType.ALL, EnumSet.of(MemberType.USER, MemberType.GROUP, MemberType.SOLUTIONUSER)));

        if (type == MemberType.SOLUTIONUSER) {
            throw new NotImplementedError("Removing solution users from a group is not yet implemented");
        }

        try {
            for (PrincipalId memberId : memberPrincipals) {
                getIDMClient().removeFromLocalGroup(tenant, memberId, id.getName());
            }
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            // TODO Check if InvalidPrincipalException is thrown for both the group being removed from and the group to remove...
            log.debug("Failed to add groups to group '{}' in tenant '{}'", groupName, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.warn("Failed to add groups to group '{}' in tenant '{}' due to a client side error", groupName, tenant, e);
            throw new BadRequestException(sm.getString("res.group.add.groups.failed", groupName, tenant), e);
        } catch (Exception e) {
            log.error("Failed to add groups to group '{}' in tenant '{}' due to a server side error", groupName, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Utility method for adding groups to a group
     *
     * @param groupName group to add to
     * @param groups list of groups to add
     * @throws Exception if there is an error with IDM
     */
    private void addGroupsToGroup(String groupName, Collection<PrincipalId> groups) throws Exception {
        for (PrincipalId groupId : groups) {
            getIDMClient().addGroupToGroup(tenant, groupId, groupName);
        }
    }

    /**
     * Utility method for adding users to a group
     *
     * @param groupName group to add to
     * @param users list of users to add
     * @throws Exception if there is an error with IDM
     */
    private void addUsersToGroup(String groupName, Collection<PrincipalId> users) throws Exception {
        for (PrincipalId userId : users) {
            getIDMClient().addUserToGroup(tenant, userId, groupName);
        }
    }

    /**
     * Utility method for adding solution users to a group
     *
     * @param groupName group to add to
     * @param users list of users to add
     * @throws Exception if there is an error with IDM
     */
    private void addSolutionUsersToGroup(String groupName, Collection<PrincipalId> users) throws Exception {
        throw new NotImplementedError("Adding solution users to a group is not yet implemented");
    }

    /**
     * Helper method to search principals in a given group.
     *
     * @throws Exception
     */
    private SearchResultDTO findPrincipals(MemberType principalType, PrincipalId id, String tenant, SearchCriteria searchCriteria, int limit) throws Exception {
        SearchResultDTO.Builder builder = SearchResultDTO.builder();
        Map<MemberType, Integer> searchLimits = computeSearchLimits(limit, principalType);
        if (principalType == MemberType.USER || principalType == MemberType.ALL) {
            Set<PersonUser> idmPersonUsers = getIDMClient().findPersonUsersInGroup(tenant, id, searchCriteria.getSearchString(), searchLimits.get(MemberType.USER));
            builder.withUsers(UserMapper.getUserDTOs(idmPersonUsers, false));
        }

        if (principalType == MemberType.GROUP || principalType == MemberType.ALL) {
            Set<Group> idmGroups = getIDMClient().findGroupsInGroup(tenant, id, searchCriteria.getSearchString(), searchLimits.get(MemberType.GROUP));
            builder.withGroups(GroupMapper.getGroupDTOs(idmGroups));
        }

        if (principalType == MemberType.SOLUTIONUSER || principalType == MemberType.ALL) {
            Set<SolutionUser> idmSolutionUsers = getIDMClient().findSolutionUsersInGroup(tenant, id.getName(), searchCriteria.getSearchString(), searchLimits.get(MemberType.SOLUTIONUSER));
            builder.withSolutionUsers(SolutionUserMapper.getSolutionUserDTOs(idmSolutionUsers));
        }

        return builder.build();
    }

    /**
     * Search limit calculator that computes number of principal entities(users, groups and solution users) to be returned in search results
     */
    private Map<MemberType, Integer> computeSearchLimits(int limit, MemberType memberType) {
        Map<MemberType, Integer> memberTypeToLimit = new HashMap<MemberType, Integer>();
        int limitPerPrincipalType = limit;
        if (memberType == MemberType.ALL) {
            // Users + Groups
            limitPerPrincipalType = limit < 0 ? -1 : limit / (MemberType.values().length - 1);
            // Solution users
            int solutionUserLimit = limitPerPrincipalType < 0 ? -1 : limitPerPrincipalType + (limit % (MemberType.values().length - 1));
            memberTypeToLimit.put(MemberType.USER, limitPerPrincipalType);
            memberTypeToLimit.put(MemberType.GROUP, limitPerPrincipalType);
            memberTypeToLimit.put(MemberType.SOLUTIONUSER, solutionUserLimit);
        } else {
            memberTypeToLimit.put(memberType, limit);
        }
        return memberTypeToLimit;
    }

    /**
     * Retrieves a principal type from a string. Will be null if the string does not match a known type.
     *
     * @param principalType string representing a certificate type
     * @return the principal type if it exists, or <tt>null</tt> if it does not.
     */
    private MemberType getPrincipalType(String principalType) {
        MemberType type = null;
        try {
            type = MemberType.valueOf(principalType.toUpperCase());
        } catch (IllegalArgumentException | NullPointerException e) {
            // ignore and we'll just return null
        }

        return type;
    }

}