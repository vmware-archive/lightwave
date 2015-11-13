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
package com.vmware.identity.rest.afd.server.test.integration.resources;

import java.security.InvalidParameterException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * Base class for running integration tests. It beholds responsibilities such as lazy initialization of idmclient, resource helpers etc
 */
public class TestBase {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(TestBase.class);

    public static final String PROPERTY_HOST = "host";

    protected CasIdmClient idmClient;
    private final String targetHost;

    public TestBase() {
        this.targetHost = System.getProperty(PROPERTY_HOST);
        if (targetHost == null || targetHost.length() == 0) {
            String errMessage = "Failed to retrieve hostname or IP address of sso server to run integration tests against";
            log.error(errMessage);
            throw new InvalidParameterException(errMessage);
        }
        idmClient = new CasIdmClient(targetHost);
    }

    /**
     * Hostname or IPaddress of Single Sign On server used as target to run integration tests
     * against
     */
    public String getTargetHost() {
        return targetHost;
    }

}
