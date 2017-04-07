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

import java.util.Collection;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.rest.afd.data.ActiveDirectoryJoinInfoDTO;
import com.vmware.identity.rest.afd.server.util.Config;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.resources.BaseResource;

/**
 * Resource that contains alternative POST endpoints for every GET endpoint throughought the other
 * resources.
 *
 * This is necessary in order to allow users to send tokens of unbounded sizes. Once the OIDC server
 * can support creating and maintaining opaque tokens and once it can exchange SAML tokens for OIDC
 * tokens, this resource should be removed and all operations should go through the standard
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

    @POST @Path("/provider/ad")
    @Consumes(MediaType.APPLICATION_JSON) @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public ActiveDirectoryJoinInfoDTO getActiveDirectoryStatus() {
        return new ActiveDirectoryResource(getRequest(), getSecurityContext()).getActiveDirectoryStatus();
    }

    @POST @Path("/vecs/ssl")
    @Produces(MediaType.APPLICATION_JSON)
    public Collection<CertificateDTO> getSSLCertificates() {
        return new VecsResource(getRequest(), getSecurityContext()).getSSLCertificates();
    }

}
