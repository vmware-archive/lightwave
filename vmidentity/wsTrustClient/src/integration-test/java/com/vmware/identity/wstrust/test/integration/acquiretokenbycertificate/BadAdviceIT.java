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

package com.vmware.identity.wstrust.test.integration.acquiretokenbycertificate;

import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.ExecutionException;

import com.vmware.identity.wstrust.client.SsoRequestException;
import com.vmware.identity.wstrust.test.util.MethodEnum;
import com.vmware.identity.wstrust.test.util.ITokenService;
import com.vmware.identity.wstrust.test.util.TestConfig;
import com.vmware.identity.wstrust.test.util.TokenServiceFactory;

import com.vmware.identity.wstrust.test.util.Assert;
import com.vmware.identity.wstrust.test.util.KeyStoreHelper;

import com.vmware.identity.wstrust.test.util.WSTrustTestBase;
import com.vmware.vim.sso.client.Advice;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.identity.wstrust.client.TokenSpec;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Acquire token by certificate for a solution user with incorrect advice
 */

@RunWith(Parameterized.class)
public class BadAdviceIT extends WSTrustTestBase {
  protected static final Logger logger = LoggerFactory.getLogger(BadAdviceIT.class);

  static String _solutionUserName = "notcnUser";
  static String _solutionCertificateStore;
  static String _solutionCertificateAlias;
  static char[] _solutionCertificateStorePassword;
  static String _userGroup;

  @Rule
  public TestName name = new TestName();

  @Parameterized.Parameters
  public static Collection<TestParam> data() {
    ArrayList<TestParam> testParams = new ArrayList<TestParam>();
    testParams.add(new TestParam(MethodEnum.SYNC));
    testParams.add(new TestParam(MethodEnum.ASYNC));
    testParams.add(new TestParam(MethodEnum.ASYNC_NULLHANDLER));

    return testParams;
  }

  @Parameterized.Parameter // first data value (0) is default
  public TestParam param;

  @BeforeClass
  public static void setup() throws Exception {

    WSTrustTestBase.setup();

    /*
     * Get Person Details from config.properties
     */
    String solutionIdKey = TestConfig.SOLUTIONNAME;
    _solutionCertificateStore = properties.getUserCertificatePath(solutionIdKey);
    _solutionCertificateAlias = properties.getUserCertificateAlias(solutionIdKey);
    _solutionCertificateStorePassword = properties.getKeystorePassword(_solutionCertificateAlias);
    _userGroup = properties.getUserGroupName(solutionIdKey);

    pmUtil.deleteSolutionUserIfExist(systemTenant, _solutionUserName);
    pmUtil.deleteGroupIfExist(systemTenant, _userGroup, systemTenant);

    Certificate certificate = new KeyStoreHelper(_solutionCertificateStore).getCertificate(_solutionCertificateAlias);

    pmUtil.CreateSolutionUser(_solutionUserName, systemTenant, false, certificate);
    pmUtil.CreateGroup(systemTenant, _userGroup);
  }

  @Test
  // @DisplayName("Retrieve a SAML token using a newly created solution user")
  public void testBadAdvice() throws Exception {
    ITokenService tokenService = TokenServiceFactory.getHokTokenServiceInstance(
                        stsURL,
                        sslTrustedRootsKeystore,
                        stsSigningCertificates,
                        param.getMethod(),
                        _solutionCertificateStore,
                        _solutionCertificateAlias,
                        _solutionCertificateStorePassword
                    );

    List<Advice> advices = properties.getAdviceList("bad.advice");

    TokenSpec spec = createTokenSpec(advices);

    Certificate certificate = new KeyStoreHelper(_solutionCertificateStore).getCertificate(_solutionCertificateAlias);

    SamlToken token = null;
    try {
      token = tokenService.acquireTokenByCertificate(certificate, spec);
    } catch (SsoRequestException e) {
      logger.debug(e.getMessage());
    } catch (ExecutionException e) {
      logger.debug(e.getMessage());
      Throwable t = e.getCause();
      if (!(t instanceof SsoRequestException)) {
        throw new RuntimeException(t);
      }
    }

    Assert.assertNull(token, "Non-null token returned");
  }

  @AfterClass
  public static void cleanup() throws Exception {
    if (pmUtil != null) {
      pmUtil.deleteSolutionUserIfExist(systemTenant, _solutionUserName);
      pmUtil.deleteGroupIfExist(systemTenant, _userGroup, systemTenant);
    }
    WSTrustTestBase.cleanup();
  }

  private TokenSpec createTokenSpec(List<Advice> advices) {
    TokenSpec.Builder specBuilder = new TokenSpec.Builder(600);
    if (advices != null && !advices.isEmpty()) {
      specBuilder.advice(advices);
    }
    return specBuilder.createTokenSpec();
  }

  private static class TestParam{
    private MethodEnum method;

    public TestParam(MethodEnum methodEnum) {
      method = methodEnum;
    }

    public MethodEnum getMethod() {
      return method;
    }
  }
}
