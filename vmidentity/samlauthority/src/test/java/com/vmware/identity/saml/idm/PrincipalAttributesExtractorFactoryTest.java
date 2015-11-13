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
package com.vmware.identity.saml.idm;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertNotNull;

import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;

public class PrincipalAttributesExtractorFactoryTest {

   private static final String HOST_NAME = "IDM";
   private static final String TENANT_NAME = "Tenant";

   private PrincipalAttributesExtractorFactory atttributesExtractorFactory;

   @Before
   public void init() {
      atttributesExtractorFactory = new IdmPrincipalAttributesExtractorFactory(
         HOST_NAME);
   }

   @Test
   public void testGetExtractor() throws Exception {
      PrincipalAttributesExtractor principalAttributesExtractor = atttributesExtractorFactory
         .getPrincipalAttributesExtractor(TENANT_NAME);
      assertNotNull(principalAttributesExtractor);
      assertTrue(principalAttributesExtractor instanceof IdmPrincipalAttributesExtractor);
   }

}
