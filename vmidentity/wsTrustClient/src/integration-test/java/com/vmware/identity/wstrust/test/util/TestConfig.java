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

package com.vmware.identity.wstrust.test.util;

import com.vmware.vim.sso.client.Advice;

import java.io.FileInputStream;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

public class TestConfig {

  public static final String USERID = "temp";
  public static final String ADVICE = "advice";
  public static final String CONFIRMATION = "confirmation";
  public static final String RENEW = "renew";
  public static final String RESTRICTION = "restriction";
  public static final String DELEGATE = "delegatable";
  public static final String DELEGATETO = "delegateto";
  public static final String SECOND_DELEGATETO = "delegateto2";
  public static final String SOLUTIONNAME = "solution1";
  public static final String SOLUTIONNAME3 = "solution3";
  public static final String SOLUTIONNAME2 = "solution2";
  public static final String MODIFIED_RESTRICTIONS = "modifiedrestriction";
  public static final String ADDITIONAL_ADVICE = "additionaladvice";

  private static final String SSO_DEFAULT_DOMAIN = "lightwave.local";
  private static final String SSO_SYSTEM_DOMAIN = SSO_DEFAULT_DOMAIN;
  private static final String STS_SERVICE_PATH = "/sts/STSService/";

  private static final String CONFIG_LOCATION = "config.properties";
  private static final String CONFIG_SYSTEM_TENANT = "system.tenant";
  private static final String CONFIG_SYSTEM_DOMAIN = "system.domain";
  private static final String CONFIG_SYSTEM_ADMIN_USERNAME = "system.admin.username";
  private static final String CONFIG_SYSTEM_ADMIN_PASSWORD = "system.admin.password";

  public static final String PARTICIPANT_1 = "urn:vc:sf";
  public static final String PARTICIPANT_2 = "urn:vc:pa";
  public static final String BAD_PARTICIPANT = "urn#$%^&*vc--pa";
  public static final String ADVICE_SOURCE_1 = "urn:vc:permissions";
  public static final String ADVICE_SOURCE_2 = "urn:sso:permissions";
  public static final String BAD_ADVICE_SOURCE = "urn-vc#$%^&*permissions";
  public static final String ADVICE_ATTR_NAME_1 = "urn:vc:admin.users";
  public static final String ADVICE_ATTR_NAME_2 = "urn:sso:admin.users";
  public static final String BAD_ADVICE_ATTR_NAME = "urnssoadmin$%^&*###users";

  public static final Advice ADVICE_1 = new Advice(
      ADVICE_SOURCE_1,
      Arrays.asList(new Advice.AdviceAttribute(ADVICE_ATTR_NAME_1))
  );
  public static final Advice ADVICE_1_VALUE = new Advice(
      ADVICE_SOURCE_1,
      Arrays.asList(new Advice.AdviceAttribute(ADVICE_ATTR_NAME_1, Arrays
          .asList("value1")))
  );
  public static final Advice BAD_ADVICE_VALUE = new Advice(
      BAD_ADVICE_SOURCE,
      Arrays.asList(new Advice.AdviceAttribute(BAD_ADVICE_ATTR_NAME, Arrays
          .asList("value1")))
  );
  public static final Advice ADVICE_2 = new Advice(
      ADVICE_SOURCE_2,
      Arrays.asList(new Advice.AdviceAttribute(ADVICE_ATTR_NAME_2))
  );

  public static final String HOK_KEYSTORE_KEY = "hok.keystore.url";
  public static final String HOK_CERT_ALIAS_KEY = "hok.cert.alias";
  public static final String KEYSTORE_PASSWORD = "keystore.password";
  public static final String COMMON_KEYSTORE_PASSWORD = "common.keystore.password";

  private static final String HOST_PROPERTY = "host";
  private static final String CONFIG_PROPERTY = "config";

  private Properties properties;
  private String host;

  public TestConfig() {
    host = System.getProperty(HOST_PROPERTY);
    if (host == null || host.length() == 0) {
      throw new IllegalArgumentException("Failed to retrieve hostname or IP address");
    }

    String configPath = System.getProperty(CONFIG_PROPERTY, CONFIG_LOCATION);

    properties = new Properties();
    try {
      FileInputStream in = new FileInputStream(getClass().getClassLoader().getResource(configPath).getFile());
      properties.load(in);
      in.close();
    } catch (IOException e) {
      throw new IllegalArgumentException("Error loading config", e);
    }

    properties.putAll(System.getProperties());
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

  public String getUserName(String userIdKey) {
    return getProperty(getUserNamePropertyKey(userIdKey));
  }

  public String getUserPassword(String userIdKey) {
    return getProperty(getUserPasswordPropertyKey(userIdKey));
  }

  public String getUserGroupName(String userIdKey)
  {
    return getProperty(getGroupNamePropertyKey(userIdKey));
  }

  public String getUserNamePropertyKey(String userIdKey) {
    return userIdKey + ".user.name";
  }

  public String getUserPasswordPropertyKey(String userIdKey) {
    return userIdKey + ".user.password";
  }

  private String getUserDomainPropertyKey(String userIdKey) {
    return userIdKey + ".user.domain";
  }

  private String getGroupNamePropertyKey(String userIdKey) {
    return userIdKey + ".group.name";
  }

  public String getUserCertificatePath(String solutionIdKey) {
    return getProperty(solutionIdKey.concat(".cert"));
  }

  public String getUserCertificateAlias(String solutionIdKey) {
    return getProperty(solutionIdKey.concat(".certalias"));
  }

  public String getHokKeystore() {
    return getProperty(HOK_KEYSTORE_KEY);
  }

  public String getHokCertAliasKey() {
    return getProperty(HOK_CERT_ALIAS_KEY);
  }

  public char[] getKeystorePassword(String userId) {
    if (getProperty(userId + "." + KEYSTORE_PASSWORD) != null
        && !(getProperty(userId + "." + KEYSTORE_PASSWORD).isEmpty())) {
      return getProperty(userId + "." + KEYSTORE_PASSWORD).toCharArray();
    } else if (getProperty(COMMON_KEYSTORE_PASSWORD) != null
        && !(getProperty(COMMON_KEYSTORE_PASSWORD).isEmpty())) {
      return getProperty(COMMON_KEYSTORE_PASSWORD).toCharArray();
    } else {
      return new char[]{};
    }
  }

  public String getSSOSystemDomain() {
    return getProperty(SSO_SYSTEM_DOMAIN);
  }

  public Set<String> getAudienceRestricitions(String restrictions) {
    if (restrictions.equalsIgnoreCase("valid.restriction")) {
      return new HashSet<String>(Arrays.asList(PARTICIPANT_1));
    }
    if (restrictions.equalsIgnoreCase("bad.restriction")) {
      return new HashSet<String>(Arrays.asList(BAD_PARTICIPANT));
    }
    return null;
  }

  public List<Advice> getAdviceList(String adviceString) {
    if (adviceString.equalsIgnoreCase("valid.advice")) {
      return Arrays.asList(ADVICE_1_VALUE);
    }
    if (adviceString.equalsIgnoreCase("bad.advice")) {
      return Arrays.asList(BAD_ADVICE_VALUE);
    }
    return null;
  }

  public URL getStsUrl(String stsHostname, int portNum, String domainName) {
    try {
      String host = "https://" + stsHostname;
      if (portNum > 0) {
        host += ":" + String.valueOf(portNum);
      }
      if (domainName == null || domainName.isEmpty()) {
        domainName = SSO_DEFAULT_DOMAIN;
      }
      URL propUrl = new URL(host);
      URL subUrl = new URL(propUrl, STS_SERVICE_PATH + domainName);
      return subUrl;
    } catch (MalformedURLException e) {
      throw new IllegalArgumentException(String.format(
          "Failed to create URL for STS server"), e);
    }
  }
}
