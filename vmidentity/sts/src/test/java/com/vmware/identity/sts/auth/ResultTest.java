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
package com.vmware.identity.sts.auth;

import java.util.Date;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.auth.Result.AuthnMethod;

/**
 * Insert your comment for ResultTest here
 */
public final class ResultTest {

   @Test
   public void testCompletedResult() {
      final String name = "user";
      final String domain = "acme.com";
      final Date authInstant = new Date();
      final AuthnMethod authMethod = Result.AuthnMethod.DIG_SIG;
      final Result result = new Result(new PrincipalId(name, domain),
         authInstant, authMethod);
      Assert.assertTrue(result.completed());
      Assert.assertEquals(new PrincipalId(name, domain),
         result.getPrincipalId());
      Assert.assertEquals(authInstant, result.getAuthnInstant());
      Assert.assertEquals(authMethod, result.getAuthnMethod());
      Assert.assertNull(result.getServerLeg());
   }

   @Test
   public void testInCompleteResult() {
      final byte[] serverLeg = new byte[] { 23, 43, 7, -6 };
      final Result result = new Result(serverLeg);
      Assert.assertFalse(result.completed());
      Assert.assertNull(result.getPrincipalId());
      Assert.assertNull(result.getAuthnInstant());
      Assert.assertNull(result.getAuthnMethod());
   }

   @Test
   public void testSecurIDResult() {
      String sessionID = "context";
      final Date authInstant = new Date();
      final AuthnMethod authMethod = Result.AuthnMethod.TIMESYNCTOKEN;
      final Result result = new Result(sessionID, authInstant);
      Assert.assertFalse(result.completed());
      Assert.assertNull(result.getPrincipalId());
      Assert.assertNull(result.getServerLeg());
      Assert.assertEquals(authInstant, result.getAuthnInstant());
      Assert.assertEquals(authMethod, result.getAuthnMethod());
      Assert.assertEquals(sessionID, result.getSessionID());
   }
}
