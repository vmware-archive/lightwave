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

import com.vmware.identity.websso.test.util.common.Assert;
import com.vmware.identity.websso.test.util.WebSSOUtils;

import java.net.URI;
import java.net.URISyntaxException;

import org.apache.http.client.utils.URIBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;

public class SampleRPConfiguration extends STSConfiguration {
  private static final Logger log = LoggerFactory.getLogger(SampleRPConfiguration.class);

  private int port;
  private String path;
  private String tenantName;
  private URI rpLogonURI;
  private URI rpMetadataURI;

  public SampleRPConfiguration(String hostName, int port, String path, String tenantName) throws URISyntaxException {
    super(hostName);

    this.port = port;
    if (path.startsWith("/")) {
      this.path = path;
    } else {
      this.path = "/" + path;
    }

    setTenantName(tenantName);
    /*
     * This class uses the format of the sample app that gets deployed from ssolib-sample.war
     */
    rpLogonURI = new URIBuilder()
        .setHost(hostName)
        .setPort(port)
        .setScheme("https")
        .setPath(path + "/logon/" + tenantName)    // Set path something similar to "/ssolib-sample/logon/Org1
        .build();
    rpMetadataURI = new URIBuilder()
        .setHost(hostName)
        .setPort(port)
        .setScheme("https")
        // Set path something similar to "/ssolib-sample/logon/Org1
        .setPath(path + "/SsoClient/Metadata/import/" + tenantName)
        .build();
    setMetadataURL(rpMetadataURI.toString());
  }

  public URI getRPLogonURI() {
    Assert.assertNotNull(rpLogonURI, "Error: rpLogonURI cannot be NULL");
    return rpLogonURI;
  }

  public URI getRPMetadataURI() {
    Assert.assertNotNull(rpMetadataURI, "Error: rpMetadataURI cannot be NULL");
    return rpMetadataURI;
  }

  public Document getSPMetadata() {
    Document metaDoc = WebSSOUtils.getDocumentFromUrl(
        getRPMetadataURI().toString());
    return metaDoc;
  }
}
