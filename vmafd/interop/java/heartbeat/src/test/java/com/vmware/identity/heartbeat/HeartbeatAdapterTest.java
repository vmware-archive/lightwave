/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.heartbeat;

import junit.framework.Assert;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.heartbeat.PointerRef;

public class HeartbeatAdapterTest {
   private static PointerRef _pHeartbeatHandle;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
   }

   @AfterClass
   public static void tearDownAfterClass() throws Exception {
   }

   @Before
   public void setUp() throws Exception {
   }

   @After
   public void tearDown() throws Exception {
   }

   @Test
   public void testStartStopHeartbeat() {
	   int error = VmAfdHeartbeatAdapter.VmAfdStartHeartBeatW(
                                             "JavaUnitTest",
                                             100,
                                             _pHeartbeatHandle
                                             );
	   Assert.assertEquals(0, error);
	   error = VmAfdHeartbeatAdapter.VmAfdStopHeartbeat(_pHeartbeatHandle);
	   Assert.assertEquals(0, error);
   }
}
