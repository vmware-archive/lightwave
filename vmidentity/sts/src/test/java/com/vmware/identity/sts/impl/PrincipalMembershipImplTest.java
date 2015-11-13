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

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.HashSet;
import java.util.Set;

import org.easymock.EasyMock;
import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.sts.authz.PrincipalMembership;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * Insert your comment for PrincipalMembershipTestImpl here
 */
public final class PrincipalMembershipImplTest {

   private static final PrincipalId PRINCIPAL_ID = new PrincipalId("user",
      "domain");
   private static final com.vmware.identity.idm.PrincipalId PRINCIPAL_ID_IDM = new com.vmware.identity.idm.PrincipalId(
      PRINCIPAL_ID.getName(), PRINCIPAL_ID.getDomain());
   private static final String GROUP_NAME = "actAsUsers";

   @Test
   public void testOK() throws InvalidPrincipalException {

      final Set<String> groupNames = new HashSet<String>();
      groupNames.add("xccvevevev242r");
      groupNames.add(GROUP_NAME + GROUP_NAME);
      groupNames.add(GROUP_NAME);

      testInt(groupNames, GROUP_NAME, true);
   }

   @Test
   public void testNOK() throws InvalidPrincipalException {

      final Set<String> groupNames = new HashSet<String>();
      groupNames.add("xccvevevev242r");
      groupNames.add(GROUP_NAME + GROUP_NAME);
      groupNames.add(GROUP_NAME + "_");

      testInt(groupNames, GROUP_NAME, false);
   }

   @Test
   public void testNoSuchPrincipal() {

      final PrincipalDiscovery principalDiscovery = EasyMock
         .createMock(PrincipalDiscovery.class);
      expect(principalDiscovery.isMemberOfSystemGroup(PRINCIPAL_ID_IDM, GROUP_NAME))
         .andThrow(
            new com.vmware.identity.sts.idm.InvalidPrincipalException(
               PRINCIPAL_ID_IDM.toString()));
      replay(principalDiscovery);

      PrincipalMembership membership = new PrincipalMembershipImpl(
         principalDiscovery);

      try {
         membership.isMemberOfSystemGroup(PRINCIPAL_ID, GROUP_NAME);
         Assert.fail();
      } catch (InvalidPrincipalException e) {
         // expected
      }

      verify(principalDiscovery);
   }

   private void testInt(Set<String> userGroups, String targetGroup,
      boolean expResult) throws InvalidPrincipalException {

      final PrincipalDiscovery principalDiscovery = EasyMock
         .createMock(PrincipalDiscovery.class);
      expect(principalDiscovery.isMemberOfSystemGroup(PRINCIPAL_ID_IDM, targetGroup))
         .andReturn(userGroups.contains(targetGroup));
      replay(principalDiscovery);

      final PrincipalMembership membership = new PrincipalMembershipImpl(
         principalDiscovery);

      Assert.assertEquals(expResult,
         membership.isMemberOfSystemGroup(PRINCIPAL_ID, targetGroup));

      verify(principalDiscovery);
   }

}
