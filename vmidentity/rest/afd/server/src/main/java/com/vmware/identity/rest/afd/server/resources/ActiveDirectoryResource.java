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
package com.vmware.identity.rest.afd.server.resources;

import javax.ws.rs.Consumes;
import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Produces;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ADIDSAlreadyExistException;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.IdmADDomainAccessDeniedException;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinInfoDTO;
import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinRequestDTO;
import com.vmware.identity.rest.afd.server.mapper.ActiveDirectoryInfoMapper;
import com.vmware.identity.rest.afd.server.util.Config;
import com.vmware.identity.rest.core.data.CredentialsDTO;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.identity.rest.core.server.util.Validate;

/**
 * RESTful Web-service endpoints to perform operations on active directory. The operations to
 * perform includes joining, leaving and getting status of AD.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class ActiveDirectoryResource extends BaseResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(ActiveDirectoryResource.class);

    public ActiveDirectoryResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    /**
     * Join SSO server to Active Directory domain
     *
     * @param username Registered username of Active directory to join (Cannot be null or empty)
     * @param password Password associated with registered username in AD (Cannot be null or empty)
     * @param domain The domain name to join to. (Cannot be null or empty)
     * @param organizationalUnit A subdivision within AD into which users,groups and others are
     *        placed.
     * @return details of joined active directory
     */
    @POST
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public ActiveDirectoryJoinInfoDTO joinActiveDirectory(ActiveDirectoryJoinRequestDTO joinRequest) {
        Validate.notEmpty(joinRequest.getUsername(), "Missing username. Username is required to join AD");
        Validate.notEmpty(joinRequest.getPassword(), "Missing password. Password is required to join AD");
        Validate.notEmpty(joinRequest.getDomain(), "Domain name of AD is missing");
        try {
            getIDMClient().joinActiveDirectory(joinRequest.getUsername(), joinRequest.getPassword(), joinRequest.getDomain(), joinRequest.getOU());
            return ActiveDirectoryInfoMapper.getActiveDirectoryDTO(getIDMClient().getActiveDirectoryJoinStatus());
        } catch (IllegalArgumentException e) {
            log.warn("Failed to join active directory on domain '{}' due to a client side error", e);
            throw new BadRequestException(sm.getString("res.server.add.joinad.failed", joinRequest.getDomain()), e);
        } catch (Exception ex) {
            log.error("Failed to join active directory on domain '{}' due to a server side error", joinRequest.getDomain(), ex);
            throw new InternalServerErrorException(sm.getString("ec.500"), ex);
        }
    }

    /**
     * Leave an active directory domain.
     *
     * @param username An unique identifier to login AD
     * @param password Secured pwd associated with username.
     * @return <code> HTTP 200 OK </code> On successfully leaving active directory
     *         <code> HTTP 500 InternalServerError </code> Otherwise
     * @throws BadRequestException On client side errors
     * @throws InternalServerErrorException Otherwise
     */
    @DELETE
    @Consumes(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public void leaveActiveDirectory(CredentialsDTO activeDirectoryCredentials) {
        Validate.notEmpty(activeDirectoryCredentials.getUsername(), "Missing username. Username is required to leave AD");
        Validate.notEmpty(activeDirectoryCredentials.getPassword(), "Missing password. Password is required to leave AD");
        try {
            getIDMClient().leaveActiveDirectory(activeDirectoryCredentials.getUsername(), activeDirectoryCredentials.getPassword());
        } catch (IdmADDomainAccessDeniedException | ADIDSAlreadyExistException e) {
            log.warn("Failed to leave active directory due to a client side error", e);
            throw new BadRequestException(sm.getString("res.server.delete.leavead.failed"), e);
        } catch (Exception e) {
            log.error("Failed to leave active directory due to a server side error", e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    /**
     * Retrieve the Active Directory Domain Join Status of the system the identity server is
     * executing on.
     *
     * @return Joined info with AD
     * @see ActiveDirectoryJoinInfoDTO
     */
    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public ActiveDirectoryJoinInfoDTO getActiveDirectoryStatus() {
        ActiveDirectoryJoinInfoDTO adJoinInfoDTO = null;
        try {
            ActiveDirectoryJoinInfo joinInfo = getIDMClient().getActiveDirectoryJoinStatus();
            adJoinInfoDTO = ActiveDirectoryInfoMapper.getActiveDirectoryDTO(joinInfo);
            return adJoinInfoDTO;
        } catch (Exception ex) {
            log.error("Failed to retrieve join status of AD", ex);
            throw new InternalServerErrorException(sm.getString("ec.500"), ex);
        }
    }

}
