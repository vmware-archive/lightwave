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
package com.vmware.identity.sts.ws;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import com.vmware.identity.sts.NoSuchIdPException;

public class TenantExtractionTest {

   @Test
   public void testTenantExtractionEasy() throws Exception {
      assertEquals("tenant", TenantExtractor.extractTenantName("/tenant"));
      assertEquals("tenant", TenantExtractor.extractTenantName("/tenant/"));
      assertEquals("tenant", TenantExtractor.extractTenantName("/tenant/asd"));
      assertEquals("tenant", TenantExtractor.extractTenantName("/tenant/asd/"));
      assertEquals("tenant",
         TenantExtractor.extractTenantName("/tenant/asd/sdf/sdf/"));
      assertEquals("t", TenantExtractor.extractTenantName("/t"));
      assertEquals("a", TenantExtractor.extractTenantName("/a/sd"));
   }

   @Test
   public void testTenantExtraction_TenantNameWithDifferentSymbols()
      throws Exception {
      assertEquals("tenant.tenant",
         TenantExtractor.extractTenantName("/tenant.tenant/asd/"));
      assertEquals("a.tenant.s",
         TenantExtractor.extractTenantName("/a.tenant.s/asd/"));
      assertEquals("te.nant",
         TenantExtractor.extractTenantName("/te.nant/as.sa/d"));
      assertEquals(".nant", TenantExtractor.extractTenantName("/.nant/as.sa/d"));
      assertEquals("tf01a3sf2s1sf08*a&sffg$",
         TenantExtractor.extractTenantName("/tf01a3sf2s1sf08*a&sffg$/"));
   }

   @Test
   public void testTenantExtraction_NullPathInfo() throws Exception {
       assertEquals("vsphere.local",
           TenantExtractor.extractTenantName(null));
   }

   @Test
   public void testTenantExtraction_EmptyPathInfo() throws Exception {
       assertEquals("vsphere.local",
               TenantExtractor.extractTenantName("/"));
   }

}
