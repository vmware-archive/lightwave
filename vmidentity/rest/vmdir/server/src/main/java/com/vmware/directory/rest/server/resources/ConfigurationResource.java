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
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.ws.rs.Consumes;
import javax.ws.rs.GET;
import javax.ws.rs.PUT;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.directory.rest.common.data.LockoutPolicyDTO;
import com.vmware.directory.rest.common.data.PasswordPolicyDTO;
import com.vmware.directory.rest.common.data.TenantConfigType;
import com.vmware.directory.rest.common.data.TenantConfigurationDTO;
import com.vmware.directory.rest.server.PathParameters;
import com.vmware.directory.rest.server.mapper.LockoutPolicyMapper;
import com.vmware.directory.rest.server.mapper.PasswordPolicyMapper;
import com.vmware.directory.rest.server.util.Config;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPasswordPolicyException;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;

public class ConfigurationResource extends BaseSubResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(ConfigurationResource.class);

    public ConfigurationResource(String tenant, @Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(tenant, request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public TenantConfigurationDTO getConfig(@PathParam(PathParameters.TENANT_NAME) String tenantName,
            @QueryParam("type") final List<String> configTypes) {

        Set<TenantConfigType> requestedConfigs = new HashSet<TenantConfigType>();
        for (String configType : configTypes) {
            validateConfigType(configType);
            requestedConfigs.add(TenantConfigType.valueOf(configType.toUpperCase()));
        }

        // Default to retrieve complete tenant configuration
        if (requestedConfigs.size() == 0) {
            requestedConfigs.add(TenantConfigType.ALL);
        }

        LockoutPolicyDTO lockoutPolicy = null;
        PasswordPolicyDTO passwordPolicy = null;

        try {
            if (requestedConfigs.contains(TenantConfigType.ALL)) {
                lockoutPolicy = getLockoutPolicy(tenantName);
                passwordPolicy = getPasswordPolicy(tenantName);
            } else {
                for (TenantConfigType type : requestedConfigs) {
                    switch (type) {
                    case LOCKOUT:
                        lockoutPolicy = getLockoutPolicy(tenantName);
                        break;

                    case PASSWORD:
                        passwordPolicy = getPasswordPolicy(tenantName);
                        break;
                    }
                }
            }

            return TenantConfigurationDTO.builder()
                    .withLockoutPolicy(lockoutPolicy)
                    .withPasswordPolicy(passwordPolicy)
                    .build();

        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve configuration details of tenant '{}'", tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (IllegalArgumentException | InvalidArgumentException e) {
            log.error("Failed to retrieve configuration details of tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException(sm.getString("res.ten.get.config.failed", Arrays.asList(requestedConfigs), tenantName), e);
        } catch (Exception e) {
            log.error("Failed to retrieve configuration details of tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

    }

    @PUT
    @Consumes(MediaType.APPLICATION_JSON)
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.ADMINISTRATOR)
    public TenantConfigurationDTO updateConfig(@PathParam(PathParameters.TENANT_NAME) String tenantName, TenantConfigurationDTO configurationDTO) {

        TenantConfigurationDTO.Builder configBuilder = TenantConfigurationDTO.builder();
        LockoutPolicyDTO lockoutPolicy = configurationDTO.getLockoutPolicy();
        PasswordPolicyDTO passwordPolicy = configurationDTO.getPasswordPolicy();

        try {
            // update lockout policy configuration of tenant
            if (lockoutPolicy != null) {
                LockoutPolicy lockoutPolicyUpdate = LockoutPolicyMapper.getLockoutPolicy(lockoutPolicy);
                getIDMClient().setLockoutPolicy(tenantName, lockoutPolicyUpdate);
                configBuilder.withLockoutPolicy(getLockoutPolicy(tenantName));
            }

            // update password policy configuration of tenant
            if (passwordPolicy != null) {
                PasswordPolicy passwordPolicyUpdate = PasswordPolicyMapper.getPasswordPolicy(passwordPolicy);
                getIDMClient().setPasswordPolicy(tenantName, passwordPolicyUpdate);
                configBuilder.withPasswordPolicy(getPasswordPolicy(tenantName));
            }
            return configBuilder.build();

        } catch (NoSuchTenantException e) {
            log.debug("Failed to update the configuration details for tenant '{}'",tenantName, e);
            throw new NotFoundException(sm.getString("ec.404"),e);
        } catch( IllegalArgumentException | InvalidArgumentException | InvalidPasswordPolicyException e){
            log.error("Failed to update the configuration details for tenant '{}' due to a client side error", tenantName, e);
            throw new BadRequestException(sm.getString("res.ten.update.config.failed", tenantName), e);
        } catch (Exception e){
            log.error("Failed to update the configuration details for tenant '{}' due to a server side error", tenantName, e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }

    private LockoutPolicyDTO getLockoutPolicy(String tenantName) throws Exception {
        return LockoutPolicyMapper.getLockoutPolicyDTO(getIDMClient().getLockoutPolicy(tenantName));
    }

    private PasswordPolicyDTO getPasswordPolicy(String tenantName) throws Exception {
        return PasswordPolicyMapper.getPasswordPolicyDTO(getIDMClient().getPasswordPolicy(tenantName));
    }

    private void validateConfigType(String configName) {
        try {
            TenantConfigType.valueOf(configName.toUpperCase());
        } catch (IllegalArgumentException e) {
            log.error("Invalid tenant configuration parameter '{}'. Valid values are {}", configName, Arrays.asList(TenantConfigType.values()), e);
            throw new BadRequestException(sm.getString("valid.invalid.type", configName, TenantConfigType.values()), e);
        }
    }
}
