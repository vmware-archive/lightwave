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

import java.util.EnumMap;

import org.apache.commons.lang.Validate;

import com.vmware.identity.authz.impl.RoleAdminCreatingRoleGroups;
import com.vmware.identity.authz.impl.RoleAdminCreatingRoleGroups.SystemGroup;
import com.vmware.vim.sso.admin.GroupDetails;
import com.vmware.vim.sso.admin.PrincipalManagement;

/**
 * Builder that is producing {@link RoleAdmin} instance.
 */
public final class RoleAdminBuilder<R extends Enum<R>> {

   private final PrincipalManagement _pm;
   private final EnumMap<R, SystemGroup> _roleGroups;

   /**
    * Create {@link RoleAdmin} builder
    *
    * @param pm
    *           principal management with read/write access to role groups;
    *           {@code not-null} value is required
    * @param role
    *           at least one role is required to initialize {@link RoleAdmin}
    *           with; {@code not-null} value is required
    * @param group
    *           group name containing users with given role; {@code not-null}
    *           and not-empty string value is required
    * @param details
    *           group details used to create a group with given name if such
    *           does not exist; {@code not-null} value is required
    */
   @SuppressWarnings("unchecked")
   public RoleAdminBuilder(PrincipalManagement pm, R role, String group,
      GroupDetails details) {

      Validate.notNull(pm);
      _pm = pm;
      
      _roleGroups = new EnumMap<R, SystemGroup>((Class<R>) role.getClass());
      addRoleGroup(role, group, details);
   }

   /**
    * Define a role with group where users having this role will be stored into.
    * <p>
    * Constructor is also calling this method with initially specified role,
    * group and details.
    *
    * @param role
    *           role definition; {@code not-null} value is required
    * @param group
    *           group name containing users with given role; {@code not-null}
    *           and not-empty string value is required
    * @param details
    *           group details used to create a group with given name if such
    *           does not exist; {@code not-null} value is required
    */
   public RoleAdminBuilder<R> addRoleGroup(R role, String group,
      GroupDetails details) {
      
      Validate.notNull(role);
      Validate.notEmpty(group);
      Validate.notNull(details);

      _roleGroups.put(role, new SystemGroup(group, details));
      return this;
   }

   /**
    * @return builder's product
    */
   public RoleAdmin<R> createRoleAdminCreatingRoleGroups() {
      return new RoleAdminCreatingRoleGroups<R>(_pm, _roleGroups);
   }

}
