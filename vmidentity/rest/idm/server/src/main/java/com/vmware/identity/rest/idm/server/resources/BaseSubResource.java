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
import java.util.EnumSet;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;

/**
 *
 * A sub resource which is based per tenant.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public abstract class BaseSubResource extends BaseResource {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(BaseSubResource.class);

    protected String tenant;

    public BaseSubResource(String tenant, Locale locale, String correlationId, String localizationPackage, SecurityContext securityContext) {
        super(locale, correlationId, localizationPackage, securityContext);
        this.tenant = tenant;
    }

    public BaseSubResource(String tenant, ContainerRequestContext request, String localizationPackage, SecurityContext securityContext) {
        super(request, localizationPackage, securityContext);
        this.tenant = tenant;
    }

    public String getTenant() {
        return tenant;
    }

    public String getSystemDomain() {
        try {
            Collection<IIdentityStoreData> providers = getIDMClient().getProviders(getTenant(), EnumSet.of(DomainType.SYSTEM_DOMAIN));
            return providers.iterator().next().getName();
        } catch (NoSuchTenantException e) {
            log.debug("Failed to retrieve system domain", e);
            throw new NotFoundException(sm.getString("ec.404"), e);
        } catch (Exception e) {
            log.error("Failed to retrieve system domain due to IDM errors", e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }
    }
}
