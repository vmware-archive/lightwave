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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.eq;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.PrincipalAttributesExtractor;

public class PrincipalAttributesExtractorTest {

   private static final String TENANT_NAME = "Tenant";

   private CasIdmClient idmClient;
   private PrincipalAttributesExtractor attributesExtractor;

   @Before
   public void init() {
      idmClient = createMock(CasIdmClient.class);
      attributesExtractor = new IdmPrincipalAttributesExtractor(TENANT_NAME,
         idmClient);
   }

   public void testGetAttributes() throws Exception {
      // TODO implement when getAttributes from IDM is working and remove mocked
      // getAttributes
      // attributesExtractor.getAttributes(principalId);
   }

   @Test
   public void testIsActive() throws Exception {
      PrincipalId principalId = new PrincipalId("test", "domain");
      expect(idmClient.isActive(eq(TENANT_NAME), eq(principalId))).andReturn(
         false);
      replay(idmClient);

      boolean active = attributesExtractor.isActive(principalId);

      assertFalse(active);
      verify(idmClient);
   }
}
