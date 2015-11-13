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
package com.vmware.identity.sts.authz.impl;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.easymock.EasyMock;
import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.sts.authz.PrincipalMembership;
import com.vmware.identity.sts.authz.RoleCheck;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.WSTrustRole;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * Insert your comment for RoleCheckImplTest here
 */
public final class RoleCheckImplTest {

   private static final PrincipalId PRINCIPAL_ID = new PrincipalId("user",
      "domain");
   private static final String GROUP_NAME = "actAsUsers";

   @Test
   public void testCheckMemberOK() throws InvalidPrincipalException {

      testCheckMemberInt(true);
   }

   @Test
   public void testCheckMemberNOK() throws InvalidPrincipalException {

      testCheckMemberInt(false);
   }

   @Test
   public void testCheckNotExistingPrincipal() throws InvalidPrincipalException {

      PrincipalMembership principalMembership = EasyMock
         .createMock(PrincipalMembership.class);
      expect(
         principalMembership.isMemberOfSystemGroup(PRINCIPAL_ID, GROUP_NAME))
         .andThrow(new InvalidPrincipalException(PRINCIPAL_ID.toString()));
      replay(principalMembership);

      RoleCheck checker = new RoleCheckImpl(principalMembership, GROUP_NAME);

      try {
         checker.hasRole(PRINCIPAL_ID, WSTrustRole.ActAsUser);
         Assert.fail();
      } catch (InvalidPrincipalException e) {
         // expected
      }

      verify(principalMembership);

   }

   private void testCheckMemberInt(boolean expHasRole)
      throws InvalidPrincipalException {

      PrincipalMembership principalMembership = EasyMock
         .createMock(PrincipalMembership.class);
      expect(
         principalMembership.isMemberOfSystemGroup(PRINCIPAL_ID, GROUP_NAME))
         .andReturn(expHasRole);
      replay(principalMembership);

      RoleCheck checker = new RoleCheckImpl(principalMembership, GROUP_NAME);

      Assert.assertEquals(expHasRole,
         checker.hasRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMembership);
   }

}
