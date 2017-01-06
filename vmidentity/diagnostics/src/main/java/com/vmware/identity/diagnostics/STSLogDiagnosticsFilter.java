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
package com.vmware.identity.diagnostics;

import org.apache.logging.log4j.ThreadContext;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.annotation.WebFilter;

import java.io.IOException;

/**
 * ServletFilter to extract request data and add it to Log4j's ThreadContext.
 *
 * The purpose of this diagnostics filter is such LOG4J Routing Appender uses information on
 * ThreadContext(aka MDC) to route the log event to a specific log appender(Rolling File appender).
 *
 * This filter gets execute per HTTP request. The sequence of operations are as below:
 * <p>
 * <li> A HTTP requests hits Tomcat server and thus this filter gets executed initially before application </li>
 * <li> The filter adds "service" param and its corresponding value to MDC based on request URL pattern </li>
 * <li> The respective logger statements like log.info/debug/etc will now choose an appender based on value for "service"
 * in MDC defined against LOG4J2.xml configuration. <li>
 * The serviceName constants in MDC are ties to log4j route. The LOG4J routing table is as below :
 *
 * URL PATTERN(Matched with)   ->  SERVICE NAME ->(Used in Log4j route)  LOG4JFILENAME
 *
 * "/sts/STSService"          ->  sts          ->           vmware-identity-sts.log
 * "/openidconnect"           ->  openidconnect ->          openidconnect.log
 * "/idm"                     ->  idm ->                    vmware-identity-rest-idm.log
 * "/afd"                     ->  afd ->                    vmware-identity-rest-afd.log
 * "/websso"                  ->  websso ->                 websso.log
 * "/vmdir"                   ->  vmdir  ->                 vmware-identity-rest-vmdir.log
 * "/lookup"                  ->  lookup  ->                vmware-identity-rest-lookup.log
 *
 * <li> Once, the application serves with response - The context for "service" is cleared </li>
 *
 * </p>
 */
@WebFilter(urlPatterns = {"/*"}, asyncSupported = true )
public class STSLogDiagnosticsFilter implements Filter {

    private static final String SERVICE = "service";

    private static final String SERVICE_REST_IDM_BASEURL = "/idm";
    private static final String SERVICE_REST_AFD_BASEURL = "/afd";
    private static final String SERVICE_REST_VMDIR_BASEURL = "/vmdir";
    private static final String SERVICE_REST_LOOKUP_BASERURL = "/lookup";
    private static final String SERVICE_STS_BASEURL = "/sts/STSService";
    private static final String SERVICE_WEBSSO_BASEURL = "/websso";
    private static final String SERVICE_OPENIDCONNECT_BASEURL = "/openidconnect";
    private static final String SERVICE_LEGACY_LOOKUPSERVER_BASEURL = "/lookupservice";
    private static final String SERVICE_LEGACY_ADMIN_BASEURL = "/sso-adminserver";

    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException, ServletException {

        HttpServletRequest httpReq = (HttpServletRequest) request;
        String servletpath = httpReq.getServletPath();
        String serviceName = null;

        try {
            if(servletpath.startsWith(SERVICE_STS_BASEURL)) {
                serviceName = "sts";
            } else if (servletpath.startsWith(SERVICE_REST_AFD_BASEURL)) {
                serviceName = "afd";
            } else if (servletpath.startsWith(SERVICE_REST_IDM_BASEURL)) {
                serviceName = "idm";
            } else if (servletpath.startsWith(SERVICE_REST_VMDIR_BASEURL)) {
                serviceName = "vmdir";
            } else if (servletpath.startsWith(SERVICE_REST_LOOKUP_BASERURL)) {
                serviceName = "lookup";
            } else if(servletpath.startsWith(SERVICE_WEBSSO_BASEURL)) {
                serviceName = "websso";
            } else if (servletpath.startsWith(SERVICE_OPENIDCONNECT_BASEURL)) {
                serviceName = "openidconnect";
            }else if (servletpath.startsWith(SERVICE_LEGACY_LOOKUPSERVER_BASEURL)) {
                serviceName = "lookupServer";
            } else if(servletpath.startsWith(SERVICE_LEGACY_ADMIN_BASEURL)) {
                serviceName = "ssoAdminServer";
            }
            ThreadContext.put(SERVICE, serviceName);
            chain.doFilter(request, response);
        } finally {
            ThreadContext.remove(SERVICE);
        }
    }

    @Override
    public void destroy() {
        // No-op
    }

    @Override
    public void init(FilterConfig filterConfiguration) throws ServletException {
        // No-op
    }

}
