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
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


import com.vmware.identity.wstrust.client.SecurityTokenService;

/**
 * Factory class to instantiate and invoke appropriate methods on Security Token Service
 * This factory abstracts the synchronous and asynchronous methods from Tests
 * This can also be extended to abstract C++ wrapper from Tests
 */
public class TokenServiceFactory {

  private static final Logger _log = LoggerFactory.getLogger(TokenServiceFactory.class);

  /**
   * @param signingCertificates
   * @param enumType
   * @param keyStoreFilePath
   * @param certAlias
   * @param password
   * @return
   */
  public static ITokenService getHokTokenServiceInstance(
      URL stsURL,
      KeyStore sslKeyStore,
      X509Certificate[] signingCertificates,
      MethodEnum enumType,
      String keyStoreFilePath,
      String certAlias,
      char[] password) {
    KeyStoreHelper ks = new KeyStoreHelper(keyStoreFilePath);
    PrivateKey privateKey = ks.getPrivateKey(certAlias, password);
    X509Certificate cert = ks.getCertificate(certAlias);
    SecurityTokenService stsService = new StsUtil(stsURL, sslKeyStore, signingCertificates).getHokSts(
        privateKey, cert);
    _log.info("Successfully retrieved HOK STS Service instance Using Certificate "
                  + keyStoreFilePath);
    return getTokenService(enumType, stsService);
  }

  /**
   * Returns the actual implementation of ITokenService
   * based on the enum
   *
   * @param enumType
   * @param stsService
   * @return The actual implementation of the ITokenService interface
   */
  private static ITokenService getTokenService(
      MethodEnum enumType,
      SecurityTokenService stsService) {
    if (enumType.equals(MethodEnum.SYNC)) {
      return new SyncTokenService(stsService);
    } else if (enumType.equals(MethodEnum.ASYNC)) {
      return new AsyncTokenService(stsService);
    } else if (enumType.equals(MethodEnum.ASYNC_NULLHANDLER)) {
      return new AsyncTokenServiceNullhandler(stsService);
    }
    return null;
  }
}
