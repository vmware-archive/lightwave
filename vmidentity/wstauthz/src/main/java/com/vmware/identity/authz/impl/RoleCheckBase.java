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
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;
import com.vmware.vim.sso.client.SamlToken;

/**
 * Base class for checking roles.
 */
public final class RoleCheckBase<R extends Enum<R>> implements RoleCheck<R> {

   private final Map<PrincipalId, R> _groupKeyRoleValue;

   public RoleCheckBase(Map<PrincipalId, R> groupKeyRoleValue) {

      assert groupKeyRoleValue != null;
      assert !groupKeyRoleValue.isEmpty();

      _groupKeyRoleValue = Collections.unmodifiableMap(groupKeyRoleValue);
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

}
