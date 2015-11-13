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

import java.util.HashMap;
import java.util.Map;

import com.vmware.identity.authz.RoleAdmin;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.GroupDetails;
import com.vmware.vim.sso.admin.PrincipalManagement;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * RoleAdmin implementation which also creates role groups if needed.
 */
public final class RoleAdminCreatingRoleGroups<R extends Enum<R>> implements
   RoleAdmin<R> {

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(RoleAdminCreatingRoleGroups.class);

   private final PrincipalManagement _pm;
   private final Map<R, SystemGroup> _roleGroups;
   private final RoleAdminBase<R> _delegate;

   public static final class SystemGroup {
      private final String name;
      private final GroupDetails details;

      public SystemGroup(String groupName, GroupDetails groupDetails) {
         name = groupName;
         details = groupDetails;
      }
   }

   public RoleAdminCreatingRoleGroups(PrincipalManagement pm,
      Map<R, SystemGroup> roleGroups) {
      assert pm != null;
      assert roleGroups != null;
      assert !roleGroups.isEmpty();

      _pm = pm;
      _roleGroups = roleGroups;

      Map<R, String> roleKeyGroupValue = new HashMap<R, String>(
         roleGroups.size());
      for (Map.Entry<R, SystemGroup> entry : roleGroups.entrySet()) {
         R role = entry.getKey();
         SystemGroup roleGroup = entry.getValue();
         roleKeyGroupValue.put(role, roleGroup.name);
      }
      _delegate = new RoleAdminBase<R>(pm, roleKeyGroupValue, false);
   }

   @Override
   public boolean grantRole(PrincipalId userId, R role)
      throws InvalidPrincipalException {

      boolean result;
      try {
         result = _delegate.grantRole(userId, role);

      } catch (InvalidPrincipalException e) {
         final SystemGroup group = _roleGroups.get(role);
         log.trace(
            "User {} not added to local group {} because of {}. Going to try creation of that group.",
            new Object[] { userId, group.name, e });

         boolean groupCreated = createGroup(group.name, group.details);

         if (groupCreated) {
             log.trace("group {} successfully created.", group.name);
         } else {
             log.trace("group {} already exists", group.name);
         }

         try {
             result = _delegate.grantRole(userId, role);
          } catch (InvalidPrincipalException ipe) {
              log.error(
                      "The group {} exists so it turns out that the user {} is invalid",
                      group.name, userId);
                   throw ipe;
          }
      }
      log.debug("Granted {} role to {} with success {}", new Object[] { role,
         userId, result });
      return result;
   }

   @Override
   public boolean revokeRole(PrincipalId userId, R role)
      throws InvalidPrincipalException {

      boolean result;
      try {
         result = _delegate.revokeRole(userId, role);

      } catch (InvalidPrincipalException e) {

         final SystemGroup group = _roleGroups.get(role);
         log.trace(
            "User {} was not removed from local group {} because of {}. Going to detect whether the group exist.",
            new Object[] { userId, group.name, e });

         if (createGroup(group.name, group.details)) {
            log.trace("The group {} does not exist");
            result = false;
         } else {
            log.trace(
               "The group {} exists so it turns out that the user {} is invalid",
               group.name, userId);

            throw e;
         }
      }
      log.debug("Revoked {} role from {} with success {}", new Object[] { role,
         userId, result });
      return result;
   }

   private boolean createGroup(final String name, final GroupDetails details) {

      boolean result = false;
      try {
         new AuthorizedPrincipalTask<Void>() {

            @Override
            protected Void executeInternal() throws InvalidPrincipalException,
               NoPermissionException, NotAuthenticatedException {

               _pm.createLocalGroup(name, details);
               return null;
            }

         }.execute();
         result = true;

      } catch (InvalidPrincipalException e) {
         assert result == false;
         // such group already exists
      }

      return result;
   }
}
