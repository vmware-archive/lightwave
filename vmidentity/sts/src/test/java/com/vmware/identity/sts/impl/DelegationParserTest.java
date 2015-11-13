/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.sts.impl;

import static org.junit.Assert.fail;

import java.security.cert.X509Certificate;
import java.util.Date;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.sts.CertificateUtil;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.idm.PrincipalDiscovery;

public class DelegationParserTest {

   private static final int AUTHN_TOKEN_DEL_COUNT = 10;
   private static final String SOLUTION_USER_NAME = "Solution";
   private static final String SOLUTION_DOMAIN = "System-Domain";
   private static final X509Certificate SOLUTION_CERT = new CertificateUtil(
      CertificateUtil.SOLUTION_STORE_JKS, CertificateUtil.PASSWORD)
      .loadCert(CertificateUtil.SOLUTION_CERT_ALIAS);
   private static final PrincipalDiscovery principalDisc = new PrincipalDiscoveryMock(
      new PrincipalId(SOLUTION_USER_NAME, SOLUTION_DOMAIN), SOLUTION_CERT);

   private final DelegationParser del = new DelegationParser(principalDisc);

   @Test
   public void testNoDelegateInReq() {
      Assert.assertNull(del.extractDelegate(TestSetup.createRST(null, null)));
   }

   @Test
   public void testDelegateInReqCannotBeFoundByNameAndThrowsSystemException() {
      boolean findUserByNameThrowsException = true;
      DelegationParser del = new DelegationParser(new PrincipalDiscoveryMock(
         new PrincipalId(SOLUTION_USER_NAME, SOLUTION_DOMAIN), SOLUTION_CERT,
         findUserByNameThrowsException));
      try {
         del.extractDelegate(TestSetup.createRST(SOLUTION_USER_NAME, null));
         fail();
      } catch (RequestFailedException e) {
         // expected
      }
   }

   @Test
   public void testParseDelegate() {
      Assert.assertEquals(
         principalDisc.findSolutionUserByName(SOLUTION_USER_NAME),
         del.extractDelegate(TestSetup.createRST(SOLUTION_USER_NAME, null)));
   }

   @Test
   public void testNoDelegationChain() {
      testParseDelegationChainInt(0, new Date());
   }

   @Test
   public void testParseDelegationChain() {
      testParseDelegationChainInt(2, new Date());
      testParseDelegationChainInt(7, new Date());
   }

   private void testParseDelegationChainInt(int chainSize,
      Date delegatedTokenExpires) {
      final int remainingDelegates = AUTHN_TOKEN_DEL_COUNT - chainSize;

      DelegationHistory history = del.extractDelegationHistory(
         createToken(chainSize, delegatedTokenExpires),
         TestSetup.createAssertion(remainingDelegates));
      Assert.assertNotNull(history);
      Assert
         .assertEquals(remainingDelegates, history.getRemainingDelegations());
      Assert.assertEquals(chainSize, history.getCurrentDelegateList().size());
      Assert.assertEquals(delegatedTokenExpires,
         history.getDelegatedTokenExpires());
   }

   private ServerValidatableSamlToken createToken(int tokenDelChainSize,
      Date delegatedTokenExpires) {

      return new SamlTokenMock(SOLUTION_USER_NAME, SOLUTION_DOMAIN,
         SOLUTION_CERT, delegatedTokenExpires, SOLUTION_USER_NAME,
         SOLUTION_DOMAIN, delegatedTokenExpires, tokenDelChainSize);
   }
}
