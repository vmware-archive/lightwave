/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.wstrust.test.util;

// Class to hold the details of the preconfigured domains.
public class PreconfiguredDomain {
  private String domainType;
  private String domainName;
  private String domainAlias;
  private String adminUser;
  private String adminPwd;
  private String principalDN;
  private String groupDN;
  private String uri;

  public PreconfiguredDomain(String domainType, String domainName,
                             String domainAlias, String adminUser, String adminPwd,
                             String principalDN, String groupDN, String uri) {
    super();
    this.domainType = domainType;
    this.domainName = domainName;
    this.domainAlias = domainAlias;
    this.adminUser = adminUser;
    this.adminPwd = adminPwd;
    this.principalDN = principalDN;
    this.groupDN = groupDN;
    this.uri = uri;
  }

  /**
   * @return the domainType
   */
  public String getDomainType() {
    return domainType;
  }

  /**
   * @return the domainName
   */
  public String getDomainName() {
    return domainName;
  }

  /**
   * @return the domainAlias
   */
  public String getDomainAlias() {
    return domainAlias;
  }

  /**
   * @return the adminUser
   */
  public String getAdminUser() {
    return adminUser;
  }

  /**
   * @return the adminPwd
   */
  public String getAdminPwd() {
    return adminPwd;
  }

  /**
   * @return the principalDN
   */
  public String getPrincipalDN() {
    return principalDN;
  }

  /**
   * @return the groupDN
   */
  public String getGroupDN() {
    return groupDN;
  }

  /**
   * @return the uri
   */
  public String getUri() {
    return uri;
  }

  @Override
  public String toString() {
    return "PreconfiguredDomain [domainType=" + domainType
        + ", domainName=" + domainName + ", domainAlias=" + domainAlias
        + ", adminUser=" + adminUser + ", adminPwd=" + adminPwd
        + ", principalDN=" + principalDN + ", groupDN=" + groupDN
        + ", uri=" + uri + "]";
  }
}
