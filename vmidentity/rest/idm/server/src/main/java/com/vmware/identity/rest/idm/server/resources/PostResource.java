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
import javax.ws.rs.core.UriInfo;

import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.EventLogDTO;
import com.vmware.identity.rest.idm.data.EventLogStatusDTO;
import com.vmware.identity.rest.idm.data.ExternalIDPDTO;
import com.vmware.identity.rest.idm.data.GroupDTO;
import com.vmware.identity.rest.idm.data.IdentityProviderDTO;
import com.vmware.identity.rest.idm.data.OIDCClientDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ResourceServerDTO;
import com.vmware.identity.rest.idm.data.SearchResultDTO;
import com.vmware.identity.rest.idm.data.ServerDetailsDTO;
import com.vmware.identity.rest.idm.data.SolutionUserDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TenantDTO;
import com.vmware.identity.rest.idm.data.UserDTO;
import com.vmware.identity.rest.idm.server.util.Config;

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

    // Server Resource

    @POST @Path("/server/status")
    @Consumes(MediaType.APPLICATION_JSON)
    public void getServerStatus() {
        new ServerResource(getRequest(), getSecurityContext()).getStatus();
    }

    @POST @Path("/server/computers")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.CONFIGURATION_USER)
    public Collection<ServerDetailsDTO> getComputers(@DefaultValue("all") @QueryParam("type") String computerType) {
        return new ServerResource(getRequest(), getSecurityContext()).getComputers(computerType);
    }

    // Tenant Resource

    @POST @Path("/tenant/{tenantName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public TenantDTO getTenant(@PathParam("tenantName") String tenantName) {
        return new TenantResource(getRequest(), getSecurityContext()).get(tenantName);
    }

    @POST @Path("/tenant/{tenantName}/search")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public SearchResultDTO searchTenantMembers(@PathParam("tenantName") String tenantName,
                                 @DefaultValue("all") @QueryParam("type") String memberType,
                                 @QueryParam("domain") String domain,
                                 @DefaultValue("200") @QueryParam("limit") int limit,
                                 @DefaultValue("NAME") @QueryParam("searchBy") String searchBy,
                                 @DefaultValue("") @QueryParam("query") String query) {
        return new TenantResource(getRequest(), getSecurityContext()).searchMembers(tenantName, memberType, domain, limit, searchBy, query);
    }

    @POST @Path("/tenant/{tenantName}/config")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public TenantConfigurationDTO getConfig(@PathParam("tenantName") String tenantName, @QueryParam("type") final List<String> configTypes) {
        return new TenantResource(getRequest(), getSecurityContext()).getConfig(tenantName, configTypes);
    }

    // Providers Resource

    @POST @Path("/tenant/{tenantName}/providers")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public Collection<IdentityProviderDTO> getAllProviders(@PathParam("tenantName") String tenantName) {
        return new IdentityProviderResource(tenantName, getRequest(), getSecurityContext()).getAll();
    }

    @POST @Path("/tenant/{tenantName}/providers/{providerName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public IdentityProviderDTO getIdentityProvider(@PathParam("tenantName") String tenantName, @PathParam("providerName") String providerName) {
        return new IdentityProviderResource(tenantName, getRequest(), getSecurityContext()).get(providerName);
    }

    // External IDP Resource

    @POST @Path("/tenant/{tenantName}/externalidp")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public Collection<ExternalIDPDTO> getAllExternalIDPs(@PathParam("tenantName") String tenantName) {
        return new ExternalIDPResource(tenantName, getRequest(), getSecurityContext()).getAll();
    }

    @POST @Path("/tenant/{tenantName}/externalidp/{entityID}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public ExternalIDPDTO getExternalIDP(@PathParam("tenantName") String tenantName, @PathParam("entityID") String entityID) {
        return new ExternalIDPResource(tenantName, getRequest(), getSecurityContext()).get(entityID);
    }

    // Certificate Resource

    @POST @Path("/tenant/{tenantName}/certificates")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    public Collection<CertificateChainDTO> getAllCertificates(@PathParam("tenantName") String tenantName, @QueryParam("scope") String certificateLocation,@QueryParam("granularity") String granularity) {
        return new CertificateResource(tenantName, getRequest(), getSecurityContext()).getCertificates(certificateLocation, granularity);
    }

    @POST @Path("/tenant/{tenantName}/certificates/privatekey")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public PrivateKeyDTO getPrivateKey(@PathParam("tenantName") String tenantName) {
        return new CertificateResource(tenantName, getRequest(), getSecurityContext()).getPrivateKey();
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

    // Relying Party Resource

    @POST @Path("/tenant/{tenantName}/relyingparty")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public Collection<RelyingPartyDTO> getAllRelyingParties(@PathParam("tenantName") String tenantName) {
        return new RelyingPartyResource(tenantName, getRequest(), getSecurityContext()).getAll();
    }

    @POST @Path("/tenant/{tenantName}/relyingparty/{relyingPartyName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public RelyingPartyDTO getRelyingParty(@PathParam("tenantName") String tenantName, @PathParam("relyingPartyName") String relyingPartyName) {
        return new RelyingPartyResource(tenantName, getRequest(), getSecurityContext()).get(relyingPartyName);
    }

    // OIDC client Resource

    @POST @Path("/tenant/{tenantName}/oidcclient/{clientId}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public OIDCClientDTO getOidcClient(@PathParam("tenantName") String tenantName, @PathParam("clientId") String clientId) {
        return new OIDCClientResource(tenantName, getRequest(), getSecurityContext()).get(clientId);
    }

    @POST @Path("/tenant/{tenantName}/oidcclient")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public Collection<OIDCClientDTO> getAllOidcClients(@PathParam("tenantName") String tenantName, @PathParam("clientId") String clientId) {
        return new OIDCClientResource(tenantName, getRequest(), getSecurityContext()).getAll();
    }

    // Resource Server Resource

    @POST @Path("/tenant/{tenantName}/resourceserver/{resourceServerName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public ResourceServerDTO getResourceServer(@PathParam("tenantName") String tenantName, @PathParam("resourceServerName") String resourceServerName) {
        return new ResourceServerResource(tenantName, getRequest(), getSecurityContext()).get(resourceServerName);
    }

    @POST @Path("/tenant/{tenantName}/resourceserver")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public Collection<ResourceServerDTO> getAllResourceServers(@PathParam("tenantName") String tenantName, @PathParam("resourceServerName") String resourceServerName) {
        return new ResourceServerResource(tenantName, getRequest(), getSecurityContext()).getAll();
    }

    // Diagnostics Resource

    @POST @Path("/tenant/{tenantName}/diagnostics/eventlog")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public Collection<EventLogDTO> getEventLog(@PathParam("tenantName") String tenantName, @Context UriInfo info) {
        return new DiagnosticsResource(tenantName, getRequest(), getSecurityContext()).getEventLog(info);
    }

    @POST @Path("/tenant/{tenantName}/diagnostics/eventlog/status")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public EventLogStatusDTO getEventLogStatus(@PathParam("tenantName") String tenantName) {
        return new DiagnosticsResource(tenantName, getRequest(), getSecurityContext()).getEventLogStatus();
    }

}
