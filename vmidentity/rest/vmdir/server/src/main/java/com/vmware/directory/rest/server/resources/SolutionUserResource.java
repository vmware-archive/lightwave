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

import java.security.cert.CertificateException;
import java.util.Collection;

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

import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.directory.rest.server.mapper.SolutionUserMapper;
import com.vmware.directory.rest.server.util.Config;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.DuplicateCertificateException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.exception.server.NotImplementedError;
import com.vmware.identity.rest.core.server.util.Validate;
import com.vmware.identity.rest.core.util.CertificateHelper;


/**
 * Operations related to solution users. Solution users are nothing but services(virtual entities).
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class SolutionUserResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(SolutionUserResource.class);

    public SolutionUserResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Add solution user to tenant
     *
     * @param solutionUsername Name of solution user prinicpal
     * @param certificate PEM format certificate.
     * @return Details of new solution user @see {@link SolutionUserDTO}
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public SolutionUserDTO create(SolutionUserDTO user) {
        Validate.notEmpty(user.getName(), sm.getString("valid.not.empty", "name"));
        Validate.notNull(user.getCertificate(), sm.getString("valid.not.null", "certificate"));
        Validate.notNull(user.getCertificate(), sm.getString("valid.not.null", "certificate#encoded"));

        try {
            SolutionDetail solutionDetail = new SolutionDetail(user.getCertificate().getX509Certificate(), user.getDescription());
            getIDMClient().addSolutionUser(tenant, user.getName(), solutionDetail);
            return SolutionUserMapper.getSolutionUserDTO(getIDMClient().findSolutionUser(tenant, user.getName()));
        } catch (NoSuchTenantException e) {
            log.debug("Failed to create solution user '{}' on tenant '{}'", user.getName(), tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException | InvalidPrincipalException | DuplicateCertificateException | CertificateException e) {
            log.error("Failed to create solution user '{}' on tenant '{}' due to a client side error", user.getName(), tenant, e);
            throw new BadRequestException(sm.getString("res.soln.create.failed", user.getName(), tenant), e);
        } catch (Exception e) {
            log.error("Failed to create solution user '{}' on tenant '{}' due to a server side error", user.getName(), tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get details of solution user from tenant
     *
     * @param name Name of solution user
     * @return a {@link SolutionUserDTO} instance (Can be null)
     * @throws {@link BadRequestException} On client errors. (bad input)
     */
    @GET @Path("/{solnName}")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public SolutionUserDTO get(@PathParam("solnName") String name) {
        try {
            SolutionUser idmSolutionUser = getIDMClient().findSolutionUser(tenant, name);
            if (idmSolutionUser == null) {
                throw new InvalidPrincipalException(String.format("The requested solution user '%s' does not exist in tenant '%s'", name, tenant));
            }
            return SolutionUserMapper.getSolutionUserDTO(idmSolutionUser);
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to retrieve solution user '{}' from tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.error("Failed to retrieve solution user '{}' from tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.soln.get.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to retrieve solution user '{}' from tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Updates certificate for a solution user.
     *
     * @param name Name of solution user
     * @param certificate The new certificate to be updated with.
     * @return
     * <code> HTTP 200 </code> If update operation succeeds.
     * <code> HTTP 500 </code> Otherwise
     * @throws BadRequestException On client side errors. If tenant/solution user doesn't exist
     * @throws InternalServerErrorException Otherwise
     */
    @PUT @Path("/{solnName}")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.ADMINISTRATOR)
    public SolutionUserDTO update(@PathParam("solnName") String name, SolutionUserDTO user) {
        Validate.notNull(user.getCertificate(), sm.getString("valid.not.null", "user certificate"));

        try {
            SolutionDetail solutionDetail = new SolutionDetail(CertificateHelper.convertToX509(user.getCertificate().getEncoded()));
            getIDMClient().updateSolutionUserDetail(tenant, name, solutionDetail);
            return SolutionUserMapper.getSolutionUserDTO(getIDMClient().findSolutionUser(tenant, user.getName()));
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to update solution user '{}' on tenant '{}'", name, tenant, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (CertificateException | InvalidArgumentException e) {
            log.debug("Failed to update solution user '{}' in tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.soln.update.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to update solution user '{}' in tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Removes solution user from tenant
     *
     * @param name Name of solution user
     * @return
     * @return
     * <code> HTTP 200 </code> If deletion succeeds.
     * <code> HTTP 500 </code> Otherwise
     * @throws BadRequestException On client side errors. If tenant/solution user doesn't exist
     * @throws InternalServerErrorException Otherwise
     */
    @DELETE @Path("/{solnName}")
    @RequiresRole(role=Role.ADMINISTRATOR)
    public void delete(@PathParam("solnName") String name) {
        try {
            getIDMClient().deletePrincipal(tenant, name);
        } catch (InvalidPrincipalException | NoSuchTenantException e) {
            log.debug("Failed to delete solution user '{}' from tenant '{}'", name, tenant);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (InvalidArgumentException e) {
            log.error("Failed to delete solution user '{}' from tenant '{}' due to a client side error", name, tenant, e);
            throw new BadRequestException(sm.getString("res.soln.delete.failed", name, tenant), e);
        } catch (Exception e) {
            log.error("Failed to delete solution user '{}' from tenant '{}' due to a server side error", name, tenant, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Get all groups associated with solution user.
     *
     * @param name Name of solution user
     * @param limit Maximum number of groups to be retrieved. No limit is set on negative value.
     *   <ul> Default value : 200 </ul>
     *   <ul> Negative value return complete set </ul>
     * @return Set of Groups associated with requested solution user.
     * @see {@link SolutionUserDTO}
     */
    @GET @Path("/{name}/groups")
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role=Role.REGULAR_USER)
    public Collection<SolutionUserDTO> getGroups(@PathParam("name") String name, @DefaultValue("200") @QueryParam("limit") int limit) {

//        Set<GroupDTO> groups = new HashSet<GroupDTO>();
//        try {
//            // TODO Implement the IDM API
//        } catch (InvalidArgumentException | InvalidPrincipalException | NoSuchTenantException e) {
//            String errorMessage = String.format("Failed to retrieve groups for solution user '%s' in tenant '%s' due to a client side error", solutionUsername, tenant);
//            log.error(errorMessage, e);
//            throw new BadRequestException(sm.getString("res.soln.get.groups.failed", solutionUsername, tenant), e);
//        } catch (Exception e) {
//            String errorMessage = String.format("Failed to retrieve groups for solution user '%s' in tenant '%s' due to a server side error", solutionUsername, tenant);
//            log.error(errorMessage, e);
//            throw new InternalServerErrorException(sm.getString("ec.500"), e);
//        }

        throw new NotImplementedError(sm.getString("ec.501"));
    }
}