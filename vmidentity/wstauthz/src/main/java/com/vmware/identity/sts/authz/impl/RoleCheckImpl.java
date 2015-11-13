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

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.authz.PrincipalMembership;
import com.vmware.identity.sts.authz.RoleCheck;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.admin.RoleManagement.WSTrustRole;
import com.vmware.vim.sso.admin.exception.InvalidPrincipalException;

/**
 * Insert your comment for RoleCheckImpl here
 */
public final class RoleCheckImpl implements RoleCheck {

   private final static IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(RoleCheckImpl.class);

   private final PrincipalMembership principalMembership;
   private final String actAsGroupName;

   public RoleCheckImpl(PrincipalMembership principalMembership,
      String actAsGroupName) {
      assert principalMembership != null;
      assert actAsGroupName != null;

      this.principalMembership = principalMembership;
      this.actAsGroupName = actAsGroupName;
   }

   @Override
   public boolean hasRole(PrincipalId principalId, WSTrustRole role)
      throws InvalidPrincipalException {

      assert principalId != null;
      assert role != null && role == WSTrustRole.ActAsUser;

      final boolean result = principalMembership.isMemberOfSystemGroup(
         principalId, actAsGroupName);
      log.debug("Check user {} has role {}, result: {}", new Object[] {
         principalId, role, result });
      return result;
   }
}
