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
package com.vmware.identity.rest.core.client.test.integration;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

public class IntegrationTestProperties {

    private static final String CONFIG_LOCATION = "src/integration-test/resources/config.properties";
    private static final String CONFIG_SYSTEM_TENANT = "system.tenant";
    private static final String CONFIG_SYSTEM_DOMAIN = "system.domain";
    private static final String CONFIG_SYSTEM_ADMIN_USERNAME = "system.admin.username";
    private static final String CONFIG_SYSTEM_ADMIN_PASSWORD = "system.admin.password";

    private static final String HOST_PROPERTY = "host";
    private static final String CONFIG_PROPERTY = "config";

    private Properties properties;
    private String host;

    public IntegrationTestProperties() {
        host = System.getProperty(HOST_PROPERTY);
        if (host == null || host.length() == 0) {
            throw new IllegalArgumentException("Failed to retrieve hostname or IP address");
        }

        String configPath = System.getProperty(CONFIG_PROPERTY, CONFIG_LOCATION);

        properties = new Properties();
        try {
            FileInputStream in = new FileInputStream(configPath);
            properties.load(in);
            in.close();
        } catch (IOException e) {
            throw new IllegalArgumentException("Error loading config", e);
        }
    }

    public String getHost() {
        return host;
    }

    public String getSystemTenant() {
        return properties.getProperty(CONFIG_SYSTEM_TENANT);
    }

    public String getSystemDomain() {
        return properties.getProperty(CONFIG_SYSTEM_DOMAIN);
    }

    public String getSystemAdminUsername() {
        return properties.getProperty(CONFIG_SYSTEM_ADMIN_USERNAME);
    }

    public String getSystemAdminPassword() {
        return properties.getProperty(CONFIG_SYSTEM_ADMIN_PASSWORD);
    }

    public String getProperty(String property) {
        return properties.getProperty(property);
    }

}
