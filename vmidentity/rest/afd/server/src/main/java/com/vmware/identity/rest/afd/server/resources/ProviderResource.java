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

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.Path;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.SecurityContext;

/**
 * A sub-resource redirector for provider
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Path("/provider")
public class ProviderResource extends BaseResource {

    public ProviderResource(@Context HttpServletRequest request, @Context SecurityContext securityContext) {
        super(request, securityContext);
    }

    @Path("/ad")
    public ActiveDirectoryResource getActiveDirectorySubResource() {
        return new ActiveDirectoryResource(getRequest(), getSecurityContext());
    }

}
