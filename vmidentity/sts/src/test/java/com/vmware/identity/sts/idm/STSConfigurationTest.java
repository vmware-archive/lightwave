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
package com.vmware.identity.sts.idm;

import static com.vmware.identity.sts.TestUtil.DEFAULT_CLOCK_TOLERANCE;

import org.junit.Assert;
import org.junit.Test;

/**
 * Insert your comment for STSConfigurationTest here
 */
public final class STSConfigurationTest {

   @Test
   public void testNewInstanceOK() {
      new STSConfiguration(DEFAULT_CLOCK_TOLERANCE);
   }


   @Test
   public void testEquals() {
      STSConfiguration instance1 = new STSConfiguration(DEFAULT_CLOCK_TOLERANCE);
      STSConfiguration instance2 = new STSConfiguration(DEFAULT_CLOCK_TOLERANCE);
      Assert.assertEquals(instance1, instance2);
   }

   @Test
   public void testEqualsFail1() {
      STSConfiguration instance1 = new STSConfiguration(DEFAULT_CLOCK_TOLERANCE);
      STSConfiguration instance2 = new STSConfiguration(DEFAULT_CLOCK_TOLERANCE+1);
      Assert.assertFalse(instance1.equals(instance2));
   }

}
