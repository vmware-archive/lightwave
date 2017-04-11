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

import java.io.OutputStreamWriter;
import java.nio.charset.Charset;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Exposes a health status endpoint for CM.
 * The only method supported is the Get.
 * CM polls this endpoint with a 'Get' request.
 */
@Controller
public class HealthStatusController {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(HealthStatusController.class);

    private static final Charset UTF8 = Charset.forName("utf-8");

    private final String healthStatusXml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><healthStatus schemaVersion=\"1.0\" xmlns=\"http://www.vmware.com/cis/cm/common/jaxb/healthstatus\"><status>GREEN</status></healthStatus>";

    public HealthStatusController()
    {}

    /**
     * Handle GET request for the health status
     */
    @RequestMapping(value = "/websso/HealthStatus", method = RequestMethod.GET)
    public void  getHealthStatus(HttpServletRequest request, HttpServletResponse response) throws ServletException {

        Validate.notNull(request, "HttpServletRequest should not be null.");
        Validate.notNull(response, "HttpServletResponse should not be null.");

        // we are going to be very simple in this request processing for now
        // and just report if we are "reachable"
        // we also do not worry about the sender's identity
        // (in reality CM will include the SAML token in the header,
        // and so generally it means they successfully obtained it through sso ...)
            logger.debug("Handling getHealthStatus HTTP request; method:{} url:{}",
                    request.getMethod(),
                    request.getRequestURL()
            );

        try {
            response.setHeader("Content-Type", "application/xml; charset=utf-8");
            response.setStatus(HttpServletResponse.SC_OK);
            OutputStreamWriter osw = new OutputStreamWriter(response.getOutputStream(), UTF8);
            try
            {
                osw.write(healthStatusXml);
            }
            finally
            {
                osw.close();
            }

            logger.debug(
                        "Handled getHealthStatus HTTP request; method:{} url:{}",
                        request.getMethod(),
                        request.getRequestURL()
                );
        } catch (Exception e) {
            logger.error("Failed to return Health Status with [%s]." + e.toString(), e);
            throw new ServletException(e); // let the exception bubble up and show in the response since this is not user-facing
        }
    }
}
