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
package com.vmware.identity.wstrust.test.integration.validatetoken;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import com.vmware.directory.rest.common.data.UserDetailsDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;
import com.vmware.identity.rest.idm.data.TokenPolicyDTO;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.test.util.Assert;
import com.vmware.identity.wstrust.test.util.ITokenService;
import com.vmware.identity.wstrust.test.util.MethodEnum;
import com.vmware.identity.wstrust.test.util.TestConfig;
import com.vmware.identity.wstrust.test.util.TokenServiceFactory;
import com.vmware.identity.wstrust.test.util.TokenUtil;
import com.vmware.identity.wstrust.test.util.WSTrustTestBase;
import com.vmware.vim.sso.client.Advice;
import com.vmware.vim.sso.client.SamlToken;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Validate token after the token is expired
 */
@RunWith(Parameterized.class)
public class ValidateTokenAfterExpiryIT extends WSTrustTestBase {
  protected static final Logger logger = LoggerFactory.getLogger(ValidateTokenAfterExpiryIT.class);

  private static String _userName;
  private static String _password;
  private static String _userGroup;

  private final static int TOKEN_VALIDITY_MINS = 1;
  private static long clockTolerance = 0;
  private static long clockTolerance_orig = 0;

  @Rule
  public TestName name = new TestName();

  @Parameterized.Parameters
  public static Collection<TestParam> data() {
    ArrayList<TestParam> testParams = new ArrayList<TestParam>();
    // SYNC
    testParams.add(new TestParam(
                       MethodEnum.SYNC,
                       true,
                       RestrictionType.RESTRICTION_TYPE_NONE,
                       AdviceType.ADVICE_TYPE_NONE
                   )
    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.SYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    // ASYNC
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    // ASYNC_NULLHANDLER
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_NONE,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_NONE
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       true,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
//    testParams.add(new TestParam(
//                       MethodEnum.ASYNC_NULLHANDLER,
//                       false,
//                       RestrictionType.RESTRICTION_TYPE_VALID,
//                       AdviceType.ADVICE_TYPE_VALID
//                   )
//    );
    return testParams;
  }

  @Parameterized.Parameter // first data value (0) is default
  public TestParam param;

  @BeforeClass
  public static void testSetUp() throws Exception {
    WSTrustTestBase.setup();

    String userIdKey = TestConfig.USERID;
    _userName = properties.getUserName(userIdKey);
    _password = properties.getUserPassword(userIdKey);
    _userGroup = properties.getUserGroupName(userIdKey);

    pmUtil.deletePersonUserIfExist(systemTenant, _userName, systemTenant);
    pmUtil.deleteGroupIfExist(systemTenant, _userGroup, systemTenant);

    logger.debug("Deleted user and group");

    UserDetailsDTO userDetailsDTO = UserDetailsDTO.builder()
        .withFirstName(_userName)
        .withEmail("testUser@vmqa.com")
        .build();

    pmUtil.CreatePersonUser(systemTenant, _userName, userDetailsDTO, _password);
    pmUtil.CreateGroup(systemTenant, _userGroup);

    logger.debug("Created user and group");

    clockTolerance_orig = idmClientForSystemTenant.tenant()
        .getConfig(systemTenant)
        .getTokenPolicy()
        .getClockToleranceMillis();

    setClockTolerance(1 * 60 * 1000L);

    clockTolerance = idmClientForSystemTenant.tenant()
        .getConfig(systemTenant)
        .getTokenPolicy()
        .getClockToleranceMillis();
  }

  @Test
  // @TestDescription("Validate Token succedded when user is deleted from database")
  public void test() throws Exception {
    String certAlias = properties.getHokCertAliasKey();
    ITokenService tokenService = TokenServiceFactory
        .getHokTokenServiceInstance(
            stsURL,
            sslTrustedRootsKeystore,
            stsSigningCertificates,
            param.GetMethod(),
            properties.getHokKeystore(),
            certAlias,
            properties.getKeystorePassword(certAlias)
        );
    Set<String> audienceRestriction = null;
    if (param.GetRestrictionType() == RestrictionType.RESTRICTION_TYPE_VALID) {
      audienceRestriction = properties.getAudienceRestricitions("valid.restriction");
    }

    List<Advice> advices = null;
    if (param.GetAdviceType() == AdviceType.ADVICE_TYPE_VALID) {
      advices = properties.getAdviceList("valid.advice");
    }
    TokenSpec spec = createTokenSpec(param.IsRenewable(), advices, audienceRestriction);
    SamlToken token = tokenService.acquireToken(
                          _userName + "@" + properties.getSystemDomain(),
                          _password,
                          spec
                      );
    Assert.assertNotNull(token, "Null token returned");
    Assert.assertTrue(
        tokenService.validateToken(token),
        "Validate Token Failed with validateToken api"
    );
    TokenUtil.validateSAMLToken(spec, token);
    // Sleep until the token validation period + clock tolerance.
    long sleepTime = (TOKEN_VALIDITY_MINS + 1) * 60 * 1000 + clockTolerance;
    logger.info("Sleeping for " + (sleepTime / 60) / 1000 + " minutes for the token to expire");
    Thread.sleep(sleepTime);
    Assert.assertFalse(
        tokenService.validateToken(token),
        "Validate Token succeeded when the time validity is expired"
    );
  }

  @AfterClass
  public static void testCleanUp() throws Exception {
    if (pmUtil != null) {
      pmUtil.deletePersonUserIfExist(systemTenant, _userName, properties.getSystemDomain());
      pmUtil.deleteGroupIfExist(systemTenant, _userGroup, properties.getSystemDomain());
    }
    if (clockTolerance_orig > 0) {
      setClockTolerance(clockTolerance_orig);
    }
    WSTrustTestBase.cleanup();
  }

  private TokenSpec createTokenSpec(boolean isRenewable, List<Advice> advices, Set<String> audienceRestriction) {
    //Token validity is 1 minute.
    TokenSpec.Builder specBuilder = new TokenSpec.Builder(TOKEN_VALIDITY_MINS * 60);
    if (advices != null) {
      specBuilder.advice(advices);
    }
    if (audienceRestriction != null) {
      specBuilder.audienceRestriction(audienceRestriction);
    }
    specBuilder.renewable(isRenewable);
    return specBuilder.createTokenSpec();
  }

  private static void setClockTolerance(Long value) throws Exception {
    TokenPolicyDTO tokenPolicyDTO = TokenPolicyDTO
        .builder()
        .withClockToleranceMillis(value)
        .build();
    TenantConfigurationDTO tenantConfigDTO = TenantConfigurationDTO
        .builder()
        .withTokenPolicy(tokenPolicyDTO)
        .build();
    idmClientForSystemTenant.tenant().updateConfig(systemTenant, tenantConfigDTO);
  }

  private enum RestrictionType {
    RESTRICTION_TYPE_NONE,
    RESTRICTION_TYPE_VALID
  }

  private enum AdviceType {
    ADVICE_TYPE_NONE,
    ADVICE_TYPE_VALID
  }

  private static class TestParam {
    private MethodEnum method;
    private boolean isRenewable;
    private RestrictionType restrictionType;
    private AdviceType adviceType;

    public TestParam(
        MethodEnum methodEnum,
        boolean isRenewable,
        RestrictionType restrictionType,
        AdviceType adviceType) {
      method = methodEnum;
      this.isRenewable = isRenewable;
      this.restrictionType = restrictionType;
      this.adviceType = adviceType;
    }

    public MethodEnum GetMethod() {
      return method;
    }

    public boolean IsRenewable() {
      return isRenewable;
    }

    public RestrictionType GetRestrictionType() {
      return restrictionType;
    }

    public AdviceType GetAdviceType() {
      return adviceType;
    }
  }
}
