/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.service;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import org.apache.commons.lang.Validate;

import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class HealthStatusController extends HttpServlet {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(HealthStatusController.class);

    private STSHealthChecker healthChecker = STSHealthChecker.getInstance();

    public void doGet(HttpServletRequest request, HttpServletResponse response) {
        Validate.notNull(request, "HttpServletRequest should not be null.");
        Validate.notNull(response, "HttpServletResponse should not be null.");

        if (healthChecker.getOverallStatus()) {
            response.setStatus(HttpServletResponse.SC_OK);
        } else {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
        }
    }

}
