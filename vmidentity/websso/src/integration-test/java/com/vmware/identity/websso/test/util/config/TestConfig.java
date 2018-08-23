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

package com.vmware.identity.websso.test.util.config;

import com.vmware.identity.websso.test.util.common.SSOConstants;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Properties;

public class TestConfig {
  private static final String HOST_PROPERTY = "host";
  private static final String CONFIG_PROPERTY = "config";
  private static final String CONFIG_LOCATION = "config.properties";

  private static final String KEY_WEBSSO_PORT = "websso.port";
  private static final String CONFIG_SYSTEM_TENANT = "system.tenant";
  private static final String CONFIG_SYSTEM_DOMAIN = "system.domain";
  private static final String CONFIG_SYSTEM_ADMIN_USERNAME = "system.admin.username";
  private static final String CONFIG_SYSTEM_ADMIN_PASSWORD = "system.admin.password";

  private static final String SSO_DEFAULT_DOMAIN = "lightwave.local";
  private static final String SSO_SYSTEM_DOMAIN = SSO_DEFAULT_DOMAIN;
  private static final String STS_SERVICE_PATH = "/sts/STSService/";

  private static final String KEY_LW_SAMPLE_RP_FQDN = "lightwave.sample.rp.fqdn";
  private static final String KEY_LW_SAMPLE_RP_PORT = "lightwave.sample.rp.port";
  private static final String KEY_LW_SAMPLE_RP_PATH = "lightwave.sample.rp.path";

  private Properties properties;
  private String host;

  public TestConfig() {
    host = System.getProperty(HOST_PROPERTY); if (host == null || host.length() == 0) {
      throw new IllegalArgumentException("Failed to retrieve hostname or IP address");
    }

    String configPath = System.getProperty(CONFIG_PROPERTY, CONFIG_LOCATION);

    properties = new Properties(); try {
      FileInputStream in = new FileInputStream(getClass().getClassLoader().getResource(configPath).getFile());
      properties.load(in); in.close();
    } catch (IOException e) {
      throw new IllegalArgumentException("Error loading config", e);
    }

    properties.putAll(System.getProperties());
  }

  public String getStsHostname() {
    return host;
  }

  public URL getStsUrl(String stsHostname, int portNum, String domainName) {
    try {
      String host = SSOConstants.SCHEME + stsHostname;
      if (portNum > 0) {
        host += ":" + String.valueOf(portNum);
      } if (domainName == null || domainName.isEmpty()) {
        domainName = SSO_DEFAULT_DOMAIN;
      }
      URL propUrl = new URL(host);
      URL subUrl = new URL(propUrl, STS_SERVICE_PATH + domainName);
      return subUrl;
    } catch (MalformedURLException e) {
      throw new IllegalArgumentException(String.format("Failed to create URL for STS server"), e);
    }
  }

  public int getWebSSOPort() {
    return Integer.parseInt(getProperty(KEY_WEBSSO_PORT));
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

  public String GetLightwaveSampleRpFQDN() {
    return properties.getProperty(KEY_LW_SAMPLE_RP_FQDN);
  }

  public int GetLightwaveSampleRpPort() {
    return Integer.parseInt(properties.getProperty(KEY_LW_SAMPLE_RP_PORT));
  }

  public String GetLightwaveSampleRpPath() {
    return properties.getProperty(KEY_LW_SAMPLE_RP_PATH);
  }

  public String getProperty(String property) {
    return properties.getProperty(property);
  }
}
