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

package com.vmware.identity.websso.test.util;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.openidconnect.client.AccessToken;
import com.vmware.identity.openidconnect.client.ClientConfig;
import com.vmware.identity.openidconnect.client.ConnectionConfig;
import com.vmware.identity.openidconnect.client.MetadataHelper;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.websso.test.util.config.SampleRPConfiguration;
import com.vmware.identity.websso.test.util.config.TestConfig;
import org.apache.http.client.utils.URIBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.security.KeyStore;
import java.security.interfaces.RSAPublicKey;

public abstract class WebSSOTestBase {

  protected static final Logger logger = LoggerFactory.getLogger(WebSSOTestBase.class);

  protected static final String KEY_ORG_1 = "org1.name";
  protected static final String KEY_ORG_2 = "org2.name";

  protected static TestConfig properties;

  protected static String domainControllerFQDN;
  protected static int domainControllerPort;

  protected static String systemTenant;
  protected static String systemTenantAdminUsername;

  protected static KeyStore sslTrustedRootsKeystore;

  protected static URL stsURL;

  protected static ConnectionConfig connectionConfigForSystemTenant;
  protected static ClientConfig clientConfigForSystemTenant;

  protected static AccessToken accessTokenSystemTenant;

  protected static HistogramCounter sampleAppRequestCounter;
  protected static HistogramCounter stsRedirectCounter;

  protected static VmdirClient vmdirClientForSystemTenant;
  protected static IdmClient idmClientForSystemTenant;

  protected static void setup(String name) throws Exception {
    properties = new TestConfig();
    sampleAppRequestCounter = HistogramCounter.createCounter(name);
    stsRedirectCounter = HistogramCounter.createCounter(name);

    domainControllerFQDN = properties.getStsHostname();
    if (domainControllerFQDN == null || domainControllerFQDN.length() == 0) {
      throw new IllegalStateException(
          "missing host argument, invoke with mvn verify -DskipIntegrationTests=false -Dhost=<host>"
      );
    }
    domainControllerPort = Integer.parseInt(properties.getProperty("oidc.op.port"));

    // create admin client with STS token
    sslTrustedRootsKeystore = KeyStore.getInstance("JKS");
    sslTrustedRootsKeystore.load(null, null);

    logger.debug("Getting SSL Trusted roots");

    // create REST afd client to populate SSL certificates
    TestUtils.populateSSLCertificates(
        domainControllerFQDN,
        domainControllerPort,
        sslTrustedRootsKeystore
    );

    systemTenant = properties.getSystemTenant();
    systemTenantAdminUsername = properties.getSystemAdminUsername();
    String systemTenantAdminPassword = properties.getSystemAdminPassword();

    stsURL = properties.getStsUrl(domainControllerFQDN, domainControllerPort, systemTenant);

    // retrieve OIDC meta data from system tenant
    MetadataHelper metadataHelper = new MetadataHelper.Builder(domainControllerFQDN)
                                                      .domainControllerPort(domainControllerPort)
                                                      .tenant(systemTenant)
                                                      .keyStore(sslTrustedRootsKeystore)
                                                      .build();

    ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
    RSAPublicKey providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

    connectionConfigForSystemTenant = new ConnectionConfig(
                                          providerMetadata,
                                          providerPublicKey,
                                          sslTrustedRootsKeystore
                                      );
    clientConfigForSystemTenant = new ClientConfig(
                                      connectionConfigForSystemTenant,
                                      null,
                                      null,
                                      TestUtils.CLOCK_TOLERANCE_IN_SECONDS
                                  );
    accessTokenSystemTenant = TestUtils.getOidcBearerTokenWithUsernamePassword(
                                  clientConfigForSystemTenant,
                                  systemTenantAdminUsername,
                                  systemTenantAdminPassword
                              );

    logger.debug("Creating IDM Client");

    idmClientForSystemTenant = TestUtils.createIdmClient(
                                    accessTokenSystemTenant,
                                    domainControllerFQDN,
                                    domainControllerPort,
                                    sslTrustedRootsKeystore,
                                    null
                                );

    logger.debug("Creating VMDir Client");

    vmdirClientForSystemTenant = TestUtils.createVMdirClient(
                                      accessTokenSystemTenant,
                                      domainControllerFQDN,
                                      domainControllerPort,
                                      sslTrustedRootsKeystore
                                  );
  }

  protected static SampleRPConfiguration getSAMLRPConfiguration() throws URISyntaxException {
    return new SampleRPConfiguration(
                properties.GetLightwaveSampleRpFQDN(),
                properties.GetLightwaveSampleRpPort(),
                properties.GetLightwaveSampleRpPath(),
                properties.getSystemTenant()
            );
  }

  protected URI GetSampleAppURI(String orgName) throws Exception {
    return new URIBuilder()
                  .setHost(properties.GetLightwaveSampleRpFQDN())
                  .setPort(properties.GetLightwaveSampleRpPort())
                  .setScheme("https")
                  .setPath("/ssolib-sample/logon/" + orgName)
                  .build();
  }

  protected static void cleanup() throws Exception {

  }

  protected String getEntityID(String metadataURL) {
    return WebSSOUtils.getEntityID(WebSSOUtils.getDocumentFromUrl(metadataURL));
  }
}
