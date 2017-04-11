/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.identity.rest.idm.data.AboutInfoDTO;
import com.vmware.identity.rest.idm.server.util.Config;

@Path("/")
public class AboutInfoResource extends BaseResource {

    // Constants
    private static final String PRODUCT_NAME_IDM = "idm";
    private static final String IDENTITY_ROOT_KEY = "Software\\VMware\\Identity";
    private static final String RELEASE_KEY = "Release";
    private static final String VERSION_KEY = "Version";

    public AboutInfoResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, Config.LOCALIZATION_PACKAGE_NAME, securityContext);
    }

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AboutInfoResource.class);

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    public AboutInfoDTO getServiceInformation() {
        try {
            log.info("trying to get about info");
            return new AboutInfoDTO(read(RELEASE_KEY), read(VERSION_KEY), PRODUCT_NAME_IDM);
        } catch (Exception e) {
            log.error("Failed to provide information about the server due to a server side error", e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

    }

    private String read(String key) throws Exception {
        String value = null;
        IRegistryKey registryRootKey = null;

        try {

            IRegistryAdapter registryAdpater = RegistryAdapterFactory.getInstance().getRegistryAdapter();
            registryRootKey = registryAdpater.openRootKey((int) RegKeyAccess.KEY_READ);
            if (registryRootKey == null) {
                new IllegalArgumentException("Unable to open Root Key");
            }
            value = registryAdpater.getStringValue(registryRootKey, IDENTITY_ROOT_KEY, key, true);
        } finally {
            if (registryRootKey != null)
                registryRootKey.close();
        }
        return value;
    }
}
