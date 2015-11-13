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

import java.util.Collections;
import java.util.Map;

import com.vmware.identity.authz.RoleAdmin;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.PrincipalManagement;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Base class for administering roles.
 */
public final class RoleAdminBase<R extends Enum<R>> implements RoleAdmin<R> {
   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(RoleAdminBase.class);

   private final PrincipalManagement _pm;
   private final Map<R, String> _roleKeyGroupValue;
   private final boolean _exclusiveRoles;

   /**
    * @param pm
    *           principal management
    * @param roleKeyGroupValue
    *           role name to group name; group name could be {@code null} in
    *           which case we assume role is granted by default
    * @param exclusiveRoles
    *           whether user could have one single role at a time; that doesn't
    *           apply to roles granted by default, i.e. which group is
    *           {@code null}
    */
   public RoleAdminBase(PrincipalManagement pm,
      Map<R, String> roleKeyGroupValue, boolean exclusiveRoles) {

      assert pm != null;
      assert roleKeyGroupValue != null;
      assert !roleKeyGroupValue.isEmpty();

      _pm = pm;
      _roleKeyGroupValue = Collections.unmodifiableMap(roleKeyGroupValue);
      _exclusiveRoles = exclusiveRoles;
   }

   /**
    * Grant an existing user with the specified role.
    *
    * @param userId
    *           person or solution user
    * @param role
    *           role to be granted
    * @return {@code true} when the user role has changed, otherwise -
    *         {@code false}
    *
    * @throws InvalidPrincipalException
    *            when there is no person or solution user with the given id
    */
   @Override
   public boolean grantRole(final PrincipalId userId, final R role)
      throws InvalidPrincipalException {

      validateRole(role);

      return new AuthorizedPrincipalTask<Boolean>() {

         @Override
         protected Boolean executeInternal() throws InvalidPrincipalException,
            NoPermissionException, NotAuthenticatedException {

            assert _roleKeyGroupValue.containsKey(role);
            final String roleGroup = _roleKeyGroupValue.get(role);
            final boolean result = roleGroup != null ? _pm.addUserToLocalGroup(
               userId, roleGroup) : false;

            if (_exclusiveRoles) {
               retainRoleExclusively(userId, role);
            }
            return result;
         }
      }.execute();
   }

   /**
    * Revoke the specified role from an existing user.
    *
    * @param userId
    *           person or solution user which role to revoke
    * @param role
    *           role to be revoked
    * @return
    *
    * @throws InvalidPrincipalException
    *            when there is no person or solution user with the given id
    */
   @Override
   public boolean revokeRole(final PrincipalId userId, final R role)
      throws InvalidPrincipalException {

      validateRole(role);

      return new AuthorizedPrincipalTask<Boolean>() {

         @Override
         protected Boolean executeInternal() throws InvalidPrincipalException,
            NoPermissionException, NotAuthenticatedException {

            assert _roleKeyGroupValue.containsKey(role);
            final String roleGroup = _roleKeyGroupValue.get(role);
            final boolean result = roleGroup != null ? _pm
               .removeFromLocalGroup(userId, roleGroup) : false;

            if (_exclusiveRoles) {
               retainRoleExclusively(userId, role);
            }
            return result;
         }

      }.execute();
   }

   //
   // Private methods
   //

   /**
    * Revokes all the assigned roles (except the given one) from the specified
    * user.<br/>
    * This method effectively removes the user from all the role groups with the
    * exception of group assigned to the given role.
    *
    * @param userId
    *           user which roles to revoke
    * @param role
    *           all roles except this one will be revoked from user; this role
    *           will remain unchanged
    *
    * @throws NoPermissionException
    * @throws NotAuthenticatedException
    * @throws InvalidPrincipalException
    */
   private void retainRoleExclusively(PrincipalId userId, R role)
      throws NoPermissionException, NotAuthenticatedException,
      InvalidPrincipalException {

      for (Map.Entry<R, String> entry : _roleKeyGroupValue.entrySet()) {
         if (role.equals(entry.getKey())) {
            continue;
         }
         String roleGroup = entry.getValue();
         if (roleGroup != null) {
            _pm.removeFromLocalGroup(userId, roleGroup);
         }
      }
   }

   private void validateRole(R role) {
      assert role != null;

      if (!_roleKeyGroupValue.containsKey(role)) {
         String msg = "Invalid role " + role;
         log.error(msg);
         throw new IllegalArgumentException(msg);
      }
   }

}
