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

import java.security.cert.X509Certificate;
import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.Assert;

import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.client.ConfirmationType;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.SamlToken.TokenDelegate;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.TokenSpec.DelegationSpec;

public final class TokenUtil {
  public static final int DEFAULT_BEARER_LIFETIME_SECONDS = 600;

  /**
   * Validates bearer token against expected specification and subject.
   *
   * @param token   token to be validated, not null
   * @param spec    expected not null token specification
   * @param subject expected not null subject
   */
  static void validateBearerToken(SamlToken token, TokenSpec spec,
                                  PrincipalId subject, String systemDomain) {
    validateTokenImpl(token, spec, subject, ConfirmationType.BEARER, null, systemDomain);
  }

  /**
   * Validates HOK token against expected specification, subject and
   * certificate.
   *
   * @param token   token to be validated, not null
   * @param spec    expected not null token specification
   * @param subject expected not null subject
   * @param cert    expected not null user certificate
   */
  static void validateHOKToken(SamlToken token, TokenSpec spec,
                               PrincipalId subject, X509Certificate cert, String systemDomain) {
    validateTokenImpl(token, spec, subject, ConfirmationType.HOLDER_OF_KEY,
        cert, systemDomain);
  }

  private static void validateTokenImpl(SamlToken token, TokenSpec spec,
                                        PrincipalId subject, ConfirmationType confType,
                                        X509Certificate confCert, String systemDomain) {
    Assert.assertNotNull(token);
    Assert.assertNotNull(token.getId());
    Assert.assertNotNull(token.toXml());
    Assert.assertEquals(subject, token.getSubject());
    Assert.assertEquals(confType, token.getConfirmationType());
    Assert.assertEquals(confCert, token.getConfirmationCertificate());
    // lifetime
    Assert.assertEquals(spec.getTokenLifetime(), getTokenLifetime(token));

    // delegation
    final DelegationSpec delegationSpec = spec.getDelegationSpec();
    Assert.assertEquals(delegationSpec != null
        && delegationSpec.isDelegable(), token.isDelegable());
    final List<TokenDelegate> delegationChain = token.getDelegationChain();
    Assert.assertNotNull(delegationChain);
    if (delegationSpec != null && delegationSpec.getDelegateTo() != null) {
      Assert.assertEquals(1, delegationChain.size());
      // tokens can be delegated only to solution users from system identity
      // source
      PrincipalId delegate = new PrincipalId(
          delegationSpec.getDelegateTo(),
          systemDomain
      );
      Assert.assertEquals(delegate, delegationChain.get(0).getSubject());
    } else {
      Assert.assertEquals(0, delegationChain.size());
    }
    // renewable
    Assert.assertEquals(spec.isRenewable(), token.isRenewable());
    // audience restriction
    Assert.assertEquals(spec.getAudienceRestriction(), token.getAudience());
    // advice
    Assert.assertEquals(spec.getAdvice(), token.getAdvice());
  }

  /**
   * Return token lifetime in seconds
   *
   * @param token
   * @return token lifetime in seconds
   * @see TokenSpec#getTokenLifetime()
   */
  static long getTokenLifetime(SamlToken token) {
    long millis = token.getExpirationTime().getTime()
        - token.getStartTime().getTime();

    return TimeUnit.MILLISECONDS.toSeconds(millis);
  }


  /**
   * Creates a TokenSpec for a token with lifetime of
   * {@link #DEFAULT_BEARER_LIFETIME_SECONDS}.
   */
  public static TokenSpec createDefaultTokenSpec() {

    return new TokenSpec.Builder(DEFAULT_BEARER_LIFETIME_SECONDS)
        .createTokenSpec();
  }


//   /**
//    * Creates a TokenSpec having the specified lifetime of TokenLifetimeSpec
//    */
//   public static TokenSpec createDefaultTokenSpec(Period validity) {
//      return new TokenSpec.Builder(createTokenLifetimeSpec(validity))
//         .createTokenSpec();
//   }
//
//   /**
//    * Creates {@link TokenLifetimeSpec} with lifetime
//    * {@link #DEFAULT_BEARER_LIFETIME_MINUTES}
//    */
//   public static TokenLifetimeSpec createTokenLifetimeSpec() {
//      return createTokenLifetimeSpec(DateUtil.createPeriodFromNow(
//         DEFAULT_BEARER_LIFETIME_MINUTES, TimeUnit.minute));
//   }

//   /**
//    * Creates {@link TokenLifetimeSpec}
//    *
//    * @param validity
//    * @return
//    */
//   public static TokenLifetimeSpec createTokenLifetimeSpec(Period validity) {
//
//      return new TokenLifetimeSpec(validity.getStartDate(), validity
//         .getEndDate());
//   }

  public static void validateSAMLToken(TokenSpec spec, SamlToken token) {
    // TODO Auto-generated method stub

  }

}
