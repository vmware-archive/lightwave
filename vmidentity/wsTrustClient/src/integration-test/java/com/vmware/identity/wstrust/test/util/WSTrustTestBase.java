/*
 * Copyright (c) 2018 VMware, Inc. All Rights Reserved.
 */
package com.vmware.identity.wstrust.test.util;

import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.openidconnect.client.AccessToken;
import com.vmware.identity.openidconnect.client.ClientConfig;
import com.vmware.identity.openidconnect.client.ConnectionConfig;
import com.vmware.identity.openidconnect.client.MetadataHelper;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateGranularity;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;

import static com.vmware.identity.wstrust.test.util.TestUtils.CLOCK_TOLERANCE_IN_SECONDS;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import static org.junit.Assert.assertFalse;

import java.net.URL;
import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPublicKey;
import java.util.List;

public abstract class WSTrustTestBase {
  protected static final Logger logger = LoggerFactory.getLogger(WSTrustTestBase.class);

  protected static TestConfig properties;

  protected static String domainControllerFQDN;
  protected static int domainControllerPort;

  protected static String systemTenant;
  protected static String systemTenantAdminUsername;

  protected static KeyStore sslTrustedRootsKeystore;

  protected static URL stsURL;

  protected static ConnectionConfig connectionConfigForSystemTenant;
  protected static ClientConfig clientConfigForSystemTenant;
  protected static X509Certificate[] stsSigningCertificates;
  protected static AccessToken accessTokenSystemTenant;

  protected static VmdirClient vmdirClientForSystemTenant;
  protected static IdmClient idmClientForSystemTenant;

  protected static PrincipalManagementServiceUtil pmUtil;

  protected static void setup() throws Exception {
    properties = new TestConfig();

    domainControllerFQDN = properties.getHost();
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
        .keyStore(sslTrustedRootsKeystore).build();

    ProviderMetadata providerMetadata = metadataHelper.getProviderMetadata();
    RSAPublicKey providerPublicKey = metadataHelper.getProviderRSAPublicKey(providerMetadata);

    connectionConfigForSystemTenant = new ConnectionConfig(providerMetadata, providerPublicKey, sslTrustedRootsKeystore);

    clientConfigForSystemTenant = new ClientConfig(
        connectionConfigForSystemTenant,
        null,
        null,
        CLOCK_TOLERANCE_IN_SECONDS
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

    pmUtil = new PrincipalManagementServiceUtil(vmdirClientForSystemTenant);

    logger.info("Getting STS Signing Certificates");

    List<CertificateChainDTO> chains =  idmClientForSystemTenant.certificate().get(
        systemTenant,
        CertificateScope.TENANT,
        CertificateGranularity.CHAIN
    );
    assertFalse(chains.isEmpty());
    List<CertificateDTO> certs = chains.get(0).getCertificates();
    stsSigningCertificates = new X509Certificate[certs.size()];
    int iCert = 0;
    for (CertificateDTO cert : certs) {
      stsSigningCertificates[iCert++] = cert.getX509Certificate();
    }

  }

  protected static void cleanup() throws Exception {

  }
}
