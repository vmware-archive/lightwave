package com.vmware.identity.rest.core.server.filters;

import java.io.IOException;

import javax.annotation.Priority;
import javax.ws.rs.Priorities;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerResponseContext;
import javax.ws.rs.container.ContainerResponseFilter;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;

@Priority(Priorities.HEADER_DECORATOR - 1)
public class LoggingContextResetFilter implements ContainerResponseFilter {

    @Override
    public void filter(ContainerRequestContext requestContext, ContainerResponseContext responseContext)
            throws IOException {
        DiagnosticsContextFactory.removeCurrentDiagnosticsContext();
    }
}
