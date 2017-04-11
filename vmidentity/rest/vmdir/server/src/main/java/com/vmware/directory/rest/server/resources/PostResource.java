package com.vmware.directory.rest.server.resources;

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

import java.util.Collection;
import java.util.List;

import javax.ws.rs.Consumes;
import javax.ws.rs.DefaultValue;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.SearchResultDTO;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.common.data.TenantConfigurationDTO;
import com.vmware.directory.rest.common.data.UserDTO;
import com.vmware.directory.rest.server.util.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.resources.BaseResource;

/**
 * Resource that contains alternative POST endpoints for every GET endpoint throughought the other
 * resources.
 *
 * This is necessary in order to allow users to send tokens of unbounded sizes. Once the OIDC
 * server can support creating and maintaining opaque tokens and once it can exchange SAML tokens for
 * OIDC tokens, this resource should be removed and all operations should go through the standard
 * endpoints.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Path("/post")
public class PostResource extends BaseResource {

    public PostResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    // Group Resource

    @POST @Path("/tenant/{tenantName}/groups/{groupName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public GroupDTO getGroup(@PathParam("tenantName") String tenantName, @PathParam("groupName") String groupName) {
        return new GroupResource(tenantName, getRequest(), getSecurityContext()).get(groupName);
    }

    @POST @Path("/tenant/{tenantName}/groups/{groupName}/members")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public SearchResultDTO getGroupMembers(@PathParam("tenantName") String tenantName, @PathParam("groupName") String groupName, @DefaultValue("all") @QueryParam("type") String memberType, @DefaultValue("200") @QueryParam("limit") int limit) {
        return new GroupResource(tenantName, getRequest(), getSecurityContext()).getMembers(groupName, memberType, limit);
    }

    @POST @Path("/tenant/{tenantName}/groups/{groupName}/parents")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public Collection<GroupDTO> getGroupParents(@PathParam("tenantName") String tenantName, @PathParam("groupName") String groupName, @DefaultValue("false") @QueryParam("nested") boolean nested) {
        return new GroupResource(tenantName, getRequest(), getSecurityContext()).getParents(groupName, nested);
    }

    // User Resource

    @POST @Path("/tenant/{tenantName}/users/{userName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public UserDTO getUser(@PathParam("tenantName") String tenantName, @PathParam("userName") String userName) {
        return new UserResource(tenantName, getRequest(), getSecurityContext()).get(userName);
    }

    @POST @Path("/tenant/{tenantName}/users/{userName}/groups")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public Collection<GroupDTO> getUserGroups(@PathParam("tenantName") String tenantName, @PathParam("userName") String userName, @DefaultValue("false") @QueryParam("nested") boolean nested) {
        return new UserResource(tenantName, getRequest(), getSecurityContext()).getGroups(userName, nested);
    }

    // Solution User Resource

    @POST @Path("/tenant/{tenantName}/solutionusers/{solnName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public SolutionUserDTO getSolutionUser(@PathParam("tenantName") String tenantName, @PathParam("solnName") String solnName) {
        return new SolutionUserResource(tenantName, getRequest(), getSecurityContext()).get(solnName);
    }

    @POST @Path("/tenant/{tenantName}/solutionusers/{solnName}/groups")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public Collection<SolutionUserDTO> getGroups(@PathParam("tenantName") String tenantName, @PathParam("solnName") String solnName, @DefaultValue("200") @QueryParam("limit") int limit) {
        return new SolutionUserResource(tenantName, getRequest(), getSecurityContext()).getGroups(solnName, limit);
    }

    // Configuration Resource
    @POST
    @Path("/tenant/{tenantName}/config")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public TenantConfigurationDTO getConfiguration(@PathParam("tenantName") String tenantName, @QueryParam("type") final List<String> configTypes) {
        return new ConfigurationResource(tenantName, getRequest(), getSecurityContext()).getConfig(tenantName, configTypes);
    }

}
