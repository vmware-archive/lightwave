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
package com.vmware.identity.rest.core.server.authorization.filter;

import java.io.IOException;
import java.lang.annotation.Annotation;

import javax.annotation.Priority;
import javax.ws.rs.Priorities;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerRequestFilter;
import javax.ws.rs.container.ResourceInfo;
import javax.ws.rs.core.Context;
import javax.ws.rs.ext.Provider;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.data.ErrorInfo;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.annotation.DynamicRole;
import com.vmware.identity.rest.core.server.authorization.annotation.RequiresRole;
import com.vmware.identity.rest.core.server.authorization.context.AuthorizationContext;
import com.vmware.identity.rest.core.server.authorization.exception.InsufficientRoleException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.request.ResourceAccessRequest;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * A filter that performs authentication checks against incoming requests
 * depending on the target resource method.
 *
 * Uses the {@link RequiresRole} annotation to determine whether the request
 * has the appropriate role for performing a call.
 */
@Provider
@Priority(Priorities.AUTHORIZATION)
public class AuthorizationRequestFilter implements ContainerRequestFilter {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AuthorizationRequestFilter.class);

    // Inject ResourceInfo so we can get annotations off classes and methods
    @Context
    private ResourceInfo resource;

    @Override
    public void filter(ContainerRequestContext context) throws IOException {
        DynamicRole dynamicRole = getMethodAnnotation(DynamicRole.class);
        RequiresRole requiresRole = getMethodAnnotation(RequiresRole.class);

        if (dynamicRole != null) {
            filterByRole(context, null);
        } else if (requiresRole != null) {
            filterByRole(context, requiresRole.role());
        }
    }

    /**
     * Runs the filter dependent on if there is a required role or not.
     *
     * If the role supplied is <tt>null</tt>, then we only perform token checking if a token exists.
     * This allows us to handle situations where role access is being performed dynamically by the
     * underlying resource call.
     *
     * @param context context to parse the request from
     * @param requiredRole the role required for method being called. If <tt>null</tt>
     * we only perform token checking if a token exists.
     */
    private void filterByRole(ContainerRequestContext context, Role requiredRole) {
        StringManager sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);

        try {
            ResourceAccessRequest request = ResourceAccessRequest.fromRequestContext(context);

            if (request == null) {

                if (requiredRole == null) {
                    context.setSecurityContext(new AuthorizationContext(null, null, false));
                } else {
                    throw new InvalidRequestException(sm.getString("auth.ire.no.token"));
                }

            } else {

                // TODO Move error handling here so that we don't have to explicitly do it everywhere else
                request.verify();
                request.validateContents();

                if (requiredRole != null) {
                    request.validateRole(requiredRole);
                }

                context.setSecurityContext(new AuthorizationContext(request));
            }
        } catch (InsufficientRoleException | InvalidRequestException | InvalidTokenException  e) {
            if (log.isWarnEnabled()) {
                ErrorInfo info = (ErrorInfo) e.getResponse().getEntity();
                log.warn("'{}': '{}'", info.getError(), info.getDetails(), e);
            }
            context.abortWith(e.getResponse());
        } catch (Exception e) {
            // Catches ServerException and generic Exceptions thrown by CasIdmClient
            // TODO fix CasIdmClient so that it is explicit about the exceptions it throws rather than throwing
            // generic Exception class
            log.error("An unexpected error occured during authorization", e);
            context.abortWith(new InternalServerErrorException(sm.getString("ec.500"), e).getResponse());
        }
    }

    private <A extends Annotation> A getMethodAnnotation(Class<A> annotationClass) {
        return resource.getResourceMethod().getAnnotation(annotationClass);
    }

}
