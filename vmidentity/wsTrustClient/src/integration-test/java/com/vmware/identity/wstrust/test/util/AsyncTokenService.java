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

import java.security.cert.Certificate;
import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.CertificateCredential;
import com.vmware.identity.wstrust.client.GSSCredential;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.UsernamePasswordCredential;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.NegotiationHandler;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.vim.sso.client.exception.SsoException;

/**
 * Async implementation of TokenServiceMethod with No handler
 */
public class AsyncTokenService implements ITokenService {

  private static final Logger _log = LoggerFactory
      .getLogger(AsyncTokenService.class);
  /*
   * Security Token Service instance
   */
  private SecurityTokenService _stsService = null;

  public AsyncTokenService(SecurityTokenService stsService) {
    _stsService = stsService;
  }

  /*
   * (non-Javadoc)
   *
   * @see
   * com.vmware.vcqa.sso.client.TokenServiceMethod#acquireToken(com.vmware
   * .vim.sso.client.Subject, java.lang.String,
   * com.vmware.vim.sso.client.TokenSpec)
   */
  @Override
  public SamlToken acquireToken(String subject, String password,
                                TokenSpec tokenSpec) throws Exception {
    Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
                                        new UsernamePasswordCredential(
                                            subject,
                                            password
                                        ),
                                        tokenSpec
                                    );
    _log.info("Successfully acquired the future<token> object for acquiretokenAsync");
    return tokenFuture.get();
  }

  /*
   * (non-Javadoc)
   *
   * @see
   * com.vmware.vcqa.sso.client.TokenServiceMethod#acquireTokenByGSS(com.vmware
   * .vim.sso.client.TokenSpec,
   * com.vmware.vim.sso.client.SecurityTokenService.NegotiationHandler)
   */
  @Override
  public SamlToken acquireTokenByGSS(TokenSpec tokenSpec,
                                     NegotiationHandler handler) throws SsoException, Exception {
    Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
                                        new GSSCredential(handler),
                                        tokenSpec
                                    );
    _log.info("Successfully acquired the future<token> object for acquiretokenGSS");
    return tokenFuture.get();
  }

  /*
   * (non-Javadoc)
   *
   * @see
   * com.vmware.vcqa.sso.client.TokenServiceMethod#acquireTokenByCertificate
   * (com.vmware.vim.sso.client.Subject, com.vmware.vim.sso.client.TokenSpec)
   */
  @Override
  public SamlToken acquireTokenByCertificate(Certificate certificate, TokenSpec tokenSpec)
      throws Exception {
    Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(new CertificateCredential(certificate), tokenSpec);
    _log.info("Successfully acquired the future<token> object for acquiretokenByCertificate");
    return tokenFuture.get();
  }

  /*
   * (non-Javadoc)
   *
   * @see
   * com.vmware.vcqa.sso.client.TokenServiceMethod#validateToken(com.vmware
   * .vim.sso.client.SamlToken)
   */
  @Override
  public boolean validateToken(SamlToken token) throws Exception {
    Future<Boolean> valid = _stsService.validateTokenAsync(token);
    _log.info("Successfully acquired the future<boolean> object for validateAsync");
    return valid.get().booleanValue();
  }

  /*
   * (non-Javadoc)
   *
   * @see
   * com.vmware.vcqa.sso.client.TokenServiceMethod#renewToken(com.vmware.vim
   * .sso.client.SamlToken, com.vmware.vim.sso.client.TokenLifetimeSpec)
   */
  @Override
  public SamlToken renewToken(SamlToken token, long extendTime)
      throws Exception {
    Future<SamlToken> tokenFuture = _stsService.renewTokenAsync(token,
        extendTime);
    _log.info("Successfully acquired the future<token> object for renewtokenAsync");
    return tokenFuture.get();
  }

  @Override
  public SamlToken acquireTokenByToken(SamlToken token, TokenSpec tokenSpec)
      throws Exception {
    Future<SamlToken> tokenFuture = _stsService.acquireTokenAsync(
                                        new TokenCredential(token),
                                        tokenSpec
                                    );
    _log.info("Successfully acquired the future<token> object for acquiretokenByTokenAsync");
    return tokenFuture.get();
  }
}
