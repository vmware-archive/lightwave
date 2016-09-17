/*
 *  Copyright (c) 2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.rest.core.server.filters;

import java.io.IOException;

import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.container.ContainerResponseContext;
import javax.ws.rs.container.ContainerResponseFilter;
import javax.ws.rs.core.MultivaluedMap;

/**
 * A CORS response filter that enables CORS feature on server side. The CORS specification will enable to make cross-origin requests
 * to resources present on a different domain than its been hosted.
 * <p>
 * Example : If an API hosted on https://www.domain-1.com/someAPI can make an inturn request to https://www.domain-2.com/someAPI iff the response
 *</p>
 * The CORS headers added are as per W3C recommendation @see <a href="https://www.w3.org/TR/cors/">https://www.w3.org/TR/cors/</a>
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class VmIdentityCORSResponseFilter implements ContainerResponseFilter {

    private static final String HEADER_ACCESS_CONTROL_ALLOWED_ORIGIN = "Access-Control-Allow-Origin";
    private static final String HEADER_ACCESS_CONTROL_ALLOWED_METHODS = "Access-Control-Allow-Methods";
    private static final String HEADER_ACCESS_CONTROL_ALLOWED_HEADERS = "Access-Control-Allow-Headers";
    private static final String VMIDENITY_ALLOWED_ORIGIN = "*"; // All domains are allowed
    private static final String VMIDENTITY_ALLOWED_METHODS = "GET, POST, DELETE, PUT, OPTIONS";
    private static final String VMIDENTITY_ALLOWED_HEADERS = "x-requested-with,Content-Type,Authorization";

    @Override
    public void filter(ContainerRequestContext requestContext,
            ContainerResponseContext responseContext) throws IOException {
        MultivaluedMap<String, Object> responseHeaders = responseContext.getHeaders();
        responseHeaders.add(HEADER_ACCESS_CONTROL_ALLOWED_ORIGIN, VMIDENITY_ALLOWED_ORIGIN);
        responseHeaders.add(HEADER_ACCESS_CONTROL_ALLOWED_METHODS, VMIDENTITY_ALLOWED_METHODS);
        responseHeaders.add(HEADER_ACCESS_CONTROL_ALLOWED_HEADERS, VMIDENTITY_ALLOWED_HEADERS);
    }

}
