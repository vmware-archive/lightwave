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

import com.vmware.identity.authz.RoleCheck;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.Group;
import com.vmware.vim.sso.admin.PrincipalDiscovery;
import com.vmware.vim.sso.admin.SolutionUser;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;
import com.vmware.vim.sso.admin.exception.SystemException;
import com.vmware.vim.sso.client.SamlToken;

/**
 * Base class for checking roles.
 */
public final class RoleCheckBase<R extends Enum<R>> implements RoleCheck<R> {

   private IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(RoleCheckBase.class);

   private final Map<PrincipalId, R> _groupKeyRoleValue;
   private final PrincipalDiscovery _pd;

   public RoleCheckBase(Map<PrincipalId, R> groupKeyRoleValue,
      PrincipalDiscovery pd) {

      assert groupKeyRoleValue != null;
      assert !groupKeyRoleValue.isEmpty();
      assert pd != null;

      _groupKeyRoleValue = Collections.unmodifiableMap(groupKeyRoleValue);
      _pd = pd;
   }

   @Override
   public R getRole(final PrincipalId userId) throws InvalidPrincipalException {
      return new AuthorizedPrincipalTask<R>() {

         @Override
         protected R executeInternal() throws InvalidPrincipalException,
            NoPermissionException, NotAuthenticatedException {

            validateIsUser(userId);

            R role = null;
            for (Group group : _pd.findDirectParentGroups(userId)) {
               role = _groupKeyRoleValue.get(group.getId());
               if (role != null) {
                  break;
               }
            }
            return role;
         }

      }.execute();
   }

   @Override
   public R getRole(final SamlToken token) throws InvalidPrincipalException {
       return new AuthorizedPrincipalTask<R>() {

           @Override
           protected R executeInternal() {
               R role = null;
               for (PrincipalId group : token.getGroupList()) {
                   role = _groupKeyRoleValue.get(group);
                   if (role != null) {
                       break;
                   }
               }

               return role;
           }
       }.execute();
   }

   /**
    * Ensure that a person or solution user with the given id exists.
    *
    * @param principalId
    *           principal id
    *
    * @throws InvalidPrincipalException
    *            when there is neither person, nor solution user with the
    *            specified id
    * @throws NotAuthenticatedException
    * @throws NoPermissionException
    * @throws SystemException
    */
   private void validateIsUser(PrincipalId principalId)
      throws InvalidPrincipalException, NotAuthenticatedException,
      NoPermissionException, SystemException {

      if (_pd.findPersonUser(principalId) == null
         && !isSolutionUser(principalId)) {

         final String msg = String.format("Principal %s is not a user",
            principalId);

         final InvalidPrincipalException exc = new InvalidPrincipalException(
            msg);

         log.debug(msg);
         throw exc;
      }
   }

   private boolean isSolutionUser(PrincipalId principalId)
      throws SystemException, NotAuthenticatedException, NoPermissionException {
      SolutionUser solutionUser = _pd.findSolutionUser(principalId.getName());
      return solutionUser != null && principalId.equals(solutionUser.getId());
   }

}
