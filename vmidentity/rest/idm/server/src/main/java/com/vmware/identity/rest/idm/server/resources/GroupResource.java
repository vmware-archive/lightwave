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

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import javax.ws.rs.DefaultValue;
import javax.ws.rs.GET;
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
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
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
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.attributes.MemberType;
import com.vmware.identity.rest.idm.server.mapper.GroupMapper;
import com.vmware.identity.rest.idm.server.mapper.SolutionUserMapper;
import com.vmware.identity.rest.idm.server.mapper.UserMapper;
import com.vmware.identity.rest.idm.server.util.Config;

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
