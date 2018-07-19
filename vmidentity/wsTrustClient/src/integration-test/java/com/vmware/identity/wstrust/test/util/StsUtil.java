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

import java.net.URL;
import java.security.Key;
import java.security.KeyStore;
import java.security.cert.X509Certificate;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.CertificateCredential;
import com.vmware.identity.wstrust.client.UsernamePasswordCredential;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.client.SamlToken;

import com.vmware.identity.wstrust.client.impl.DefaultSecurityTokenServiceFactory;

import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.ConnectionConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.SSLTrustedManagerConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.vim.sso.client.exception.SsoException;


/**
 * Utility class for working with the {@link SecurityTokenService}.
 */

/**
 * Utility class for working with the {@link SecurityTokenService}.
 */
public class StsUtil {

  protected static final Logger log = LoggerFactory.getLogger(StsUtil.class);

  /**
   * The default token lifetime in seconds
   */
  public static final long DEFAULT_BEARER_LIFETIME_SECONDS = 60000;

  public static final String HOK_KEYSTORE = "/sslcert/chain1.jks";
  public static final String HOK_CERT_ALIAS = "hokuser";
  private static ConnectionConfig _connectionConfig;
  private static String _ststhumbPrint;
  private static URL _thPrintForUrl;

  /**
   * The domain name corresponding to the system identity source as provided by
   * STS.
   * <p>
   * TODO: Remove this field after "20101020_ECR_System Domain with Name.docx"
   * when STS will also provide local users in UPN format instead of short
   * names.
   */
  public static final String SSO_SYSTEM_DOMAIN = "lightwave.local";

  /**
   * The certificates which factory methods will use to configure STS
   */
  private static X509Certificate[] _trustedCertificates;

  public StsUtil(URL stsURI, KeyStore sslKeyStore, X509Certificate[] trustedCertificates) {
    _connectionConfig = new ConnectionConfig(stsURI, new SSLTrustedManagerConfig(sslKeyStore));
    assert trustedCertificates != null;
    _trustedCertificates = trustedCertificates;
  }

  /**
   * Creates a TokenSpec for a token with lifetime of
   * {@link #DEFAULT_BEARER_LIFETIME_SECONDS}.
   */
  public static TokenSpec createDefaultTokenSpec() {

    return new TokenSpec.Builder(DEFAULT_BEARER_LIFETIME_SECONDS)
                        .createTokenSpec();
  }

  /**
   * Get the query string representing the given user and domain. The result is
   * intended to be used for acquire token methods.
   *
   * @param subjectName   subject name; {@code not-null} value is required
   * @param subjectDomain subject domain; {@code not-null} value is required
   * @return valid query string value
   */
  public static String getSubjectQueryString(String subjectName,
                                             String subjectDomain) {
    assert subjectName != null;
    assert subjectDomain != null;

    String upn = subjectName;
    upn += '@' + subjectDomain;
    return upn;
  }

  /**
   * Get the <i>Security Token Service</i> configured to trust the same
   * certificates that are <i>specified at object creation</i>. The produced
   * service will be capable to work only with <b>bearer tokens</b>.
   *
   * @return valid security token service instance
   */
  public SecurityTokenService getSts() {

    return createSts(null);
  }

  /**
   * Get the <i>Security Token Service</i> configured to trust the same
   * certificates that are <i>specified at object creation</i>. The produced
   * service will be capable to work only with <b>holder-of-key tokens</b>.
   *
   * @param privateKey private key which sign the messages to STS; should be
   *                   pair with the given certificate's public key; {@code not-null}
   *                   value is required
   * @param cert       certificate to be embedded into the token; should be pair with
   *                   the given private key; {@code not-null} value is required
   * @return valid security token service instance
   */
  public SecurityTokenService getHokSts(Key privateKey,
                                        X509Certificate cert) {

    return createSts(new HolderOfKeyConfig(privateKey, cert, null));
  }

  private SecurityTokenService createSts(HolderOfKeyConfig hok) {

    SecurityTokenServiceConfig config = new SecurityTokenServiceConfig(
        _connectionConfig, _trustedCertificates, null, hok);

    return DefaultSecurityTokenServiceFactory.getSecurityTokenService(config);
  }

  // * * * * * * * * * * * *

  // AcquireToken methods

  // * * * * * * * * * * * *

  /**
   * Acquire a bearer token for the given subject. The token lifetime will be
   * {@link #DEFAULT_BEARER_LIFETIME_SECONDS} minutes.
   *
   * @param subject  token subject; {@code not-null} value is required
   * @param password subject password; {@code not-null} value is required
   * @return a valid token instance
   * @throws SsoException
   */
  public SamlToken acquireToken(PrincipalId subject,
                                char[] password)
      throws SsoException {
    String query = getSubjectQueryString(subject.getName(),
        subject.getDomain());

    return getSts().acquireToken(
        new UsernamePasswordCredential(
            query,
            String.valueOf(password)
        ),
        createDefaultTokenSpec()
    );
  }

  /**
   * Acquire a bearer token for the given subject. The token lifetime will be
   * {@link #DEFAULT_BEARER_LIFETIME_SECONDS} minutes.
   *
   * @param userId   token subject; {@code not-null} value is required
   * @param password subject password; {@code not-null} value is required
   * @return a valid token instance
   * @throws SsoException
   */
  public SamlToken acquireToken(String userId,
                                char[] password)
      throws SsoException {
    PrincipalId subject = new PrincipalId(userId, null);
    String query = getSubjectQueryString(
        subject.getName(),
        subject.getDomain()
    );
    return getSts().acquireToken(
        new UsernamePasswordCredential(
            query,
            String.valueOf(password)
        ),
        createDefaultTokenSpec()
    );
  }

  /**
   * Acquire a holder-of-key token for the given subject. The token lifetime
   * will be {@link #DEFAULT_BEARER_LIFETIME_SECONDS} minutes.
   *
   * @param subject    token subject; {@code not-null} value is required
   * @param privateKey private key which sign the messages to STS; should be
   *                   pair with the given certificate's public key; {@code not-null}
   *                   value is required
   * @param cert       certificate to be embedded into the token; should be pair with
   *                   the given private key; {@code not-null} value is required
   * @return a valid token instance
   * @throws SsoException
   */
  public SamlToken acquireTokenByCertificate(PrincipalId subject,
                                             Key privateKey,
                                             X509Certificate cert)
      throws SsoException {

    SecurityTokenService sts = getHokSts(privateKey, cert);

    return sts.acquireToken(new CertificateCredential(cert), createDefaultTokenSpec());
  }

  public SamlToken getAdminBearerToken(String adminUser,
                                       String adminPwd)
      throws SsoException {
    return getSts().acquireToken(
        new UsernamePasswordCredential(
            adminUser,
            adminPwd
        ),
        StsUtil.createDefaultTokenSpec()
    );
  }
}
