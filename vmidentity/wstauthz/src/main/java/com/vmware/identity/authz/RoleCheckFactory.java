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
package com.vmware.identity.authz;

import java.util.Map;

import org.apache.commons.lang.Validate;

import com.vmware.identity.authz.impl.RoleCheckBase;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.PrincipalDiscovery;

/**
 * Factory that is producing {@link RoleCheck} instances.
 */
public final class RoleCheckFactory {

   public static <R extends Enum<R>> RoleCheck<R> createRoleCheck(
      Map<PrincipalId, R> groupKeyRoleValue, PrincipalDiscovery pd) {

      Validate.notEmpty(groupKeyRoleValue);
      Validate.notNull(pd);

      return new RoleCheckBase<R>(groupKeyRoleValue, pd);
   }

   private RoleCheckFactory() {
      // disallow instantiating
   }

}
