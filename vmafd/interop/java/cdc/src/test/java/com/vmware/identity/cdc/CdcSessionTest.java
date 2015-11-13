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

public class CdcSessionTest {
   private CdcSession _ipcSession;
   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
   }

   @AfterClass
   public static void tearDownAfterClass() throws Exception {
   }

   @Before
   public void setUp() throws Exception {
	   if (_ipcSession == null){
           _ipcSession = CdcFactory.createCdcSessionViaIPC();
	   }
   }

   @After
   public void tearDown() throws Exception {
	   if (_ipcSession != null){
		   _ipcSession.close();
	   }
	   _ipcSession = null;
   }

   @Test
   public void testGetDCName(){
      CdcDCEntry dcEntry = _ipcSession.getAffinitizedDC("vsphere.local", 0);
      Assert.assertNotNull(dcEntry);
   }

   @Test
   public void testEnableClientAffinity() {
	   _ipcSession.enableClientAffinity();
   }
   
   @Test
   public void testDisableClientAffinity() {
	   _ipcSession.disableClientAffinity();
   }
}
