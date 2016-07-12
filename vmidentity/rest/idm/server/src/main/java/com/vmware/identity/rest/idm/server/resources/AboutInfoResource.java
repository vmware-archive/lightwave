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
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.server.resources.BaseResource;
import com.vmware.identity.rest.idm.data.AboutInfo;

@Path("/")
public class AboutInfoResource extends BaseResource {

    public AboutInfoResource(@Context ContainerRequestContext request, @Context SecurityContext securityContext) {
        super(request, securityContext);
    }

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AboutInfoResource.class);

    @GET
    @Produces(MediaType.APPLICATION_JSON)
    @RequiresRole(role = Role.REGULAR_USER)
    public AboutInfo getServiceInformation() {
        try {
            log.info("trying to get about info");
            return new AboutInfo();
        } catch (Exception e) {
            log.error("Failed to provide information about the server due to a server side error", e);
            throw new InternalServerErrorException(sm.getString("ec.500"), e);
        }

    }

}
