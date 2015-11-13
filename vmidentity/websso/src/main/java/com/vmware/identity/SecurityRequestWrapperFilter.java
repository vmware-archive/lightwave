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

package com.vmware.identity;

import org.springframework.web.filter.OncePerRequestFilter;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

/**
 * Intercept the request to wrap it using SecurityRequestWrapper.
 * This is done to set secure flag for request based on the  X-Forwarded-Proto
 * set by the reverse proxy which contains the protocol of the originating request.
 *
 * The request scheme is used by SAML workflow to generate redirect or post url.
 * The wrapper ensures redirect and post url uses https scheme.
 */
public class SecurityRequestWrapperFilter extends OncePerRequestFilter {
   private static final IDiagnosticsLogger _logger = DiagnosticsLoggerFactory.getLogger(SecurityRequestWrapperFilter.class);

   @Override
   protected void doFilterInternal(HttpServletRequest request,
         HttpServletResponse response, FilterChain filterChain)
         throws ServletException, IOException {

      String fwdProto = request.getHeader("X-Forwarded-Proto");
      //This is done to ensure the re-directs are done on https if originating request was https.
      // The proxy in between can terminate the SSL and convert the request to http request.
      if ("https".equalsIgnoreCase(fwdProto) && !"https".equalsIgnoreCase(request.getScheme())) {
         _logger.debug("X-Forwarded-Proto set to https, so encapsulate it with secure request.");
         SecurityRequestWrapper secureRequest = new SecurityRequestWrapper(request);
         filterChain.doFilter(secureRequest, response);
      }
      else {
         filterChain.doFilter(request, response);
      }
   }
}
