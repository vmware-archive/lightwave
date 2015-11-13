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
package com.vmware.identity.authz.impl;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import org.easymock.EasyMock;
import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.authz.RoleAdmin;
import com.vmware.identity.authz.RoleAdminBuilder;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.GroupDetails;
import com.vmware.vim.sso.admin.PrincipalManagement;
import com.vmware.vim.sso.admin.RoleManagement.WSTrustRole;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Insert your comment for RoleAdminImplTest here
 */
public final class RoleAdminImplTest {

   private static final PrincipalId PRINCIPAL_ID = new PrincipalId("user",
      "domain");
   private static final String NOT_EXISTING_GROUP = "NotExistingGroup";
   private static final String EXISTING_GROUP = "ExistingGroup";
   private static final String SYSTEM_DOMAIN = "SYSTEM_DOMAIN";

   @Test
   public void testGrantNoGroup() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(
         principalMgmt.addUserToLocalGroup(PRINCIPAL_ID, NOT_EXISTING_GROUP))
         .andThrow(new InvalidPrincipalException(NOT_EXISTING_GROUP)).times(1);
      expect(
         principalMgmt.createLocalGroup(EasyMock.eq(NOT_EXISTING_GROUP),
            EasyMock.anyObject(GroupDetails.class))).andReturn(
         new PrincipalId(NOT_EXISTING_GROUP, SYSTEM_DOMAIN)).times(1);
      expect(
         principalMgmt.addUserToLocalGroup(PRINCIPAL_ID, NOT_EXISTING_GROUP))
         .andReturn(true).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, NOT_EXISTING_GROUP);

      Assert.assertTrue(admin.grantRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMgmt);
   }

   @Test
   public void testGrantNoUser() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(principalMgmt.addUserToLocalGroup(PRINCIPAL_ID, EXISTING_GROUP))
         .andThrow(new InvalidPrincipalException(PRINCIPAL_ID.toString()))
         .times(2);
      expect(
         principalMgmt.createLocalGroup(EasyMock.eq(EXISTING_GROUP),
            EasyMock.anyObject(GroupDetails.class))).andThrow(
         new InvalidPrincipalException(EXISTING_GROUP)).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, EXISTING_GROUP);

      try {
         admin.grantRole(PRINCIPAL_ID, WSTrustRole.ActAsUser);
         Assert.fail();
      } catch (InvalidPrincipalException e) {
         // expected
      }

      verify(principalMgmt);
   }

   @Test
   public void testGrantExistingGroup() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(principalMgmt.addUserToLocalGroup(PRINCIPAL_ID, EXISTING_GROUP))
         .andReturn(true).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, EXISTING_GROUP);

      Assert.assertTrue(admin.grantRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMgmt);
   }

   @Test
   public void testGrantTwice() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(principalMgmt.addUserToLocalGroup(PRINCIPAL_ID, EXISTING_GROUP))
         .andReturn(false).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, EXISTING_GROUP);

      Assert.assertFalse(admin.grantRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMgmt);
   }

   @Test
   public void testRevokeNoGroup() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(
         principalMgmt.removeFromLocalGroup(PRINCIPAL_ID, NOT_EXISTING_GROUP))
         .andThrow(new InvalidPrincipalException(NOT_EXISTING_GROUP)).times(1);
      expect(
         principalMgmt.createLocalGroup(EasyMock.eq(NOT_EXISTING_GROUP),
            EasyMock.anyObject(GroupDetails.class))).andReturn(
         new PrincipalId(NOT_EXISTING_GROUP, SYSTEM_DOMAIN)).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, NOT_EXISTING_GROUP);

      Assert.assertFalse(admin.revokeRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMgmt);
   }

   @Test
   public void testRevokeNoUser() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(principalMgmt.removeFromLocalGroup(PRINCIPAL_ID, EXISTING_GROUP))
         .andThrow(new InvalidPrincipalException(PRINCIPAL_ID.toString()))
         .times(1);
      expect(
         principalMgmt.createLocalGroup(EasyMock.eq(EXISTING_GROUP),
            EasyMock.anyObject(GroupDetails.class))).andThrow(
         new InvalidPrincipalException(EXISTING_GROUP)).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, EXISTING_GROUP);

      try {
         admin.revokeRole(PRINCIPAL_ID, WSTrustRole.ActAsUser);
         Assert.fail();
      } catch (InvalidPrincipalException e) {
         // expected
      }

      verify(principalMgmt);
   }

   @Test
   public void testRevokeExistingGroup() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(principalMgmt.removeFromLocalGroup(PRINCIPAL_ID, EXISTING_GROUP))
         .andReturn(true).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, EXISTING_GROUP);

      Assert.assertTrue(admin.revokeRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMgmt);
   }

   @Test
   public void testRevokeTwice() throws NoPermissionException,
      NotAuthenticatedException, InvalidPrincipalException {

      PrincipalManagement principalMgmt = EasyMock
         .createMock(PrincipalManagement.class);
      expect(principalMgmt.removeFromLocalGroup(PRINCIPAL_ID, EXISTING_GROUP))
         .andReturn(false).times(1);
      replay(principalMgmt);

      RoleAdmin<WSTrustRole> admin = createRoleAdmin(principalMgmt, EXISTING_GROUP);

      Assert.assertFalse(admin.revokeRole(PRINCIPAL_ID, WSTrustRole.ActAsUser));

      verify(principalMgmt);
   }

   private RoleAdmin<WSTrustRole> createRoleAdmin(PrincipalManagement pm, String groupName) {
      return new RoleAdminBuilder<WSTrustRole>(pm, WSTrustRole.ActAsUser,
         groupName, new GroupDetails(groupName + " description"))
         .createRoleAdminCreatingRoleGroups();
   }
}
