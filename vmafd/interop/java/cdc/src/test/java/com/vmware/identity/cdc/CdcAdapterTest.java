/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.cdc;

import junit.framework.Assert;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class CdcAdapterTest {
   private static PointerRef _serverHandle;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      _serverHandle = new PointerRef();
      int error = CdcAdapter.VmAfdOpenServerW(null, null, null, _serverHandle);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(_serverHandle));
   }

   @AfterClass
   public static void tearDownAfterClass() throws Exception {
      CdcAdapter.VmAfdCloseServer(_serverHandle);
   }

   @Before
   public void setUp() throws Exception {
   }

   @After
   public void tearDown() throws Exception {
   }

   @Test
   public void testEnableDisableClientAffinity() {
	   
	   int error = CdcAdapter.CdcEnableClientAffinity(_serverHandle);
	   Assert.assertEquals(0, error);
	   
	   error = CdcAdapter.CdcDisableClientAffinity(_serverHandle);
	   Assert.assertEquals(0, error);
   }
}