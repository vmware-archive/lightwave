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
package com.vmware.identity.wstrust.test.integration.acquiretokenbycertificate;

import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.concurrent.ExecutionException;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.InvalidTokenRequestException;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.test.util.Assert;
import com.vmware.identity.wstrust.test.util.ITokenService;
import com.vmware.identity.wstrust.test.util.TestConfig;
import com.vmware.identity.wstrust.test.util.KeyStoreHelper;
import com.vmware.identity.wstrust.test.util.MethodEnum;
import com.vmware.identity.wstrust.test.util.TokenServiceFactory;
import com.vmware.identity.wstrust.test.util.WSTrustTestBase;
import com.vmware.vim.sso.client.SamlToken;

/**
 * Acquire token by certificate for a solution user
 * with delegateTo as a non-solution user
 */

@RunWith(Parameterized.class)
public class BadDelegateIT extends WSTrustTestBase {
  protected static final Logger logger = LoggerFactory.getLogger(BadDelegateIT.class);

  private static String _solutionUserName = "notcnUser";
  private static String _solutionCertificateStore;
  private static String _solutionCertificateAlias;
  private static char[] _solutionCertificateStorePassword;

  @Rule
  public TestName name = new TestName();

  @Parameterized.Parameters
  public static Collection<TestParam> data() {
    TestConfig properties1 = new TestConfig();
    ArrayList<TestParam> testParams = new ArrayList<TestParam>();
    testParams.add(new TestParam(MethodEnum.SYNC, properties1.getProperty("sso.admin.user.name"),
                                 TestConfig.SOLUTIONNAME, true));
    testParams.add(new TestParam(MethodEnum.SYNC,
                                 String.format("%s@%s", properties1.getProperty("sso.admin.user.name"),
                                               properties1.getProperty("sso.admin.user.domain")),
                                 TestConfig.SOLUTIONNAME, true));
    // testParams.add(new TestParam(MethodEnum.SYNC, "ad1User.user.name", TestConfig.SOLUTIONNAME, true));
    // testParams.add(new TestParam(MethodEnum.SYNC, "ad2User.user.name", TestConfig.SOLUTIONNAME, true));
    // testParams.add(new TestParam(MethodEnum.SYNC, "ldapUser.user.name", TestConfig.SOLUTIONNAME, true));
    testParams.add(new TestParam(MethodEnum.ASYNC, properties1.getProperty("sso.admin.user.name"), TestConfig
        .SOLUTIONNAME, true));
    // testParams.add(new TestParam(MethodEnum.ASYNC, "ad1User.user.name", TestConfig.SOLUTIONNAME, true));
    // testParams.add(new TestParam(MethodEnum.ASYNC, "ad2User.user.name", TestConfig.SOLUTIONNAME, true));
    // testParams.add(new TestParam(MethodEnum.ASYNC, "ldapUser.user.name", TestConfig.SOLUTIONNAME, true));
    testParams.add(
        new TestParam(MethodEnum.ASYNC_NULLHANDLER,
                      properties1.getProperty("sso.admin.user.name"),
                      TestConfig.SOLUTIONNAME,
                      true
        )
    );
    //    testParams.add(
    //        new TestParam(MethodEnum.ASYNC_NULLHANDLER,
    //                      "ad1User.user.name",
    //                      TestConfig.SOLUTIONNAME,
    //                      true
    //        )
    //    );
    //    testParams.add(
    //        new TestParam(MethodEnum.ASYNC_NULLHANDLER,
    //                      "ad2User.user.name",
    //                      TestConfig.SOLUTIONNAME,
    //                      true
    //        )
    //    );
    //    testParams.add(
    //        new TestParam(MethodEnum.ASYNC_NULLHANDLER,
    //                      "ldapUser.user.name",
    //                      TestConfig.SOLUTIONNAME,
    //                      true
    //        )
    //    );

    return testParams;
  }

  @Parameterized.Parameter // first data value (0) is default
  public TestParam param;

  @BeforeClass
  public static void testSetUp() throws Exception {

    WSTrustTestBase.setup();

    /*
     * Get Person Details from config.properties
     */
    String solutionIdKey = TestConfig.SOLUTIONNAME;
    _solutionCertificateStore = properties.getUserCertificatePath(solutionIdKey);
    _solutionCertificateAlias = properties.getUserCertificateAlias(solutionIdKey);
    _solutionCertificateStorePassword = properties.getKeystorePassword(_solutionCertificateAlias);

    pmUtil.deleteSolutionUserIfExist(systemTenant, _solutionUserName);

    logger.debug("Deleted test solution user and group");

    Certificate certificate = new KeyStoreHelper(_solutionCertificateStore).getCertificate(_solutionCertificateAlias);

    pmUtil.CreateSolutionUser(_solutionUserName, systemTenant, false, certificate);

    logger.debug("Created test user and group");
  }

  @Test
  public void testBadDelegate() throws Exception {
    ITokenService tokenService =  TokenServiceFactory.getHokTokenServiceInstance(
                                      stsURL,
                                      sslTrustedRootsKeystore,
                                      stsSigningCertificates,
                                      param.GetMethod(),
                                      _solutionCertificateStore,
                                      _solutionCertificateAlias,
                                      _solutionCertificateStorePassword
                                  );
    Certificate certificate = new KeyStoreHelper(_solutionCertificateStore).getCertificate(_solutionCertificateAlias);
    TokenSpec spec = createTokenSpec(param.IsDelegatable(), param.GetDelegateTo());
    SamlToken token = null;
    try {
      token = tokenService.acquireTokenByCertificate(certificate, spec);
    } catch (InvalidTokenRequestException e) {
      logger.debug(e.getMessage());
    } catch (ExecutionException e) {
      logger.debug(e.getMessage());
      Throwable t = e.getCause();
      if (!(t instanceof InvalidTokenRequestException)) {
        throw new RuntimeException(t);
      }
    }
    Assert.assertNull(token, "Non-null token returned");
  }

  @AfterClass
  public static void testCleanUp() throws Exception {
    if (pmUtil != null) {
      pmUtil.deleteSolutionUserIfExist(systemTenant, _solutionUserName);
    }
    WSTrustTestBase.cleanup();
  }

  private TokenSpec createTokenSpec(Boolean isDelegatable, String delegateTo) {
    TokenSpec.Builder specBuilder = new TokenSpec.Builder(600);
    specBuilder.delegationSpec(
                  new TokenSpec.DelegationSpec(
                      isDelegatable,
                      delegateTo
                  )
    );
    return specBuilder.createTokenSpec();
  }

  private static class TestParam {
    private MethodEnum method;
    private String delegateTo;
    private String userId;
    private Boolean isDelegatable;

    public TestParam(MethodEnum methodEnum, String delegateTo, String userId, Boolean isDelegatable) {
      method = methodEnum;
      this.delegateTo = delegateTo;
      this.userId = userId;
      this.isDelegatable = isDelegatable;
    }

    public MethodEnum GetMethod() {
      return method;
    }

    public String GetDelegateTo() { return delegateTo; }

    public String GetUserId() { return userId; }

    public Boolean IsDelegatable() { return isDelegatable; }
  }
}
