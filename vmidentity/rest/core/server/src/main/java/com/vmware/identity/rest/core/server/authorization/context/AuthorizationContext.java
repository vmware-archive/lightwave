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
package com.vmware.identity.rest.core.server.authorization.context;

import java.security.Principal;

import javax.ws.rs.core.SecurityContext;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.authorization.request.ResourceAccessRequest;

/**
 * An injectable security context object that provides access to
 * security and authorization related information.
 *
 * @see javax.ws.rs.core.Context
 */
public class AuthorizationContext implements SecurityContext {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AuthorizationContext.class);

    private Principal principal;
    private Role role;
    private boolean secure;

    /**
     * Constructs a new {@link AuthorizationContext} using a {@link ResourceAccessRequest}.
     *
     * @param request request object used to create the context
     */
    public AuthorizationContext(ResourceAccessRequest request) {
        this(new AuthorizationPrincipal(request.getToken()),
             Role.findByRoleName(request.getToken().getRole()),
             request.isSecure());
    }

    /**
     * Constructs a new {@link AuthorizationContext}.
     *
     * @param principal the principal object for this context
     * @param role the role associated with this context
     * @param secure boolean indicating whether this request was made using a secure channel, such as HTTPS.
     */
    public AuthorizationContext(Principal principal, Role role, boolean secure) {
        this.principal = principal;
        this.role = role;
        this.secure = secure;
    }

    @Override
    public Principal getUserPrincipal() {
        return principal;
    }

    @Override
    public boolean isUserInRole(String role) {
        if (this.role == null) {
            return false;
        }

        return this.role.is(Role.valueOf(role));
    }

    @Override
    public boolean isSecure() {
        return secure;
    }

    @Override
    public String getAuthenticationScheme() {
        log.warn("getAuthenticationScheme is unsupported");
        return "custom";
    }

}
