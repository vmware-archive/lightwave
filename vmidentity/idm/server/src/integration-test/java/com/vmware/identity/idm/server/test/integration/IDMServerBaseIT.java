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

package com.vmware.identity.idm.server.test.integration;

import java.util.Properties;

import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.performance.PerformanceMonitorFactory;
import com.vmware.identity.idm.server.test.integration.util.NoopPerfMonitor;

/**
 * Base Class for IDM Server Integration Test
 */
public class IDMServerBaseIT {

    protected static String systemTenant;
    protected static String systemTenantAdminUsername;
    protected static String systemTenantAdminPassword;
    protected static String domainControllerFQDN;
    protected static String domainDN;

    public static void setUp(String config) throws Exception {
        PerformanceMonitorFactory.setPerformanceMonitor(new NoopPerfMonitor());
        Properties properties = new Properties();
        properties.load(IDMServerBaseIT.class.getClassLoader().getResourceAsStream(config));
        systemTenantAdminUsername = properties.getProperty("admin.user");
        systemTenantAdminPassword = properties.getProperty("admin.password");
        systemTenant = properties.getProperty("tenant");
        domainControllerFQDN = System.getProperty("host");
        if (domainControllerFQDN == null || domainControllerFQDN.length() == 0) {
            throw new IllegalStateException("missing host argument, invoke with mvn verify -DskipIntegrationTests=false -Dhost=<host>");
        }
        domainDN = ServerUtils.getDomainDN(systemTenant);
    }
}