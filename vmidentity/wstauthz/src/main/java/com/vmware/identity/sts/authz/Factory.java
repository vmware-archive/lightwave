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
package com.vmware.identity.sts.authz;

import com.vmware.identity.authz.RoleAdminBuilder;
import com.vmware.identity.authz.RoleAdmin;
import com.vmware.identity.sts.authz.impl.RoleCheckImpl;
import com.vmware.vim.sso.admin.GroupDetails;
import com.vmware.vim.sso.admin.PrincipalManagement;
import com.vmware.vim.sso.admin.RoleManagement.WSTrustRole;

/**
 * This class provides factory methods for setting up and checking WSTrust roles
 */
public final class Factory {

   private static final String actAsGroupName = "ActAsUsers";
   private static final String ACTAS_GROUP_DESCRIPTION = "Marks users eligible to do act as delegation";

   /**
    * Creates a role administration instance.
    *
    * @param principalMgmt
    *           principal management, required
    * @return non null instance
    */
   public static RoleAdmin<WSTrustRole> createRoleAdmin(PrincipalManagement principalMgmt) {
      return new RoleAdminBuilder<WSTrustRole>(principalMgmt,
         WSTrustRole.ActAsUser, actAsGroupName, new GroupDetails(
            ACTAS_GROUP_DESCRIPTION)).createRoleAdminCreatingRoleGroups();
   }

   /**
    * Creates a role checking instance.
    *
    * @param principalMembership
    *           principal membership, required
    * @return non null instance
    */
   public static RoleCheck createRoleCheck(
      PrincipalMembership principalMembership) {

      return new RoleCheckImpl(principalMembership, actAsGroupName);
   }
}
